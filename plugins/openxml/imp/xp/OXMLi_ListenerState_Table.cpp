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
#include <OXMLi_ListenerState_Table.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_FontManager.h>

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>

// External includes
#include <string>

OXMLi_ListenerState_Table::OXMLi_ListenerState_Table():
	OXMLi_ListenerState()
{

}

void OXMLi_ListenerState_Table::startElement (OXMLi_StartElementRequest * rqst)
{
	if (nameMatches(rqst->pName, NS_W_KEY, "tbl"))
	{
		OXML_Element_Table* pTable = new OXML_Element_Table("");
		m_tableStack.push(pTable);
		OXML_SharedElement table(pTable);
		rqst->stck->push(table);
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tr"))
	{
		OXML_Element_Table* table = m_tableStack.top();
		OXML_Element_Row* pRow = new OXML_Element_Row("", table);
		m_rowStack.push(pRow);
		OXML_SharedElement row(pRow);
		rqst->stck->push(row);
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tc"))
	{
		OXML_Element_Table* table = m_tableStack.top();				
		OXML_Element_Row* row = m_rowStack.top();				
		OXML_SharedElement cell(new OXML_Element_Cell("", table, row, 0, 1, 0, 1));
		rqst->stck->push(cell);
		rqst->handled = true;
	}
	//TODO: more coming here
}

void OXMLi_ListenerState_Table::endElement (OXMLi_EndElementRequest * rqst)
{
	if (nameMatches(rqst->pName, NS_W_KEY, "tbl"))
	{
		OXML_SharedElement table = rqst->stck->top();
		rqst->stck->pop(); //pop table
		if(rqst->stck->empty())
		{
			OXML_SharedSection last = OXML_Document::getCurrentSection();
			last->appendElement(table);
		}
		else
		{
			OXML_SharedElement container = rqst->stck->top();
			container->appendElement(table);
		}
		m_tableStack.pop();
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tr"))
	{
		OXML_SharedElement row = rqst->stck->top();
		rqst->stck->pop(); //pop row
		OXML_SharedElement table = rqst->stck->top();
		table->appendElement(row);
		m_rowStack.pop();
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tc"))
	{
		OXML_SharedElement cell = rqst->stck->top();
		rqst->stck->pop(); //pop cell
		OXML_SharedElement row = rqst->stck->top();
		row->appendElement(cell);
		rqst->handled = true;
	}
	//TODO: more coming here
}

void OXMLi_ListenerState_Table::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//don't do anything here
}
