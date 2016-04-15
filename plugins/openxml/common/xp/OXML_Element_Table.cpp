/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
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
#include <OXML_Element_Table.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_Table::OXML_Element_Table(const std::string & id) : 
	OXML_Element(id, TBL_TAG, TABLE),
	m_currentRowNumber(0),
	m_currentColNumber(0)
{
}

OXML_Element_Table::~OXML_Element_Table()
{

}

UT_Error OXML_Element_Table::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;

	err = exporter->startTable();
	if(err != UT_OK)
		return err;

	err = this->serializeProperties(exporter);
	if(err != UT_OK)
		return err;

	//set the row numbers
	std::vector<OXML_Element*>::size_type i;
	OXML_ElementVector children = getChildren();
	for (i = 0; i < children.size(); i++)
	{
		OXML_Element_Row* r = static_cast<OXML_Element_Row*>(children[i].get());
		r->setRowNumber(i);
	}

	err = this->serializeChildren(exporter);
	if(err != UT_OK)
		return err;

	return exporter->finishTable();
}

UT_Error OXML_Element_Table::serializeProperties(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;
	const gchar* szValue = NULL;

	if(getProperty("table-column-props", szValue) == UT_OK)
	{
		err = exporter->startTableGrid(TARGET_DOCUMENT);
		if(err != UT_OK)
			return err;

		std::string col(szValue);
		std::string token("");

		std::string::size_type prev = -1;
		std::string::size_type pos = col.find_first_of("/");
		
		while (pos != std::string::npos) 
		{
			token = col.substr(prev+1, pos-prev-1);
			columnWidth.push_back(token);
			err = exporter->setGridCol(TARGET_DOCUMENT, token.c_str());
			if(err != UT_OK)
				return err;
			prev = pos;	
			pos = col.find_first_of("/", pos + 1);
		}
		
		err = exporter->finishTableGrid(TARGET_DOCUMENT);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("table-row-heights", szValue) == UT_OK)
	{
		std::string rowHeights(szValue);
		std::string token("");

		std::string::size_type prev = -1;
		std::string::size_type pos = rowHeights.find_first_of("/");
		
		while (pos != std::string::npos) 
		{
			token = rowHeights.substr(prev+1, pos-prev-1);
			rowHeight.push_back(token);
			prev = pos;	
			pos = rowHeights.find_first_of("/", pos + 1);
		}
	}

	err = exporter->startTableProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	if(getProperty("background-color", szValue) == UT_OK)
	{
		err = exporter->setBackgroundColor(TARGET_DOCUMENT, szValue);
		if(err != UT_OK)
			return err;
	}

	err = exporter->startTableBorderProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	const gchar* borderType = NULL;
	const gchar* color = NULL;
	const gchar* size = NULL;

	//left border
	borderType = "single";
	if(getProperty("left-style", szValue) == UT_OK)
	{
		if(strcmp(szValue, "1") != 0)
		{
			 borderType = "dashed";
		}
	}

	color = NULL; 
	if(getProperty("left-color", szValue) == UT_OK)
	{
		color = szValue;
	}

	size = NULL;
	if(getProperty("left-thickness", szValue) == UT_OK)
	{
		size = szValue;
	}

	err = exporter->setTableBorder(TARGET_DOCUMENT, "left", borderType, color, size);
	if(err != UT_OK)
		return err;

	//right border
	borderType = "single";
	if(getProperty("right-style", szValue) == UT_OK)
	{
		if(strcmp(szValue, "1") != 0)
		{
			 borderType = "dashed";
		}
	}

	color = NULL; 
	if(getProperty("right-color", szValue) == UT_OK)
	{
		color = szValue;
	}

	size = NULL;
	if(getProperty("right-thickness", szValue) == UT_OK)
	{
		size = szValue;
	}
	err = exporter->setTableBorder(TARGET_DOCUMENT, "right", borderType, color, size);
	if(err != UT_OK)
		return err;

	//top border
	borderType = "single";
	if(getProperty("top-style", szValue) == UT_OK)
	{
		if(strcmp(szValue, "1") != 0)
		{
			 borderType = "dashed";
		}
	}

	color = NULL; 
	if(getProperty("top-color", szValue) == UT_OK)
	{
		color = szValue;
	}

	size = NULL;
	if(getProperty("top-thickness", szValue) == UT_OK)
	{
		size = szValue;
	}
	err = exporter->setTableBorder(TARGET_DOCUMENT, "top", borderType, color, size);
	if(err != UT_OK)
		return err;

	//bottom border
	borderType = "single";
	if(getProperty("bot-style", szValue) == UT_OK)
	{
		if(strcmp(szValue, "1") != 0)
		{
			 borderType = "dashed";
		}
	}
	
	color = NULL; 
	if(getProperty("bot-color", szValue) == UT_OK)
	{
		color = szValue;
	}
	
	size = NULL;
	if(getProperty("bot-thickness", szValue) == UT_OK)
	{
		size = szValue;
	}
	err = exporter->setTableBorder(TARGET_DOCUMENT, "bottom", borderType, color, size);
	if(err != UT_OK)
		return err;


	err = exporter->finishTableBorderProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	return exporter->finishTableProperties(TARGET_DOCUMENT);
}

