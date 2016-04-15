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
#include <OXML_Element_Row.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_Row::OXML_Element_Row(const std::string & id, OXML_Element_Table* tbl) : 
	OXML_Element(id, TR_TAG, ROW), numCols(0), table(tbl),
	m_rowNumber(0), m_currentColumnNumber(0)
{
	if(tbl)	
		tbl->addRow(this);
}

OXML_Element_Row::~OXML_Element_Row()
{

}

UT_Error OXML_Element_Row::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;

	m_rowNumber = table->getCurrentRowNumber();
	err = exporter->startRow();
	if(err != UT_OK)
		return err;

	err = this->serializeProperties(exporter);
	if(err != UT_OK)
		return err;

	err = this->serializeChildren(exporter);
	if(err != UT_OK)
		return err;

	return exporter->finishRow();
}

UT_Error OXML_Element_Row::serializeChildren(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;
	
	OXML_ElementVector children = getChildren();
	
	UT_sint32 left = 0; 
	OXML_Element_Cell* cell = NULL;

	//during the loop check to see if we are missing any cells due to vertical merging
	//if so let's add them manually
	OXML_ElementVector::size_type i;
	for(i=0; i < children.size(); i++)
	{
		cell = static_cast<OXML_Element_Cell*>(children[i].get());
		
		//go through missing cells and serialize the correct ones
		std::vector<OXML_Element_Cell*>::const_iterator it;
		for( it = m_missingCells.begin(); it < m_missingCells.end() && (left < cell->getLeft()); ++it )
		{
			OXML_Element_Cell* pCell = *it;
			if((pCell->getLeft() == left)) 
			{ //found missing cell
				left = pCell->getRight();
				ret = pCell->serialize(exporter);
				if(ret != UT_OK)
					return ret;			
			}
		}
			
		left = cell->getRight();

		ret = cell->serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}

	//right most vertically merged cells
	for(; left < numCols; left++){
		OXML_Element_Cell temp("", table, this, left, left+1, -1, 0); 
		OXML_SharedElement shared_paragraph(new OXML_Element_Paragraph(""));

		ret = temp.appendElement(shared_paragraph);
		if(ret != UT_OK)
			return ret;			

		ret = temp.serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}

	return ret;
}


UT_Error OXML_Element_Row::serializeProperties(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;

	err = exporter->startRowProperties(TARGET);
	if(err != UT_OK)
		return err;

	std::string height = table->getRowHeight(m_rowNumber);
	if(height.compare("0in"))
	{
		err = exporter->setRowHeight(TARGET, height.c_str());
		if(err != UT_OK)
			return err;	
	}

	return exporter->finishRowProperties(TARGET);
}

UT_Error OXML_Element_Row::addChildrenToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;
	UT_Error temp = UT_OK;

	const gchar * szValue = NULL;
	const gchar * bgColor = NULL;

	getProperty("background-color", bgColor);

	std::vector<OXML_Element*>::size_type i;
	OXML_ElementVector children = getChildren();
	for (i = 0; i < children.size(); i++)
	{
		m_currentColumnNumber = i;

		if(bgColor && ((children[i]->getProperty("background-color", szValue) != UT_OK) || !szValue))
		{			
			children[i]->setProperty("background-color", bgColor);
		}

		temp = children[i]->addToPT(pDocument);
		if (temp != UT_OK)
			ret = temp;
	}
	return ret;
}

UT_Error OXML_Element_Row::addToPT(PD_Document * pDocument)
{
	m_rowNumber = table->getCurrentRowNumber();
	return addChildrenToPT(pDocument);
}

void OXML_Element_Row::setNumCols(UT_sint32 columns)
{
	numCols = columns;	
}

void OXML_Element_Row::setRowNumber(int row)
{
	m_rowNumber = row;
}


void OXML_Element_Row::addCell(OXML_Element_Cell* cell)
{
	m_cells.push_back(cell);
	cell->inheritProperties(this);
}

void OXML_Element_Row::addMissingCell(OXML_Element_Cell* cell)
{
	m_missingCells.push_back(cell);
	cell->setRow(this);
}

bool OXML_Element_Row::incrementBottomVerticalMergeStart(OXML_Element_Cell* cell)
{
	int top = cell->getTop();
	int left = cell->getLeft();

	std::vector<OXML_Element_Cell*>::const_iterator it;
	for( it = m_cells.begin(); it < m_cells.end(); ++it )
	{
		OXML_Element_Cell* pCell = *it;
		if((pCell->getLeft() == left) && (pCell->getTop() < top) && pCell->startsVerticalMerge())
		{
			pCell->setBottom(pCell->getBottom()+1);
			pCell->setLastVerticalContinuationCell(cell);
			return true;
		}
	}
	return false;	
}

bool OXML_Element_Row::incrementRightHorizontalMergeStart(OXML_Element_Cell* cell)
{
	int top = cell->getTop();
	int left = cell->getLeft();

	std::vector<OXML_Element_Cell*>::reverse_iterator it;
	for( it = m_cells.rbegin(); it < m_cells.rend(); ++it )
	{
		OXML_Element_Cell* pCell = *it;
		if((pCell->getTop() == top) && (pCell->getLeft() < left) && pCell->startsHorizontalMerge())
		{
			pCell->setRight(pCell->getRight()+1);
			pCell->setLastHorizontalContinuationCell(cell);
			return true;
		}
	}
	return false;	
}
