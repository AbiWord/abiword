/* AbiSource Program Utilities
 * Copyright (C) 1998-2001 AbiSource, Inc.
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

#ifndef UT_ICONV_H
#define UT_ICONV_H

#ifdef __cplusplus
/* make freebsd happy */
extern "C" {
#include <iconv.h>
}
#else
#include <iconv.h>
#endif /* c++ */

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

typedef iconv_t UT_iconv_t;

#ifdef __cplusplus

#include "ut_exception.h"

class ABI_EXPORT auto_iconv
{
 public:

  explicit auto_iconv(UT_iconv_t iconv);

  explicit auto_iconv(const char * in_charset, const char *out_charset)
    UT_THROWS((UT_iconv_t));
  ~auto_iconv();
  operator UT_iconv_t();

  UT_iconv_t getHandle ();

 private:

  auto_iconv(const auto_iconv&);	// no impl
  void operator=(const auto_iconv&);	// no impl
  UT_iconv_t m_h;
};

#endif /* c++ */

UT_BEGIN_EXTERN_C

const char * ucs2Internal ();
#define UCS_2_INTERNAL ucs2Internal()
const char * ucs4Internal ();
#define UCS_INTERNAL ucs4Internal()

UT_iconv_t  UT_iconv_open( const char* to, const char* from );
size_t      UT_iconv( UT_iconv_t cd, const char **inbuf, 
		      size_t *inbytesleft, char **outbuf, 
		      size_t *outbytesleft );
int         UT_iconv_close( UT_iconv_t cd );
void        UT_iconv_reset( UT_iconv_t cd );
int         UT_iconv_isValid ( UT_iconv_t cd );

char *      UT_convert (const char *str,
			UT_sint32 len,
			const char *from_codeset,
			const char *to_codeset,
			UT_uint32 *bytes_read,
			UT_uint32 *bytes_written);

char *      UT_convert_cd (const char *str,
			   UT_sint32 len,
			   UT_iconv_t cd,
			   UT_uint32 *bytes_read,
			   UT_uint32 *bytes_written);

UT_END_EXTERN_C

#endif /* UT_ICONV_H */




