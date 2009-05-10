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
#include <OXML_Element_Cell.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

//External includes
#include <sstream>

OXML_Element_Cell::OXML_Element_Cell(std::string id, OXML_Element_Table* tbl, OXML_Element_Row* rw,
									 UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bottom) : 
	OXML_Element(id, TC_TAG, CELL),
	m_iLeft(left), 
	m_iRight(right), 
	m_iTop(top), 
	m_iBottom(bottom),
	table(tbl),
	row(rw)
{

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
	
	err = exporter->setColumnWidth(TARGET_DOCUMENT, table->getColumnWidth(getLeft()).c_str());
	if(err != UT_OK)
		return err;

	if(getProperty("background-color", szValue) == UT_OK)
	{
		err = exporter->setBackgroundColor(TARGET_DOCUMENT, szValue);
		if(err != UT_OK)
			return err;
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
		err = exporter->setVerticalMerge(TARGET_DOCUMENT, "restart");
		if(err != UT_OK)
			return err;
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

	m_iTop = row->getRowNumber(); //top
	m_iLeft = row->getCurrentColumnNumber(); //left
	m_iBottom = m_iTop + 1;
	m_iRight = m_iLeft + 1; //TODO: merged cells not implemented yet
	
	//add props:bot-attach, left-attach, right-attach, top-attach
	std::stringstream out;
	out << m_iTop;
	std::string sTop = out.str();
	out.str("");
	out << m_iBottom;
	std::string sBottom = out.str();
	out.str("");
	out << m_iLeft;
	std::string sLeft = out.str();
	out.str("");
	out << m_iRight;
	std::string sRight = out.str();
	
	ret = setProperty("top-attach", sTop.c_str());
	if(ret != UT_OK)
		return ret;	

	ret = setProperty("bot-attach", sBottom.c_str());
	if(ret != UT_OK)
		return ret;	

	ret = setProperty("left-attach", sLeft.c_str());
	if(ret != UT_OK)
		return ret;	

	ret = setProperty("right-attach", sRight.c_str());
	if(ret != UT_OK)
		return ret;	

	const gchar** cell_props = getAttributesWithProps();

	if(!pDocument->appendStrux(PTX_SectionCell, cell_props))
		return UT_ERROR;

	ret = addChildrenToPT(pDocument);
	if(ret != UT_OK)
		return ret;

	if(!pDocument->appendStrux(PTX_EndCell,NULL))
		return UT_ERROR;
	
	return ret;
}


UT_sint32 OXML_Element_Cell::getLeft()
{
	return m_iLeft;
}

UT_sint32 OXML_Element_Cell::getRight()
{
	return m_iRight;
}

UT_sint32 OXML_Element_Cell::getTop()
{
	return m_iTop;
}

UT_sint32 OXML_Element_Cell::getBottom()
{
	return m_iBottom;
}
