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
#include "ut_types.h"

//  An implementation of the FG_Graphic interface for raster files.  The
//  internal file format happens to be PNG.
class ABI_EXPORT FG_GraphicRaster : public FG_Graphic
{
public:
	static FG_Graphic*	createFromChangeRecord(const fl_ContainerLayout *pFL, 
											   const PX_ChangeRecord_Object* pcro);
	static FG_Graphic*	createFromStrux(const fl_ContainerLayout *pFL);

	FG_GraphicRaster();
	virtual ~FG_GraphicRaster();

	virtual FGType		getType(void);
	virtual FG_Graphic * clone(void);
	virtual double		getWidth(void);
	virtual double		getHeight(void);
	virtual const char * getDataId(void) const;
	virtual GR_Image*	regenerateImage(GR_Graphics* pG);
	virtual GR_Image*	generateImage(GR_Graphics* pG,
									  const PP_AttrProp * pSpanAP,
									  double maxW, double maxH);

	virtual UT_Error   	insertIntoDocument(PD_Document* pDoc, UT_uint32 res,
										   UT_uint32 iPos, const char* szName);
	virtual UT_Error   	insertAtStrux(PD_Document* pDoc, 
									  UT_uint32 res,
									  UT_uint32 iPos,
									  PTStruxType iStruxType, 
									  const char* szName);

	bool				setRaster_PNG(UT_ByteBuf* pBB);
	UT_ByteBuf*			getRaster_PNG(void);

	virtual const char * getWidthProp(void);
	virtual const char * getHeightProp(void);

protected:
	UT_ByteBuf* m_pbbPNG;
	bool m_bOwnPNG;

	UT_sint32 m_iWidth;
	UT_sint32 m_iHeight;
	double m_iMaxW;
	double m_iMaxH;
	const PP_AttrProp* m_pSpanAP;
	const XML_Char* m_pszDataID;
};

#endif /* FG_GRAPHICRASTER_H */
