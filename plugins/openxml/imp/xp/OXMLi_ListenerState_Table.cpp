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
		pTable->setCurrentRowNumber(-1);
		pTable->setCurrentColNumber(-1);
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tr"))
	{
		OXML_Element_Table* table = m_tableStack.top();
		OXML_Element_Row* pRow = new OXML_Element_Row("", table);
		m_rowStack.push(pRow);
		OXML_SharedElement row(pRow);
		rqst->stck->push(row);
		rqst->handled = true;
		table->incrementCurrentRowNumber();
		table->setCurrentColNumber(0);
		pRow->setRowNumber(table->getCurrentRowNumber());
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tc"))
	{
		OXML_Element_Table* table = m_tableStack.top();				
		OXML_Element_Row* row = m_rowStack.top();				
		OXML_Element_Cell* pCell = new OXML_Element_Cell("", table, row, 
								table->getCurrentColNumber(), table->getCurrentColNumber()+1, //left right
								table->getCurrentRowNumber(), table->getCurrentRowNumber()+1); //top,bottom
		m_cellStack.push(pCell);
		OXML_SharedElement cell(pCell);
		rqst->stck->push(cell);
		rqst->handled = true;
		table->incrementCurrentColNumber();
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "gridSpan"))
	{
		OXML_Element_Table* table = m_tableStack.top();				
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val)
		{
			int span = atoi(val);
			int left = table->getCurrentColNumber()-1;
			int right = left + span;
			//change current cell's right index
			OXML_Element_Cell* cell = m_cellStack.top();
			cell->setRight(right);
			//update column index of current table			
			table->setCurrentColNumber(right);
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "vMerge"))
	{
		OXML_Element_Cell* cell = m_cellStack.top();				
		cell->setVerticalMergeStart(false); //default to continue if the attribute is missing
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val && !strcmp(val, "restart")) 
		{
			cell->setVerticalMergeStart(true);
		}
		rqst->handled = true;
	}

	//Table Properties
	else if(nameMatches(rqst->pName, NS_W_KEY, "gridCol") && 
			contextMatches(rqst->context->back(), NS_W_KEY, "tblGrid"))
	{
		OXML_Element_Table* table = m_tableStack.top();				
		const gchar* w = attrMatches(NS_W_KEY, "w", rqst->ppAtts);
		if(w) 
		{
			//append this width to table-column-props property
			const gchar* tableColumnProps = NULL;
			UT_Error ret = table->getProperty("table-column-props", tableColumnProps);
			if((ret != UT_OK) || !tableColumnProps)
				tableColumnProps = "";				
			std::string cols(tableColumnProps);
			cols += _TwipsToPoints(w);
			cols += "pt/";
			ret = table->setProperty("table-column-props", cols);
			if(ret != UT_OK)
				UT_DEBUGMSG(("FRT:OpenXML importer can't set table-column-props:%s\n", cols.c_str()));				
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "trHeight") && 
			contextMatches(rqst->context->back(), NS_W_KEY, "trPr"))
	{
		OXML_Element_Table* table = m_tableStack.top();				
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val) 
		{
			const gchar* tableRowHeights = NULL;
			UT_Error ret = table->getProperty("table-row-heights", tableRowHeights);
			if((ret != UT_OK) || !tableRowHeights)
				tableRowHeights = "";				
			std::string rowHeights(tableRowHeights);
			rowHeights += _TwipsToPoints(val);
			rowHeights += "pt/";
			ret = table->setProperty("table-row-heights", rowHeights);
			if(ret != UT_OK)
				UT_DEBUGMSG(("FRT:OpenXML importer can't set table-row-heights:%s\n", rowHeights.c_str()));				
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "left") ||
			nameMatches(rqst->pName, NS_W_KEY, "right") ||
			nameMatches(rqst->pName, NS_W_KEY, "top") ||
			nameMatches(rqst->pName, NS_W_KEY, "bottom"))
	{
		rqst->handled = true;
		const gchar* color = attrMatches(NS_W_KEY, "color", rqst->ppAtts);
		if(color && strcmp(color, "auto")) 
		{
			UT_Error ret = UT_OK;
			std::string borderName(rqst->pName);
			borderName = borderName.substr(strlen(NS_W_KEY)+1);
			if(!borderName.compare("bottom"))
				borderName = "bot";
			std::string borderStyle = borderName + "-style";
			borderName += "-color";

			OXML_Element* element = NULL;

			if(contextMatches(rqst->context->back(), NS_W_KEY, "tcBorders"))
			{
				element = m_cellStack.top();
				if(!element)
					return;	

				ret = element->setProperty(borderStyle, "0");
				if(ret != UT_OK)
					UT_DEBUGMSG(("FRT:OpenXML importer can't set %s:0\n", borderStyle.c_str()));				
			}
			else if(contextMatches(rqst->context->back(), NS_W_KEY, "tblBorders"))
			{
				element = m_tableStack.top();
				if(!element)
					return;	

				ret = element->setProperty(borderStyle, "1");
				if(ret != UT_OK)
					UT_DEBUGMSG(("FRT:OpenXML importer can't set %s:0\n", borderStyle.c_str()));				
			}

			if(!element)
				return;
	
			ret = element->setProperty(borderName, color);
			if(ret != UT_OK)
				UT_DEBUGMSG(("FRT:OpenXML importer can't set %s:%s\n", borderName.c_str(), color));				
		}
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
		OXML_Element_Cell* pCell = m_cellStack.top();
		if(!pCell->startsVerticalMerge())
		{
			OXML_Element_Table* table = m_tableStack.top();
			if(!table->incrementBottomVerticalMergeStart(pCell->getLeft(), pCell->getTop()))
			{
				//this means there is no cell before this starting a vertical merge
				//revert back to vertical merge start instead of continue
				pCell->setVerticalMergeStart(true);
				UT_DEBUGMSG(("FRT:OpenXML importer, invalid <vMerge val=continue> attribute.\n"));
			}
		}
		else
		{
			OXML_Element_Row* pRow = m_rowStack.top();
			row->appendElement(cell);
		}
		m_cellStack.pop();
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "gridSpan") ||
			nameMatches(rqst->pName, NS_W_KEY, "vMerge") ||
			nameMatches(rqst->pName, NS_W_KEY, "gridCol") ||
			nameMatches(rqst->pName, NS_W_KEY, "trHeight") ||
			nameMatches(rqst->pName, NS_W_KEY, "left") ||
			nameMatches(rqst->pName, NS_W_KEY, "right") ||
			nameMatches(rqst->pName, NS_W_KEY, "top") ||
			nameMatches(rqst->pName, NS_W_KEY, "bottom"))
	{
		rqst->handled = true;
	}	
	//TODO: more coming here
}

void OXMLi_ListenerState_Table::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//don't do anything here
}
