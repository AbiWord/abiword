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

#if defined(__QNXNTO__) || defined(__BEOS__)
#include <stdlib.h>

//We have to do this since wctomb clashes with the class name
int my_wctomb( char* s, wchar_t wchar, int *state ) {
	return wctomb(s, wchar);
}

#endif

int UT_Wctomb::wctomb(char * pC,int &length,wchar_t wc)
{
#if defined(__QNXNTO__) || defined(__BEOS__)
  size_t len=my_wctomb(pC,wc, &m_state);
#else
  size_t len=wcrtomb(pC,wc, &m_state);
#endif
  if(len==(size_t)-1)return 0;
  length=len;
  return 1;
}
