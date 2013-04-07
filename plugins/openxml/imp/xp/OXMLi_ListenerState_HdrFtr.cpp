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
#include <OXMLi_ListenerState_HdrFtr.h>

// Internal includes
#include <OXML_Element.h>
#include <OXML_Section.h>
#include <OXML_Document.h>
#include <OXML_Types.h>

// AbiWord includes
#include <ut_debugmsg.h>

// External includes
#include <cstring>

OXMLi_ListenerState_HdrFtr::OXMLi_ListenerState_HdrFtr(std::string partId) : 
	OXMLi_ListenerState(), 
	m_partId(partId)
{

}

OXMLi_ListenerState_HdrFtr::~OXMLi_ListenerState_HdrFtr()
{
}

void OXMLi_ListenerState_HdrFtr::startElement (OXMLi_StartElementRequest * rqst)
{
	if (nameMatches(rqst->pName, NS_W_KEY, "hdr") || nameMatches(rqst->pName, NS_W_KEY, "ftr"))
	{
		OXML_SharedElement dummy(new OXML_Element("", P_TAG, BLOCK));
		rqst->stck->push(dummy);
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_HdrFtr::endElement (OXMLi_EndElementRequest * rqst)
{
	if (nameMatches(rqst->pName, NS_W_KEY, "hdr") || nameMatches(rqst->pName, NS_W_KEY, "ftr"))
	{
		OXML_SharedSection s(new OXML_Section(m_partId));

		if(!rqst->stck->empty())
		{
			OXML_SharedElement container = rqst->stck->top();
			s->setChildren( container->getChildren() );
		}

		OXML_Document * doc = OXML_Document::getInstance();
		UT_return_if_fail( this->_error_if_fail(doc != NULL) );

		if (nameMatches(rqst->pName, NS_W_KEY, "hdr"))
			doc->addHeader(s);
		else
			doc->addFooter(s);

		rqst->handled = true;
	}
}

void OXMLi_ListenerState_HdrFtr::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//Nothing to do here
}

