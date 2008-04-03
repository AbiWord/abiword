/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 2001
 * 
 * This file is the work of:
 *    Dom Lachowicz <dominicl@seas.upenn.edu>
 *    Mike Nordell  <tamlin@alognet.se>
 *
 *    The UT_convert method was completed by Dom and Mike and was
 *    based upon work done by various members of the GLib team 
 *    (http://www.gtk.org)
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
#include <stdlib.h>
#include <errno.h>

#include "ut_iconv.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include <glib.h>

#include "xap_EncodingManager.h"

/************************************************************************/
/************************************************************************/
/*
 * This file represents my own personal assault on iconv, the most horrid
 * utility ever, which is yet somehow still essential.
 *
 * Issues - 
 * 1) freebsd: requires extern "C" around iconv.h
 * 2) invalid iconv handles (== iconv_t -1 (usually))
 * 3) iconv resetting (vlad's i18n issues)
 * 4) ICONV_CONST passed to iconv()
 * 5) UCS2 internally to AbiWord
 * 6) byte-order problems
 * 7) good C/C++ linkage
 * 
 * Provides solutions to all of the above plus -
 * 1) 1-shot conversions (UT_convert, UT_convert_cd)
 * 2) wrapper class around an iconv_t handle
 */

/*!
 * This class is a nice wrapper around an iconv_t type
 */
auto_iconv::auto_iconv(UT_iconv_t iconv) 
  : m_h(iconv) 
{ 
}

/*!
 * Convert characters from in_charset to out_charset
 */
auto_iconv::auto_iconv(const char * in_charset, const char *out_charset)
  UT_THROWS((UT_iconv_t))
{
	m_h = UT_ICONV_INVALID;

	UT_iconv_t cd = UT_iconv_open (out_charset, in_charset);
	
	if (!UT_iconv_isValid(cd))
		UT_THROW(cd);
	
	m_h = cd;
}

/*!
 * Public destructor
 */
auto_iconv::~auto_iconv() 
{ 
  if (UT_iconv_isValid(m_h)) 
    { 
      UT_iconv_close(m_h); 
    } 
}

/*!
 * Returns the internal iconv_t handle
 */
auto_iconv::operator UT_iconv_t() 
{ 
  return m_h;
}

UT_iconv_t auto_iconv::getHandle ()
{
  return m_h;
}

/************************************************************************/
/************************************************************************/

//
// everything below this line is extern "C"
//

static const char * s_ucs2_internal = 0;
static const char * s_ucs4_internal = 0;

static const char * s_ucs2_list[] = {
	"UCS-2-INTERNAL",
	"UCS-2-LE",
	"UCS-2-BE",
	"UCS-2LE",
	"UCS-2BE",
	"UTF16-LE",
	"UTF16-BE",
	"UCS2",
	"UCS-2",
	"UTF-16",
	0
};

static const char * s_ucs4_list[] = {
	"UCS-4-INTERNAL",
	"UCS-4-LE",
	"UCS-4-BE",
	"UCS-4LE",
	"UCS-4BE",
	"UTF-32LE",
	"UTF-32BE",
	"UCS4",
	"UCS-4",
	"UTF32",
	"UTF-32",
	0
};

static void s_internal_init ()
{
	static const char * latin = "ISO-8859-1";

	UT_iconv_t handle = UT_ICONV_INVALID;

	s_ucs2_internal = 0;
	s_ucs4_internal = 0;

	const char ** pszEnc = s_ucs2_list;
	while (*pszEnc)
		{
			if (!UT_iconv_isValid(handle = UT_iconv_open (*pszEnc, latin)))
				{
					pszEnc++;
					continue;
				}
			const char ibuf = 0x20;
			const char * iptr = &ibuf;
			size_t ilen = 1;
			UT_UCS2Char obuf[2];
			char * optr = reinterpret_cast<char *>(obuf);
			size_t olen = 2;

			bool success = ((size_t)(-1) != UT_iconv (handle, &iptr, &ilen, &optr, &olen));

			UT_iconv_close (handle);
			handle = UT_ICONV_INVALID;

			if (success) success = (olen == 0);
			if (success) success = (obuf[0] == 0x20);
			if (success)
				{
					s_ucs2_internal = *pszEnc;
					break;
				}
			pszEnc++;
		}
	UT_ASSERT(s_ucs2_internal);
	if (s_ucs2_internal == 0)
		{
			s_ucs2_internal = s_ucs2_list[0];
			UT_DEBUGMSG(("WARNING! this test failed to determine correct UCS-2 setting!\n"));
		}
	UT_DEBUGMSG(("using '%s' for UCS-2 internal\n", s_ucs2_internal));

	pszEnc = s_ucs4_list;
	while (*pszEnc)
		{
			if (!UT_iconv_isValid(handle = UT_iconv_open (*pszEnc, latin)))
				{
					pszEnc++;
					continue;
				}
			const char ibuf = 0x20;
			const char * iptr = &ibuf;
			size_t ilen = 1;
			UT_UCS4Char obuf[4];
			char * optr = reinterpret_cast<char *>(&obuf);
			size_t olen = 4;

			bool success = ((size_t)(-1) != UT_iconv (handle, &iptr, &ilen, &optr, &olen));

			UT_iconv_close (handle);
			handle = UT_ICONV_INVALID;

			if (success) success = (olen == 0);
			if (success) success = (obuf[0] == 0x20);
			if (success)
				{
					s_ucs4_internal = *pszEnc;
					break;
				}
			pszEnc++;
		}
	UT_ASSERT(s_ucs4_internal);
	if (s_ucs4_internal == 0)
		{
			s_ucs4_internal = s_ucs4_list[0];
			UT_DEBUGMSG(("WARNING! this test failed to determine correct UCS-4 setting!\n"));
		}
	UT_DEBUGMSG(("using '%s' for UCS-4 internal\n", s_ucs4_internal));
}

/*!
 * \return the internal iconv UCS-2 charset name
 */
const char * ucs2Internal ()
{
#if defined(WIN32)
  // we special-case the win32 build, otherwise spelling and other stuff
  // just doesn't work
  return "UCS-2LE";
#elif defined(_LIBICONV_H)
  // libiconv seems to prefer UCS-2-INTERNAL to UCS-2BE and UCS-2LE
  return "UCS-2-INTERNAL";
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__QNXNTO__) || defined(__NetBSD__)
  // we special case the BSDs since spelling just doesn't work
  return "UCS2";
#else
  // general case, found by hub and dom
	if (s_ucs2_internal == 0) 
		s_internal_init ();
	return s_ucs2_internal;
#endif
}

