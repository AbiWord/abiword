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

// UTF-8 can use up to 6 bytes
#define MY_MB_LEN_MAX 6

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

	if (UT_iconv_isValid(cd))
		UT_iconv_close(cd);

    cd = UT_iconv_open("UCS-2", charset );
};

UT_Mbtowc::UT_Mbtowc(const char* from_charset): m_bufLen(0)
{
    cd = UT_iconv_open("UCS-2", from_charset);
    UT_ASSERT(UT_iconv_isValid(cd));
};

UT_Mbtowc::UT_Mbtowc(): m_bufLen(0)
{
    cd = UT_iconv_open("UCS-2", XAP_EncodingManager::get_instance()->getNative8BitEncodingName() );
    UT_ASSERT(UT_iconv_isValid(cd)); 
};

UT_Mbtowc::UT_Mbtowc(const UT_Mbtowc& v): m_bufLen(0)
{
	// Shouldn't a copy also copy the encoding?
    cd = UT_iconv_open("UCS-2", XAP_EncodingManager::get_instance()->getNative8BitEncodingName() );
    UT_ASSERT(UT_iconv_isValid(cd));
};

UT_Mbtowc::~UT_Mbtowc()
{
    /* libiconv is stupid - we'll get segfault if we don't check  - VH */
  if (UT_iconv_isValid(cd))
	    UT_iconv_close(cd);
};

int UT_Mbtowc::mbtowc(UT_UCS4Char &wc,char mb)
{
    if(++m_bufLen>MY_MB_LEN_MAX) {
      initialize();
      return 0;
    }
    m_buf[m_bufLen-1]=mb;
    const char* inptr = m_buf;
    unsigned char outbuf[2];
    char* outptr = (char* )outbuf;
    size_t inlen = m_bufLen, outlen = 2;
    size_t len  = UT_iconv(cd,&inptr,&inlen,&outptr,&outlen);
    if (len!=(size_t)-1) {
	bool swap = XAP_EncodingManager::swap_stou;
	unsigned short val = outbuf[swap] | (outbuf[!swap]<<8);
	wc = (UT_UCS4Char)val;
	m_bufLen = 0;
	return 1;
    } else {
	if (errno==EINVAL)
	{
			UT_iconv_reset(cd); /* reset iconv, pointer might be messed up. */
			return 0; /* need more chars */
	}
	else {
	    initialize();/*wrong seq*/
	    return 0;
	}
    }
};

UT_UCS2_mbtowc::Converter::Converter (const char * from_charset) :
  m_count(1),
  m_cd(UT_iconv_open (ucs2Internal (), from_charset))
{
  // 
}

UT_UCS2_mbtowc::Converter::~Converter ()
{
  if (UT_iconv_isValid (m_cd)) UT_iconv_close (m_cd);
}

void UT_UCS2_mbtowc::Converter::initialize ()
{
  UT_iconv_reset (m_cd);
}

void UT_UCS2_mbtowc::initialize (bool clear)
{
  m_converter->initialize ();
  if (clear) m_bufLen = 0;
}

UT_UCS2_mbtowc::UT_UCS2_mbtowc () :
  m_converter(new Converter(XAP_EncodingManager::get_instance()->getNative8BitEncodingName())),
  m_bufLen(0)
{
  // 
}

UT_UCS2_mbtowc::UT_UCS2_mbtowc (const char * from_charset) :
  m_converter(new Converter(from_charset)),
  m_bufLen(0)
{
  // 
}

UT_UCS2_mbtowc::UT_UCS2_mbtowc (const UT_UCS2_mbtowc & rhs) :
  m_converter(rhs.m_converter),
  m_bufLen(0)
{
  m_converter->ref ();
}

UT_UCS2_mbtowc::~UT_UCS2_mbtowc ()
{
  if (!m_converter->unref ()) delete m_converter;
}

