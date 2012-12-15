/***************************************************************
 *
 * OpenPCD - PIC18 copy protection bug - flash dumper
 *
 * Copyright 2010 Milosch Meriac <meriac@openpcd.de>
 *
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
#include "pic18fregs.h"

// same configuratio as the HID reader
code char at __CONFIG1H c1h = 0x22;

code char at __CONFIG2H c2h = 0x0F;
code char at __CONFIG2L c2l = 0x0A;

code char at __CONFIG3H c3h = 0x01;

code char at __CONFIG4L c4l = 0x81;

code char at __CONFIG5H c5h = 0x00;
code char at __CONFIG5L c5l = 0x00;

code char at __CONFIG6H c6h = 0x80;
code char at __CONFIG6L c6l = 0x00;

code char at __CONFIG7H c7h = 0x40;
code char at __CONFIG7L c7l = 0x0F;

// add some data to the end to verify if dumping works
//code short at 0x800  test2 = 0x1234;
//code short at 0x7FFE test1 = 0x5678;

#define LED_GREEN	PORTBbits.RB1
#define LED_RED		PORTBbits.RB2

typedef __code unsigned char *CODEPTR;

void
main ()
{
  CODEPTR c;
  TRISB = 0b11111001;
  TRISCbits.TRISC6 = 0;

  // Gobally disable IRQs
  INTCONbits.GIE = 0;

  // init USART peripheral
  RCSTAbits.SPEN = 1;
  // baud rate to 115200 Baud
  SPBRG = 6;
  // enable TX + high speed mode
  TXSTA = 0b00100100;

  // light red LED to indicate dump process
  LED_RED = 0;
  LED_GREEN = 1;

  c = 0;
  do
    {
      TXREG = *c++;
      while (!TXSTAbits.TRMT);
      ClrWdt ();
    }
  while (c != (CODEPTR) 0x8000);

  // turn off red LED
  // light green LED to indicate stopped dump process
  LED_RED = 1;
  LED_GREEN = 0;

  // sit there idle
  for (;;)
    ClrWdt ();
}
