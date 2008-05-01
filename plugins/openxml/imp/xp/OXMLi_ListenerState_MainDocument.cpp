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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// Class definition include
#include <OXMLi_ListenerState_MainDocument.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Document.h>
#include <OXML_Section.h>

// AbiWord includes
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

	if (!strcmp(rqst->pName, "body")) {
		//This signals the start of the first section.
		OXML_SharedSection sect(new OXML_Section());
		sect->setBreakType(CONTINUOUS_BREAK); //First section of the document does not have breaks at beginning and end
		OXML_Document * doc = OXML_Document::getInstance();
		UT_return_if_fail(_error_if_fail(doc != NULL)); 
		UT_return_if_fail(_error_if_fail( UT_OK == doc->appendSection(sect) )); 

		rqst->handled = true;
	}
}

void OXMLi_ListenerState_MainDocument::endElement (OXMLi_EndElementRequest * rqst)
{
	UT_return_if_fail( this->_error_if_fail(rqst != NULL) );

	if (!strcmp(rqst->pName, "body")) {
		//end of the body, nothing to do
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_MainDocument::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//Nothing to do here.
}

