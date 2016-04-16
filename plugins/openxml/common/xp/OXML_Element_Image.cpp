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
#include <OXML_Element_Image.h>

// AbiWord includes
#include <ut_std_string.h>
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
	UT_Error err = UT_OK;
	const gchar* szValue;
	const gchar* height = "1.0in";
	const gchar* width = "1.0in";
	const gchar* xpos = "0.0in";
	const gchar* ypos = "0.0in";
	const gchar* wrapMode = NULL;
	bool bPositionedImage = false;

	bPositionedImage = (getAttribute("strux-image-dataid", szValue) == UT_OK);

	if (!bPositionedImage)
	{
		getAttribute("dataid", szValue);
	}

	std::string sEscValue = UT_escapeXML(szValue);

	std::string filename("");
	filename += sEscValue;

	std::string extension;
	if(!exporter->getDoc()->getDataItemFileExtension(szValue, extension))
		extension = ".png";
	filename += extension;

	std::string relId("rId");
	relId += getId();

	err = exporter->setImageRelation(filename.c_str(), relId.c_str());
	if(err != UT_OK)
		return err;

	if(bPositionedImage)
	{
		// positioned image
		getProperty("wrap-mode", wrapMode);
		getProperty("frame-height", height);
		getProperty("frame-width", width);
		getProperty("xpos", xpos);
		getProperty("ypos", ypos);
		err = exporter->setPositionedImage(getId().c_str(), relId.c_str(), filename.c_str(), width, height, xpos, ypos, wrapMode);
		if(err != UT_OK)
			return err;
	}
	else
	{
		// inline image
		getProperty("height", height);
		getProperty("width", width);
		err = exporter->setImage(getId().c_str(), relId.c_str(), filename.c_str(), width, height);
		if(err != UT_OK)
			return err;
	}
	return UT_OK;
}

UT_Error OXML_Element_Image::addToPT(PD_Document * pDocument)
{
	OXML_Document* doc = OXML_Document::getInstance();
	if(!doc)
	{
		return UT_OK;
	}
	OXML_SharedImage sImage = doc->getImageById(getId());
	if(!sImage)
	{
		UT_DEBUGMSG(("SERHAT: Skipping image element in import, since fail occured in import of image data previously\n"));
		return UT_OK;
	}

	UT_Error ret = UT_OK;
	bool bInline = false;
	const gchar* szValue = NULL;

	ret = getProperty("height", szValue);
	if(ret == UT_OK && szValue)
	{
		bInline = true;
	}

	if(!bInline)
	{
		ret = setProperty("frame-type", "image");
		if(ret != UT_OK)
			return ret;
	}

	if(getId().empty())
	{
		return UT_OK;
	}

	if(bInline)
	{
		ret = setAttribute("dataid", getId().c_str());
		if(ret != UT_OK)
			return ret;
	}
	else
	{
		ret = setAttribute("strux-image-dataid", getId().c_str());
		if(ret != UT_OK)
			return ret;
	}

	const PP_PropertyVector atts = getAttributesWithProps();

	if(bInline)
	{
		if(!pDocument->appendObject(PTO_Image, atts))
			return UT_ERROR;
	}
	else
	{
		ret = pDocument->appendStrux(PTX_SectionFrame, atts) ? UT_OK : UT_ERROR;
		if(ret != UT_OK)
			return ret;

		ret = this->addChildrenToPT(pDocument);
		if(ret != UT_OK)
			return ret;

		ret = pDocument->appendStrux(PTX_EndFrame, PP_NOPROPS) ? UT_OK : UT_ERROR;
		if(ret != UT_OK)
			return ret;
	}
	return UT_OK;
}
