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
#include "ut_assert.h"
#include "xap_EncodingManager.h"

void UT_Wctomb::initialize()
{
    UT_iconv_reset(cd);
};

void UT_Wctomb::setOutCharset(const char* charset)
{
    UT_iconv_close(cd);
    cd = UT_iconv_open(charset,ucs4Internal());
    //UT_ASSERT(cd!=(iconv_t)-1); //it's better to return "?" instead of crashing
};

UT_Wctomb::UT_Wctomb(const char* to_charset)
{
    cd = UT_iconv_open(to_charset,ucs4Internal());
    UT_ASSERT(UT_iconv_isValid(cd)); //it's better to return "?" instead of crashing
};

UT_Wctomb::UT_Wctomb()
{
    cd = UT_iconv_open(XAP_EncodingManager::get_instance()->getNative8BitEncodingName(),UCS_INTERNAL);
    UT_ASSERT(UT_iconv_isValid(cd));
};

UT_Wctomb::UT_Wctomb(const UT_Wctomb& v)
{
  // Shouldn't a copy also copy the encoding?
    cd = UT_iconv_open(XAP_EncodingManager::get_instance()->getNative8BitEncodingName(),UCS_2_INTERNAL);
    UT_ASSERT(UT_iconv_isValid(cd));
};

UT_Wctomb::~UT_Wctomb()
{
    if (UT_iconv_isValid(cd))
	    UT_iconv_close(cd);
};

int UT_Wctomb::wctomb(char * pC,int &length,UT_UCS4Char wc)
{
    char buf[sizeof(UT_UCS4Char)];
    char* obuf = pC;
    const char* ibuf = buf;

    {
	int swap = XAP_EncodingManager::swap_utos ? 1 : 0;
        UT_UCS4Char val = wc;
	unsigned char b0 = val&0xff, b1 = val>>8, b2 = val>>16, b3 = val >> 24;
	buf[swap ] = b0;
	buf[!swap] = b1;
	buf[swap + 2] = b2;
	buf[!swap + 2] = b3;
    }

    size_t inlen = 4, outlen = 100;
    size_t len = UT_iconv(cd,&ibuf,&inlen,&obuf,&outlen);
    if (len==(size_t)-1)
	return 0;
    length = 100-outlen;
    return 1;
};

void UT_Wctomb::wctomb_or_fallback(char * pC,int &length,UT_UCS4Char wc)
{
    if (!wctomb(pC,length,wc)) {
	pC[0]='?';
	length=1;
    };
};
