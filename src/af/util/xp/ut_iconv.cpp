/* AbiSource Program Utilities
 * Copyright (C) 1998-2001AbiSource, Inc.
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
#include <iconv.h>
#include <stdlib.h>
#include <errno.h>

#define return_val_if_fail(c, v) if(!(c)) return (v)
#define return_if_fail(c) if(!(c)) return

/*!
 * Borrowed from GLib 2.0
 *
 */
extern "C"
bool      UT_convert                       (const char *str,
					    UT_uint32  len,
					    const char *to_codeset,
					    const char *from_codeset,
					    UT_uint32  *bytes_read,
					    UT_uint32  *bytes_written)
{
  char *dest;
  char *outp;
  const char *p;
  size_t inbytes_remaining;
  size_t outbytes_remaining;
  size_t err;
  iconv_t cd;
  size_t outbuf_size;
  bool have_error = false;
  
  return_val_if_fail (str != NULL, NULL);
  return_val_if_fail (to_codeset != NULL, NULL);
  return_val_if_fail (from_codeset != NULL, NULL);

  cd = iconv_open (to_codeset, from_codeset);

  if (cd == (iconv_t) -1)
    {
      if (bytes_read)
        *bytes_read = 0;
      
      if (bytes_written)
        *bytes_written = 0;
      
      return NULL;
    }

  if (len < 0)
    len = strlen (str);

  p = str;
  inbytes_remaining = len;

  /* Due to a GLIBC bug, round outbuf_size up to a multiple of 4 */
  /* + 1 for nul in case len == 1 */
  outbuf_size = ((len + 3) & ~3) + 1;
  
  outbytes_remaining = outbuf_size - 1; /* -1 for nul */
  outp = dest = (char *) malloc (outbuf_size);

 again:
  
  err = iconv (cd, const_cast<ICONV_CONST char **>(&p), &inbytes_remaining, 
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
	    size_t used = outp - dest;

	    /* glibc's iconv can return E2BIG even if there is space
	     * remaining if an internal buffer is exhausted. The
	     * folllowing is a heuristic to catch this. The 16 is
	     * pretty arbitrary.
	     */
	    if (used + 16 > outbuf_size)
	      {
		outbuf_size = (outbuf_size - 1) * 2 + 1;
		dest = (char *) realloc (dest, outbuf_size);
		
		outp = dest + used;
		outbytes_remaining = outbuf_size - used - 1; /* -1 for nul */
	      }

	    goto again;
	  }
	case EILSEQ:
	  have_error = true;
	  break;
	default:
	  have_error = true;
	  break;
	}
    }

  *outp = '\0';
  
  iconv_close (cd);

  if (bytes_read)
    *bytes_read = p - str;
  else
    {
      if ((p - str) != len) 
	{
          if (!have_error)
            {
                have_error = true;
            }
	}
    }

  if (bytes_written)
    *bytes_written = outp - dest;	/* Doesn't include '\0' */

  if (have_error)
    {
      if (dest)
	free (dest);
    }

  return have_error;
}
