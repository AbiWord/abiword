/* AbiWord -- Embedded graphics for layout
 * Copyright (C) 1999 Matt Kimball
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef FG_GRAPHICVECTOR_H
#define FG_GRAPHICVECTOR_H

#include "fg_Graphic.h"
#include "ut_types.h"

class UT_ByteBuf;

//  An implementation of the FG_Graphic interface for vector files.  The
//  internal file format happens to be SVG.
class ABI_EXPORT FG_GraphicVector : public FG_Graphic
{
public:
	static FG_Graphic*	createFromChangeRecord(const fl_ContainerLayout *pFL,
											   const PX_ChangeRecord_Object* pcro);
	static FG_Graphic*	createFromStrux(const fl_ContainerLayout *pFL);

	FG_GraphicVector();
	virtual ~FG_GraphicVector();

	virtual FGType		getType(void) const;
    virtual const std::string & getMimeType(void) const;
	virtual FG_Graphic * clone(void) const;
	virtual double		getWidth(void) const;
	virtual double		getHeight(void) const;
	virtual const char * getDataId(void) const { return m_pszDataID;}
	virtual const char * createDataItem(PD_Document *pDoc, const char * szName) const;
	virtual GR_Image*	generateImage(GR_Graphics* pG,
									  const PP_AttrProp * pSpanAP,
									  UT_sint32 maxW, UT_sint32 maxH);
	virtual GR_Image*	regenerateImage(GR_Graphics* pG);

	virtual UT_Error   	insertIntoDocument(PD_Document* pDoc, UT_uint32 res,
										   UT_uint32 iPos, const char* szName) const;

	virtual UT_Error   	insertAtStrux(PD_Document* pDoc,
									  UT_uint32 res,
									  UT_uint32 iPos,
									  PTStruxType iStruxType,
									  const char* szName) const;

	bool				setVector_SVG(const UT_ConstByteBufPtr & pBB);
	const UT_ConstByteBufPtr & getBuffer(void) const;

	virtual const char * getWidthProp(void);
	virtual const char * getHeightProp(void);

protected:
	UT_ConstByteBufPtr m_pbbSVG;

	UT_sint32 m_iWidth, m_iHeight;
	UT_sint32 m_iMaxW;
	UT_sint32 m_iMaxH;
	const PP_AttrProp* m_pSpanAP;
	const gchar* m_pszDataID;
};

typedef std::unique_ptr<FG_GraphicVector> FG_GraphicVectorPtr;
// no Const since we use FG_ConstGraphicPtr.

#endif /* FG_GRAPHICVECTOR_H */
