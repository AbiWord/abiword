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

OXML_Element_Cell::OXML_Element_Cell(std::string id, UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bottom) : 
	OXML_Element(id, TC_TAG, CELL),
	m_iLeft(left), 
	m_iRight(right), 
	m_iTop(top), 
	m_iBottom(bottom)
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

	err = exporter->startCellProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	UT_sint32 hspan = getRight()-getLeft();
	UT_sint32 vspan = getBottom()-getTop();
	bool isVertCont = getTop() == -1;

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


UT_Error OXML_Element_Cell::addToPT(PD_Document * /*pDocument*/)
{
	//TODO
	return UT_OK;
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
