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
#include "ut_string_class.h"

class GR_Graphics;
class UT_ByteBuf;

class ABI_EXPORT GR_Image
{
public:
	GR_Image();
	virtual ~GR_Image();
   	
	virtual UT_sint32	getDisplayWidth(void) const;
	virtual UT_sint32	getDisplayHeight(void) const;

   	virtual bool		convertToBuffer(UT_ByteBuf** ppBB) const;
	virtual bool		convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

	void				getName(char* szName) const;
	void getName ( UT_String & copy ) const;
								
   	enum GRType {
	   GRT_Unknown,
	   GRT_Raster,
	   GRT_Vector
	};
   
	static GRType		getBufferType(const UT_ByteBuf* pBB);
   	virtual GRType		getType() const;
   	virtual bool		render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

 protected:
	void setName ( const char * szName );
	void setName ( const UT_String & szName );

   	void setDisplaySize(UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

private:
   	UT_String		m_szName;
	UT_sint32			m_iDisplayWidth;
	UT_sint32			m_iDisplayHeight;
};

class ABI_EXPORT GR_RasterImage : public GR_Image
{
public:
   	virtual GRType		getType() const { return GRT_Raster; }
};

class ABI_EXPORT GR_ImageFactory
{
public:
   	virtual GR_Image*	createNewImage(const char* pszName, GR_Image::GRType iType = GR_Image::GRT_Raster) = 0;
};

#include "gr_VectorImage.h"

#endif /* GR_IMAGE */
