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

class GR_Graphics;

class GR_VectorImage : public GR_Image
{
public:
	GR_VectorImage(const char* szName);
	virtual ~GR_VectorImage();
	
   	virtual void		setDisplaySize(UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
	
   	virtual UT_Bool		convertToBuffer(UT_ByteBuf** ppBB) const;
	virtual UT_Bool		convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

   	virtual GRType		getType() { return GRT_Vector; }
   	virtual UT_Bool		render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

   	void _startElement(const XML_Char* name, const XML_Char **atts);
   	void _endElement(const XML_Char* name);
   	void _charData(const XML_Char* text, int len);
   
protected:

   	UT_Bool m_status;
   	UT_Stack *m_context;
	UT_Vector m_elements;  
   
};

#endif /* GR_VECTORIMAGE */
