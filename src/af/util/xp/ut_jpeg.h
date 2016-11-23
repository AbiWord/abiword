/* AbiSource Program Utilities
 * Copyright (C) 2009 Hubert Figuiere
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


#ifndef _UT_JPEG_H_
#define _UT_JPEG_H_

#include "ut_types.h"
#include "ut_bytebuf.h"

ABI_EXPORT bool UT_JPEG_getDimensions(const UT_ConstByteBufPtr & pBB, UT_sint32& iImageWidth,
                                      UT_sint32& iImageHeight);

ABI_EXPORT bool UT_JPEG_getRGBData(const UT_ByteBuf* pBB, UT_Byte* pDest,
								   UT_sint32 iDestRowSize, bool bBGR, bool bFlipHoriz);

#endif
