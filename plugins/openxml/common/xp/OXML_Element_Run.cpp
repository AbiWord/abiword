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
#include <OXML_Element_Run.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_misc.h>
#include <pd_Document.h>

// External includes
#include <string>

OXML_Element_Run::OXML_Element_Run(std::string id) : 
	OXML_Element(id, R_TAG, SPAN)
{
	//Intentionally empty
}

OXML_Element_Run::~OXML_Element_Run()
{

}

UT_Error OXML_Element_Run::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;

	err = exporter->startRun();
	if(err != UT_OK)
		return err;

	err = this->serializeChildren(exporter);
	if(err != UT_OK)
		return err;

	return exporter->finishRun();
}

UT_Error OXML_Element_Run::addToPT(PD_Document * pDocument)
{
	UT_return_val_if_fail(pDocument != NULL, UT_ERROR);

	UT_Error ret = UT_OK;

	const gchar ** atts = getAttributesWithProps();
	if (atts != NULL) {
		//We open the formatting tag
		ret = pDocument->appendFmt(atts) ? UT_OK : UT_ERROR;
		if(ret != UT_OK)
		{
			UT_ASSERT_HARMLESS(ret == UT_OK);
			return ret;
		}
	}

	ret = addChildrenToPT(pDocument);
	if(ret != UT_OK)
	{
		UT_ASSERT_HARMLESS(ret == UT_OK);
		return ret;
	}

	if (atts != NULL) {
		//We close the formatting tag
		ret = pDocument->appendFmt((const gchar **)NULL) ? UT_OK : UT_ERROR;
		UT_return_val_if_fail(ret == UT_OK, ret);
	}
	return ret;
}

