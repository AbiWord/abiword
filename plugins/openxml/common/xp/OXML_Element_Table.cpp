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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// Class definition include
#include <OXML_Element_Table.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_Table::OXML_Element_Table(std::string id) : 
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

	err = exporter->startTableProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	err = exporter->startTableBorderProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	err = exporter->finishTableBorderProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	return exporter->finishTableProperties(TARGET_DOCUMENT);
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
		temp = children[i]->addToPT(pDocument);
		if (temp != UT_OK)
			ret = temp;
	}
	return ret;
}

UT_Error OXML_Element_Table::addToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;
	if(!pDocument->appendStrux(PTX_SectionTable, NULL))
		return UT_ERROR;

	ret = addChildrenToPT(pDocument);
	if(ret != UT_OK)
		return ret;

	if(!pDocument->appendStrux(PTX_EndTable,NULL))
		return UT_ERROR;

	return ret;
}

int OXML_Element_Table::getCurrentRowNumber()
{
	return m_currentRowNumber;
}

int OXML_Element_Table::getCurrentColNumber()
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
}

std::string OXML_Element_Table::getColumnWidth(int colIndex)
{
	if((colIndex < 0) || (colIndex >= (int)columnWidth.size()))
		return "0in"; 
	return columnWidth.at(colIndex);
}

bool OXML_Element_Table::incrementBottomVerticalMergeStart(int left, int top)
{
	std::vector<OXML_Element_Row*>::reverse_iterator rit;
	for( rit=m_rows.rbegin(); rit < m_rows.rend(); ++rit )
	{
		OXML_Element_Row* pRow = *rit;
		if(pRow->incrementBottomVerticalMergeStart(left, top))
			return true;
	}
	return false;	
}
