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

#ifndef UT_BASE64_H
#define UT_BASE64_H

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_bytebuf.h"

UT_Bool UT_Base64Encode(UT_ByteBuf * pDest, const UT_ByteBuf * pSrc);
UT_Bool UT_Base64Decode(UT_ByteBuf * pDest, const UT_ByteBuf * pSrc);

#ifdef UT_TEST
#include "ut_test.h"
void UT_Base64_Test(FILE * fp);
#endif

#endif /* UT_BASE64_H */

