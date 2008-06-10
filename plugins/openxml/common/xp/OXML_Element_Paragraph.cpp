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
#include <OXML_Element_Paragraph.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_Paragraph::OXML_Element_Paragraph(std::string id) : 
	OXML_Element(id, P_TAG, BLOCK)
{
}

OXML_Element_Paragraph::~OXML_Element_Paragraph()
{

}

UT_Error OXML_Element_Paragraph::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;

	err = exporter->startParagraph();
	if(err != UT_OK)
		return err;

	err = this->serializeProperties(exporter);
	if(err != UT_OK)
		return err;

	err = this->serializeChildren(exporter);
	if(err != UT_OK)
		return err;

	return exporter->finishParagraph();
}

UT_Error OXML_Element_Paragraph::serializeProperties(IE_Exp_OpenXML* exporter)
{
	//TODO: Add all the property serializations here
	UT_Error err = UT_OK;
	const gchar* szValue = NULL;

	err = exporter->startParagraphProperties();
	if(err != UT_OK)
		return err;

	if(getProperty("text-align", szValue) == UT_OK)
	{
		if(!strcmp(szValue, "justify"))
		{
			err = exporter->setTextAlignment("both");
		}
		else if(!strcmp(szValue, "center"))
		{
			err = exporter->setTextAlignment("center");
		}
		else if(!strcmp(szValue, "right"))
		{
			err = exporter->setTextAlignment("right");
		}
		else if(!strcmp(szValue, "left"))
		{
			err = exporter->setTextAlignment("left");
		}

		if(err != UT_OK)
			return err;
	}

	if(getProperty("text-indent", szValue) == UT_OK)
	{
		err = exporter->setTextIndentation(szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("margin-left", szValue) == UT_OK)
	{
		err = exporter->setParagraphLeftMargin(szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("margin-right", szValue) == UT_OK)
	{
		err = exporter->setParagraphRightMargin(szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("margin-bottom", szValue) == UT_OK)
	{
		err = exporter->setParagraphBottomMargin(szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("margin-top", szValue) == UT_OK)
	{
		err = exporter->setParagraphTopMargin(szValue);
		if(err != UT_OK)
			return err;
	}

	return exporter->finishParagraphProperties();
}


UT_Error OXML_Element_Paragraph::addToPT(PD_Document * pDocument)
{
	//TODO Move the OXML_Element addToPT case P_TAG code here
	//the importer needs to be updated to create OXML_Element_Paragraph object
	//instead of generic OXML_Element object for representing a paragraph
	return OXML_Element::addToPT(pDocument);
}
