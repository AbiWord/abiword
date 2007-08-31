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
#include <stdlib.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_svg.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "fl_ContainerLayout.h"
#include "px_CR_Object.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"

#include "fg_GraphicVector.h"
#include "ut_string_class.h"

FG_Graphic* FG_GraphicVector::createFromChangeRecord(const fl_ContainerLayout* pFL,
													 const PX_ChangeRecord_Object* pcro)
{
	FG_GraphicVector* pFG = new FG_GraphicVector();

	bool bFoundDataItem = false;
	const PD_Document* pDoc = pFL->getDocument();
	PT_BlockOffset blockOffset = pcro->getBlockOffset();

	/*
	  Get the attribute list for this offset, lookup the dataid
	  for the image, and get the dataItem.  The bytes in the
	  dataItem should be a SVG image.
	*/
	pFL->getSpanAP(blockOffset, false, pFG->m_pSpanAP);
	if (pFG->m_pSpanAP)
	{
		bool bFoundDataID = pFG->m_pSpanAP->getAttribute("dataid", pFG->m_pszDataID);
		if (bFoundDataID && pFG->m_pszDataID)
		{
			bFoundDataItem = pDoc->getDataItemDataByName(static_cast<const char*>(pFG->m_pszDataID), &pFG->m_pbbSVG, NULL, NULL);
		}
	}

	if (!bFoundDataItem)
		DELETEP(pFG);

	return pFG;
}


FG_Graphic* FG_GraphicVector::createFromStrux(const fl_ContainerLayout *pFL)
{
	FG_GraphicVector* pFG = new FG_GraphicVector();

	bool bFoundDataItem = false;
	const PD_Document* pDoc = pFL->getDocument();
	/*
	  Get the attribute list for this offset, lookup the dataid
	  for the image, and get the dataItem.  The bytes in the
	  dataItem should be a SVG image.
	*/
	pFL->getAP(pFG->m_pSpanAP);
	
	if (pFG->m_pSpanAP)
	{
		bool bFoundDataID = pFG->m_pSpanAP->getAttribute(PT_STRUX_IMAGE_DATAID, pFG->m_pszDataID);
		if (bFoundDataID && pFG->m_pszDataID)
		{
			bFoundDataItem = pDoc->getDataItemDataByName(static_cast<const char*>(pFG->m_pszDataID), &pFG->m_pbbSVG, NULL, NULL);
		}
	}

	if (!bFoundDataItem)
		DELETEP(pFG);

	return pFG;
}


FG_GraphicVector::FG_GraphicVector()
{
	m_pbbSVG = NULL;
	m_bOwnSVG = false;
	m_pSpanAP = NULL;
	m_pszDataID = NULL;
}

FG_GraphicVector::~FG_GraphicVector()
{
	if (m_bOwnSVG)
		DELETEP(m_pbbSVG);
	else
		m_pbbSVG = NULL;
}

FG_Graphic * FG_GraphicVector::clone(void)
{
	FG_GraphicVector * pClone = new FG_GraphicVector();
	pClone->m_pbbSVG = m_pbbSVG;
	pClone->m_pSpanAP = m_pSpanAP;
	pClone->m_pszDataID = m_pszDataID;
	pClone->m_iWidth = m_iWidth; 
	pClone->m_iHeight = m_iHeight;
	pClone->m_iMaxW = m_iMaxW;
	pClone->m_iMaxH = m_iMaxH;
	return static_cast<FG_Graphic *>(pClone);
}

FGType FG_GraphicVector::getType(void)
{
	return FGT_Vector;
}

double FG_GraphicVector::getWidth(void)
{
	UT_ASSERT(m_pbbSVG);

	return m_iWidth / 72.0;
}

double FG_GraphicVector::getHeight(void)
{
	UT_ASSERT(m_pbbSVG);

	return m_iHeight / 72.0;
}


/*!
 * Return the width property of the span that contains this image.
 */
const char * FG_GraphicVector::getWidthProp(void)
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
const char * FG_GraphicVector::getHeightProp(void)
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
GR_Image * FG_GraphicVector::regenerateImage(GR_Graphics * pG)
{
	return generateImage(pG,m_pSpanAP,m_iMaxW,m_iMaxH);
}

