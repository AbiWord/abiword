/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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
#include <errno.h>

#include "ut_mbtowc.h"
#include "ut_locale.h"


UT_UCS2_mbtowc::Converter::Converter (const char * from_charset) :
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
  m_converter(new Converter(UT_LocaleInfo::system().getEncoding().utf8_str())),
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

UT_UCS2_mbtowc::~UT_UCS2_mbtowc ()
{
  delete m_converter;
}

void UT_UCS2_mbtowc::setInCharset (const char * from_charset)
{
  Converter * converter = new Converter(from_charset);
  if (converter)
    {
      delete m_converter;
      m_converter = converter;
    }
}

int UT_UCS2_mbtowc::mbtowc (UT_UCS2Char & wc, char mb)
{
  if(++m_bufLen > iMbLenMax)
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
  m_converter(new Converter(UT_LocaleInfo::system().getEncoding().utf8_str())),
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

UT_UCS4_mbtowc::~UT_UCS4_mbtowc ()
{
  delete m_converter;
}

void UT_UCS4_mbtowc::setInCharset (const char * from_charset)
{
  Converter * converter = new Converter(from_charset);
  if (converter)
    {
      delete m_converter;
      m_converter = converter;
    }
}

int UT_UCS4_mbtowc::mbtowc (UT_UCS4Char & wc, char mb)
{
  if(++m_bufLen > iMbLenMax)
    {
      initialize ();
      return 0;
    }
  m_buf[m_bufLen-1] = mb;

  const char * inptr = m_buf;

  // need eight bytes in order for the hack below to work (see below)
  UT_UCS4Char ucs4[2];
  char * outptr = reinterpret_cast<char *>(&ucs4);

  size_t inlen = m_bufLen;
  size_t outlen = sizeof (ucs4[0]);

  const UT_iconv_t cd = m_converter->cd ();

  size_t len = UT_iconv (const_cast<UT_iconv_t>(cd), &inptr, &inlen, &outptr, &outlen);

  // This is a nasty hack I would prefer not to have to do
  // here. Sometimes iconv does not write the translated value out,
  // and it sits somewhere internatlly until the next call; on the
  // next call it issues the previous character and the new one sits
  // somewhere in. I discovered this in the rtf importer when trying
  // to hanlde cp1255, and it is caused by the fact that some
  // combinations of characters in 1255 can translate into a
  // precombined Unicode glyph, and iconv is waiting for the next
  // character. I flush it out by feeding it a NULL character.
  // (The output buffer in that case needs to be 8 bytes, so that the
  // iconv can output both the cached charcter and the NULL, otherwise
  // it will complain.)
  // Tomas, May 2, 2003
  if(len == 0 && outlen == sizeof(ucs4[0]))
  {
	  char c = 0;
	  inptr = &c;
	  inlen = 1;
	  outlen = sizeof (ucs4);
	  len = UT_iconv (const_cast<UT_iconv_t>(cd), &inptr, &inlen, &outptr, &outlen);
  }
  
  if (len != (size_t)-1)
    {
      wc = ucs4[0];
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
