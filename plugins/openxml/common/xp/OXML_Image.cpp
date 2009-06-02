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
#include <OXML_Image.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Document.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_misc.h>
#include <pd_Document.h>

// External includes
#include <string>

OXML_Image::OXML_Image() : 
	OXML_ObjectWithAttrProp(),
	m_id(""),
	m_mimeType(""),
	m_data(NULL)
{

}

OXML_Image::~OXML_Image()
{
	if(m_data)
		delete m_data;
}

void OXML_Image::setId(const char* imageId)
{
	m_id = imageId;
}

void OXML_Image::setMimeType(const char* imageMimeType)
{
	m_mimeType = imageMimeType;
}

void OXML_Image::setData(const UT_ByteBuf* imageData)
{
	m_data = imageData;
}

const char* OXML_Image::getId()
{
	return m_id.c_str();	
}

UT_Error OXML_Image::serialize(IE_Exp_OpenXML* exporter)
{
	std::string filename = m_id;

	if(!m_mimeType.compare("") || !m_mimeType.compare("image/png"))
	{
		filename += ".png";
	}
	else if(!m_mimeType.compare("image/svg+xml"))
	{
		filename += ".svg";
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	return exporter->writeImage(filename.c_str(), m_data);
}

UT_Error OXML_Image::addToPT(PD_Document * pDocument)
{
	if (!pDocument->createDataItem(m_id.c_str(), false, m_data, (void*)m_mimeType.c_str(), NULL))
	{            
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return UT_ERROR;
    }    
	return UT_OK;
}