//
//  We will generate an image at the proper resolution for display in the
//  graphics object we are given.
//
GR_Image* FG_GraphicVector::generateImage(GR_Graphics* pG,
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
	if (bFoundWidthProperty && bFoundHeightProperty && pszWidth && pszHeight && pszWidth[0] && pszHeight[0])
	{
		iDisplayWidth = UT_convertToLogicalUnits(static_cast<const char*>(pszWidth));
		iDisplayHeight = UT_convertToLogicalUnits(static_cast<const char*>(pszHeight));
	}
	else
	{
		UT_sint32 iLogicalWidth, iLogicalHeight; // throwaway
		UT_SVG_getDimensions(m_pbbSVG, pG, iDisplayWidth, iDisplayHeight, iLogicalWidth, iLogicalHeight);
	}
	UT_DEBUGMSG(("SVG image: width = %d, height = %d\n",static_cast<int>(iDisplayWidth),static_cast<int>(iDisplayHeight)));
	UT_ASSERT(iDisplayWidth > 0);
	UT_ASSERT(iDisplayHeight > 0);

	if (maxW != 0 && iDisplayWidth > maxW) iDisplayWidth = maxW;
	if (maxH != 0 && iDisplayHeight > maxH) iDisplayHeight = maxH;
	m_iMaxH = maxH;
	m_iMaxW = maxW;
	GR_Image *pImage = pG->createNewImage(static_cast<const char*>(m_pszDataID), m_pbbSVG, iDisplayWidth, iDisplayHeight, GR_Image::GRT_Vector);
	return pImage;
}


const char *  FG_GraphicVector::createDataItem(PD_Document *pDoc, const char * szName)
{
	UT_return_val_if_fail(pDoc,NULL);
	UT_ASSERT(szName);
	char * mimetype = g_strdup("image/svg+xml");
   	pDoc->createDataItem(szName, false, m_pbbSVG, mimetype, NULL);
	return szName;
}

//
//  We need to be able to add a representation of ourselves to an
//  existing document.  This added representation can be used to
//  reconstruct an equivalent FG_GraphicVector object after this one
//  is discarded.
//
UT_Error FG_GraphicVector::insertIntoDocument(PD_Document* pDoc, UT_uint32 res,
											  UT_uint32 iPos, const char* szName)
{
	UT_return_val_if_fail(pDoc, UT_ERROR);
	UT_ASSERT(szName);

	/*
	  Create the data item
	*/
	char * mimetype = g_strdup("image/svg+xml");
   	pDoc->createDataItem(szName, false, m_pbbSVG, mimetype, NULL);

	UT_String szProps;

	szProps += "width:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_iWidth)/res, "3.2");
	szProps += "; height:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_iHeight)/res, "3.2");

	const gchar*	attributes[] = {
		"dataid", szName,
		PT_PROPS_ATTRIBUTE_NAME, szProps.c_str(),
		NULL, NULL
	};


	pDoc->insertObject(iPos, PTO_Image, attributes, NULL);

	// TODO: better error checking in this function
	return UT_OK;
}

/*!
 * Insert an image at the strux given. This will become the
 * background image for the container defined by the strux.
 */
UT_Error FG_GraphicVector::insertAtStrux(PD_Document* pDoc, 
										 UT_uint32 res,
										 UT_uint32 iPos,
										 PTStruxType iStruxType,
										 const char* szName)
{
	UT_return_val_if_fail(pDoc, UT_ERROR);
	UT_ASSERT(szName);

	/*
	  Create the data item
	*/
	char * mimetype = g_strdup("image/svg+xml");
   	pDoc->createDataItem(szName, false, m_pbbSVG, mimetype, NULL);


	/*
	  Insert the object into the document.
	*/
	UT_String szProps;

	szProps += "width:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_iWidth)/res, "3.2");
	szProps += "; height:";
	szProps += UT_convertInchesToDimensionString(DIM_IN, static_cast<double>(m_iHeight)/res, "3.2");

	const gchar*	attributes[] = {
		PT_STRUX_IMAGE_DATAID, szName,
		PT_PROPS_ATTRIBUTE_NAME, szProps.c_str(),
	   	NULL, NULL
	};

	pDoc->changeStruxFmt(PTC_AddFmt,iPos,iPos,attributes,NULL,iStruxType);


	// TODO: better error checking in this function
	return UT_OK;
}


bool FG_GraphicVector::setVector_SVG(const UT_ByteBuf* pBB)
{
	if (m_bOwnSVG)
		DELETEP(m_pbbSVG);

	m_pbbSVG = pBB;
	m_bOwnSVG = true;

	UT_sint32 layoutWidth;
	UT_sint32 layoutHeight;

	return UT_SVG_getDimensions(pBB, 0, m_iWidth, m_iHeight, layoutWidth, layoutHeight);
}

const UT_ByteBuf* FG_GraphicVector::getVector_SVG(void) const
{
	UT_ASSERT(m_pbbSVG);

	return m_pbbSVG;
}
