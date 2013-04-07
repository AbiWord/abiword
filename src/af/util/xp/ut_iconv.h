/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifndef UT_ICONV_H
#define UT_ICONV_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

typedef void * UT_iconv_t;

#define UT_ICONV_INVALID ((UT_iconv_t)(-1))

#ifdef __cplusplus


class ABI_EXPORT auto_iconv
{
 public:

  explicit auto_iconv(UT_iconv_t iconv);

  explicit auto_iconv(const char * in_charset, const char *out_charset)
      throw(UT_iconv_t);
  ~auto_iconv();
  operator UT_iconv_t();

  UT_iconv_t getHandle ();

 private:

  auto_iconv(const auto_iconv&);	// no impl
  void operator=(const auto_iconv&);	// no impl
  UT_iconv_t m_h;
};

#endif /* c++ */

G_BEGIN_DECLS

ABI_EXPORT const char * ucs2Internal ();
#define UCS_2_INTERNAL ucs2Internal()
ABI_EXPORT const char * ucs4Internal ();
#define UCS_INTERNAL ucs4Internal()

ABI_EXPORT UT_iconv_t  UT_iconv_open( const char* to, const char* from );
ABI_EXPORT size_t      UT_iconv( UT_iconv_t cd, const char **inbuf,
		      size_t *inbytesleft, char **outbuf,
		      size_t *outbytesleft );
ABI_EXPORT int         UT_iconv_close( UT_iconv_t cd );
ABI_EXPORT void        UT_iconv_reset( UT_iconv_t cd );
ABI_EXPORT int         UT_iconv_isValid ( UT_iconv_t cd );

ABI_EXPORT char *      UT_convert (const char *str,
			UT_sint32 len,
			const char *from_codeset,
			const char *to_codeset,
			UT_uint32 *bytes_read,
			UT_uint32 *bytes_written);

ABI_EXPORT char *      UT_convert_cd (const char *str,
			   UT_sint32 len,
			   UT_iconv_t cd,
			   UT_uint32 *bytes_read,
			   UT_uint32 *bytes_written);

G_END_DECLS

#endif /* UT_ICONV_H */




