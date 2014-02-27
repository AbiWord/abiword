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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef GR_IMAGE_H
#define GR_IMAGE_H

#include <string>
#include "ut_types.h"
#include "ut_string_class.h"
#include "ut_misc.h"
#include "ut_vector.h"

class GR_Graphics;
class UT_ByteBuf;

class ABI_EXPORT GR_Image_Point
{
public:
  GR_Image_Point(void) : m_iX(0),m_iY(0)
    {}
  UT_sint32 m_iX;
  UT_sint32 m_iY;
};

class ABI_EXPORT GR_Image
{
public:
	GR_Image();
	virtual ~GR_Image();

	virtual UT_sint32	getDisplayWidth(void) const;
	virtual UT_sint32	getDisplayHeight(void) const;

   	virtual bool		convertToBuffer(UT_ByteBuf** ppBB) const = 0;
	virtual bool		convertFromBuffer(const UT_ByteBuf* pBB, const std::string& mimetype, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) = 0;
	/*!
	 * This should be overridden by platform code. The idea is to create a
	 * new image from the rectangular segment in device units defined by
	 * UT_Rect rec. The Image should be deleted by the calling routine.
	 */
    virtual GR_Image *  createImageSegment(GR_Graphics * pG, const UT_Rect & rec) = 0;
	virtual void        scaleImageTo(GR_Graphics * pG, const UT_Rect & rec);
	void				getName(char* szName) const;
	void getName ( std::string & name) const;
	void getName ( UT_String & copy ) const;
	/*!
	 * Returns true if the image has any alpha in it.
	 */
	virtual bool hasAlpha(void) const = 0;
	/*!
	 * Returns true if pixel at point (x,y) in device units is transparent.
	 */
	virtual bool isTransparentAt(UT_sint32 x, UT_sint32 y) = 0;
	void         GenerateOutline(void);
	void         DestroyOutline(void);
	UT_sint32 GetOffsetFromLeft(GR_Graphics * pG, UT_sint32 pad, UT_sint32 yTop, UT_sint32 height);
	UT_sint32 GetOffsetFromRight(GR_Graphics * pG, UT_sint32 pad, UT_sint32 yTop, UT_sint32 height);

	bool isOutLinePresent(void) const
	  { return (m_vecOutLine.getItemCount() > 0);}
   	enum GRType {
	   GRT_Unknown,
	   GRT_Raster,
	   GRT_Vector
	};

	static GRType		getBufferType(const UT_ByteBuf* pBB);
   	virtual GRType		getType() const;
   	virtual bool		render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

   	void setDisplaySize(UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

protected:
	void setName ( const char * szName );
	void setName ( const UT_String & szName );

private:
   	std::string 		m_szName;
	UT_sint32			m_iDisplayWidth;
	UT_sint32			m_iDisplayHeight;
	UT_GenericVector<GR_Image_Point *> m_vecOutLine;
};

class ABI_EXPORT GR_RasterImage : public GR_Image
{
public:
	virtual bool hasAlpha(void) const = 0;
	virtual bool isTransparentAt(UT_sint32 x, UT_sint32 y) = 0;
   	virtual GRType		getType() const { return GRT_Raster; }
};

class ABI_EXPORT GR_ImageFactory
{
public:
	virtual ~GR_ImageFactory() {}

   	virtual GR_Image*	createNewImage(const char* pszName, GR_Image::GRType iType = GR_Image::GRT_Raster) = 0;
};

#include "gr_VectorImage.h"

#endif /* GR_IMAGE */
