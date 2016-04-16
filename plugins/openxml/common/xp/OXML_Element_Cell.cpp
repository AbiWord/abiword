/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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
#include <OXML_Element_Cell.h>

// AbiWord includes
#include "ut_types.h"
#include "ut_std_string.h"
#include "ut_string.h"
#include "pd_Document.h"

OXML_Element_Cell::OXML_Element_Cell(const std::string & id, OXML_Element_Table* tbl, OXML_Element_Row* rw,
									 UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bottom) : 
	OXML_Element(id, TC_TAG, CELL),
	m_iLeft(left), 
	m_iRight(right), 
	m_iTop(top), 
	m_iBottom(bottom),
	m_startVerticalMerge(true),
	m_startHorizontalMerge(true),
	m_table(tbl),
	m_row(rw),
	m_horizontalTail(NULL),
	m_verticalTail(NULL)
{
	if(rw)
		rw->addCell(this);
}

OXML_Element_Cell::~OXML_Element_Cell()
{

}

UT_Error OXML_Element_Cell::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;

	err = exporter->startCell();
	if(err != UT_OK)
		return err;

	err = this->serializeProperties(exporter);
	if(err != UT_OK)
		return err;

	err = this->serializeChildren(exporter);
	if(err != UT_OK)
		return err;

	return exporter->finishCell();
}

UT_Error OXML_Element_Cell::serializeProperties(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;
	const gchar* szValue = NULL;
	const gchar* borderType = NULL;
	const gchar* color = NULL;
	const gchar* size = NULL;

	err = exporter->startCellProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	UT_sint32 hspan = getRight()-getLeft();
	UT_sint32 vspan = getBottom()-getTop();
	bool isVertCont = getTop() == -1;
	
	err = exporter->setColumnWidth(TARGET_DOCUMENT, m_table->getColumnWidth(getLeft()).c_str());
	if(err != UT_OK)
		return err;

	if(getProperty("background-color", szValue) == UT_OK)
	{
		err = exporter->setBackgroundColor(TARGET_DOCUMENT, szValue);
		if(err != UT_OK)
			return err;

		const gchar * bgColor = szValue;

		OXML_ElementVector children = getChildren();
		OXML_ElementVector::size_type i;
		for (i = 0; i < children.size(); i++)
		{
			if(children[i]->getTag() == TBL_TAG)
			{
				if((children[i]->getProperty("background-color", szValue) != UT_OK) || !szValue)
				{			
					children[i]->setProperty("background-color", bgColor);
				}
			}
			else if((children[i]->getProperty("bgcolor", szValue) != UT_OK) || !szValue)
			{			
				children[i]->setProperty("bgcolor", bgColor);
			}
		}
	}
	
	err = exporter->startCellBorderProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

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

	if(!isVertCont)
	{
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
	}

	if(vspan == 1)
	{
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
	}

	err = exporter->finishCellBorderProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	if(hspan > 1)
	{
		err = exporter->setGridSpan(TARGET_DOCUMENT, hspan);
		if(err != UT_OK)
			return err;
	}

	if(vspan > 1)
	{
		if(!isVertCont)
		{
			err = exporter->setVerticalMerge(TARGET_DOCUMENT, "restart");
			if(err != UT_OK)
				return err;
		}
		
		//add the remaining part of the cell as a missing cell
		OXML_Element_Cell* missingCell = new OXML_Element_Cell("", m_table, NULL, m_iLeft, m_iRight, -1, 1); //vertically continued cell
		OXML_SharedElement dummy(new OXML_Element_Paragraph(""));
		missingCell->appendElement(dummy); //cells can't be empty!
		m_table->addMissingCell(m_row->getRowNumber()+1, missingCell);
	}

	if(isVertCont)
	{
		err = exporter->setVerticalMerge(TARGET_DOCUMENT, "continue");
		if(err != UT_OK)
			return err;
	}

	return exporter->finishCellProperties(TARGET_DOCUMENT);
}


