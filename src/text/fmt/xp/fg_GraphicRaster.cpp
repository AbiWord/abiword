/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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

#include <string.h>

#include <string>

#include "ut_assert.h"
#include "ut_png.h"
#include "ut_jpeg.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_bytebuf.h"
#include "fl_ContainerLayout.h"
#include "px_CR_Object.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "fg_GraphicRaster.h"


FG_Graphic* FG_GraphicRaster::createFromChangeRecord(const fl_ContainerLayout* pFL,
													 const PX_ChangeRecord_Object* pcro)
{
	FG_GraphicRaster* pFG = new FG_GraphicRaster();

	bool bFoundDataItem = false;
	const PD_Document* pDoc = pFL->getDocument();
	PT_BlockOffset blockOffset = pcro->getBlockOffset();

	/*
	  Get the attribute list for this offset, lookup the dataid
	  for the image, and get the dataItem.  The bytes in the
	  dataItem should be a PNG image.
	*/
	pFL->getSpanAP(blockOffset, false, pFG->m_pSpanAP);
	
	if (pFG->m_pSpanAP)
	{
		bool bFoundDataID = pFG->m_pSpanAP->getAttribute("dataid", pFG->m_pszDataID);
		if (bFoundDataID && pFG->m_pszDataID)
		{
			std::string mime_type;
			bFoundDataItem = pDoc->getDataItemDataByName(pFG->m_pszDataID,
                                                         pFG->m_pbb,
                                                         &mime_type, NULL);
            if(bFoundDataItem) 
            {
                if(mime_type == "image/jpeg") 
                {
                    pFG->m_format = JPEG_FORMAT;
                }
            }
		}
	}

	if (!bFoundDataItem)
		DELETEP(pFG);

	return pFG;
}


FG_Graphic* FG_GraphicRaster::createFromStrux(const fl_ContainerLayout* pFL)
{
	FG_GraphicRaster* pFG = new FG_GraphicRaster();

	bool bFoundDataItem = false;
	const PD_Document* pDoc = pFL->getDocument();
	/*
	  Get the attribute list for this offset, lookup the dataid
	  for the image, and get the dataItem.  The bytes in the
	  dataItem should be a PNG image.
	*/
	pFL->getAP(pFG->m_pSpanAP);
	
	if (pFG->m_pSpanAP)
	{
		bool bFoundDataID = pFG->m_pSpanAP->getAttribute(PT_STRUX_IMAGE_DATAID, pFG->m_pszDataID);
		if (bFoundDataID && pFG->m_pszDataID)
		{
			std::string mime_type;
			bFoundDataItem = pDoc->getDataItemDataByName(pFG->m_pszDataID, 
                                                         pFG->m_pbb,
                                                         &mime_type, NULL);
            if(bFoundDataItem) 
            {
                if(mime_type == "image/jpeg") 
                {
                    pFG->m_format = JPEG_FORMAT;
                }
            }

		}
		pFG->m_iWidth = UT_convertToPoints(pFG->getWidthProp());
		pFG->m_iHeight = UT_convertToPoints(pFG->getHeightProp());
	}

	if (!bFoundDataItem)
		DELETEP(pFG);

	return pFG;
}


FG_GraphicRaster::FG_GraphicRaster()
    : m_format(PNG_FORMAT)
    , m_pbb(NULL)
	, m_iWidth(0)
	, m_iHeight(0)
	, m_iMaxW(0)
	, m_iMaxH(0)
	, m_pSpanAP(NULL)
	, m_pszDataID(NULL)
{
	xxx_UT_DEBUGMSG(("GraphRaster created %x \n",this));
}

FG_GraphicRaster::~FG_GraphicRaster()
{
	xxx_UT_DEBUGMSG(("GraphRaster Deleted %x \n",this));
	
}

FG_Graphic * FG_GraphicRaster::clone(void) const
{
	FG_GraphicRaster * pClone = new FG_GraphicRaster();
    pClone->m_format = m_format;
	pClone->m_pbb = m_pbb;
	pClone->m_pSpanAP = m_pSpanAP;
	pClone->m_pszDataID = m_pszDataID;
	pClone->m_iWidth = m_iWidth; 
	pClone->m_iHeight = m_iHeight;
	pClone->m_iMaxW = m_iMaxW;
	pClone->m_iMaxH = m_iMaxH;
	return pClone;
}

