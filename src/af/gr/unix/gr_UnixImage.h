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

#ifndef GR_UNIXIMAGE_H
#define GR_UNIXIMAGE_H

#include "gr_Image.h"

class GR_UnixImage : public GR_Image
{
public:
	// Shaw, I assume we'll need more args to this constructor, like a GdkImage
	GR_UnixImage(const char* pszName);
	~GR_UnixImage();

	virtual UT_sint32	getWidth(void) const;
	virtual UT_sint32	getHeight(void) const;
	virtual UT_Bool		getByteBuf(UT_ByteBuf** ppBB) const;
	virtual UT_Bool		convertFromPNG(const UT_ByteBuf* pBB);
	
protected:

};

class GR_UnixImageFactory : public GR_ImageFactory
{
public:
	virtual GR_Image*	createNewImage(const char* pszName);
};

#endif /* GR_UNIXIMAGE_H */
