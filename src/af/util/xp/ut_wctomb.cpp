/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <string.h>
#include "ut_wctomb.h"

void UT_Wctomb::initialize()
{
  memset (&m_state, '\0', sizeof (m_state)); 
}

UT_Wctomb::UT_Wctomb()
{
  initialize();
}

#if ( defined(__OpenBSD__)) || ( defined(__FreeBSD__))
enum
{
  T1      = 0x00,
  Tx      = 0x80,
  T2      = 0xC0,
  T3      = 0xE0,
  T4      = 0xF0,
  T5      = 0xF8,
  T6      = 0xFC,
  
  Bit1    = 7,
  Bitx    = 6,
  Bit2    = 5,
  Bit3    = 4,
  Bit4    = 3,
  Bit5    = 2,
  Bit6    = 2,
  
  Mask1   = (1<<Bit1)-1,
  Maskx   = (1<<Bitx)-1,
  Mask2   = (1<<Bit2)-1,
  Mask3   = (1<<Bit3)-1,
  Mask4   = (1<<Bit4)-1,
  Mask5   = (1<<Bit5)-1,
  Mask6   = (1<<Bit6)-1,
  
  Wchar1  = (1UL<<Bit1)-1,
  Wchar2  = (1UL<<(Bit2+Bitx))-1,
  Wchar3  = (1UL<<(Bit3+2*Bitx))-1,
  Wchar4  = (1UL<<(Bit4+3*Bitx))-1,
  Wchar5  = (1UL<<(Bit5+4*Bitx))-1
  
#ifndef EILSEQ
  , /* we hate ansi c's comma rules */
  EILSEQ  = 123
#endif /* PLAN9 */
};
	
#endif

#if defined(__QNXNTO__) || defined(__BEOS__) || defined(__OpenBSD__) || defined(__FreeBSD__)
#include <stdlib.h>

//We have to do this since wctomb clashes with the class name
int my_wctomb( char* s, wchar_t wchar, int *state ) {
#if (! defined(__OpenBSD__)) && (! defined(__FreeBSD__))
	return wctomb(s, wchar);
#else
	if(s == 0)
	  return 0;               /* no shift states */
	if(wchar & ~Wchar2) {
	  if(wchar & ~Wchar4) {
	    if(wchar & ~Wchar5) {
	      /* 6 bytes */
	      s[0] = T6 | ((wchar >> 5*Bitx) & Mask6);
	      s[1] = Tx | ((wchar >> 4*Bitx) & Maskx);
	      s[2] = Tx | ((wchar >> 3*Bitx) & Maskx);
	      s[3] = Tx | ((wchar >> 2*Bitx) & Maskx);
	      s[4] = Tx | ((wchar >> 1*Bitx) & Maskx);
	      s[5] = Tx |  (wchar & Maskx);
	      return 6;
	    }
	    /* 5 bytes */
	    s[0] = T5 |  (wchar >> 4*Bitx);
	    s[1] = Tx | ((wchar >> 3*Bitx) & Maskx);
	    s[2] = Tx | ((wchar >> 2*Bitx) & Maskx);
	    s[3] = Tx | ((wchar >> 1*Bitx) & Maskx);
	    s[4] = Tx |  (wchar & Maskx);
	    return 5;
	  }
	  if(wchar & ~Wchar3) {
	    /* 4 bytes */
	    s[0] = T4 |  (wchar >> 3*Bitx);
	    s[1] = Tx | ((wchar >> 2*Bitx) & Maskx);
	    s[2] = Tx | ((wchar >> 1*Bitx) & Maskx);
	    s[3] = Tx |  (wchar & Maskx);
	    return 4;
	  }
	  /* 3 bytes */
	  s[0] = T3 |  (wchar >> 2*Bitx);
	  s[1] = Tx | ((wchar >> 1*Bitx) & Maskx);
	  s[2] = Tx |  (wchar & Maskx);
	  return 3;
	}
	if(wchar & ~Wchar1) {
	  /* 2 bytes */
	  s[0] = T2 | (wchar >> 1*Bitx);
	  s[1] = Tx | (wchar & Maskx);
	  return 2;
	}
	/* 1 byte */
	s[0] = T1 | wchar;
	return 1;

#endif
}

#endif

int UT_Wctomb::wctomb(char * pC,int &length,wchar_t wc)
{
#if defined(__QNXNTO__) || defined(__BEOS__) || defined(__OpenBSD__) || defined(__FreeBSD__)
  size_t len=my_wctomb(pC,wc, &m_state);
#else
  size_t len=wcrtomb(pC,wc, &m_state);
#endif
  if(len==(size_t)-1)return 0;
  length=len;
  return 1;
}
