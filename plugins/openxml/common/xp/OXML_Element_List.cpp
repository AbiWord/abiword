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
#include <OXML_Element_List.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_List::OXML_Element_List(std::string id) : 
	OXML_Element(id, LST_TAG, LIST)
{
}

OXML_Element_List::~OXML_Element_List()
{

}

UT_Error OXML_Element_List::serialize(IE_Exp_OpenXML* exporter)
{
	//TODO: serialize list here
	return serializeProperties(exporter);
}

UT_Error OXML_Element_List::serializeProperties(IE_Exp_OpenXML* exporter)
{
	//TODO: Add all the property serializations here
	UT_Error err = UT_OK;
	const gchar* szValue = NULL;

	err = exporter->startListProperties(TARGET_DOCUMENT);
	if(err != UT_OK)
		return err;

	err = exporter->setListLevel(TARGET_DOCUMENT, "0");
	if(err != UT_OK)
		return err;

	err = exporter->setListFormat(TARGET_DOCUMENT, "1");
	if(err != UT_OK)
		return err;

	return exporter->finishListProperties(TARGET_DOCUMENT);
}


UT_Error OXML_Element_List::addToPT(PD_Document * /*pDocument*/)
{
	//TODO
	return UT_OK;
}
