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

#include "ut_iconv.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "xap_EncodingManager.h"

/************************************************************************/
/************************************************************************/

/*
 * This file represents my own personal assault on iconv, the most horrid
 * utility ever, which is yet somehow still essential.
 *
 * Issues - 
 * 1) freebsd: requires extern "C" around iconv.h
 * 2) invalid iconv handles (== iconv_t -1)
 * 3) iconv resetting
 * 4) ICONV_CONST passed to iconv()
 * 5) UCS2 internally to AbiWord
 * 6) byte-order problems
 * 7) good C/C++ linkage
 * 
 * Provides solutions to all of the above plus -
 * 1) 1-shot conversions (UT_convert)
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
auto_iconv::operator iconv_t() 
{ 
  return m_h;
}

/************************************************************************/
/************************************************************************/

//
// everything below this line is extern "C"
//

const char * ucs2Internal ()
{

#if defined(WIN32)
  // we special-case the win32 build, otherwise spelling and other stuff
  // just doesn't work
  return "UCS-2LE";
#else
  // general case, found by hub and dom
  if (XAP_EncodingManager__swap_stou)
    return "UCS-2BE";
  else
    return "UCS-2LE";
#endif

}

/************************************************************************/
/************************************************************************/

/*!
 * Returns true is the internal handle is valid, false is not
 */
int UT_iconv_isValid ( UT_iconv_t cd )
{
  return (cd != (UT_iconv_t)-1);
}

UT_iconv_t  UT_iconv_open( const char* to, const char* from )
{
    return iconv_open( to, from );
}

size_t  UT_iconv( UT_iconv_t cd, const char **inbuf, 
		  size_t *inbytesleft, char **outbuf, size_t *outbytesleft )
{
  // this should take care of iconv problems with different compilers
  // known issues:
  // 1) gcc3.0 doesn't like const_cast<const pointer>()
  // 2) some iconv implementations don't use a const char ** inbuf
  //    while some (newer, conformant ones) do
  ICONV_CONST char ** buf = const_cast<char **>(inbuf);
  return iconv( cd, buf, inbytesleft, outbuf, outbytesleft );
}

int  UT_iconv_close( UT_iconv_t cd )
{
    return iconv_close( cd );
}

void UT_iconv_reset(UT_iconv_t cd)
{
    // this insane code is needed by iconv brokenness.  see
    // http://www.abisource.com/mailinglists/abiword-dev/01/April/0135.html
    if (XAP_EncodingManager::get_instance()->cjk_locale())
		UT_iconv(cd, NULL, NULL, NULL, NULL);
};

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
 * Returns a freshly allocated output string, which is 0-terminated
 * (though I am not sure that has any significance in the general case).
 * TODO: Check for out-of-memory allocations etc.
 */
extern "C"
char * UT_convert(const char*	str,
		  UT_uint32	len,
		  const char*	to_codeset,
		  const char*	from_codeset,
		  UT_uint32*	bytes_read_arg,
		  UT_uint32*	bytes_written_arg)
{

	if (!str || !to_codeset || !from_codeset)
	{
		return NULL;
	}

	// The following two variables are used to be used in absence of given arguments
	// (to not have to check for NULL pointers upon assignment).
	UT_uint32 bytes_read_local;
	UT_uint32 bytes_written_local;

	UT_uint32& bytes_read = bytes_read_arg ? *bytes_read_arg : bytes_read_local; 
	UT_uint32& bytes_written = bytes_written_arg ? *bytes_written_arg : bytes_written_local;

	UT_TRY
	  {
	    auto_iconv cd(to_codeset, from_codeset);

	    if (len < 0)
	      {
		len = strlen(str);
	      }
	    
	    const char* p = str;
	    size_t inbytes_remaining = len;
	    
	    /* Due to a GLIBC bug, round outbuf_size up to a multiple of 4 */
	    /* + 1 for nul in case len == 1 */
	    size_t outbuf_size = ((len + 3) & ~3) + 15;
	    size_t outbytes_remaining = outbuf_size - 1; /* -1 for nul */
	    
	    char* pDest = (char*)malloc(outbuf_size);
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
			   * folllowing is a heuristic to catch this. The 16 is
			   * pretty arbitrary.
			   */
			  if (used + 16 > outbuf_size)
			    {
			      outbuf_size = outbuf_size  + 15;
			      pDest = (char*)realloc(pDest, outbuf_size);
			      
			      outp = pDest + used;
			      outbytes_remaining = outbuf_size - used - 1; /* -1 for nul */
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

	    if (have_error)
	      return NULL;

	    return pDest;
	  }
	UT_CATCH(UT_CATCH_ANY)
	  {
	    bytes_read = 0;
	    bytes_written = 0;
	    return NULL;
	  }
	UT_END_CATCH
}
