/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

// Class definition include
#include <OXMLi_ListenerState_Textbox.h>

// Internal includes
#include <OXML_Document.h>

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>

// External includes
#include <string>

OXMLi_ListenerState_Textbox::OXMLi_ListenerState_Textbox():
	OXMLi_ListenerState(), m_style("")
{

}

void OXMLi_ListenerState_Textbox::startElement (OXMLi_StartElementRequest * rqst)
{
	if(nameMatches(rqst->pName, NS_V_KEY, "shape"))
	{
		const gchar* style = attrMatches(NS_V_KEY, "style", rqst->ppAtts);
		if(style)
		{
			m_style = style;
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_V_KEY, "textbox"))
	{
		OXML_SharedElement textboxElem(new OXML_Element_TextBox(""));
		
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
						textboxElem->setProperty("frame-width", attrValue);
					}
					else if(!attrName.compare("height"))
					{
						textboxElem->setProperty("frame-height", attrValue);
					}
					//TODO: more attributes coming
				}	
				//finally update the start point for the next attribute
				attrStart = attrEnd+1;
			}
		}
		
		rqst->stck->push(textboxElem);
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "txbxContent"))
	{
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Textbox::endElement (OXMLi_EndElementRequest * rqst)
{
	if(nameMatches(rqst->pName, NS_V_KEY, "shape"))
	{
		m_style = "";
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_V_KEY, "textbox"))
	{
		rqst->handled = (_flushTopLevel(rqst->stck, rqst->sect_stck) == UT_OK);
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "txbxContent"))
	{
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Textbox::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//don't do anything here
}
