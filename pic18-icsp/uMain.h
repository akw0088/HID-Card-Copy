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


//---------------------------------------------------------------------------
#ifndef uMainH
#define uMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include "inc\ftd2xx.h"
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFM_Main:public TForm
{
  __published:			// Von der IDE verwaltete Komponenten
  TGroupBox * GB_Devices;
  TComboBox *CB_Devices;
  TButton *BT_Connect;
  TTimer *Timer;
  TButton *Button1;
  TMemo *Memo;
  TButton *Button3;
  TButton *Button4;
  TButton *Button5;
  TButton *Button2;
  TButton *Button6;
  TButton *Button7;
  TButton *Button8;
  void __fastcall BT_ConnectClick (TObject * Sender);
  void __fastcall OnIcdEnter (TObject * Sender);
  void __fastcall Button1Click (TObject * Sender);
  void __fastcall OnErase (TObject * Sender);
  void __fastcall Button4Click (TObject * Sender);
  void __fastcall OnErase1 (TObject * Sender);
  void __fastcall OnEraseBoot (TObject * Sender);
  void __fastcall OnErasePanels (TObject * Sender);
  void __fastcall OnFlashEepromDumper (TObject * Sender);
  void __fastcall Button8Click (TObject * Sender);
private:			// Anwender-Deklarationen
    FT_HANDLE m_Handle;
  int __fastcall ICD_TickTx (UCHAR tick);
  int __fastcall ICD_Enter (DWORD data);
  int __fastcall ICD_Leave (void);
  int __fastcall ICD_Write (UCHAR cmd, USHORT data);
  UCHAR __fastcall ICD_ReadTblPtr (void);
  void __fastcall ICD_SetTblPtr (DWORD addr);
  void __fastcall ICD_WriteMem (DWORD addr, UCHAR data);
  int __fastcall ICD_Wait (DWORD time);
  int __fastcall ICD_BulkErase (USHORT cmd);
public:			// Anwender-Deklarationen
    __fastcall TFM_Main (TComponent * Owner);
    __fastcall ~ TFM_Main (void);
};
//---------------------------------------------------------------------------
extern PACKAGE TFM_Main *FM_Main;
//---------------------------------------------------------------------------
#endif