FGType FG_GraphicRaster::getType(void) const
{
	return FGT_Raster;
}

const UT_ConstByteBufPtr& FG_GraphicRaster::getBuffer() const
{
    return m_pbb;
}


const char * FG_GraphicRaster::getDataId(void) const
{
	return m_pszDataID;
}

double FG_GraphicRaster::getWidth(void) const
{
	UT_ASSERT(m_pbb);

	return m_iWidth;
}

double FG_GraphicRaster::getHeight(void) const
{
	UT_ASSERT(m_pbb);

	return m_iHeight;
}

/*!
 * Return the width property of the span that contains this image.
 */
const char * FG_GraphicRaster::getWidthProp(void)
{
	const gchar * szWidth = NULL;
	m_pSpanAP->getProperty("width", szWidth);
	if(szWidth == NULL)
	{
		szWidth = "0in";
	}
	return szWidth;
}


/*!
 * Return the Height property of the span that contains this image.
 */
const char * FG_GraphicRaster::getHeightProp(void)
{
	const gchar * szHeight = NULL;
	m_pSpanAP->getProperty("height", szHeight);
	if(szHeight == NULL)
	{
		szHeight = "0in";
	}
	return szHeight;
}

/*!
 * If either the graphics class or graphics resolution changes, regenerate
 * the image.
 */
GR_Image * FG_GraphicRaster::regenerateImage(GR_Graphics * pG)
{
	return generateImage(pG,m_pSpanAP,m_iMaxW,m_iMaxH);
}

//
//  We will generate an image at the proper resolution for display in the
//  graphics object we are given.
//
GR_Image* FG_GraphicRaster::generateImage(GR_Graphics* pG,
										  const PP_AttrProp * pSpanAP,
										  UT_sint32 maxW, UT_sint32 maxH)
{
	UT_ASSERT(m_pSpanAP);
	UT_ASSERT(m_pszDataID);

	/*
	  We need to know the display size of the new image.
	*/

	const gchar *pszWidth;
	const gchar *pszHeight;
	if(pSpanAP != NULL)
	{
		m_pSpanAP = pSpanAP;
	}
	bool bFoundWidthProperty = m_pSpanAP->getProperty("width", pszWidth);
	bool bFoundHeightProperty = m_pSpanAP->getProperty("height", pszHeight);

	UT_sint32 iDisplayWidth = 0;
	UT_sint32 iDisplayHeight = 0;
	// try frame-width, frame-height
	if(!bFoundWidthProperty || !bFoundHeightProperty)
	{
	     bFoundWidthProperty = m_pSpanAP->getProperty("frame-width", pszWidth);
	     bFoundHeightProperty = m_pSpanAP->getProperty("frame-height", pszHeight);
	}
	if (bFoundWidthProperty && bFoundHeightProperty && pszWidth && pszHeight && pszWidth[0] && pszHeight[0])
	{
		iDisplayWidth = UT_convertToLogicalUnits(static_cast<const char*>(pszWidth));
		iDisplayHeight = UT_convertToLogicalUnits(static_cast<const char*>(pszHeight));
	}

	if((iDisplayWidth==0) || (iDisplayHeight == 0)) 
	{
		UT_sint32 iImageWidth;
		UT_sint32 iImageHeight;

        switch(m_format) 
        {
        case PNG_FORMAT:
            UT_PNG_getDimensions(m_pbb, iImageWidth, iImageHeight);
            break;
        case JPEG_FORMAT:
            UT_JPEG_getDimensions(m_pbb, iImageWidth, iImageHeight);
            break;
        default:
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
            break;
        }

		iDisplayWidth = pG->tlu(iImageWidth);
		iDisplayHeight = pG->tlu(iImageHeight);
	}

	UT_ASSERT(iDisplayWidth > 0);
	UT_ASSERT(iDisplayHeight > 0);

	if (maxW != 0 && iDisplayWidth > maxW) {
		iDisplayHeight = iDisplayHeight * maxW / iDisplayWidth;
		iDisplayWidth = maxW;
	}
	if (maxH != 0 && iDisplayHeight > maxH) {
		iDisplayWidth = iDisplayWidth * maxH / iDisplayHeight;
		iDisplayHeight = maxH;
	}

	m_iMaxW = maxW;
	m_iMaxH = maxH;
   	GR_Image *pImage = pG->createNewImage(m_pszDataID, m_pbb, getMimeType(), iDisplayWidth, iDisplayHeight, GR_Image::GRT_Raster);

	return pImage;
}

