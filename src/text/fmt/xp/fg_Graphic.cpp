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


#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"

#include "ut_string.h"
#include "fl_Layout.h"
#include "px_CR_Object.h"
#include "pp_AttrProp.h"

FG_Graphic* FG_Graphic::createFromChangeRecord(const fl_Layout* pFL,
					       const PX_ChangeRecord_Object* pcro)
{
   PT_BlockOffset blockOffset = pcro->getBlockOffset();
   
   // Get the attribute list for this offset.
   const PP_AttrProp* pSpanAP;
   bool bFoundSpanAP = pFL->getSpanAttrProp(blockOffset, false, &pSpanAP);
   if (bFoundSpanAP && pSpanAP)
   {
      const XML_Char *pszDataID;
      bool bFoundDataID = pSpanAP->getAttribute((XML_Char*)"dataid", pszDataID);
      
      if (bFoundDataID && pszDataID)
      {
	   char * pszMimeType = NULL;
	   bFoundDataID = pFL->getDocument()->getDataItemDataByName((char*)pszDataID, NULL, (void**)&pszMimeType, NULL);
	   
	   // figure out what type to create
	   
	   if (!bFoundDataID || !pszMimeType || UT_strcmp(pszMimeType, "image/svg-xml") != 0) {
	      return FG_GraphicRaster::createFromChangeRecord(pFL, pcro);
	   } else {
	      return FG_GraphicVector::createFromChangeRecord(pFL, pcro);
	   }
      }
   }
   return NULL;
}

FG_Graphic::~FG_Graphic() {
	// do nothing for now.
}

