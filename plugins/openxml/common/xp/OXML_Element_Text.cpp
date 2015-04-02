/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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
#include <OXML_Element_Text.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_Text::OXML_Element_Text() : 
	OXML_Element("", T_TAG, SPAN), 
	m_pString(NULL), 
	m_range(UNKNOWN_RANGE)
{
}

OXML_Element_Text::~OXML_Element_Text()
{
	DELETEP(m_pString);
}

OXML_Element_Text::OXML_Element_Text(const gchar * text, int length) :
	OXML_Element("", T_TAG, SPAN),
	m_pString(NULL)
{
	setText(text, length);
}

void OXML_Element_Text::setText(const gchar * text, int /*length*/)
{
	DELETEP(m_pString);
	try {
		std::string str(text);
		m_pString = new UT_UCS4String(str);
	} catch(...) {
		m_pString = NULL;
	}
}

const UT_UCS4Char * OXML_Element_Text::getText_UCS4String()
{
	UT_return_val_if_fail(m_pString != NULL, NULL);
	return m_pString->ucs4_str();
}

const char * OXML_Element_Text::getText()
{
	UT_return_val_if_fail(m_pString != NULL, NULL);
	if(getType() == LIST)
	{
		const char* pStr = m_pString->utf8_str();
		if(pStr && (strlen(pStr) > 0) && (pStr[0] == '\t'))
			return pStr+1; //get rid of the initial tab
	}
	return m_pString->utf8_str();
}

UT_Error OXML_Element_Text::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;
	bool bList = false;
	const gchar* szValue = NULL;
	err = getAttribute("style", szValue);
	if(err == UT_OK && szValue)
	{
		if(!strcmp(szValue, "List Paragraph"))
		{
			bList = true;
		}
	}
	err = getAttribute("type", szValue);
	if(err == UT_OK && szValue)
	{
		if(!strcmp(szValue, "list_label"))
		{
			bList = true;
		}
	}
	err = getProperty("list-style", szValue);
	if(err == UT_OK && szValue)
	{
		bList = true;
	}

	err = exporter->startText(TARGET);
	if(err != UT_OK)
		return err;

	const UT_UCS4Char * text = getText_UCS4String();
	if(text)
		err = exporter->writeText(TARGET, text, bList);

	if(err != UT_OK)
		return err;
	
	return exporter->finishText(TARGET);
}

UT_Error OXML_Element_Text::addToPT(PD_Document * pDocument)
{
	UT_return_val_if_fail(pDocument != NULL && m_pString != NULL, UT_ERROR);

	//a text tag does not have children, so no need to call addChildrenToPT()
	return pDocument->appendSpan(m_pString->ucs4_str(), m_pString->length()) ? UT_OK : UT_ERROR; 
}

