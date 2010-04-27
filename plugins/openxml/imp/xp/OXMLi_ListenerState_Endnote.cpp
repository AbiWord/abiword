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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// Class definition include
#include <OXMLi_ListenerState_Endnote.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_Types.h>

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>

// External includes
#include <string>

void OXMLi_ListenerState_Endnote::startElement (OXMLi_StartElementRequest * rqst)
{
	if (nameMatches(rqst->pName, NS_W_KEY, "endnotes"))
	{
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "endnote"))
	{
		const gchar* id = attrMatches(NS_W_KEY, "id", rqst->ppAtts);
		if(id)
		{
			OXML_SharedSection sect(new OXML_Section(id));
			rqst->sect_stck->push(sect);
		}
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Endnote::endElement (OXMLi_EndElementRequest * rqst)
{
	if (nameMatches(rqst->pName, NS_W_KEY, "endnotes"))
	{
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "endnote"))
	{
		if(rqst->sect_stck->empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		OXML_SharedSection sect = rqst->sect_stck->top();
		rqst->sect_stck->pop();
		OXML_Document* pDoc = OXML_Document::getInstance();
		if(pDoc)
		{				
			if(pDoc->addEndnote(sect) != UT_OK)
				return;
		}
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Endnote::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//nothing to do here
}
