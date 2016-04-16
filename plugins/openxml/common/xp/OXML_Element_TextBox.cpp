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
#include <OXML_Element_TextBox.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_misc.h>
#include <pd_Document.h>

// External includes
#include <string>

OXML_Element_TextBox::OXML_Element_TextBox(const std::string & id) : 
	OXML_Element(id, TXTBX_TAG, TEXTBOX)
{
	//Intentionally empty
}

OXML_Element_TextBox::~OXML_Element_TextBox()
{

}

UT_Error OXML_Element_TextBox::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;

	std::string tbId = "textboxId";
	tbId += getId();

	err = exporter->startTextBox(TARGET, tbId.c_str());
	if(err != UT_OK)
		return err;

	err = this->serializeProperties(exporter);
	if(err != UT_OK)
		return err;

	err = exporter->startTextBoxContent(TARGET);
	if(err != UT_OK)
		return err;

	err = this->serializeChildren(exporter);
	if(err != UT_OK)
		return err;

	err = exporter->finishTextBoxContent(TARGET);
	if(err != UT_OK)
		return err;

	return exporter->finishTextBox(TARGET);
}

UT_Error OXML_Element_TextBox::serializeProperties(IE_Exp_OpenXML* exporter)
{
	//TODO: Add all the property serializations here
	UT_Error err = UT_OK;
	const gchar* szValue = NULL;

	err = exporter->startTextBoxProperties(TARGET);
	if(err != UT_OK)
		return err;

	if(getProperty("frame-width", szValue) == UT_OK)
	{
		err = exporter->setTextBoxWidth(TARGET, szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("frame-height", szValue) == UT_OK)
	{
		err = exporter->setTextBoxHeight(TARGET, szValue);
		if(err != UT_OK)
			return err;
	}

	return exporter->finishTextBoxProperties(TARGET);
}

UT_Error OXML_Element_TextBox::addToPT(PD_Document* pDocument)
{
	UT_Error ret = UT_OK;

	ret = setProperty("frame-type", "textbox");
	if(ret != UT_OK)
		return ret;

	ret = setProperty("position-to", "column-above-text");
	if(ret != UT_OK)
		return ret;

	ret = setProperty("wrap-mode", "wrapped-both");
	if(ret != UT_OK)
		return ret;

	ret = setProperty("background-color", "ffffff");
	if(ret != UT_OK)
		return ret;

	ret = setProperty("bg-style", "1");
	if(ret != UT_OK)
		return ret;

	const PP_PropertyVector attr = this->getAttributesWithProps();
	ret = pDocument->appendStrux(PTX_SectionFrame, attr) ? UT_OK : UT_ERROR;
	if(ret != UT_OK)
		return ret;

	ret = this->addChildrenToPT(pDocument);
	if(ret != UT_OK)
		return ret;

	ret = pDocument->appendStrux(PTX_EndFrame, PP_NOPROPS) ? UT_OK : UT_ERROR;
	return ret;
}

