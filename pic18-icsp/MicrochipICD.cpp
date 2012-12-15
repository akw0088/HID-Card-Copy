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
#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
USEFORM ("uMain.cpp", FM_Main);
//---------------------------------------------------------------------------
WINAPI
_tWinMain (HINSTANCE, HINSTANCE, LPTSTR, int)
{
  try
  {
    Application->Initialize ();
    Application->MainFormOnTaskBar = true;
    Application->CreateForm (__classid (TFM_Main), &FM_Main);
    Application->Run ();
  }
  catch (Exception & exception)
  {
    Application->ShowException (&exception);
  }
  catch ( ...)
  {
    try
    {
      throw Exception ("");
    }
    catch (Exception & exception)
    {
      Application->ShowException (&exception);
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
