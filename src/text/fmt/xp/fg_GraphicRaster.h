/* AbiWord -- Embedded graphics for layout
 * Copyright (C) 1999 Matt Kimball
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

#ifndef FG_GRAPHICRASTER_H
#define FG_GRAPHICRASTER_H

#include "fg_Graphic.h"
#include "ut_bytebuf.h"

//  An implementation of the FG_Graphic interface for raster files.  The
//  internal file format happens to be PNG.
class FG_GraphicRaster : public FG_Graphic
{
public:
	static FG_Graphic*	createFromChangeRecord(const fl_Layout *pFL, 
										const PX_ChangeRecord_Object* pcro);

	FG_GraphicRaster();
	virtual ~FG_GraphicRaster();

	virtual FGType		getType(void);

	virtual double		getWidth(void);
	virtual double		getHeight(void);

	virtual GR_Image*	generateImage(GR_Graphics* pG);

	virtual UT_ErrorCode   	insertIntoDocument(PD_Document* pDoc, double fDPI,
										   UT_uint32 iPos, const char* szName);

	UT_Bool				setRaster_PNG(UT_ByteBuf* pBB);
	UT_ByteBuf*			getRaster_PNG(void);

protected:
	UT_ByteBuf* m_pbbPNG;
	UT_Bool m_bOwnPNG;

	UT_sint32 m_iWidth, m_iHeight;
	const PP_AttrProp* m_pSpanAP;
	const XML_Char* m_pszDataID;
};

#endif /* FG_GRAPHICRASTER_H */
