/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <string.h>
#include "ut_wctomb.h"
#include "ut_assert.h"
#include "ut_locale.h"

void UT_Wctomb::initialize()
{
    UT_iconv_reset(cd);
}

void UT_Wctomb::setOutCharset(const char* charset)
{
    UT_iconv_close(cd);
    cd = UT_iconv_open(charset,ucs4Internal());
}

UT_Wctomb::UT_Wctomb(const char* to_charset)
{
    cd = UT_iconv_open(to_charset,UCS_INTERNAL);
    UT_ASSERT(UT_iconv_isValid(cd)); // it's better to return "?" instead of crashing
}

UT_Wctomb::UT_Wctomb()
{
    cd = UT_iconv_open(UT_LocaleInfo::system().getEncoding().c_str(),UCS_INTERNAL);
    UT_ASSERT(UT_iconv_isValid(cd));
}

UT_Wctomb::~UT_Wctomb()
{
    if (UT_iconv_isValid(cd))
	    UT_iconv_close(cd);
}

int UT_Wctomb::wctomb(char * pC,int &length,UT_UCS4Char wc, int max_len /* = 100 */)
{
  char* obuf = pC;
  const char* ibuf = reinterpret_cast<const char *>(&wc);
  
  size_t inlen = 4, outlen = max_len;
  size_t len = UT_iconv(cd,&ibuf,&inlen,&obuf,&outlen);
  if (len==(size_t)-1)
    return 0;
  length = max_len-outlen;
  return 1;
}

void UT_Wctomb::wctomb_or_fallback(char * pC,int &length,UT_UCS4Char wc, int max_len /* = 100 */)
{
  if (!wctomb(pC,length,wc,max_len)) {
    pC[0]='?';
    length=1;
  }
}
