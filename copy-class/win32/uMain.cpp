//---------------------------------------------------------------------------
#include <vcl.h>
#include <winscard.h>
#pragma hdrstop
//---------------------------------------------------------------------------
#include "des.h"
#include "uMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
#pragma link "winscard.lib"
#pragma link "lib/scardsyn.lib"
//---------------------------------------------------------------------------
TFM_Main *FM_Main;
//---------------------------------------------------------------------------
static const UCHAR hid_decryption_key[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//---------------------------------------------------------------------------
extern "C" ULONG __stdcall SCardCLICCTransmit (IN SCARDHANDLE ulHandleCard,
					       IN PUCHAR pucSendData,
					       IN ULONG ulSendDataBufLen,
					       IN OUT PUCHAR pucReceivedData,
					       IN OUT PULONG
					       pulReceivedDataBufLen);
//---------------------------------------------------------------------------
__fastcall
TFM_Main::TFM_Main (TComponent * Owner):
TForm (Owner)
{
  int i;
  wchar_t readers[1024], *wc;
  DWORD res, length = sizeof (readers) / sizeof (readers[0]);

  // set up resizing constraints
  Constraints->MinWidth = Width;
  Constraints->MinHeight = Height;

  // init random pool
  randomize ();

  hContext = hCard = NULL;
  m_PrevLines = NULL;

  res = SCardEstablishContext (SCARD_SCOPE_USER, NULL, NULL, &hContext);
  if (res != SCARD_S_SUCCESS)
    ShowMessage ("Can't establish RFID reader context");
  else
    {
      res = SCardListReaders (hContext, NULL, readers, &length);
      if (res != SCARD_S_SUCCESS)
	ShowMessage ("Can't retrieve list of installed readers");
      else
	{
	  wc = readers;
	  while (*wc)
	    {
	      i = CB_Readers->Items->Add (UnicodeString (wc));
	      if (wcsstr (wc, L"-CL"))
		CB_Readers->ItemIndex = i;
	      wc += wcslen (wc) + 1;
	    }
	  if (CB_Readers->Items->Count)
	    {
	      BT_CardReload->Enabled = true;
	      BT_CardReload->Click ();
	      return;
	    }
	  else
	    ShowMessage ("Can't find a single smart card reader");
	}
    }
  Application->Terminate ();
}

//---------------------------------------------------------------------------
__fastcall
TFM_Main::~
TFM_Main (void)
{
  if (hCard)
    SCardReleaseContext (hCard);
  if (hContext)
    SCardReleaseContext (hContext);
  if (m_PrevLines)
    delete m_PrevLines;
}

//---------------------------------------------------------------------------
void __fastcall
TFM_Main::FormKeyDown (TObject * Sender, WORD & Key, TShiftState Shift)
{
  if (Key == VK_F5)
    BT_CardReload->Click ();
}

//---------------------------------------------------------------------------

void __fastcall
TFM_Main::BT_CardReloadClick (TObject * Sender)
{
  DWORD res;
  DWORD dwActiveProtocol;
  SCARD_READERSTATE state;
  bool done;

  if (hCard)
    BT_CardCloseClick (this);

  res = SCardConnect (hContext,
		      CB_Readers->Text.c_str (),
		      SCARD_SHARE_SHARED,
		      SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		      &hCard, &dwActiveProtocol);

  done = (res == SCARD_S_SUCCESS);

  ED_AuthKey->Enabled = done;
  BT_Auth->Enabled = done;
  RG_KeyType->Enabled = done;

  ED_Serial->Text = done ? CardSelect (0x04) : UnicodeString ();
  ED_ConfigBlock->Text = done ? CardSelect (0x08) : UnicodeString ();
  ED_AppIssuer->Text = done ? CardSelect (0x0C) : UnicodeString ();

  if (done)
    ActiveControl = BT_Auth;
}

//---------------------------------------------------------------------------

void __fastcall
TFM_Main::BT_CardCloseClick (TObject * Sender)
{
  if (hCard)
    {
      SCardReleaseContext (hCard);
      hCard = NULL;
    }
}

//---------------------------------------------------------------------------
UnicodeString __fastcall
TFM_Main::CardSelect (UCHAR p2)
{
  int res, i;
  UnicodeString hex;
  UCHAR ucReceivedData[64] = { 0 };
  ULONG ulNoOfDataReceived = sizeof (ucReceivedData);
  UCHAR data[] = { 0x80, 0xA6, 0x00, p2, p2 ? 0x08 : 0x00 };

  res =
    SCardCLICCTransmit (hCard, data, sizeof (data), ucReceivedData,
			&ulNoOfDataReceived);
  if ((res != SCARD_S_SUCCESS) || (ulNoOfDataReceived < 2))
    ShowMessage ("Error in SCardCLICCTransmit");
  else
    {
      res = ucReceivedData[ulNoOfDataReceived - 2];
      if (res != 0x90)
	return Format ("APDU Error=0x%02X", OPENARRAY (TVarRec, (res)));
      else
	{
	  res = ulNoOfDataReceived - 2;
	  for (i = 0; i < res; i++)
	    hex += IntToHex (ucReceivedData[i], 2);
	  return hex;
	}
    }
  return "Error";
}

//---------------------------------------------------------------------------
void __fastcall
TFM_Main::BT_AuthClick (TObject * Sender)
{
  const wchar_t *p;
  wchar_t nibble;
  __int64 k;

  k = 0;
  p = ED_AuthKey->Text.c_str ();
  while ((nibble = *p++) != 0)
    {
      if (nibble >= '0' && nibble <= '9')
	nibble -= '0';
      else if (nibble >= 'a' && nibble <= 'f')
	nibble -= 'a' - 0xA;
      else if (nibble >= 'A' && nibble <= 'F')
	nibble -= 'A' - 0xA;
      else
	nibble = 0;

      k = (k << 4) | nibble;
    }

  ED_AuthKey->Text = IntToHex (k, 16);

  if (RG_KeyType->ItemIndex)
    BT_ReadCard->Enabled = CardAuth (k, RG_KeyType->ItemIndex - 1) > 0;
  else
    BT_ReadCard->Enabled = CardAuth (k, 0) || CardAuth (k, 1);

  if (BT_ReadCard->Enabled)
    ActiveControl = BT_ReadCard;
}

//---------------------------------------------------------------------------
int __fastcall
TFM_Main::CardAuth (__int64 key, UCHAR type)
{
  int res, i;
  UnicodeString hex;
  UCHAR ucReceivedData[64] = { 0 };
  ULONG ulNoOfDataReceived = sizeof (ucReceivedData);
  UCHAR data[13] = { 0x80, 0x82, 0x00, 0xF0, 0x08 };

  for (i = 0; i < 8; i++)
    {
      data[5 + 8 - 1 - i] = (UCHAR) key;
      key >>= 8;
    }

  res =
    SCardCLICCTransmit (hCard, data, sizeof (data), ucReceivedData,
			&ulNoOfDataReceived);
  if ((res != SCARD_S_SUCCESS) || (ulNoOfDataReceived < 2))
    {
      StatusBar->SimpleText = "Error in LoadKey SCardCLICCTransmit";
      return 0;
    }

  res = ucReceivedData[ulNoOfDataReceived - 2];
  if (res != 0x90)
    {
      StatusBar->SimpleText = "APDU LoadKey error=0x" + IntToHex (res, 2);
      return 0;
    }

  data[1] = 0x88;
  data[2] = type;

  ulNoOfDataReceived = sizeof (ucReceivedData);
  res =
    SCardCLICCTransmit (hCard, data, 4, ucReceivedData, &ulNoOfDataReceived);
  if ((res != SCARD_S_SUCCESS) || (ulNoOfDataReceived < 2))
    StatusBar->SimpleText = "Error in Authenticate SCardCLICCTransmit";
  else
    {
      res = ucReceivedData[ulNoOfDataReceived - 2];
      if (res != 0x90)
	{
	  res = res << 8 | ucReceivedData[ulNoOfDataReceived - 1];
	  StatusBar->SimpleText =
	    "APDU Authenticate error=0x" + IntToHex (res, 4);
	}
      else
	{
	  if (type && CB_Decrypt->Checked)
	    CB_Decrypt->Checked = false;

	  StatusBar->SimpleText =
	    "Authenticated with Key(" + IntToStr (type + 1) + ")";
	  return 1;
	}
    }

  return 0;
}

//---------------------------------------------------------------------------

void __fastcall
TFM_Main::ClearStatusPanel (TObject * Sender, TMouseButton Button,
			    TShiftState Shift, int X, int Y)
{
  StatusBar->SimpleText = "";
}

//---------------------------------------------------------------------------
void __fastcall
TFM_Main::BT_ReadCardClick (TObject * Sender)
{
  int i, sel;
  UnicodeString hex;

  // remember previous content and clear Memo
  if (!m_PrevLines)
    m_PrevLines = new TStringList ();
  m_PrevLines->Text = Memo->Lines->Text;
  Memo->Clear ();

  for (i = 0; i <= 0xFF; i++)
    {
      hex = CardRead (i);
      if (hex.IsEmpty ())
	break;
      else
	{
	  Memo->Lines->Add (IntToHex (i, 2) + ": " + hex);
	  SetLineNumberColor (i, clBlue);
	}
    }

  // set memo active
  ActiveControl = Memo;
}

//---------------------------------------------------------------------------
UnicodeString __fastcall
TFM_Main::HexDump (PUCHAR data, ULONG len)
{
  ULONG t;
  UnicodeString hex = "";

  for (t = 0; t < len; t++)
    hex += (t ? " " : "") + IntToHex (*data++, 2);

  return hex;
}

//---------------------------------------------------------------------------
UnicodeString __fastcall
TFM_Main::CardRead (UCHAR block)
{
  int res;
  UCHAR ucReceivedData[64] = { 0 };
  UCHAR ucDecryptedReceivedData[64] = { 0 };
  ULONG ulNoOfDataReceived = sizeof (ucReceivedData);
  UCHAR data[] = { 0x80, 0xB0, 0x00, block, 0x08 };
  UCHAR *output = ucDecryptedReceivedData;
  int outputLength;

  res =
    SCardCLICCTransmit (hCard, data, sizeof (data), ucReceivedData,
			&ulNoOfDataReceived);

  if ((res != SCARD_S_SUCCESS) || (ulNoOfDataReceived < 2))
    {
      ShowMessage ("Error in CardRead SCardCLICCTransmit");
      return "";
    }
  else
    {
      res = ucReceivedData[ulNoOfDataReceived - 2];
      if (res == 0x6A)
	return "";

      if (res != 0x90)
	return Format ("APDU CardRead Error=0x%02X",
		       OPENARRAY (TVarRec, (res)));
      else
	{
	  outputLength = ulNoOfDataReceived - 2;

	  if (CB_Decrypt->Checked && (block >= 7) && (block <= 9))
	    {
	      decrypt_3des (hid_decryption_key,
			    ucReceivedData,
			    ulNoOfDataReceived, &output, &outputLength);

	      return HexDump (output, outputLength);
	    }
	  else
	    return HexDump (ucReceivedData, outputLength);
	}
    }
}

//---------------------------------------------------------------------------


void __fastcall
TFM_Main::MemoKeyPress (TObject * Sender, wchar_t & Key)
{
  if (Key >= ' ' && Key < 0x100)
    {
      if (Key >= 'a' && Key <= 'f')
	Key = (Key - 'a') + 'A';
      else
	if (!((Key >= 'A' && Key <= 'F') ||
	      (Key >= '0') && (Key <= '9') || (Key == ' ') || (Key == ':')))
	Key = NULL;
    }
}

//---------------------------------------------------------------------------
void __fastcall
TFM_Main::MemoMouseDown (TObject * Sender, TMouseButton Button,
			 TShiftState Shift, int X, int Y)
{
  Caption = Memo->ActiveLineNo;

}

//---------------------------------------------------------------------------
void __fastcall
TFM_Main::SetLineNumberColor (int Line, TColor Color)
{
  if (Line < Memo->Lines->Count)
    {
      Memo->SelStart = SendMessage (Memo->Handle, EM_LINEINDEX, Line, 0);
      Memo->SelLength = 3;
      Memo->SelAttributes->Color = Color;
      Memo->SelLength = 0;
    }
}

//---------------------------------------------------------------------------
void __fastcall
TFM_Main::PrettyPrint (void)
{
  int lines, i, j, sel, len, sellen, selstart;
  UnicodeString prev, curr;
  wchar_t *c, *p;

  lines = Memo->Lines->Count;

  for (i = 0; i < lines; i++)
    {
      prev = m_PrevLines->Strings[i];
      curr = Memo->Lines->Strings[i];

      len = curr.Length ();
      if (prev.Length () < len)
	len = prev.Length ();

      c = curr.c_str ();
      p = prev.c_str ();

      sellen = selstart = 0;
      for (j = 0; j <= len; j++)
	{
	  if ((j < len) && (*c++ != *p++))
	    {
	      if (!sellen)
		selstart = j;
	      sellen++;
	    }
	  else if (sellen)
	    {
	      Memo->SelStart = sel + selstart;
	      Memo->SelLength = sellen;
	      Memo->SelAttributes->Color = clRed;
	    }
	}
    }
  Memo->SelLength = 0;
}

//---------------------------------------------------------------------------

void __fastcall
TFM_Main::BT_CopyToClipboardClick (TObject * Sender)
{
  bool deselected = Memo->SelLength == 0;

  if (deselected)
    Memo->SelectAll ();

  Memo->CopyToClipboard ();

  if (deselected)
    Memo->SelLength = 0;
}

//---------------------------------------------------------------------------
