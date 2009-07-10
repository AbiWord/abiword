/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiSource
 * 
 * Copyright (C) 2009 Firat Kiyak <firatkiyak@gmail.com>
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

// Class definition include
#include <OXMLi_ListenerState_Image.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_FontManager.h>
#include <OXMLi_PackageManager.h>

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>
#include <ie_impGraphic.h>
#include <fg_GraphicRaster.h>

// External includes
#include <string>

OXMLi_ListenerState_Image::OXMLi_ListenerState_Image():
	OXMLi_ListenerState()
{

}

void OXMLi_ListenerState_Image::startElement (OXMLi_StartElementRequest * rqst)
{
	if(nameMatches(rqst->pName, NS_W_KEY, "drawing"))
	{
		OXML_SharedElement imgElem(new OXML_Element_Image(""));
		rqst->stck->push(imgElem);
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_WP_KEY, "extent"))
	{
		OXML_SharedElement imgElem = rqst->stck->top();
		if(!imgElem)
			return;

		const gchar * cx = attrMatches(NS_WP_KEY, "cx", rqst->ppAtts); //width
		if(cx)
		{
			std::string width(_EmusToInches(cx));
			width += "in";
			if(imgElem->setProperty("width", width) != UT_OK)
			{
				UT_DEBUGMSG(("FRT:OpenXML importer image width property can't be set\n"));
			}
		}
	
		const gchar * cy = attrMatches(NS_WP_KEY, "cy", rqst->ppAtts); //height
		if(cy)
		{
			std::string height(_EmusToInches(cy));
			height += "in";
			if(imgElem->setProperty("height", height) != UT_OK)
			{
				UT_DEBUGMSG(("FRT:OpenXML importer image height property can't be set\n"));
			}
		}

		rqst->handled = true;
	}
	else if (nameMatches(rqst->pName, NS_A_KEY, "blip"))
	{
		OXML_SharedElement imgElem = rqst->stck->top();
		if(!imgElem)
			return;

		const gchar * id = attrMatches(NS_R_KEY, "embed", rqst->ppAtts);
		std::string imageId(id);
		if(id)
		{
			imgElem->setId(id);

			UT_Error err = UT_OK;
			FG_Graphic* pFG = NULL;
		
			OXMLi_PackageManager * mgr = OXMLi_PackageManager::getInstance();
			const UT_ByteBuf* imageData = mgr->parseImageStream(id);

			err = IE_ImpGraphic::loadGraphic (*imageData, IEGFT_Unknown, &pFG);
			if ((err != UT_OK) || !pFG) 
			{
				DELETEP(imageData);
				UT_DEBUGMSG(("FRT:OpenXML importer can't import the picture with id:%s\n", id));
				return;
			}
			DELETEP(imageData);

			OXML_Document * doc = OXML_Document::getInstance();
			UT_return_if_fail(_error_if_fail(doc != NULL));
				
			OXML_Image* img = new OXML_Image();
			img->setId(imageId.c_str());
			img->setGraphic(pFG);

			OXML_SharedImage shrImg(img);
			if(doc->addImage(shrImg) != UT_OK)
				return;
			rqst->handled = true;
		}
	}	
	//TODO: more coming here
}

void OXMLi_ListenerState_Image::endElement (OXMLi_EndElementRequest * rqst)
{
	if(nameMatches(rqst->pName, NS_W_KEY, "drawing"))
	{
		//image is done
		UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) ); 
		rqst->handled = true;
	}
	else if (nameMatches(rqst->pName, NS_A_KEY, "blip") || 
			nameMatches(rqst->pName, NS_WP_KEY, "extent"))
	{
		rqst->handled = true;
	}
	//TODO: more coming here
}

void OXMLi_ListenerState_Image::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//don't do anything here
}
