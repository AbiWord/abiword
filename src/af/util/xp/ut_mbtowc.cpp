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
#include <limits.h>
#include "ut_mbtowc.h"

UT_Mbtowc::UT_Mbtowc()
{
  initialize();
}

void UT_Mbtowc::initialize()
{
  memset(&m_state,'\0', sizeof (m_state));
  m_bufLen=0;
}

#if defined(__QNXNTO__) || defined(__BEOS__)

#include <stdlib.h>

int my_mbtowc( wchar_t *pwc, const char *s, size_t n, int *state) {
	return mbtowc(pwc, s, n);
}

#endif

int UT_Mbtowc::mbtowc(wchar_t &wc,char mb)
{
  if(++m_bufLen>MB_LEN_MAX)
	{
	  initialize();
	  return 0;
	}
  m_buf[m_bufLen-1]=mb;
#if defined(__QNXNTO__) || defined(__BEOS__)
  size_t thisLen=my_mbtowc(&wc,m_buf,m_bufLen,&m_state);
#else
  size_t thisLen=mbrtowc(&wc,m_buf,m_bufLen,&m_state);
#endif
  if(thisLen>MB_LEN_MAX)return 0;
  if(thisLen==0)thisLen=1;
  m_bufLen-=thisLen;
  return 1;
}