UT_Error OXML_Element_Table::serializeChildren(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;

	OXML_ElementVector::size_type i;
	OXML_ElementVector children = getChildren();
	for (i = 0; i < children.size(); i++)
	{
		m_currentRowNumber = i;
		ret = children[i]->serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}

	return ret;
}

UT_Error OXML_Element_Table::addChildrenToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;
	UT_Error temp = UT_OK;
	std::vector<OXML_Element*>::size_type i;
	OXML_ElementVector children = getChildren();
	
	for (i = 0; i < children.size(); i++)
	{
		m_currentRowNumber = i;
		if(children[i]->getTag() != BOOK_TAG)
		{
			temp = children[i]->addToPT(pDocument);
			if (temp != UT_OK)
				ret = temp;
		}
	}
	return ret;
}

UT_Error OXML_Element_Table::addToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;

	const gchar * bgColor = NULL;
	if(getProperty("background-color", bgColor) != UT_OK)
		bgColor = NULL;

	//OpenXML supports bookmarks anywhere in the tables
	//We will append children bookmarks that go inside table here
	//to point to the beginning of table instead of correct locations
	//TODO: this needs to be fixed in piece table?
	OXML_ElementVector children = getChildren();
	OXML_ElementVector::size_type i;
	for (i = 0; i < children.size(); i++)
	{
		if(bgColor)
		{
			children[i]->setProperty("background-color", bgColor); //apply directly to row
		}
					
		if(children[i]->getTag() == BOOK_TAG)
		{
			ret = children[i]->addToPT(pDocument);
			if (ret != UT_OK)
				return ret;
		}
	}

	const gchar ** atts = getAttributesWithProps();
	if(!pDocument->appendStrux(PTX_SectionTable, atts))
		return UT_ERROR;
	
	ret = addChildrenToPT(pDocument);
	if(ret != UT_OK)
		return ret;
	
	if(!pDocument->appendStrux(PTX_EndTable,NULL))
		return UT_ERROR;

	return ret;
}

int OXML_Element_Table::getCurrentRowNumber() const
{
	return m_currentRowNumber;
}

int OXML_Element_Table::getCurrentColNumber() const
{
	return m_currentColNumber;
}

void OXML_Element_Table::setCurrentRowNumber(int row)
{
	m_currentRowNumber = row;
}

void OXML_Element_Table::setCurrentColNumber(int col)
{
	m_currentColNumber = col;
}

void OXML_Element_Table::incrementCurrentRowNumber()
{
	m_currentRowNumber++;
}

void OXML_Element_Table::incrementCurrentColNumber()
{
	m_currentColNumber++;
}

void OXML_Element_Table::addRow(OXML_Element_Row* row)
{
	m_rows.push_back(row);
	row->inheritProperties(this);
}

std::string OXML_Element_Table::getColumnWidth(int colIndex) const
{
	if((colIndex < 0) || (colIndex >= (int)columnWidth.size()))
		return "0in"; 
	return columnWidth.at(colIndex);
}

std::string OXML_Element_Table::getRowHeight(int rowIndex) const
{
	if((rowIndex < 0) || (rowIndex >= (int)rowHeight.size()))
		return "0in"; 
	return rowHeight.at(rowIndex);
}

bool OXML_Element_Table::incrementBottomVerticalMergeStart(OXML_Element_Cell* cell)
{
	std::vector<OXML_Element_Row*>::reverse_iterator rit;
	for( rit=m_rows.rbegin(); rit < m_rows.rend(); ++rit )
	{
		OXML_Element_Row* pRow = *rit;
		if(pRow->incrementBottomVerticalMergeStart(cell))
			return true;
	}
	return false;	
}

bool OXML_Element_Table::incrementRightHorizontalMergeStart(OXML_Element_Cell* cell)
{
	std::vector<OXML_Element_Row*>::reverse_iterator rit;
	for( rit=m_rows.rbegin(); rit < m_rows.rend(); ++rit )
	{
		OXML_Element_Row* pRow = *rit;
		if(pRow->incrementRightHorizontalMergeStart(cell))
			return true;
		cell->setTop(cell->getTop()-1); //decrement top if we can't find the starting cell in this row
	}
	return false;	
}

void OXML_Element_Table::addMissingCell(unsigned int rowNumber, OXML_Element_Cell* cell)
{
	OXML_ElementVector::size_type i;
	OXML_ElementVector children = getChildren();
	for (i = 0; i < children.size(); i++)
	{
		OXML_Element_Row* r = static_cast<OXML_Element_Row*>(children[i].get());
		if(i == rowNumber)
		{
			r->addMissingCell(cell);
			return;
		}
	}
}

void OXML_Element_Table::applyStyle(OXML_SharedStyle style)
{
	inheritProperties(style.get());
}
