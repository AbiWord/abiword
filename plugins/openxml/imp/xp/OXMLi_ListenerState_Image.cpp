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
	OXMLi_ListenerState(), m_style("")
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
		if(rqst->stck->empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

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
		if(rqst->stck->empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		OXML_SharedElement imgElem = rqst->stck->top();
		if(!imgElem)
			return;

		const gchar * id = attrMatches(NS_R_KEY, "embed", rqst->ppAtts);
		if(id)
		{
			std::string imageId(id);
			imgElem->setId(id);
			rqst->handled = addImage(imageId);
		}
	}	
	else if(nameMatches(rqst->pName, NS_V_KEY, "shape"))
	{
		const gchar* style = attrMatches(NS_V_KEY, "style", rqst->ppAtts);
		if(style)
		{
			m_style = style;
		}
		//don't handle the request here in case shape contains some other structure, ex: textbox
	}
	else if(nameMatches(rqst->pName, NS_V_KEY, "imagedata"))
	{
		const gchar* id = attrMatches(NS_R_KEY, "id", rqst->ppAtts);
		if(id)
		{
			std::string imageId(id);
			OXML_SharedElement imgElem(new OXML_Element_Image(imageId));
			rqst->stck->push(imgElem);

			if(!addImage(imageId))
				return;

			if(m_style.compare(""))
			{
				//parse and apply style here
				std::string attrName("");
				std::string attrValue("");
				size_t attrStart = 0;
				size_t attrEnd = 0;
				while(attrStart < m_style.length())
				{
					attrEnd = m_style.find(';', attrStart);
					if(attrEnd == std::string::npos)
					{
						//this should be the last attribute
						attrEnd = m_style.length(); 
					}
					std::string attrNameValPair = m_style.substr(attrStart, attrEnd-attrStart);
					size_t seperator = attrNameValPair.find(':');
					if(seperator != std::string::npos)
					{
						attrName = attrNameValPair.substr(0, seperator);
						attrValue = attrNameValPair.substr(seperator+1);
					
						//convert and apply attributes here
						if(!attrName.compare("width"))
						{
							imgElem->setProperty("width", attrValue);
						}
						else if(!attrName.compare("height"))
						{
							imgElem->setProperty("height", attrValue);
						}
						//TODO: more attributes coming
					}	
					//finally update the start point for the next attribute
					attrStart = attrEnd+1;
				}
			}
			rqst->handled = true;
		}
	}

	//TODO: more coming here
}

void OXMLi_ListenerState_Image::endElement (OXMLi_EndElementRequest * rqst)
{
	if(nameMatches(rqst->pName, NS_W_KEY, "drawing") || 
		nameMatches(rqst->pName, NS_V_KEY, "imagedata"))
	{
		//image is done
		rqst->handled = (_flushTopLevel(rqst->stck, rqst->sect_stck) == UT_OK);
	}
	else if (nameMatches(rqst->pName, NS_A_KEY, "blip") || 
			nameMatches(rqst->pName, NS_WP_KEY, "extent"))
	{
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_V_KEY, "shape"))
	{
		m_style = "";
	}
	//TODO: more coming here
}

void OXMLi_ListenerState_Image::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//don't do anything here
}


//helper functions
bool OXMLi_ListenerState_Image::addImage(const std::string & id)
{
	UT_Error err = UT_OK;
	FG_Graphic* pFG = NULL;
		
	OXMLi_PackageManager * mgr = OXMLi_PackageManager::getInstance();
	const UT_ByteBuf* imageData = mgr->parseImageStream(id.c_str());

	if (!imageData)
		return false;

	err = IE_ImpGraphic::loadGraphic (*imageData, IEGFT_Unknown, &pFG);
	if ((err != UT_OK) || !pFG) 
	{
		DELETEP(imageData);
		UT_DEBUGMSG(("FRT:OpenXML importer can't import the picture with id:%s\n", id.c_str()));
		return false;
	}
	DELETEP(imageData);

	OXML_Document * doc = OXML_Document::getInstance();
	if(!doc)
		return false;
				
	OXML_Image* img = new OXML_Image();
	img->setId(id.c_str());
	img->setGraphic(pFG);

	OXML_SharedImage shrImg(img);

	return doc->addImage(shrImg) == UT_OK;
}
