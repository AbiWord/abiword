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


#if 0 /* big if 0 */
#if defined(__OpenBSD__) || defined(__FreeBSD__)
#include <errno.h>
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

UT_Mbtowc::UT_Mbtowc()
{
  initialize();
}

void UT_Mbtowc::initialize()
{
  memset(&m_state,'\0', sizeof (m_state));
  m_bufLen=0;
}

#if defined(__QNXNTO__) || defined(__BEOS__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined (TARGET_OS_MAC)

#include <stdlib.h>

int my_mbtowc( wchar_t *pwc, const char *s, size_t n, int *state) {
#if (! defined(__OpenBSD__)) && (! defined(__FreeBSD__))
	return mbtowc(pwc, s, n);
#else
	/* Open/FreeBSD has no support for wide chars */
	typedef unsigned char uchar;
	uchar *us;
	int c0, c1, c2, c3, c4, c5;
	wchar_t wc;
	
	if(s == 0)
	  return 0;               /* no shift states */
	
	if(*state < 1)
	  goto badlen;
	us = (uchar*)s;
	c0 = us[0];
	if(c0 >= T3) {
	  if(*state < 3)
	    goto badlen;
	  c1 = us[1] ^ Tx;
	  c2 = us[2] ^ Tx;
	  if((c1|c2) & T2)
	    goto bad;
	  if(c0 >= T5) {
	    if(*state < 5)
	      goto badlen;
	    c3 = us[3] ^ Tx;
	    c4 = us[4] ^ Tx;
	    if((c3|c4) & T2)
	      goto bad;
	    if(c0 >= T6) {
	      /* 6 bytes */
	      if(*state < 6)
		goto badlen;
	      c5 = us[5] ^ Tx;
	      if(c5 & T2)
		goto bad;
	      wc = ((((((((((c0 & Mask6) << Bitx) |
			   c1) << Bitx) | c2) << Bitx) |
		       c3) << Bitx) | c4) << Bitx) | c5;
	      if(wc <= Wchar5)
		goto bad;
	      *pwc = wc;
	      return 6;
	    }
	    /* 5 bytes */
	    wc = ((((((((c0 & Mask5) << Bitx) |
		       c1) << Bitx) | c2) << Bitx) |
		   c3) << Bitx) | c4;
	    if(wc <= Wchar4)
	      goto bad;
	    *pwc = wc;
	    return 5;
	  }
	  if(c0 >= T4) {
	    /* 4 bytes */
	    if(*state < 4)
	      goto badlen;
	    c3 = us[3] ^ Tx;
	    if(c3 & T2)
	      goto bad;
	    wc = ((((((c0 & Mask4) << Bitx) |
		     c1) << Bitx) | c2) << Bitx) |
	      c3;
	    if(wc <= Wchar3)
	      goto bad;
	    *pwc = wc;
	    return 4;
	  }
	  /* 3 bytes */
	  wc = ((((c0 & Mask3) << Bitx) |
		 c1) << Bitx) | c2;
	  if(wc <= Wchar2)
	    goto bad;
	  *pwc = wc;
	  return 3;
	}
	if(c0 >= T2) {
	  /* 2 bytes */
	  if(*state < 2)
	    goto badlen;
	  c1 = us[1] ^ Tx;
	  if(c1 & T2)
	    goto bad;
	  wc = ((c0 & Mask2) << Bitx) |
	    c1;
	  if(wc <= Wchar1)
	    goto bad;
	  *pwc = wc;
	  return 2;
	}
	/* 1 byte */
	if(c0 >= Tx)
	  goto bad;
	*pwc = c0;
	return 1;
	
 bad:
	errno = EILSEQ;
	return -1;
 badlen:
	return -2;

	
#endif
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
#if defined(__QNXNTO__) || defined(__BEOS__) || defined(__OpenBSD__) || defined (__FreeBSD__) || defined (TARGET_OS_MAC)
  size_t thisLen=my_mbtowc(&wc,m_buf,m_bufLen,&m_state);
#else
  size_t thisLen=mbrtowc(&wc,m_buf,m_bufLen,&m_state);
#endif
  if(thisLen>MB_LEN_MAX)return 0;
  if(thisLen==0)thisLen=1;
  m_bufLen-=thisLen;
  return 1;
}

#else /* big if 0 */
#include "ut_assert.h"
#include "xap_EncodingManager.h"
#include "errno.h"

void UT_Mbtowc::initialize()
{
    UT_iconv_reset(cd);
    m_bufLen = 0;
};

void UT_Mbtowc::setInCharset(const char* charset)
{
    m_bufLen = 0;
    iconv_close(cd);
    cd = iconv_open("UCS-2", charset );
};

UT_Mbtowc::UT_Mbtowc(): m_bufLen(0)
{
    cd = iconv_open("UCS-2", XAP_EncodingManager::get_instance()->getNativeEncodingName() );
    UT_ASSERT(cd!=(iconv_t)-1);    
};

UT_Mbtowc::UT_Mbtowc(const UT_Mbtowc& v): m_bufLen(0)
{
    cd = iconv_open("UCS-2", XAP_EncodingManager::get_instance()->getNativeEncodingName() );
    UT_ASSERT(cd!=(iconv_t)-1);    
};

UT_Mbtowc::~UT_Mbtowc()
{
    /*libiconv is stupid - we'll get segfault if we don't check  - VH */
    if (cd!=(iconv_t)-1)
	    iconv_close(cd);
};

int UT_Mbtowc::mbtowc(wchar_t &wc,char mb)
{
    if(++m_bufLen>MB_LEN_MAX) {
      initialize();
      return 0;
    }
    m_buf[m_bufLen-1]=mb;
    const char* inptr = m_buf;
    unsigned char outbuf[2];
    char* outptr = (char* )outbuf;
    size_t inlen = m_bufLen, outlen = 2;
    size_t len  = iconv(cd,const_cast<ICONV_CONST char **>(&inptr),&inlen,&outptr,&outlen);
    if (len!=(size_t)-1) {
	bool swap = XAP_EncodingManager::swap_stou;
	unsigned short val = outbuf[swap] | (outbuf[!swap]<<8);
	wc = (wchar_t )val;
	m_bufLen = 0;
	return 1;
    } else {
	if (errno==EINVAL)
	    return 0; /* need more chars */
	else {
	    initialize();/*wrong seq*/
	    return 0;
	};
    };
};

#endif /* big if 0 */
