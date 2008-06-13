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
	m_name(name)
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
	
	err = exporter->startStyle(m_name.c_str());
	if(err != UT_OK)
		return err;

	//TODO: add more properties
	if(getProperty("font-size", szValue) == UT_OK)
	{
		err = exporter->setFontSize(TARGET_STYLES, szValue);
		if(err != UT_OK)
			return err;
	}

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

