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
#include <OXMLi_ListenerState_Field.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_FontManager.h>

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>

// External includes
#include <string>

OXMLi_ListenerState_Field::OXMLi_ListenerState_Field():
	OXMLi_ListenerState()
{

}

void OXMLi_ListenerState_Field::startElement (OXMLi_StartElementRequest * rqst)
{
	if (nameMatches(rqst->pName, NS_W_KEY, "fldSimple"))
	{
		const gchar* instr = attrMatches(NS_W_KEY, "instr", rqst->ppAtts);
		if(instr)
		{
			std::string type(instr);
			OXML_SharedElement elem(new OXML_Element_Field("", type, ""));
			rqst->stck->push(elem);
		}
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Field::endElement (OXMLi_EndElementRequest * rqst)
{
	if (nameMatches(rqst->pName, NS_W_KEY, "fldSimple"))
	{
		if(rqst->stck->size() < 2)
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		OXML_SharedElement field = rqst->stck->top();
		rqst->stck->pop();

		OXML_SharedElement parent = rqst->stck->top();
		if(parent)
			parent->appendElement(field);

		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Field::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//don't do anything here
}
