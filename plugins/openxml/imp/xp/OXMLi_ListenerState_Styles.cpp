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
#include <OXMLi_ListenerState_Styles.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_Types.h>

// AbiWord includes
#include <ut_debugmsg.h>

// External includes
#include <cstring>

OXMLi_ListenerState_Styles::OXMLi_ListenerState_Styles() : 
	OXMLi_ListenerState(), 
	m_pCurrentStyle(NULL),
	m_szValZero(false)
{

}

OXMLi_ListenerState_Styles::~OXMLi_ListenerState_Styles()
{
}

void OXMLi_ListenerState_Styles::startElement (OXMLi_StartElementRequest * rqst)
{
	UT_return_if_fail( _error_if_fail(rqst != NULL) );

	if (nameMatches(rqst->pName, NS_W_KEY, "docDefaults")) {
		m_pCurrentStyle = new OXML_Style("Normal", "Normal");
		m_pCurrentStyle->setAttribute(PT_TYPE_ATTRIBUTE_NAME, "P");
		m_pCurrentStyle->setAttribute(PT_FOLLOWEDBY_ATTRIBUTE_NAME, "Current Settings");

		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "rPr") || nameMatches(rqst->pName, NS_W_KEY, "pPr")) {
		//Push a dummy element onto the stack to collect the formatting for the current style.
		OXML_SharedElement dummy(new OXML_Element_Paragraph(""));
		rqst->stck->push(dummy);

		rqst->handled = true;

	} else if (nameMatches(rqst->pName, NS_W_KEY, "tblPr")) {
		//Push a dummy element onto the stack to collect the formatting for the current style.
		OXML_SharedElement dummy(new OXML_Element_Table(""));
		rqst->stck->push(dummy);
		//don't handle the request so that table listener state can adjust its internal state

	} else if (nameMatches(rqst->pName, NS_W_KEY, "trPr")) {
		//Push a dummy element onto the stack to collect the formatting for the current style.
		OXML_SharedElement dummy(new OXML_Element_Row("", NULL));
		rqst->stck->push(dummy);
		//don't handle the request so that table listener state can adjust its internal state

	} else if (nameMatches(rqst->pName, NS_W_KEY, "tcPr")) {
		//Push a dummy element onto the stack to collect the formatting for the current style.
		OXML_SharedElement dummy(new OXML_Element_Cell("", NULL, NULL, 0,0,0,0));
		rqst->stck->push(dummy);
		//don't handle the request so that table listener state can adjust its internal state

	} else if (nameMatches(rqst->pName, NS_W_KEY, "style")) {
		const gchar * id = attrMatches(NS_W_KEY, "styleId", rqst->ppAtts);
		const gchar * type = attrMatches(NS_W_KEY, "type", rqst->ppAtts);
		UT_return_if_fail( _error_if_fail( id != NULL ));
		if (!strcmp(id, "Normal")) id = "_Normal"; //Cannot interfere with document defaults
		m_pCurrentStyle = new OXML_Style(id, "");
		if(m_pCurrentStyle == NULL)
		{
			UT_DEBUGMSG(("SERHAT: Cannot create an OXML_Style object with the given id!\n"));
			return;
		}

		if (!type || !*type) {
			// default to paragraph in the case of a missing/blank attribute
			// (as specified by the spec: 2.7.3.17)
			type = "P";
		}
		else if (!strcmp(type, "character")) {
			type = "C"; //Type is C for "character"
		} else {
			type = "P"; //Type is P for "paragraph", "numbering", and "table"
		}
		m_pCurrentStyle->setAttribute(PT_TYPE_ATTRIBUTE_NAME, type);

		rqst->handled = true;
	} else if (	nameMatches(rqst->pName, NS_W_KEY, "name") ||
				nameMatches(rqst->pName, NS_W_KEY, "basedOn") ||
				nameMatches(rqst->pName, NS_W_KEY, "next")) {
		const gchar * val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		UT_return_if_fail( _error_if_fail( m_pCurrentStyle != NULL && val != NULL ));
		if (!strcmp(val, "Normal")) val = "_Normal"; //Cannot interfere with document defaults

		if (nameMatches(rqst->pName, NS_W_KEY, "name")) {
			m_pCurrentStyle->setName(val);
		} else if (nameMatches(rqst->pName, NS_W_KEY, "basedOn")) {
			//For now, we use the ID as reference, until all styles have been parsed
			m_pCurrentStyle->setAttribute(PT_BASEDON_ATTRIBUTE_NAME, val);
		} else if (nameMatches(rqst->pName, NS_W_KEY, "next")) {
			//For now, we use the ID as reference, until all styles have been parsed
			m_pCurrentStyle->setAttribute(PT_FOLLOWEDBY_ATTRIBUTE_NAME, val);
		}
		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "sz")) {
		const gchar * val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		UT_return_if_fail( this->_error_if_fail(val != NULL) );
		if(!strcmp(val, "0"))
		{
			m_szValZero = true;
			rqst->handled = true;
		}
	}
}

void OXMLi_ListenerState_Styles::endElement (OXMLi_EndElementRequest * rqst)
{
	UT_return_if_fail( _error_if_fail(rqst != NULL) );

	if (nameMatches(rqst->pName, NS_W_KEY, "docDefaults") || nameMatches(rqst->pName, NS_W_KEY, "style")) {
		UT_return_if_fail(_error_if_fail(m_pCurrentStyle != NULL));

		OXML_Document * doc = OXML_Document::getInstance();
		UT_return_if_fail( _error_if_fail(doc != NULL) );
		OXML_SharedStyle styl(m_pCurrentStyle);
		doc->addStyle(styl);
		m_pCurrentStyle = NULL;

		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "rPr") || 
			   nameMatches(rqst->pName, NS_W_KEY, "pPr") ||
			   nameMatches(rqst->pName, NS_W_KEY, "tblPr") ||
			   nameMatches(rqst->pName, NS_W_KEY, "trPr") ||
			   nameMatches(rqst->pName, NS_W_KEY, "tcPr")) {
		//Retrieve the formatting collected by the Common listener state.
		OXML_SharedElement dummy = rqst->stck->top();
		PP_PropertyVector props = dummy->getProperties();
		if (!props.empty()) {
			//Pass the retrieved properties to a new style object
			UT_return_if_fail(_error_if_fail(UT_OK == m_pCurrentStyle->appendProperties(props)));
		}
		rqst->stck->pop();

		rqst->handled = !nameMatches(rqst->pName, NS_W_KEY, "tblPr") &&
						!nameMatches(rqst->pName, NS_W_KEY, "trPr") &&
						!nameMatches(rqst->pName, NS_W_KEY, "tcPr");
	} else if (nameMatches(rqst->pName, NS_W_KEY, "sz")) {
		if(m_szValZero)
		{
			rqst->handled = true;
		}
		m_szValZero = false;
	}
}

void OXMLi_ListenerState_Styles::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	UT_ASSERT ( UT_SHOULD_NOT_HAPPEN );
}

