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

#include <string.h>

#include "ut_assert.h"
#include "ut_png.h"
#include "ut_bytebuf.h"
#include "fl_Layout.h"
#include "px_CR_Object.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"

#include "fg_GraphicRaster.h"

FG_Graphic* FG_GraphicRaster::createFromChangeRecord(const fl_Layout* pFL, 
											const PX_ChangeRecord_Object* pcro)
{
	FG_GraphicRaster* pFG = new FG_GraphicRaster();

	UT_Bool bFoundDataItem = UT_FALSE;
	const PD_Document* pDoc = pFL->getDocument();
	PT_BlockOffset blockOffset = pcro->getBlockOffset();

	/*
	  Get the attribute list for this offset, lookup the dataid
	  for the image, and get the dataItem.  The bytes in the
	  dataItem should be a PNG image.
	*/
	UT_Bool bFoundSpanAP = pFL->getSpanAttrProp(blockOffset, UT_FALSE,
												&pFG->m_pSpanAP);
	if (bFoundSpanAP && pFG->m_pSpanAP)
	{
		UT_Bool bFoundDataID = pFG->m_pSpanAP->getAttribute("dataid", pFG->m_pszDataID);
		if (bFoundDataID && pFG->m_pszDataID)
		{
			bFoundDataItem = pDoc->getDataItemDataByName(pFG->m_pszDataID, (const UT_ByteBuf **)&pFG->m_pbbPNG, NULL, NULL);
		}
	}

	if (!bFoundDataItem)
		DELETEP(pFG);

	return pFG;
}


FG_GraphicRaster::FG_GraphicRaster()
{
	m_pbbPNG = NULL;
	m_bOwnPNG = UT_FALSE;
	m_pSpanAP = NULL;
	m_pszDataID = NULL;
}

FG_GraphicRaster::~FG_GraphicRaster()
{
	if (m_bOwnPNG)
		DELETEP(m_pbbPNG);
	else
		m_pbbPNG = NULL;
}

FGType FG_GraphicRaster::getType(void)
{
	return FGT_Raster;
}

double FG_GraphicRaster::getWidth(void)
{
	UT_ASSERT(m_pbbPNG);

	return m_iWidth / 72.0;
}

double FG_GraphicRaster::getHeight(void)
{
	UT_ASSERT(m_pbbPNG);

	return m_iHeight / 72.0;
}

//
//  We will generate an image at the proper resolution for display in the
//  graphics object we are given.
//
GR_Image* FG_GraphicRaster::generateImage(GR_Graphics* pG)
{
	UT_ASSERT(m_pSpanAP);
	UT_ASSERT(m_pszDataID);

	/*
	  We need to know the display size of the new image.
	*/

	const XML_Char *pszWidth;
	const XML_Char *pszHeight;
	UT_Bool bFoundWidthProperty = m_pSpanAP->getProperty("width", pszWidth);
	UT_Bool bFoundHeightProperty = m_pSpanAP->getProperty("height", pszHeight);

	UT_sint32 iDisplayWidth = 0;
	UT_sint32 iDisplayHeight = 0;
	UT_sint32 iLayoutWidth = 0;
	UT_sint32 iLayoutHeight = 0;
	if (bFoundWidthProperty && bFoundHeightProperty && pszWidth && pszHeight && pszWidth[0] && pszHeight[0])
	{
		iDisplayWidth = pG->convertDimension(pszWidth);
		iDisplayHeight = pG->convertDimension(pszHeight);
		iLayoutWidth = UT_convertToLayoutUnits(pszWidth);
		iLayoutHeight = UT_convertToLayoutUnits(pszHeight);
	}
	else
	{
		UT_sint32 iImageWidth;
		UT_sint32 iImageHeight;
			
		UT_PNG_getDimensions(m_pbbPNG, iImageWidth, iImageHeight);
			
#if 0					
		if (pG->queryProperties(GR_Graphics::DGP_SCREEN))
		{
			iDisplayWidth = iImageWidth;
			iDisplayHeight = iImageHeight;
		}
		else
#endif						
		{
			double fScale = pG->getResolution() / 72.0;
			
			iDisplayWidth = (UT_sint32) (iImageWidth * fScale);
			iDisplayHeight = (UT_sint32) (iImageHeight * fScale);

			fScale = 1440.0 / 72.0;
			iLayoutWidth = (UT_sint32) (iImageWidth * fScale);
			iLayoutHeight = (UT_sint32) (iImageHeight * fScale);

		}
	}

	UT_ASSERT(iDisplayWidth > 0);
	UT_ASSERT(iDisplayHeight > 0);

	GR_Image *pImage = pG->createNewImage(m_pszDataID, m_pbbPNG, iDisplayWidth, iDisplayHeight);

	pImage->setLayoutSize(iLayoutWidth, iLayoutHeight);

	return pImage;
}

//
//  We need to be able to add a representation of ourselves to an
//  existing document.  This added representation can be used to 
//  reconstruct an equivalent FG_GraphicRaster object after this one
//  is discarded.
//
UT_ErrorCode FG_GraphicRaster::insertIntoDocument(PD_Document* pDoc, double fDPI,
										 UT_uint32 iPos, const char* szName)
{
	UT_ASSERT(pDoc);
	UT_ASSERT(szName);

	/*
	  Create the data item
	*/
	pDoc->createDataItem(szName, UT_FALSE, m_pbbPNG, NULL, NULL);

	/*
	  Insert the object into the document.
	*/
	char szProps[256];

	strcpy(szProps,"width:");
	strcat(szProps,UT_convertToDimensionString(DIM_IN, (double)m_iWidth/fDPI, "3.2"));
	strcat(szProps,"; height:");
	strcat(szProps,UT_convertToDimensionString(DIM_IN, (double)m_iHeight/fDPI, "3.2"));

	const XML_Char*	attributes[] = {
		"dataid", szName,
		"PROPS", szProps,
		NULL, NULL
	};

	pDoc->insertObject(iPos, PTO_Image, attributes, NULL);

	// TODO: better error checking in this function
	return UT_OK;
}

UT_Bool FG_GraphicRaster::setRaster_PNG(UT_ByteBuf* pBB)
{
	if (m_bOwnPNG)
		DELETEP(m_pbbPNG);

	m_pbbPNG = pBB;
	m_bOwnPNG = UT_TRUE;

	//  We want to calculate the dimensions of the image here.
	UT_PNG_getDimensions(pBB, m_iWidth, m_iHeight);

	return UT_TRUE;
}

UT_ByteBuf* FG_GraphicRaster::getRaster_PNG(void)
{
	UT_ASSERT(m_pbbPNG);

	return m_pbbPNG;
}
