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

#ifndef UT_XPM2BMP_H
#define UT_XPM2BMP_H

#ifndef XP_MAC_TARGET_QUARTZ
# include <QuickDraw.h>
#else
# include <CoreGraphics/CGImage.h>
#endif

class UT_RGBColor;

UT_Bool UT_Xpm2Bmp(UT_uint32 maxWidth,
				   UT_uint32 maxHeight,
				   const char ** pIconData,
				   UT_uint32 sizeofData,
				   UT_RGBColor * pBackgroundColor,
#ifndef XP_MAC_TARGET_QUARTZ
				   PixMapHandle pBitmap);
#else
				   CGImageRef pBitmap);
#endif

#endif /* UT_XPM2BMP_H */

