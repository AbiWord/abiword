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
		if(m_tableStack.empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

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
		if(m_tableStack.empty() || m_rowStack.empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

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
		if(m_tableStack.empty() || m_cellStack.empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

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
		if(m_cellStack.empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		OXML_Element_Cell* cell = m_cellStack.top();				
		cell->setVerticalMergeStart(false); //default to continue if the attribute is missing
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val && !strcmp(val, "restart")) 
		{
			cell->setVerticalMergeStart(true);
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "hMerge"))
	{
		if(m_cellStack.empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		OXML_Element_Cell* cell = m_cellStack.top();				
		cell->setHorizontalMergeStart(false); //default to continue if the attribute is missing
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val && !strcmp(val, "restart")) 
		{
			cell->setHorizontalMergeStart(true);
		}
		rqst->handled = true;
	}

	//Table Properties
	else if(nameMatches(rqst->pName, NS_W_KEY, "gridCol") && 
			contextMatches(rqst->context->back(), NS_W_KEY, "tblGrid"))
	{
		if(m_tableStack.empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

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
			{	
				UT_DEBUGMSG(("FRT:OpenXML importer can't set table-column-props:%s\n", cols.c_str()));
			}
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "trHeight") && 
			contextMatches(rqst->context->back(), NS_W_KEY, "trPr"))
	{
		if(m_tableStack.empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

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
			{
				UT_DEBUGMSG(("FRT:OpenXML importer can't set table-row-heights:%s\n", rowHeights.c_str()));
			}
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
		const gchar* sz = attrMatches(NS_W_KEY, "sz", rqst->ppAtts);
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);

		UT_Error ret = UT_OK;

		std::string borderName(rqst->pName);
		borderName = borderName.substr(strlen(NS_W_KEY)+1);
		if(!borderName.compare("bottom"))
			borderName = "bot";

		std::string borderStyle = borderName + "-style";
		std::string borderColor = borderName + "-color";
		std::string borderThickness = borderName + "-thickness";

		OXML_Element* element = NULL;

		if(rqst->context->empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		if(contextMatches(rqst->context->back(), NS_W_KEY, "tcBorders"))
			element = m_cellStack.empty() ? NULL : m_cellStack.top();
		else if(contextMatches(rqst->context->back(), NS_W_KEY, "tblBorders"))
			element = m_tableStack.empty() ? NULL : m_tableStack.top();

		if(!element)
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		if(color && strcmp(color, "auto")) 
		{
			ret = element->setProperty(borderColor, color);
			if(ret != UT_OK)
			{
				UT_DEBUGMSG(("FRT:OpenXML importer can't set %s:%s\n", borderColor.c_str(), color));
			}
		}
		if(sz) 
		{
			std::string szVal(_EighthPointsToPoints(sz));
			szVal += "pt";
			ret = element->setProperty(borderThickness, szVal);
			if(ret != UT_OK)
			{
				UT_DEBUGMSG(("FRT:OpenXML importer can't set %s:%s\n", borderThickness.c_str(), color));
			}
		}

		std::string styleValue = "1"; //single line border by default
		if(val)
		{
			if(!strcmp(val, "dashed"))
				styleValue = "0"; 
		}

		ret = element->setProperty(borderStyle, styleValue);
		if(ret != UT_OK)
		{
			UT_DEBUGMSG(("FRT:OpenXML importer can't set %s:0\n", borderStyle.c_str()));
		}
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "shd"))
	{
		const gchar* fill = attrMatches(NS_W_KEY, "fill", rqst->ppAtts);

		UT_Error ret = UT_OK;
		OXML_Element* element = NULL;

		if(rqst->context->empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		if(contextMatches(rqst->context->back(), NS_W_KEY, "tcPr"))
			element = m_cellStack.empty() ? NULL : m_cellStack.top();
		else if(contextMatches(rqst->context->back(), NS_W_KEY, "tblPr"))
			element = m_tableStack.empty() ? NULL : m_tableStack.top();

		if(!element)
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		if(fill && strcmp(fill, "auto")) 
		{
			ret = element->setProperty("background-color", fill);
			if(ret != UT_OK)
			{
				UT_DEBUGMSG(("FRT:OpenXML importer can't set background-color:%s\n", fill));	
			}
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tblStyle"))
	{
		if(m_tableStack.empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		OXML_Element_Table* table = m_tableStack.top();				
		const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
		if(val && table) 
		{
			std::string styleName(val);
			OXML_Document* doc = OXML_Document::getInstance();
			if(doc)
				table->applyStyle(doc->getStyleById(styleName));
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tblPr"))
	{
		if(m_tableStack.empty())
		{
			//we must be in tblStyle in styles, so let's push the table instance to m_tableStack
			OXML_Element_Table* tbl = static_cast<OXML_Element_Table*>(rqst->stck->top().get());
			m_tableStack.push(tbl);
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "trPr"))
	{
		if(m_rowStack.empty())
		{
			//we must be in styles, so let's push the row instance to m_rowStack
			OXML_Element_Row* row = static_cast<OXML_Element_Row*>(rqst->stck->top().get());
			m_rowStack.push(row);
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tcPr"))
	{
		if(m_cellStack.empty())
		{
			//we must be in styles, so let's push the cell instance to m_cellStack
			OXML_Element_Cell* cell = static_cast<OXML_Element_Cell*>(rqst->stck->top().get());
			m_cellStack.push(cell);
		}
		rqst->handled = true;
	}
	//TODO: more coming here
}

void OXMLi_ListenerState_Table::endElement (OXMLi_EndElementRequest * rqst)
{
	if (nameMatches(rqst->pName, NS_W_KEY, "tbl"))
	{
		if(m_tableStack.empty() || rqst->stck->empty())
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		OXML_SharedElement table = rqst->stck->top();
		rqst->stck->pop(); //pop table
		if(rqst->stck->empty())
		{
			OXML_SharedSection last = rqst->sect_stck->top();
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
		if(m_rowStack.empty() || (rqst->stck->size() < 2))
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		OXML_SharedElement row = rqst->stck->top();
		rqst->stck->pop(); //pop row
		OXML_SharedElement table = rqst->stck->top();
		table->appendElement(row);
		m_rowStack.pop();
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tc"))
	{
		if(m_tableStack.empty() || m_cellStack.empty() || (rqst->stck->size() < 2))
		{
			rqst->handled = false;
			rqst->valid = false;
			return;
		}

		OXML_SharedElement cell = rqst->stck->top();
		rqst->stck->pop(); //pop cell
		OXML_SharedElement row = rqst->stck->top();
		OXML_Element_Cell* pCell = m_cellStack.top();
		if(!pCell->startsHorizontalMerge() && !pCell->startsVerticalMerge())
		{
			//do nothing in this case
		}
		else if(!pCell->startsVerticalMerge())
		{
			OXML_Element_Table* table = m_tableStack.top();
			if(!table->incrementBottomVerticalMergeStart(pCell))
			{
				//this means there is no cell before this starting a vertical merge
				//revert back to vertical merge start instead of continue
				pCell->setVerticalMergeStart(true);
				UT_DEBUGMSG(("FRT:OpenXML importer, invalid <vMerge val=continue> attribute.\n"));
			}
		}
		else if(!pCell->startsHorizontalMerge())
		{
			OXML_Element_Table* table = m_tableStack.top();
			if(!table->incrementRightHorizontalMergeStart(pCell))
			{
				//this means there is no cell before this starting a horizontal merge
				//revert back to horizontal merge start instead of continue
				pCell->setHorizontalMergeStart(true);
				UT_DEBUGMSG(("FRT:OpenXML importer, invalid <hMerge val=continue> attribute.\n"));
			}
		}
		else //(pCell->startsHorizontalMerge() && pCell->startsVerticalMerge())
		{
			row->appendElement(cell);
		}
		m_cellStack.pop();
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "gridSpan") ||
			nameMatches(rqst->pName, NS_W_KEY, "vMerge") ||
			nameMatches(rqst->pName, NS_W_KEY, "hMerge") ||
			nameMatches(rqst->pName, NS_W_KEY, "gridCol") ||
			nameMatches(rqst->pName, NS_W_KEY, "trHeight") ||
			nameMatches(rqst->pName, NS_W_KEY, "left") ||
			nameMatches(rqst->pName, NS_W_KEY, "right") ||
			nameMatches(rqst->pName, NS_W_KEY, "top") ||
			nameMatches(rqst->pName, NS_W_KEY, "bottom") ||
			nameMatches(rqst->pName, NS_W_KEY, "tblStyle"))
	{
		rqst->handled = true;
	}	
	else if(nameMatches(rqst->pName, NS_W_KEY, "tblPr"))
	{
		if(!rqst->context->empty() && !contextMatches(rqst->context->back(), NS_W_KEY, "tbl") && !m_tableStack.empty())
		{
			m_tableStack.pop(); //pop the dummy table
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "trPr"))
	{
		if(!rqst->context->empty() && !contextMatches(rqst->context->back(), NS_W_KEY, "tr") && !m_rowStack.empty())
		{
			m_rowStack.pop(); //pop the dummy row
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "tcPr"))
	{
		if(!rqst->context->empty() && !contextMatches(rqst->context->back(), NS_W_KEY, "tc") && !m_cellStack.empty())
		{
			m_cellStack.pop(); //pop the dummy cell
		}
		rqst->handled = true;
	}
	else if(nameMatches(rqst->pName, NS_W_KEY, "shd"))
	{
		std::string contextTag = rqst->context->empty() ? "" : rqst->context->back();
		rqst->handled = contextMatches(contextTag, NS_W_KEY, "tcPr") || contextMatches(contextTag, NS_W_KEY, "tblPr");
	}
	//TODO: more coming here
}

void OXMLi_ListenerState_Table::charData (OXMLi_CharDataRequest * /*rqst*/)
{
	//don't do anything here
}