UT_Error OXML_Element_Cell::addToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;

	if(!startsHorizontalMerge() || !startsVerticalMerge())
		return UT_OK;

	//add props:bot-attach, left-attach, right-attach, top-attach
	std::string sTop = UT_std_string_sprintf("%d", m_iTop);
	std::string sBottom = UT_std_string_sprintf("%d", m_iBottom);
	std::string sLeft = UT_std_string_sprintf("%d", m_iLeft);
	std::string sRight = UT_std_string_sprintf("%d", m_iRight);

	ret = setProperty("top-attach", sTop);
	if(ret != UT_OK)
		return ret;	

	ret = setProperty("bot-attach", sBottom);
	if(ret != UT_OK)
		return ret;	

	ret = setProperty("left-attach", sLeft);
	if(ret != UT_OK)
		return ret;	

	ret = setProperty("right-attach", sRight);
	if(ret != UT_OK)
		return ret;	



	const gchar * szValue = NULL;
	const gchar * bgColor = NULL;

	if((getProperty("background-color", bgColor) == UT_OK) && bgColor)
	{
		OXML_ElementVector children = getChildren();
		OXML_ElementVector::size_type i;
		for (i = 0; i < children.size(); i++)
		{
			if(children[i]->getTag() == TBL_TAG)
			{
				if((children[i]->getProperty("background-color", szValue) != UT_OK) || !szValue)
				{			
					children[i]->setProperty("background-color", bgColor);
				}
			}
			else if((children[i]->getProperty("bgcolor", szValue) != UT_OK) || !szValue)
			{			
				children[i]->setProperty("bgcolor", bgColor);
			}
		}
	}

	if(!bgColor)
		bgColor = "ffffff";

	if((getProperty("top-style", szValue) != UT_OK) || !szValue)
	{
		ret = setProperty("top-color", bgColor); 
		if(ret != UT_OK)
			return ret;	
	}		

	szValue = NULL;
	if((getProperty("left-style", szValue) != UT_OK) || !szValue)
	{
		ret = setProperty("left-color", bgColor); 
		if(ret != UT_OK)
			return ret;	
	}

	szValue = NULL;
	if((getProperty("right-style", szValue) != UT_OK) || !szValue)
	{
		ret = setProperty("right-color", bgColor); 
		if(ret != UT_OK)
			return ret;	
	}

	szValue = NULL;
	if((getProperty("bot-style", szValue) != UT_OK) || !szValue)
	{
		ret = setProperty("bot-color", bgColor); 
		if(ret != UT_OK)
			return ret;	
	}

	const PP_PropertyVector cell_props = getAttributesWithProps();

	if(!pDocument->appendStrux(PTX_SectionCell, cell_props))
		return UT_ERROR;

	ret = addChildrenToPT(pDocument);
	if(ret != UT_OK)
		return ret;

	if(!pDocument->appendStrux(PTX_EndCell, PP_NOPROPS))
		return UT_ERROR;

	return ret;
}

void OXML_Element_Cell::setLeft(UT_sint32 left)
{
	m_iLeft = left;
}

void OXML_Element_Cell::setRight(UT_sint32 right)
{
	m_iRight = right;
}

void OXML_Element_Cell::setTop(UT_sint32 top)
{
	m_iTop = top;
}

void OXML_Element_Cell::setBottom(UT_sint32 bottom)
{
	m_iBottom = bottom;
}


//start=false for vertical merge = continous cells
void OXML_Element_Cell::setVerticalMergeStart(bool start)
{
	m_startVerticalMerge = start;
}

//start=false for horizontal merge = continous cells
void OXML_Element_Cell::setHorizontalMergeStart(bool start)
{
	m_startHorizontalMerge = start;
}

void OXML_Element_Cell::setLastHorizontalContinuationCell(OXML_Element_Cell* cell)
{
	if(!cell)
		return;

	m_horizontalTail = cell;

	const gchar* szValue = NULL;

	m_horizontalTail->getProperty("right-color", szValue);
	if(szValue)
		setProperty("right-color", szValue);

	szValue = NULL;
	m_horizontalTail->getProperty("right-style", szValue);
	if(szValue)
		setProperty("right-style", szValue);
	
	szValue = NULL;
	m_horizontalTail->getProperty("right-thickness", szValue);
	if(szValue)
		setProperty("right-thickness", szValue);
}

void OXML_Element_Cell::setLastVerticalContinuationCell(OXML_Element_Cell* cell)
{
	if(!cell)
		return;

	m_verticalTail = cell;

	const gchar* szValue = NULL;

	m_verticalTail->getProperty("bot-color", szValue);
	if(szValue)
		setProperty("bot-color", szValue);

	szValue = NULL;
	m_verticalTail->getProperty("bot-style", szValue);
	if(szValue)
		setProperty("bot-style", szValue);
	
	szValue = NULL;
	m_verticalTail->getProperty("bot-thickness", szValue);
	if(szValue)
		setProperty("bot-thickness", szValue);
}
