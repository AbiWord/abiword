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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */


#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"

#include "ut_string.h"
#include "pp_AttrProp.h"
#include "px_CR_Object.h"
#include "gr_Graphics.h"
#include "gr_Image.h"
#include "fl_ContainerLayout.h"

FG_GraphicPtr FG_Graphic::createFromChangeRecord(const fl_ContainerLayout* pFL,
											   const PX_ChangeRecord_Object* pcro)
{
	PT_BlockOffset blockOffset = pcro->getBlockOffset();

	// Get the attribute list for this offset.
	const PP_AttrProp* pSpanAP = NULL;
	pFL->getSpanAP(blockOffset, false, pSpanAP);
	if (pSpanAP)
	{
		const gchar *pszDataID;
		bool bFoundDataID = pSpanAP->getAttribute("dataid", pszDataID);

		if (bFoundDataID && pszDataID)
		{
			std::string mimeType;
			UT_ConstByteBufPtr bb;
			bFoundDataID = pFL->getDocument()->getDataItemDataByName(pszDataID, bb,
                                                                                 &mimeType,
                                                                                 NULL);

			// figure out what type to create

			if (!bFoundDataID || mimeType.empty()
				|| (mimeType != "image/svg+xml")) {
				return FG_GraphicRaster::createFromChangeRecord(pFL, pcro);
			} else {
				return FG_GraphicVector::createFromChangeRecord(pFL, pcro);
			}
		}
	}
	return FG_GraphicPtr();
}

FG_GraphicPtr FG_Graphic::createFromStrux(const fl_ContainerLayout* pFL)
{
   
	// Get the attribute list for this offset.
	const PP_AttrProp* pSpanAP = NULL;
	pFL->getAP(pSpanAP);
	if (pSpanAP)
	{
		const gchar *pszDataID;
		bool bFoundDataID = pSpanAP->getAttribute("strux-image-dataid", pszDataID);
      
		if (bFoundDataID && pszDataID)
		{
			std::string mimeType;
			UT_ConstByteBufPtr bb;
			bFoundDataID = pFL->getDocument()->getDataItemDataByName(pszDataID, bb,
                                                                                 &mimeType,
                                                                                 NULL);
	   
			// figure out what type to create
	   
			if (!bFoundDataID || mimeType.empty() ||
				(mimeType != "image/svg+xml"))
			{
				return FG_GraphicRaster::createFromStrux(pFL);
			} else {
				return FG_GraphicVector::createFromStrux(pFL);
			}
		}
	}
	return FG_GraphicPtr();
}

FG_Graphic::~FG_Graphic() {
	// do nothing for now.
}



