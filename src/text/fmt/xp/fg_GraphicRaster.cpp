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
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "fl_Layout.h"
#include "px_CR_Object.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "ut_debugmsg.h"
#include "fg_GraphicRaster.h"

#include "ut_string_class.h"

FG_Graphic* FG_GraphicRaster::createFromChangeRecord(const fl_Layout* pFL,
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
	bool bFoundSpanAP = pFL->getSpanAttrProp(blockOffset, false,
											 &pFG->m_pSpanAP);
	if (bFoundSpanAP && pFG->m_pSpanAP)
	{
		bool bFoundDataID = pFG->m_pSpanAP->getAttribute("dataid", pFG->m_pszDataID);
		if (bFoundDataID && pFG->m_pszDataID)
		{
			bFoundDataItem = pDoc->getDataItemDataByName((char*)pFG->m_pszDataID, (const UT_ByteBuf **)&pFG->m_pbbPNG, NULL, NULL);
		}
	}

	if (!bFoundDataItem)
		DELETEP(pFG);

	return pFG;
}


FG_GraphicRaster::FG_GraphicRaster()
{
	m_pbbPNG = NULL;
	m_bOwnPNG = false;
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

const char * FG_GraphicRaster::getDataId(void) const
{
	return (const char *) m_pszDataID;
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

/*!
 * Return the width property of the span that contains this image.
 */
const char * FG_GraphicRaster::getWidthProp(void)
{
	const XML_Char * szWidth = NULL;
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
	const XML_Char * szHeight = NULL;
	m_pSpanAP->getProperty("height", szHeight);
	if(szHeight == NULL)
	{
		szHeight = "0in";
	}
	return szHeight;
}

//
//  We will generate an image at the proper resolution for display in the
//  graphics object we are given.
//
GR_Image* FG_GraphicRaster::generateImage(GR_Graphics* pG,const PP_AttrProp * pSpanAP)
{
	UT_ASSERT(m_pSpanAP);
	UT_ASSERT(m_pszDataID);

	/*
	  We need to know the display size of the new image.
	*/

	const XML_Char *pszWidth;
	const XML_Char *pszHeight;
	if(pSpanAP != NULL)
	{
		m_pSpanAP = pSpanAP;
	}
	bool bFoundWidthProperty = m_pSpanAP->getProperty("width", pszWidth);
	bool bFoundHeightProperty = m_pSpanAP->getProperty("height", pszHeight);
	if( bFoundWidthProperty && bFoundHeightProperty)
	{
		UT_DEBUGMSG(("SEVIOR: generate image  height= %s width = %s \n",pszHeight,pszWidth));
	}
	UT_sint32 iDisplayWidth = 0;
	UT_sint32 iDisplayHeight = 0;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iLayoutWidth = 0;
	UT_sint32 iLayoutHeight = 0;
#endif
	if (bFoundWidthProperty && bFoundHeightProperty && pszWidth && pszHeight && pszWidth[0] && pszHeight[0])
	{
		iDisplayWidth = pG->convertDimension((char*)pszWidth);
		iDisplayHeight = pG->convertDimension((char*)pszHeight);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iLayoutWidth = UT_convertToLayoutUnits(pszWidth);
		iLayoutHeight = UT_convertToLayoutUnits(pszHeight);
#endif
	}
	else
	{
		UT_sint32 iImageWidth;
		UT_sint32 iImageHeight;

		UT_PNG_getDimensions(m_pbbPNG, iImageWidth, iImageHeight);

		double fScale = pG->getResolution() / 100.0;

		iDisplayWidth = (UT_sint32) (iImageWidth * fScale);
		iDisplayHeight = (UT_sint32) (iImageHeight * fScale);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		//			fScale = 1440.0 / 72.0;
		fScale = 14.399999999999999;
		iLayoutWidth = (UT_sint32) (iImageWidth * fScale);
		iLayoutHeight = (UT_sint32) (iImageHeight * fScale);
#endif
	}

	UT_ASSERT(iDisplayWidth > 0);
	UT_ASSERT(iDisplayHeight > 0);

   	GR_Image *pImage = pG->createNewImage((char*)m_pszDataID, m_pbbPNG, iDisplayWidth, iDisplayHeight, GR_Image::GRT_Raster);

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	//TF#TODO
	pImage->setLayoutSize(iLayoutWidth, iLayoutHeight);
#endif
	return pImage;
}

//
//  We need to be able to add a representation of ourselves to an
//  existing document.  This added representation can be used to
//  reconstruct an equivalent FG_GraphicRaster object after this one
//  is discarded.
//
UT_Error FG_GraphicRaster::insertIntoDocument(PD_Document* pDoc, double fDPI,
											  UT_uint32 iPos, const char* szName)
{
	UT_ASSERT(pDoc);
	UT_ASSERT(szName);

	/*
	  Create the data item
	*/
	const char* mimetypePNG = NULL;
	mimetypePNG = UT_strdup("image/png");
   	pDoc->createDataItem(szName, false, m_pbbPNG, (void *) mimetypePNG, NULL);

	/*
	  Insert the object into the document.
	*/
	UT_String szProps;

	szProps += "width:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, (double)m_iWidth/fDPI, "3.2");
	szProps += "; height:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, (double)m_iHeight/fDPI, "3.2");

#ifndef __MRC__
	const XML_Char*	attributes[] = {
		"dataid", szName,
		PT_PROPS_ATTRIBUTE_NAME, szProps.c_str(),
	   	NULL, NULL
	};
#else
	// MrCPP does not like the above
	const XML_Char * attributes[] = {
		"dataid", NULL,
		PT_PROPS_ATTRIBUTE_NAME, NULL,
	   	NULL, NULL
	};
	attributes [1] = szName;
	attributes [3] = szProps;
#endif

	pDoc->insertObject(iPos, PTO_Image, attributes, NULL);

	// TODO: better error checking in this function
	return UT_OK;
}

bool FG_GraphicRaster::setRaster_PNG(UT_ByteBuf* pBB)
{
	if (m_bOwnPNG)
		DELETEP(m_pbbPNG);

	m_pbbPNG = pBB;
	m_bOwnPNG = true;

	//  We want to calculate the dimensions of the image here.
	return UT_PNG_getDimensions(pBB, m_iWidth, m_iHeight);
}

UT_ByteBuf* FG_GraphicRaster::getRaster_PNG(void)
{
	UT_ASSERT(m_pbbPNG);

	return m_pbbPNG;
}
