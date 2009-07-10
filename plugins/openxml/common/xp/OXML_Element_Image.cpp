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
#include <OXML_Element_Image.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_Image::OXML_Element_Image(const std::string & id) : 
	OXML_Element(id, IMG_TAG, IMAGE)
{
}

OXML_Element_Image::~OXML_Element_Image()
{

}

UT_Error OXML_Element_Image::serialize(IE_Exp_OpenXML* exporter)
{
	//TODO: serialize image here
	UT_Error err = UT_OK;
	const gchar* szValue;
	const gchar* height;
	const gchar* width;

	if(getAttribute("dataid", szValue) == UT_OK)
	{

		if(getProperty("height", height) != UT_OK)
			height = "1.0in";
					
		if(getProperty("width", width) != UT_OK)
			width = "1.0in";

		UT_UTF8String sEscValue = szValue;
		sEscValue.escapeXML();

		std::string filename("");
		filename += sEscValue.utf8_str();

		std::string extension;
		if(!exporter->getDoc()->getDataItemFileExtension(szValue, extension))
			extension = ".png";
		filename += extension;

		std::string relId("rId");
		relId += getId();

		err = exporter->setImageRelation(filename.c_str(), relId.c_str());
		if(err != UT_OK)
			return err;
		
		err = exporter->setImage(getId().c_str(), relId.c_str(), filename.c_str(), width, height);
		if(err != UT_OK)
			return err;
	}

	return UT_OK;
}

UT_Error OXML_Element_Image::addToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;
	
	ret = setAttribute("dataid", getId().c_str());
	if(ret != UT_OK)
		return ret;

	const gchar ** atts = getAttributesWithProps();

	if(!pDocument->appendObject(PTO_Image, atts))
		return UT_ERROR;

	return UT_OK;
}
