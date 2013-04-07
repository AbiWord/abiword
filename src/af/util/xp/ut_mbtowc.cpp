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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <string.h>
#include <limits.h>

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
  m_converter(new Converter(UT_LocaleInfo::system().getEncoding().c_str())),
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
	size_t inlen = m_bufLen;

	const UT_iconv_t cd = m_converter->cd ();
	if (!UT_iconv_isValid ( cd ) )
	{
		initialize(true);
		return 0;
	}

	gsize bytes_read = 0;
	gsize bytes_written = 0;
	GError* error = NULL;
	gchar* out = g_convert_with_iconv(inptr, inlen, (GIConv)cd, &bytes_read, &bytes_written, &error);
	if (out && bytes_written == 2)
	{
		memcpy(&wc, out, 2);
		m_bufLen = 0;
		FREEP(out);
		return 1;
	}

	FREEP(out);

	if (bytes_written != 2 || (out == NULL && !error))
	{
		// reset iconv, pointer might be messed up; need more chars...
		initialize (false);
		return 0;
	}

	// wrong seq
	initialize (true);
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
  m_converter(new Converter(UT_LocaleInfo::system().getEncoding().c_str())),
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
	size_t inlen = m_bufLen;

	const UT_iconv_t cd = m_converter->cd ();
	if (!UT_iconv_isValid ( cd ) )
	{
		initialize(true);
		return 0;
	}

	gsize bytes_read = 0;
	gsize bytes_written = 0;
	GError* error = NULL;
	gchar* out = g_convert_with_iconv(inptr, inlen, (GIConv)cd, &bytes_read, &bytes_written, &error);
	if (out && bytes_written == 4)
	{
		memcpy(&wc, out, 4);
		m_bufLen = 0;
		FREEP(out);
		return 1;
	}

	FREEP(out);

	if (bytes_written != 4 && (out == NULL && !error))
	{
		// reset iconv, pointer might be messed up; need more chars...
		initialize (false);
		return 0;
	}

	// wrong seq
	initialize (true);
	return 0;
}
