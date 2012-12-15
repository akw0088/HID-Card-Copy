/***************************************************************
 *
 * OpenPCD - PIC18 copy protection bug - ICSP tool
 *
 * Copyright 2010 Milosch Meriac <meriac@openpcd.de>
 *
 * Compiled with Embarcadero C++ Builder XE (formerly Borland)
 * see http://www.openicsp.org/ for further information
 *
 ***************************************************************

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/


// ---------------------------------------------------------------------------
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
// ---------------------------------------------------------------------------
#include <vcl.h>
#include <windows.h>
#pragma hdrstop
#pragma link "lib\\ftd2xx.lib"
#include "uMain.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFM_Main *FM_Main;
// ---------------------------------------------------------------------------
#define ICD_TX_BITS 16
#define KEY_SEQUENCE 0x4D434850UL
// ---------------------------------------------------------------------------
#define PIN_CLR (1<<1)		// Yellow = Vpp/MCLR
// Red    = Vdd
// Black  = Vss
#define PIN_PGD (1<<2)		// Green  = PGD
#define PIN_PGC (1<<0)		// Orange = PGC
#define PIN_PGD_IN (1<<3)	// Brown  = PGM
#define PIN_OUT (PIN_PGC|PIN_CLR|PIN_PGD)
// ---------------------------------------------------------------------------
#define PGM_CORE_INST                    0	// 0b0000
#define PGM_TABLAT_OUT                   2	// 0b0010
#define PGM_TABLE_READ                   8	// 0b1000
#define PGM_TABLE_READ_POST_INC          9	// 0b1001
#define PGM_TABLE_READ_POST_DEC         10	// 0b1010
#define PGM_TABLE_READ_PRE_INC          11	// 0b1011
#define PGM_TABLE_WRITE                 12	// 0b1100
#define PGM_TABLE_WRITE_POST_INC2       13	// 0b1101
#define PGM_TABLE_WRITE_POST_INC2_PGM   14	// 0b1110
#define PGM_TABLE_WRITE_PGM             15	// 0b1111
// ---------------------------------------------------------------------------
#define CODE_OFFSET 0x0000
// ---------------------------------------------------------------------------
const unsigned short code_dumper[] = {
  0xF90E, 0x936E, 0x949C, 0xF29E,
  0xAB8E, 0x060E, 0xAF6E, 0x240E,
//      0xAB8E, 0x070E, 0xAF6E, 0x240E,
  0xAC6E, 0x8194, 0x8182, 0x006A,
  0x016A, 0x026A, 0x00C0, 0xF6FF,

  0x01C0, 0xF7FF, 0x02C0, 0xF8FF,
  0x0900, 0xF5CF, 0xADFF, 0x002A,
  0xD8B0, 0x012A, 0xD8B0, 0x022A,
  0xACA2, 0xFED7, 0x0400, 0x0050,

  0x05E1, 0x0150, 0x800A, 0x02E1,
  0x0250, 0x01E0, 0xE7D7, 0x8184,
  0x8192, 0x0400, 0xFED7, 0x1200,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

// ---------------------------------------------------------------------------
const unsigned short eeprom_dumper[] = {
  0xF90E, 0x936E, 0x949C, 0xF29E,
  0xAB8E, 0x060E, 0xAF6E, 0x240E,
  0xAC6E, 0x8194, 0x8182, 0xA96A,
  0xA69C, 0xA69E, 0xA680, 0xA8CF,

  0xADFF, 0xA92A, 0xACA2, 0xFED7,
  0x0400, 0xA950, 0xF7E1, 0x8184,
  0x8192, 0x0400, 0xFED7, 0x1200,
  0x0000, 0x0000, 0x0000, 0x0000
};

// ---------------------------------------------------------------------------
__fastcall
TFM_Main::TFM_Main (TComponent * Owner):
TForm (Owner)
{
  FT_DEVICE_LIST_INFO_NODE *devInfo, *p;
  FT_STATUS ftStatus;
  DWORD NumDevs;
  UnicodeString s;
  int i;

  m_Handle = NULL;

  if ((FT_CreateDeviceInfoList (&NumDevs) == FT_OK) && (NumDevs > 0))
    {
      p = devInfo = new FT_DEVICE_LIST_INFO_NODE[NumDevs];
      if ((FT_GetDeviceInfoList (p, &NumDevs) == FT_OK) && (NumDevs > 0))
	while (NumDevs--)
	  {
	    s = UnicodeString (p->Description) + " [" +
	      UnicodeString (p->SerialNumber) + "]";
	    CB_Devices->Items->Add (s);
	    p++;
	  }
      free (devInfo);
    }

  if (CB_Devices->Items->Count)
    {
      CB_Devices->ItemIndex = 0;
      CB_Devices->Enabled = true;
      BT_Connect->Enabled = true;
    }
}

// ---------------------------------------------------------------------------
__fastcall
TFM_Main::~TFM_Main (void)
{
  if (m_Handle)
    {
      ICD_Leave ();
      FT_Close (m_Handle);
      m_Handle = NULL;
    }
}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::BT_ConnectClick (TObject * Sender)
{
  FT_STATUS ftStatus;
  DWORD Written, Read;
  UCHAR data;

  if (FT_Open (CB_Devices->ItemIndex, &m_Handle) == FT_OK)
    {
      // reset lines to 0
      data = 0x00;

      if ((FT_SetBitMode (m_Handle, PIN_OUT, 0x4) == FT_OK) &&
	  (FT_SetBaudRate (m_Handle, 1000000) == FT_OK) &&
	  (ICD_Leave () == FT_OK))
	{
	  CB_Devices->Enabled = false;
	  BT_Connect->Enabled = false;
	  Timer->Enabled = true;
	}
      else
	{
	  ShowMessage ("Can't connect");
	  FT_Close (m_Handle);
	  m_Handle = NULL;
	}

    }
}

// ---------------------------------------------------------------------------
int __fastcall
TFM_Main::ICD_TickTx (UCHAR tick)
{
  int res;
  UCHAR data;
  DWORD count;

  if (!m_Handle)
    return FT_INVALID_HANDLE;
  else if ((res = FT_Write (m_Handle, &tick, sizeof (tick), &count)) != FT_OK)
    return res;
  else
    return FT_Read (m_Handle, &data, sizeof (data), &count);
}

// ---------------------------------------------------------------------------
int __fastcall
TFM_Main::ICD_Leave (void)
{
  return ICD_TickTx (0x00);
}

// ---------------------------------------------------------------------------
int __fastcall
TFM_Main::ICD_Enter (DWORD data)
{
  int res, i;
  UCHAR tx[32 * 2 + 2], *p, out;
  DWORD count;

  if (!m_Handle)
    return FT_INVALID_HANDLE;

  // toggle MCLR before transmitting key sequence
  ICD_TickTx (0x00);
  ICD_TickTx (PIN_CLR);
  ICD_TickTx (0x00);
  SleepEx (1, FALSE);

  // assemble key sequence
  p = tx;
  for (i = 0; i < (int) (sizeof (tx) - 2); i++)
    {
      // enabled data bit, MSB first
      out = data & (1 << 31) ? PIN_PGD : 0;
      // add clock for every second tick
      if (i & 1)
	{
	  out |= PIN_PGC;
	  data <<= 1;
	}
      *p++ = out;
    }
  // add terminating byte to set all signals to zero again
  *p++ = 0;
  // finally raise CLR
  *p++ = PIN_CLR;

  if ((res = FT_Write (m_Handle, &tx, sizeof (tx), &count)) != FT_OK)
    return res;
  else
    return FT_Read (m_Handle, &tx, sizeof (tx), &count);
}

// ---------------------------------------------------------------------------
int __fastcall
TFM_Main::ICD_Wait (DWORD time)
{
  int res, i;
  UCHAR tx[4 * 2], *p, out;
  DWORD count;

  p = tx;
  // send out 4 PGC's
  for (i = 0; i < sizeof (tx); i++)
    *p++ = (i & 1) ? PIN_PGC | PIN_CLR : PIN_CLR;

  if ((res = FT_Write (m_Handle, &tx, sizeof (tx), &count)) != FT_OK)
    return res;
  if ((res = FT_Read (m_Handle, &tx, sizeof (tx), &count)) != FT_OK)
    return res;

  // hold 4th PGC for 'time' milliseconds
  SleepEx (time, FALSE);

  // release PGC
  return ICD_TickTx (PIN_CLR);
}

// ---------------------------------------------------------------------------
UCHAR __fastcall
TFM_Main::ICD_ReadTblPtr (void)
{
  int res, i;
  UCHAR tx[(4 + 16) * 2 + 1], *p, out, cmd;
  DWORD count;

  if (!m_Handle)
    return FT_INVALID_HANDLE;

  p = tx;
  cmd = PGM_TABLE_READ_POST_INC;
  // transmit CMD
  for (i = 0; i < (4 + 16); i++)
    {
      // keep reset high
      out = PIN_CLR | PIN_PGC;
      // get CMD LSB first
      if (cmd & 1)
	out |= PIN_PGD;
      cmd >>= 1;
      // shift out PGD data + PGC
      *p++ = out;
      // shift out PGD only - no PGC
      *p++ = out ^ PIN_PGC;
    }
  *p++ = PIN_CLR;

  if ((res = FT_Write (m_Handle, &tx, sizeof (tx), &count)) != FT_OK)
    return res;
  if ((res = FT_Read (m_Handle, &tx, sizeof (tx), &count)) != FT_OK)
    return res;

  out = 0;
  for (i = 0; i < 8; i++)
    out = (out >> 1) | ((tx[i * 2 + (1 + 2 * 12)] & PIN_PGD_IN) ? 0x80 : 0);
  return out;
}

// ---------------------------------------------------------------------------
int __fastcall
TFM_Main::ICD_Write (UCHAR cmd, USHORT data)
{
  int res, i;
  UCHAR tx[(4 + 16) * 2 + 1], *p, out;
  DWORD count;

  if (!m_Handle)
    return FT_INVALID_HANDLE;

  p = tx;
  // transmit CMD
  for (i = 0; i < 4; i++)
    {
      // keep reset high
      out = PIN_CLR | PIN_PGC;
      // get CMD LSB first
      if (cmd & 1)
	out |= PIN_PGD;
      cmd >>= 1;
      // shift out PGD data + PGC
      *p++ = out;
      // shift out PGD only - no PGC
      *p++ = out ^ PIN_PGC;
    }
  // transmit payload data
  for (i = 0; i < 16; i++)
    {
      // keep reset high + PGC
      out = PIN_CLR | PIN_PGC;
      // get DATA LSB first
      if (data & 1)
	out |= PIN_PGD;
      data >>= 1;
      // shift out PGD data + PGC
      *p++ = out;
      // shift out PGD only - no PGC
      *p++ = out ^ PIN_PGC;
    }
  // all lines to GND except of reset line
  *p++ = PIN_CLR;

  if ((res = FT_Write (m_Handle, &tx, sizeof (tx), &count)) != FT_OK)
    return res;
  else
    return FT_Read (m_Handle, &tx, sizeof (tx), &count);
}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::ICD_SetTblPtr (DWORD addr)
{
  // MOVLW xx
  ICD_Write (PGM_CORE_INST, 0x0E00 | ((addr >> 16) & 0xFF));
  // MOVWF TBLPTRU
  ICD_Write (PGM_CORE_INST, 0x6EF8);
  // MOVLW xx
  ICD_Write (PGM_CORE_INST, 0x0E00 | ((addr >> 8) & 0xFF));
  // MOVWF TBLPTRH
  ICD_Write (PGM_CORE_INST, 0x6EF7);
  // MOVLW xx
  ICD_Write (PGM_CORE_INST, 0x0E00 | ((addr >> 0) & 0xFF));
  // MOVWF TBLPTRL
  ICD_Write (PGM_CORE_INST, 0x6EF6);
}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::ICD_WriteMem (DWORD addr, UCHAR data)
{
  // set table pointer
  ICD_SetTblPtr (addr);
  // write data to TBLPTR(=addr)
  ICD_Write (PGM_TABLE_WRITE, (((USHORT) data) << 8) | data);
}

// ---------------------------------------------------------------------------
int __fastcall
TFM_Main::ICD_BulkErase (USHORT cmd)
{
  if (cmd != 0x3F8F)
    {
      // BSF EECON1, EEPGD
      ICD_Write (PGM_CORE_INST, 0x8EA6);
      // BCF EECON1, CFGS
      ICD_Write (PGM_CORE_INST, 0x9CA6);
      // BSF EECON1, WREN
      ICD_Write (PGM_CORE_INST, 0x84A6);
    }

  // write 0180h to 3C0005h:3C0004h
  ICD_WriteMem (0x3C0005, (cmd >> 8) & 0xFF);
  ICD_WriteMem (0x3C0004, (cmd >> 0) & 0xFF);

  // issue NOP twice
  ICD_Write (PGM_CORE_INST, 0x0000);
  ICD_Write (PGM_CORE_INST, 0x0000);

  ICD_Leave ();
}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::OnIcdEnter (TObject * Sender)
{
  // ICD_Enter(0x4D434850);
  ICD_Leave ();
}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::Button1Click (TObject * Sender)
{
  int i, j;
  UnicodeString s;

  // set table pointer
  ICD_SetTblPtr (CODE_OFFSET + 0x0);

  Memo->Clear ();
  for (i = 0x0; i < 0x2000; i++)
    {
      s = IntToHex (i * 0x10, 4) + ":";
      for (j = 0; j < 0x10; j++)
	{
	  if (j)
	    {
	      if ((j & 0x3) == 0)
		s += " ";
	      if ((j & 0x7) == 0)
		s += " ";
	    }
	  s += " " + IntToHex (ICD_ReadTblPtr (), 2);
	}
      SleepEx (5, FALSE);
      Memo->Lines->Add (s);
    }
  // ICD_BulkErase(0x82);

  ICD_Leave ();
}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::OnErase (TObject * Sender)
{
  // BSF EECON1, EEPGD
  ICD_Write (PGM_CORE_INST, 0x8EA6);
  // BCF EECON1, CFGS
  ICD_Write (PGM_CORE_INST, 0x9CA6);
  // BSF EECON1, WREN
  ICD_Write (PGM_CORE_INST, 0x84A6);

  // write to address 0x000000
  ICD_SetTblPtr (64 * 0);

  // BSF EECON1, FREE
  ICD_Write (PGM_CORE_INST, 0x88A6);
  // BSF EECON1, WR
  ICD_Write (PGM_CORE_INST, 0x82A6);
  ICD_Wait (10);

  ICD_Leave ();
}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::OnErase1 (TObject * Sender)
{
  // BSF EECON1, EEPGD
  ICD_Write (PGM_CORE_INST, 0x8EA6);
  // BCF EECON1, CFGS
  ICD_Write (PGM_CORE_INST, 0x9CA6);
  // BSF EECON1, WREN
  ICD_Write (PGM_CORE_INST, 0x84A6);

  // write to address 0x000000
  ICD_SetTblPtr (64 * 1);

  // BSF EECON1, FREE
  ICD_Write (PGM_CORE_INST, 0x88A6);
  // BSF EECON1, WR
  ICD_Write (PGM_CORE_INST, 0x82A6);
  ICD_Wait (10);

  ICD_Leave ();

}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::Button4Click (TObject * Sender)
{
  int i, j;
  const unsigned short *p;
  unsigned short data;

  // BSF EECON1, EEPGD
  ICD_Write (PGM_CORE_INST, 0x8EA6);
  // BCF EECON1, CFGS
  ICD_Write (PGM_CORE_INST, 0x8CA6);

  ICD_WriteMem (0x3C0006, 0x00);

  // BSF EECON1, EEPGD
  ICD_Write (PGM_CORE_INST, 0x8EA6);
  // BCF EECON1, CFGS
  ICD_Write (PGM_CORE_INST, 0x9CA6);

  ICD_SetTblPtr (CODE_OFFSET);

  p = code_dumper;
  for (i = 0; i < (int) (sizeof (code_dumper) / sizeof (code_dumper[0])); i++)
    {
      data = *p++;
      data = (data >> 8) | (data << 8);

      if ((i & 0x3) == 0x3)
	{
	  // write 2 bytes and start programming
	  ICD_Write (PGM_TABLE_WRITE_PGM, data);

	  // NOP - hold PGC high for time P9 and low for time P10
	  ICD_Wait (1);
	  SleepEx (100, FALSE);
	  ICD_Leave ();

	  j = ((i + 1) * 2);
	  ShowMessage ("Cycle Power for Write=0x" + IntToHex (j, 4));

	  // BSF EECON1, EEPGD
	  ICD_Write (PGM_CORE_INST, 0x8EA6);
	  // BCF EECON1, CFGS
	  ICD_Write (PGM_CORE_INST, 0x8CA6);

	  ICD_WriteMem (0x3C0006, 0x00);

	  // BSF EECON1, EEPGD
	  ICD_Write (PGM_CORE_INST, 0x8EA6);
	  // BCF EECON1, CFGS
	  ICD_Write (PGM_CORE_INST, 0x9CA6);

	  ICD_SetTblPtr (CODE_OFFSET + j);
	}
      else
	// write 2 bytes and start programming
	ICD_Write (PGM_TABLE_WRITE_POST_INC2, data);
    }

  ICD_Leave ();
}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::OnEraseBoot (TObject * Sender)
{
  // BSF EECON1, EEPGD
  ICD_Write (PGM_CORE_INST, 0x8EA6);
  // BCF EECON1, CFGS
  ICD_Write (PGM_CORE_INST, 0x9CA6);
  // BSF EECON1, WREN
  ICD_Write (PGM_CORE_INST, 0x84A6);

  ICD_WriteMem (0x3C0004, 0x83);

  // issue NOP twice
  ICD_Write (PGM_CORE_INST, 0x0000);
  ICD_Write (PGM_CORE_INST, 0x0000);

  ICD_Leave ();
}

// ---------------------------------------------------------------------------
void __fastcall
TFM_Main::OnErasePanels (TObject * Sender)
{
  int i;

  for (i = 0; i < 4; i++)
    {
      ShowMessage ("Cycle Power for Panel Erase=" + IntToStr (i));

      // BSF EECON1, EEPGD
      ICD_Write (PGM_CORE_INST, 0x8EA6);
      // BCF EECON1, CFGS
      ICD_Write (PGM_CORE_INST, 0x9CA6);
      // BSF EECON1, WREN
      ICD_Write (PGM_CORE_INST, 0x84A6);

      ICD_WriteMem (0x3C0004, 0x88 + i);

      // issue NOP twice
      ICD_Write (PGM_CORE_INST, 0x0000);
      ICD_Write (PGM_CORE_INST, 0x0000);

      ICD_Leave ();
    }
}

//---------------------------------------------------------------------------
void __fastcall
TFM_Main::OnFlashEepromDumper (TObject * Sender)
{
  int i, j;
  const unsigned short *p;
  unsigned short data;

  // BSF EECON1, EEPGD
  ICD_Write (PGM_CORE_INST, 0x8EA6);
  // BCF EECON1, CFGS
  ICD_Write (PGM_CORE_INST, 0x8CA6);

  ICD_WriteMem (0x3C0006, 0x00);

  // BSF EECON1, EEPGD
  ICD_Write (PGM_CORE_INST, 0x8EA6);
  // BCF EECON1, CFGS
  ICD_Write (PGM_CORE_INST, 0x9CA6);

  ICD_SetTblPtr (CODE_OFFSET);

  p = eeprom_dumper;
  for (i = 0; i < (int) (sizeof (eeprom_dumper) / sizeof (eeprom_dumper[0]));
       i++)
    {
      data = *p++;
      data = (data >> 8) | (data << 8);

      if ((i & 0x3) == 0x3)
	{


	  // write 2 bytes and start programming
	  ICD_Write (PGM_TABLE_WRITE_PGM, data);

	  // NOP - hold PGC high for time P9 and low for time P10
	  ICD_Wait (1);
	  SleepEx (100, FALSE);
	  ICD_Leave ();

	  j = CODE_OFFSET + ((i + 1) * 2);

	  ShowMessage ("Cycle Power for EEprom Write=0x" + IntToHex (j, 4));

	  // BSF EECON1, EEPGD
	  ICD_Write (PGM_CORE_INST, 0x8EA6);
	  // BCF EECON1, CFGS
	  ICD_Write (PGM_CORE_INST, 0x8CA6);

	  ICD_WriteMem (0x3C0006, 0x00);

	  // BSF EECON1, EEPGD
	  ICD_Write (PGM_CORE_INST, 0x8EA6);
	  // BCF EECON1, CFGS
	  ICD_Write (PGM_CORE_INST, 0x9CA6);

	  ICD_SetTblPtr (j);
	}
      else
	// write 2 bytes and start programming
	ICD_Write (PGM_TABLE_WRITE_POST_INC2, data);
    }

  ICD_Leave ();
}

//---------------------------------------------------------------------------
void __fastcall
TFM_Main::Button8Click (TObject * Sender)
{
  // BSF EECON1, EEPGD
  ICD_Write (PGM_CORE_INST, 0x8EA6);
  // BCF EECON1, CFGS
  ICD_Write (PGM_CORE_INST, 0x9CA6);
  // BSF EECON1, WREN
  ICD_Write (PGM_CORE_INST, 0x84A6);

  ICD_WriteMem (0x3C0005, 0x01);
  ICD_WriteMem (0x3C0004, 0x80);

  ICD_Wait (10);

  ICD_Leave ();

}
//---------------------------------------------------------------------------