/*!
 * \return the internal iconv UCS-4 charset name
 */
const char * ucs4Internal ()
{
#if defined(WIN32)
  // we special-case the win32 build, otherwise spelling and other stuff
  // just doesn't work
  return "UCS-4LE";
#elif defined(_LIBICONV_H)
  // libiconv seems to prefer UCS-4-INTERNAL to UCS-4BE and UCS-4LE
  return "UCS-4-INTERNAL";
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  // we special case the BSDs since spelling just doesn't work
  return "UCS4";
#else
  // general case, found by hub and dom
	if (s_ucs4_internal == 0) 
		s_internal_init ();
	return s_ucs4_internal;
#endif
}

/************************************************************************/
/************************************************************************/

/*!
 * Returns true if the internal handle is valid, false if not
 */
int UT_iconv_isValid ( UT_iconv_t cd )
{
  return (cd != UT_ICONV_INVALID);
}

UT_iconv_t  UT_iconv_open( const char* to, const char* from )
{
  if ( to && from )
	  return (UT_iconv_t)g_iconv_open(to, from);

  return UT_ICONV_INVALID;
}

size_t UT_iconv( UT_iconv_t cd, const char **inbuf, 
				 size_t *inbytesleft, char **outbuf, size_t *outbytesleft )
{
  // this should take care of iconv problems with different compilers
  // known issues:
  // 1) gcc3.0 doesn't like const_cast<const pointer>()
  // 2) some iconv implementations don't use a const char ** inbuf
  //    while some (newer, conformant ones) do

  if ( !UT_iconv_isValid ( cd ) )
    return (size_t)-1;

  return g_iconv((GIConv)cd, (char **)inbuf, inbytesleft, outbuf, outbytesleft);
}

int  UT_iconv_close( UT_iconv_t cd )
{
  if ( UT_iconv_isValid ( cd ) )
    return g_iconv_close( (GIConv) cd );

  return -1;
}

void UT_iconv_reset(UT_iconv_t cd)
{
    // this insane code is needed by iconv brokenness.  see
    // http://www.abisource.com/mailinglists/abiword-dev/01/April/0135.html
    if (XAP_EncodingManager::get_instance()->cjk_locale())
		UT_iconv(cd, NULL, NULL, NULL, NULL);
}

