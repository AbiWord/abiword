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

#ifndef GR_IMAGE_H
#define GR_IMAGE_H

#include "ut_types.h"

#define	GR_IMAGE_MAX_NAME_LEN	63

class UT_ByteBuf;

class GR_Image
{
public:
	GR_Image();
	virtual ~GR_Image();
	
	virtual UT_sint32	getDisplayWidth(void) const = 0;
	virtual UT_sint32	getDisplayHeight(void) const = 0;


	virtual UT_Bool		convertToPNG(UT_ByteBuf** ppBB) const = 0;
	virtual UT_Bool		convertFromPNG(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) = 0;

	void				getName(char* szName) const;

	void				setLayoutSize(UT_sint32 iLayoutWidth, UT_sint32 iLayoutHeight);
	UT_sint32			getLayoutWidth(void) const { return m_iLayoutWidth;}
	UT_sint32			getLayoutHeight(void) const	{ return m_iLayoutHeight;}
								
		
	
protected:
	char				m_szName[GR_IMAGE_MAX_NAME_LEN+1];
	UT_sint32			m_iLayoutWidth;
	UT_sint32			m_iLayoutHeight;
};

class GR_StretchableImage : public GR_Image
{
public:
	GR_StretchableImage();
	virtual ~GR_StretchableImage();

	void				setDisplaySize(UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
	
	virtual UT_sint32	getDisplayWidth(void) const { return m_iDisplayWidth; }
	virtual UT_sint32	getDisplayHeight(void) const { return m_iDisplayHeight; }

protected:
	UT_sint32			m_iDisplayWidth;
	UT_sint32			m_iDisplayHeight;
};

class GR_ImageFactory
{
public:
	virtual GR_Image*	createNewImage(const char* pszName) = 0;
};

#endif /* GR_IMAGE */
