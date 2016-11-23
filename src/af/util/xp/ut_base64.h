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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef UT_BASE64_H
#define UT_BASE64_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "ut_bytebuf.h"

ABI_EXPORT bool UT_Base64Encode(const UT_ByteBufPtr & pDest, const UT_ConstByteBufPtr & pSrc);
ABI_EXPORT bool UT_Base64Decode(const UT_ByteBufPtr & pDest, const UT_ConstByteBufPtr & pSrc);

ABI_EXPORT bool UT_UTF8_Base64Encode(char *& b64ptr, size_t & b64len, const char *& binptr, size_t & binlen);
ABI_EXPORT bool UT_UTF8_Base64Decode(char *& binptr, size_t & binlen, const char *& b64ptr, size_t & b64len);

#ifdef UT_TEST
#include "ut_test.h"
void UT_Base64_Test(FILE * fp);
#endif

#endif /* UT_BASE64_H */