/*!
 * Borrowed from GLib 2.0 and (heavily) modified
 *
 * \param str Pointer to the input string.
 * \param len Length of the input string to convert.
 * \param from_codeset The "codeset" of the string pointed to by 'str'.
 * \param to_codeset The "codeset" we want for the output.
 * \param bytes_read optional, supply NULL if you don't want this.
 * \param bytes_written optional, supply NULL if you don't want this.
 *
 * \return Returns a freshly allocated output string, which is terminated by
 * a zero byte. Note that if the output codeset's terminator is not
 * a zero byte (e.g., UCS-2, where it is two zero bytes), you can 
 * get correct termination by including the input string's terminator 
 * in the length passed as 'len'. E.g., if 'str' is null-terminated 
 * US-ASCII "foo", given 'len' as 4.
 *
 * \todo Check for out-of-memory allocations etc.
 */
char * UT_convert(const char*	str,
		  UT_sint32	len,
		  const char*	from_codeset,
		  const char*	to_codeset,
		  UT_uint32*	bytes_read_arg,
		  UT_uint32*	bytes_written_arg)
{

	if (!str || !from_codeset || !to_codeset)
	{
		return NULL;
	}

	UT_TRY
	  {
	    auto_iconv converter(from_codeset, to_codeset);
	    return UT_convert_cd(str, len, converter, bytes_read_arg, bytes_written_arg);
	  }
	UT_CATCH(UT_CATCH_ANY)
	  {
	    if (bytes_read_arg)
		*bytes_read_arg = 0;
	    if (bytes_written_arg)
		*bytes_written_arg = 0;
	    return NULL;
	  }
	UT_END_CATCH
}

/*! This function is almost the same as the other UT_convert function,
 * only that it takes an UT_iconv_t instead of a from and to codeset.
 * This is useful if you need to do a conversion multiple times
 */
char * UT_convert_cd(const char *str,
		     UT_sint32 len,
		     UT_iconv_t cd,
		     UT_uint32 *bytes_read_arg,
		     UT_uint32 *bytes_written_arg)
{
  if ( !UT_iconv_isValid ( cd ) || !str || len < 0 )
    {
      return NULL ;
    }

	// The following two variables are used to be used in absence of given arguments
	// (to not have to check for NULL pointers upon assignment).
	UT_uint32 bytes_read_local;
	UT_uint32 bytes_written_local;

	UT_uint32& bytes_read = bytes_read_arg ? *bytes_read_arg : bytes_read_local; 
	UT_uint32& bytes_written = bytes_written_arg ? *bytes_written_arg : bytes_written_local;

	const char* p = str;
	size_t inbytes_remaining = len;

	/* Due to a GLIBC bug, round outbuf_size up to a multiple of 4 */
	/* + 1 for nul in case len == 1 */
	size_t outbuf_size = ((len + 3) & ~3) + 15;
	size_t outbytes_remaining = outbuf_size - 4; /* -4 for null (allow for ucs4 0) */

	char* pDest = static_cast<char*>(g_try_malloc(outbuf_size));
	char* outp = pDest;

	bool have_error = false;
	bool bAgain = true;

	while (bAgain)
	  {
	    size_t err = UT_iconv(cd,
	                          &p,
				  &inbytes_remaining,
				  &outp, &outbytes_remaining);

	    if (err == (size_t) -1)
	      {
	        switch (errno)
		  {
		  case EINVAL:
		    /* Incomplete text, do not report an error */
		    bAgain = false;
		    break;
		  case E2BIG:
		    {
		      size_t used = outp - pDest;

		      /* glibc's iconv can return E2BIG even if there is space
		       * remaining if an internal buffer is exhausted. The
		       * following is a heuristic to catch this. The 16 is
		       * pretty arbitrary.
		       */
		      if (used + 16 > outbuf_size)
		        {
		          outbuf_size = outbuf_size  + 15;
		          pDest = static_cast<char*>(g_try_realloc(pDest, outbuf_size));

		          outp = pDest + used;
		          outbytes_remaining = outbuf_size - used - 4; /* -1 for nul */
		        }

		      bAgain = true;
		      break;
		    }
		  default:
		    have_error = true;
		    bAgain = false;
		    break;
		  }
	      }
	    else 
	      {
		bAgain = false;
	      }
	  }

	// append 4 0s as a string terminator (so that even ucs4 string will be correctly terminated)
	for(UT_uint32 i = 0; i < 4; ++i)
		*outp++ = '\0';

	const UT_sint32 nNewLen = p - str;

	if (bytes_read_arg)
	  {
	    bytes_read = nNewLen;
	  }
	else
	  {
	    if (nNewLen != len) 
	      {
	        have_error = true;
	      }
	  }

	bytes_written = outp - pDest;	/* Doesn't include '\0' */

	if (have_error && pDest)
	  {
	    g_free(pDest);
	  }

	if (have_error)
	  return NULL;

	return pDest;
}
