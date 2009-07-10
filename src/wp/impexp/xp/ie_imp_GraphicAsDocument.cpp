/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_GraphicAsDocument.h"
#include "pd_Document.h"
#include "ie_impGraphic.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"

/*****************************************************************/
/*****************************************************************/

/*
 * The user tried to open an image directly, so we'll create an
 * empty document and put the graphic in that.
 */

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Imp_GraphicAsDocument::_loadFile(GsfInput * input)
{
	UT_Error error;

   	UT_DEBUGMSG(("trying to open an image as a document...\n"));
   
	if (!getDoc()->appendStrux(PTX_Section, NULL) ||
	    !getDoc()->appendStrux(PTX_Block, NULL))
     		return UT_IE_NOMEMORY;
   
   	FG_Graphic* pFG;
   	error = m_pGraphicImporter->importGraphic(input, &pFG);
   	DELETEP(m_pGraphicImporter);

   	if (error != UT_OK) return error;
   
   	const UT_ByteBuf * buf;
    buf = pFG->getBuffer();

   	const gchar* propsArray[3];
   	propsArray[0] = "dataid";
   	propsArray[1] = "image_0";
   	propsArray[2] = NULL;
   
   	if (!getDoc()->appendObject(PTO_Image, propsArray)) {
	   delete pFG;
	   return UT_IE_NOMEMORY;
	}

   	if (!getDoc()->createDataItem("image_0", false,
					buf, pFG->getMimeType(), NULL)) {
	   delete pFG;
	   return UT_IE_NOMEMORY;
	}

   	delete pFG;

	return UT_OK;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_GraphicAsDocument::~IE_Imp_GraphicAsDocument()
{
   	DELETEP(m_pGraphicImporter);
}

IE_Imp_GraphicAsDocument::IE_Imp_GraphicAsDocument(PD_Document * pDocument)
  : IE_Imp(pDocument), m_pGraphicImporter(0)
{
}
