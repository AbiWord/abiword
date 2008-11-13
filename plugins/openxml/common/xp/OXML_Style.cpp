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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// Class definition include
#include <OXML_Style.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Document.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_misc.h>
#include <pd_Document.h>

// External includes
#include <string>

OXML_Style::OXML_Style(const std::string & id, const std::string & name) : 
	OXML_ObjectWithAttrProp(), 
	m_id(id), 
	m_name(name),
	m_basedon(""),
	m_followedby("")
{
	setAttribute(PT_NAME_ATTRIBUTE_NAME, name.c_str());
}

OXML_Style::~OXML_Style()
{
}

UT_Error OXML_Style::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;
	const gchar* szValue = NULL;
	
	err = exporter->startStyle(m_name, m_basedon, m_followedby);
	if(err != UT_OK)
		return err;

	// PARAGRAPH PROPERTIES

	err = exporter->startParagraphProperties(TARGET_STYLES);
	if(err != UT_OK)
		return err;

	if(getProperty("widows", szValue) == UT_OK)
	{
		err = exporter->setWidows(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("text-align", szValue) == UT_OK)
	{
		if(!strcmp(szValue, "justify"))
		{
			err = exporter->setTextAlignment(TARGET_STYLES, "both");
		}
		else if(!strcmp(szValue, "center"))
		{
			err = exporter->setTextAlignment(TARGET_STYLES, "center");
		}
		else if(!strcmp(szValue, "right"))
		{
			err = exporter->setTextAlignment(TARGET_STYLES, "right");
		}
		else if(!strcmp(szValue, "left"))
		{
			err = exporter->setTextAlignment(TARGET_STYLES, "left");
		}

		if(err != UT_OK)
			return err;
	}

	if(getProperty("text-indent", szValue) == UT_OK)
	{
		err = exporter->setTextIndentation(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("margin-left", szValue) == UT_OK)
	{
		err = exporter->setParagraphLeftMargin(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("margin-right", szValue) == UT_OK)
	{
		err = exporter->setParagraphRightMargin(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("margin-bottom", szValue) == UT_OK)
	{
		err = exporter->setParagraphBottomMargin(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("margin-top", szValue) == UT_OK)
	{
		err = exporter->setParagraphTopMargin(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("line-height", szValue) == UT_OK)
	{
		err = exporter->setLineHeight(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	err = exporter->finishParagraphProperties(TARGET_STYLES);
	if(err != UT_OK)
		return err;

	// RUN PROPERTIES

	err = exporter->startRunProperties(TARGET_STYLES);
	if(err != UT_OK)
		return err;

	if(getProperty("font-weight", szValue) == UT_OK)
	{
		if(!strcmp(szValue, "bold"))
		{
			err = exporter->setBold(TARGET_STYLES);
			if(err != UT_OK)
				return err;
		}
	}

	if(getProperty("font-style", szValue) == UT_OK)
	{
		if(!strcmp(szValue, "italic"))
		{
			err = exporter->setItalic(TARGET_STYLES);
			if(err != UT_OK)
				return err;
		}
	}

	if(getProperty("font-size", szValue) == UT_OK)
	{
		err = exporter->setFontSize(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("font-family", szValue) == UT_OK)
	{
		err = exporter->setFontFamily(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}
	
	if(getProperty("text-decoration", szValue) == UT_OK)
	{
		if(strstr(szValue, "underline"))
		{
			err = exporter->setUnderline(TARGET_STYLES);
			if(err != UT_OK)
				return err;
		}

		if(strstr(szValue, "overline"))
		{
			err = exporter->setOverline(TARGET_STYLES);
			if(err != UT_OK)
				return err;
		}

		if(strstr(szValue, "line-through"))
		{
			err = exporter->setLineThrough(TARGET_STYLES);
			if(err != UT_OK)
				return err;
		}
	}

	if(getProperty("text-position", szValue) == UT_OK)
	{
		if(!strcmp(szValue, "superscript"))
		{
			err = exporter->setSuperscript(TARGET_STYLES);
			if(err != UT_OK)
				return err;
		}

		else if(!strcmp(szValue, "subscript"))
		{
			err = exporter->setSubscript(TARGET_STYLES);
			if(err != UT_OK)
				return err;
		}
	}

	if(getProperty("color", szValue) == UT_OK)
	{
		err = exporter->setTextColor(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	if(getProperty("bgcolor", szValue) == UT_OK)
	{
		err = exporter->setBackgroundColor(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

	err = exporter->finishRunProperties(TARGET_STYLES);
	if(err != UT_OK)
		return err;

	return exporter->finishStyle();
}

UT_Error OXML_Style::addToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;

	//First, we change the ID reference for BASEDON and FOLLOWEDBY to a name reference.
	OXML_Document * doc = OXML_Document::getInstance();
	UT_return_val_if_fail( doc != NULL, UT_ERROR );

	const gchar * buf = NULL;
	getAttribute(PT_BASEDON_ATTRIBUTE_NAME, buf);
	if (buf != NULL) {
		OXML_SharedStyle other = doc->getStyleById(buf);
		if (other.get() != NULL) {
			setAttribute(PT_BASEDON_ATTRIBUTE_NAME, other->getName().c_str());
		} else {
			setAttribute(PT_BASEDON_ATTRIBUTE_NAME, "Normal");
		}
	} else {
		//We base all styles on Normal if nothing else
		setAttribute(PT_BASEDON_ATTRIBUTE_NAME, "Normal");
	}

	getAttribute(PT_FOLLOWEDBY_ATTRIBUTE_NAME, buf);
	if (buf != NULL) {
		OXML_SharedStyle other = doc->getStyleById(buf);
		if (other)
			setAttribute(PT_FOLLOWEDBY_ATTRIBUTE_NAME, other->getName().c_str());
	}


	const gchar ** atts = getAttributesWithProps();
	if (atts != NULL) ret = pDocument->appendStyle(atts) ? UT_OK : UT_ERROR;
	if(ret != UT_OK)
	{
		UT_ASSERT_HARMLESS(ret == UT_OK);
		return ret;
	}
	return UT_OK;
}

