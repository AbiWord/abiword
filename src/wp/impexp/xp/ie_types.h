/* AbiWord
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

#ifndef IE_TYPES_H
#define IE_TYPES_H

/*
  NOTE:  File filters can support one or MORE of the following
  NOTE:  file types.  However, each of these file types can be
  NOTE:  handled by only ONE filter.  So, the number of file types
  NOTE:  supported by all the filters combined should be equal
  NOTE:  to (or less than) the number below.
*/

#include "ut_types.h"

typedef UT_sint32 IEFileType;
#define IEFT_Bogus   (IEFileType)-1
#define IEFT_Unknown (IEFileType)0

typedef UT_sint32 IEGraphicFileType;
#define IEGFT_Bogus (IEGraphicFileType)-1
#define IEGFT_Unknown (IEGraphicFileType)0

// temporary hack so that we don't bust too many things - DOM
#define IEGFT_PNG IE_ImpGraphic::fileTypeForSuffix(".png")
#define IEGFT_SVG IE_ImpGraphic::fileTypeForSuffix(".svg")
#define IEGFT_BMP IE_ImpGraphic::fileTypeForSuffix(".bmp")
#define IEGFT_DIB IEGFT_BMP
#define IEGFT_JPEG IE_ImpGraphic::fileTypeForSuffix(".jpg")
#define IEGFT_WMF IE_ImpGraphic::fileTypeForSuffix(".wmf")

#endif /* IE_TYPES_H */