static const std::string s_none;
static const std::string s_png_type = "image/png";
static const std::string s_jpeg_type = "image/jpeg";

const std::string & FG_GraphicRaster::getMimeType() const
{
    switch(m_format) {
    case PNG_FORMAT:
        return s_png_type;
    case JPEG_FORMAT:
        return s_jpeg_type;
    }
    return s_none;
}

//
//  We need to be able to add a representation of ourselves to an
//  existing document.  This added representation can be used to
//  reconstruct an equivalent FG_GraphicRaster object after this one
//  is discarded.
//
UT_Error FG_GraphicRaster::insertIntoDocument(PD_Document* pDoc, UT_uint32 res,
											  UT_uint32 iPos, const char* szName) const
{
	UT_return_val_if_fail(pDoc, UT_ERROR);
	UT_ASSERT(szName);

	/*
	  Create the data item
	*/
   	pDoc->createDataItem(szName, false, m_pbb, getMimeType(), NULL);

	/*
	  Insert the object into the document.
	*/
	std::string szProps;

	szProps += "width:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_iWidth)/res, "3.2");
	szProps += "; height:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_iHeight)/res, "3.2");

	const PP_PropertyVector attributes = {
		"dataid", szName,
		PT_PROPS_ATTRIBUTE_NAME, szProps
	};
	pDoc->insertObject(iPos, PTO_Image, attributes, PP_NOPROPS);

	// TODO: better error checking in this function
	return UT_OK;
}

const char *  FG_GraphicRaster::createDataItem(PD_Document *pDoc, const char * szName) const
{
	UT_return_val_if_fail(pDoc,NULL);
	UT_ASSERT(szName);

   	pDoc->createDataItem(szName, false, m_pbb, getMimeType(), NULL);
	return szName;
}

/*!
 * Insert an image at the strux given. This will become the
 * background image for the container defined by the strux.
 */
UT_Error FG_GraphicRaster::insertAtStrux(PD_Document* pDoc, 
										 UT_uint32 res,
										 UT_uint32 iPos,
										 PTStruxType iStruxType,
										 const char* szName) const
{
	UT_return_val_if_fail(pDoc, UT_ERROR);
	UT_ASSERT(szName);


	/*
	  Create the data item
	*/
   	pDoc->createDataItem(szName, false, m_pbb, getMimeType(), NULL);

	/*
	  Insert the object into the document.
	*/
	std::string szProps;

	szProps += "width:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_iWidth)/res, "3.2");
	szProps += "; height:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_iHeight)/res, "3.2");

	PP_PropertyVector attributes = {
		PT_STRUX_IMAGE_DATAID, szName,
		PT_PROPS_ATTRIBUTE_NAME, szProps,
	};

	pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, attributes, PP_NOPROPS, iStruxType);


	// TODO: better error checking in this function
	return UT_OK;
}

bool FG_GraphicRaster::setRaster_PNG(const UT_ConstByteBufPtr & pBB)
{
	m_pbb = pBB;
    m_format = PNG_FORMAT;

	//  We want to calculate the dimensions of the image here.
	return UT_PNG_getDimensions(pBB, m_iWidth, m_iHeight);
}


bool FG_GraphicRaster::setRaster_JPEG(const UT_ConstByteBufPtr & pBB)
{
	m_pbb = pBB;
    m_format = JPEG_FORMAT;

	//  We want to calculate the dimensions of the image here.
	return UT_JPEG_getDimensions(pBB, m_iWidth, m_iHeight);
}