void UT_UCS2_mbtowc::setInCharset (const char * from_charset)
{
  Converter * converter = new Converter(from_charset);
  if (converter)
    {
      if (!m_converter->unref ()) delete m_converter;
      m_converter = converter;
    }
}

int UT_UCS2_mbtowc::mbtowc (UT_UCS2Char & wc, char mb)
{
  if(++m_bufLen > MY_MB_LEN_MAX)
    {
      initialize ();
      return 0;
    }
  m_buf[m_bufLen-1] = mb;

  const char * inptr = m_buf;

  UT_UCS2Char ucs2;
  char * outptr = reinterpret_cast<char *>(&ucs2);

  size_t inlen = m_bufLen;
  size_t outlen = sizeof (UT_UCS2Char);

  const UT_iconv_t cd = m_converter->cd ();

  size_t len = UT_iconv (const_cast<UT_iconv_t>(cd), &inptr, &inlen, &outptr, &outlen);
  if (len != (size_t)-1)
    {
      wc = ucs2;
      m_bufLen = 0;
      return 1;
    }
  if (errno == EINVAL)
    {
      /* reset iconv, pointer might be messed up; need more chars...
       */
      initialize (false);
    }
  else
    {
      initialize (true); /* wrong seq */
    }
  return 0;
}

UT_UCS4_mbtowc::Converter::Converter (const char * from_charset) :
  m_count(1),
  m_cd(UT_iconv_open (UCS_INTERNAL, from_charset))
{
  // 
}

UT_UCS4_mbtowc::Converter::~Converter ()
{
  if (UT_iconv_isValid (m_cd)) UT_iconv_close (m_cd);
}

void UT_UCS4_mbtowc::Converter::initialize ()
{
  UT_iconv_reset (m_cd);
}

void UT_UCS4_mbtowc::initialize (bool clear)
{
  m_converter->initialize ();
  if (clear) m_bufLen = 0;
}

UT_UCS4_mbtowc::UT_UCS4_mbtowc () :
  m_converter(new Converter(XAP_EncodingManager::get_instance()->getNative8BitEncodingName())),
  m_bufLen(0)
{
  // 
}

UT_UCS4_mbtowc::UT_UCS4_mbtowc (const char * from_charset) :
  m_converter(new Converter(from_charset)),
  m_bufLen(0)
{
  // 
}

UT_UCS4_mbtowc::UT_UCS4_mbtowc (const UT_UCS4_mbtowc & rhs) :
  m_converter(rhs.m_converter),
  m_bufLen(0)
{
  m_converter->ref ();
}

UT_UCS4_mbtowc::~UT_UCS4_mbtowc ()
{
  if (!m_converter->unref ()) delete m_converter;
}

void UT_UCS4_mbtowc::setInCharset (const char * from_charset)
{
  Converter * converter = new Converter(from_charset);
  if (converter)
    {
      if (!m_converter->unref ()) delete m_converter;
      m_converter = converter;
    }
}

int UT_UCS4_mbtowc::mbtowc (UT_UCS4Char & wc, char mb)
{
  if(++m_bufLen > MY_MB_LEN_MAX)
    {
      initialize ();
      return 0;
    }
  m_buf[m_bufLen-1] = mb;

  const char * inptr = m_buf;

  UT_UCS4Char ucs4;
  char * outptr = reinterpret_cast<char *>(&ucs4);

  size_t inlen = m_bufLen;
  size_t outlen = sizeof (UT_UCS4Char);

  const UT_iconv_t cd = m_converter->cd ();

  size_t len = UT_iconv (const_cast<UT_iconv_t>(cd), &inptr, &inlen, &outptr, &outlen);
  if (len != (size_t)-1)
    {
      wc = ucs4;
      m_bufLen = 0;
      return 1;
    }
  if (errno == EINVAL)
    {
      /* reset iconv, pointer might be messed up; need more chars...
       */
      initialize (false);
    }
  else
    {
      initialize (true); /* wrong seq */
    }
  return 0;
}
