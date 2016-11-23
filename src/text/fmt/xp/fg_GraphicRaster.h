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

#ifndef FG_GRAPHICRASTER_H
#define FG_GRAPHICRASTER_H

#include "fg_Graphic.h"
#include "ut_bytebuf.h"
#include "ut_types.h"

//  An implementation of the FG_Graphic interface for raster files.  The
//  internal file format happens to be PNG OR JPEG
class ABI_EXPORT FG_GraphicRaster : public FG_Graphic
{
public:
    enum Format {
        PNG_FORMAT = 0,
        JPEG_FORMAT
    };
	static FG_Graphic*	createFromChangeRecord(const fl_ContainerLayout *pFL,
											   const PX_ChangeRecord_Object* pcro);
	static FG_Graphic*	createFromStrux(const fl_ContainerLayout *pFL);

	FG_GraphicRaster();
	virtual ~FG_GraphicRaster();

	virtual FGType		getType(void) const;
    virtual const std::string & getMimeType() const;
	virtual FG_Graphic * clone(void) const;
	virtual double		getWidth(void) const;
	virtual double		getHeight(void) const;
	virtual const char * getDataId(void) const;
	virtual const char * createDataItem(PD_Document *pDoc, const char * szName) const;
	virtual GR_Image*	regenerateImage(GR_Graphics* pG);
	virtual GR_Image*	generateImage(GR_Graphics* pG,
									  const PP_AttrProp * pSpanAP,
									  UT_sint32 maxW, UT_sint32 maxH);

	virtual UT_Error   	insertIntoDocument(PD_Document* pDoc, UT_uint32 res,
										   UT_uint32 iPos, const char* szName) const;
	virtual UT_Error   	insertAtStrux(PD_Document* pDoc,
									  UT_uint32 res,
									  UT_uint32 iPos,
									  PTStruxType iStruxType,
									  const char* szName) const;

	bool			setRaster_PNG(const UT_ConstByteBufPtr & pBB);
	bool                    setRaster_JPEG(const UT_ConstByteBufPtr & pBB);
	virtual const UT_ConstByteBufPtr &	getBuffer() const;
    Format              getFormat() const
    {
        return m_format;
    }

	virtual const char * getWidthProp(void);
	virtual const char * getHeightProp(void);

protected:
    Format            m_format;
	UT_ConstByteBufPtr m_pbb;

	UT_sint32 m_iWidth, m_iHeight;
	UT_sint32 m_iMaxW;
	UT_sint32 m_iMaxH;
	const PP_AttrProp* m_pSpanAP;
	const gchar* m_pszDataID;
};

typedef std::unique_ptr<FG_GraphicRaster> FG_GraphicRasterPtr;
// no Const since we use FG_ConstGraphicPtr.

#endif /* FG_GRAPHICRASTER_H */
