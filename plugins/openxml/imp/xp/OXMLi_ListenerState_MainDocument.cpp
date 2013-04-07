/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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
#include <OXMLi_ListenerState_MainDocument.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Document.h>
#include <OXML_Section.h>

// AbiWord includes
#include <ut_math.h>
#include <ut_debugmsg.h>

// External includes
#include <cstring>

OXMLi_ListenerState_MainDocument::OXMLi_ListenerState_MainDocument() : 
	OXMLi_ListenerState()
{

}

OXMLi_ListenerState_MainDocument::~OXMLi_ListenerState_MainDocument()
{
}

void OXMLi_ListenerState_MainDocument::startElement (OXMLi_StartElementRequest * rqst)
{
	UT_return_if_fail( this->_error_if_fail(rqst != NULL) );

	if (nameMatches(rqst->pName, NS_W_KEY, "body")) {
		//This signals the start of the first section.
		OXML_SharedSection sect(new OXML_Section());
		sect->setBreakType(CONTINUOUS_BREAK); //First section of the document does not have breaks at beginning and end
		rqst->sect_stck->push(sect);
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "pgSz"))
	{
		const gchar* width = attrMatches(NS_W_KEY, "w", rqst->ppAtts);
		const gchar* height = attrMatches(NS_W_KEY, "h", rqst->ppAtts);
		const gchar* orientation = attrMatches(NS_W_KEY, "orient", rqst->ppAtts);

		OXML_Document* doc = OXML_Document::getInstance();

		if(!width || !height)
		{
			rqst->handled = true;
			return;
		}
		
		doc->setPageWidth(_TwipsToInches(width));
		doc->setPageHeight(_TwipsToInches(height));

		if(orientation) //this is an optional attribute
		{
			doc->setPageOrientation(orientation);
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "pgMar"))
	{
		const gchar* t = attrMatches(NS_W_KEY, "top", rqst->ppAtts);
		const gchar* l = attrMatches(NS_W_KEY, "left", rqst->ppAtts);
		const gchar* r = attrMatches(NS_W_KEY, "right", rqst->ppAtts);
		const gchar* b = attrMatches(NS_W_KEY, "bottom", rqst->ppAtts);

		OXML_Document* doc = OXML_Document::getInstance();

		if(!doc || !t || !l || !r || !b)
		{
			rqst->handled = true;
			return;
		}

		std::string top("");
		top += _TwipsToInches(t);
		top += "in";

		std::string left("");
		left += _TwipsToInches(l);
		left += "in";

		std::string right("");
		right += _TwipsToInches(r);
		right += "in";

		std::string bottom("");
		bottom += _TwipsToInches(b);
		bottom += "in";
		
		doc->setPageMargins(top, left, right, bottom);
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_MainDocument::endElement (OXMLi_EndElementRequest * rqst)
{
	UT_return_if_fail( this->_error_if_fail(rqst != NULL) );

	if (nameMatches(rqst->pName, NS_W_KEY, "body")) {
		//end of the body, append all sections one by one in reverse order
		//TODO: there should be a better way of doing this
		OXMLi_SectionStack reversedStck;
		while(!rqst->sect_stck->empty()){		
			OXML_SharedSection sect = rqst->sect_stck->top();
			rqst->sect_stck->pop();
			reversedStck.push(sect);
		}		
		while(!reversedStck.empty()){		
			OXML_SharedSection sect = reversedStck.top();
			reversedStck.pop();
			OXML_Document * doc = OXML_Document::getInstance();
			UT_return_if_fail(_error_if_fail(doc != NULL)); 
			UT_return_if_fail(_error_if_fail( UT_OK == doc->appendSection(sect) )); 
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "pgSz") || nameMatches(rqst->pName, NS_W_KEY, "pgMar"))
	{
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_MainDocument::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//Nothing to do here.
}

