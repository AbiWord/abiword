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

#ifndef GR_VECTORIMAGE_H
#define GR_VECTORIMAGE_H

#include "ut_types.h"
#include "gr_Image.h"

#include "ut_xml.h"
#include "ut_stack.h"
#include "ut_vector.h"

class UT_svg;
class UT_SVGMatrix;
class GR_Graphics;

class ABI_EXPORT GR_VectorImage : public GR_Image
{
public:
	GR_VectorImage(const char* szName);
	virtual ~GR_VectorImage();
	
   	virtual bool		convertToBuffer(UT_ByteBuf** ppBB) const;
	virtual bool		convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

    virtual GR_Image *  createImageSegment(GR_Graphics * pG, const UT_Rect & rec)
		{ UT_ASSERT_NOT_REACHED() ; return NULL; }

   	virtual GRType		getType() const { return GRT_Vector; }
   	virtual bool		render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
	virtual bool            hasAlpha(void) const;
	virtual bool            isTransparentAt(UT_sint32 x, UT_sint32 y);


private:
   	bool m_status;
   	UT_Stack *m_context;
	UT_Vector m_elements;  
   
	UT_sint32 m_iDisplayOx;
	UT_sint32 m_iDisplayOy;

	UT_svg *m_pSVG;
	UT_ByteBuf *m_pBB_Image;
public:
	UT_sint32 getDisplayOx() const { return m_iDisplayOx; }
	UT_sint32 getDisplayOy() const { return m_iDisplayOy; }

	UT_svg *getSVG() const { return m_pSVG; }

	UT_sint32 m_iTreeLevel;
	UT_GenericVector<UT_SVGMatrix*> m_SVG_Matrix;
	UT_SVGMatrix *m_CurrentMatrix;
};

#endif /* GR_VECTORIMAGE */
