/* AbiSource Program Utilities
 * Copyright (C) 2001
 * 
 * This file is the work of:
 *    Dom Lachowicz <dominicl@seas.upenn.edu>
 *    Mike Nordell  <tamlin@alognet.se>
 *    and various members of the GLib team (http://www.gtk.org)
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

#include "ut_iconv.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*!
 * This class is a nice wrapper around an iconv_t type
 */
auto_iconv::auto_iconv(iconv_t iconv) 
  : m_h(iconv) 
{ 
}

/*!
 * Convert characters from in_charset to out_charset
 */
auto_iconv::auto_iconv(const char * in_charset, const char *out_charset)
{
  m_h = iconv_open (in_charset, out_charset);
}

/*!
 * Public destructor
 */
auto_iconv::~auto_iconv() 
{ 
  if (m_h != (iconv_t)-1) 
    { 
      iconv_close(m_h); 
    } 
}

/*!
 * Returns the internal iconv_t handle
 */
auto_iconv::operator iconv_t() 
{ 
  return m_h;
}

/*!
 * Returns true is the internal handle is valid, false is not
 */
bool auto_iconv::is_valid() const 
  { 
    return m_h != (iconv_t)-1; 
  }

/*!
 * Borrowed from GLib 2.0 and modified
 *
 * str - Pointer to the input string.
 * len - Length of the input string to convert. If len
 *     is zero the whole string (strlen(str) ) is used for conversion.
 * to_codeset - The "codeset" of the string pointed to by 'str'.
 * from_codeset - The "codeset" we want for the output.
 * bytes_read - optional, supply NULL if you don't want this.
 * bytes_written - optional, supply NULL if you don't want this.
 *
 */
extern "C"
bool UT_convert(const char*	str,
		UT_uint32	len,
		const char*	to_codeset,
		const char*	from_codeset,
		UT_uint32*	bytes_read_arg,
		UT_uint32*	bytes_written_arg)
{
	if (!str || !to_codeset || !from_codeset)
	{
		return 0;
	}

	// The following two variables are used to be used in absence of given arguments
	// (to not have to check for NULL pointers upon assignment).
	UT_uint32 bytes_read_local;
	UT_uint32 bytes_written_local;

	UT_uint32& bytes_read = bytes_read_arg ? *bytes_read_arg : bytes_read_local; 
	UT_uint32& bytes_written = bytes_written_arg ? *bytes_written_arg : bytes_written_local;

	auto_iconv cd(to_codeset, from_codeset);

	if (!cd.is_valid())
	{
		bytes_read = 0;
		bytes_written = 0;
		return NULL;
	}

	if (len < 0)
	{
		len = strlen(str);
	}

	const char* p = str;
	size_t inbytes_remaining = len;

	/* Due to a GLIBC bug, round outbuf_size up to a multiple of 4 */
	/* + 1 for nul in case len == 1 */
	size_t outbuf_size = ((len + 3) & ~3) + 1;
	size_t outbytes_remaining = outbuf_size - 1; /* -1 for nul */

	char* pDest = (char*)malloc(outbuf_size);
	char* outp = pDest;

	bool have_error = false;
	bool bAgain = true;

	while (bAgain)
	{
  		size_t err = iconv(cd,
				   const_cast<ICONV_CONST char**>(&p),
				   &inbytes_remaining,
				   &outp, &outbytes_remaining);

		if (err == (size_t) -1)
		{
			switch (errno)
			{
			case EINVAL:
				/* Incomplete text, do not report an error */
				break;
			case E2BIG:
				{
					size_t used = outp - pDest;

					/* glibc's iconv can return E2BIG even if there is space
					 * remaining if an internal buffer is exhausted. The
					 * folllowing is a heuristic to catch this. The 16 is
					 * pretty arbitrary.
					*/
					if (used + 16 > outbuf_size)
					{
						outbuf_size = (outbuf_size - 1) * 2 + 1;
						pDest = (char*)realloc(pDest, outbuf_size);
						
						outp = pDest + used;
						outbytes_remaining = outbuf_size - used - 1; /* -1 for nul */
					}

					bAgain = false;
					break;
				}
			default:
				have_error = true;
				break;
			}
		}
	}

	*outp = '\0';
  
	const size_t nNewLen = p - str;

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
		free(pDest);
	}

	return have_error;
}
