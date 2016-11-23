/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiSource
 * 
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
 * Copyright (C) 2009 Hubert Figuiere
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


#include <string>

#include "ut_types.h"
#include "ut_misc.h"
#include "pd_Document.h"
#include "fg_Graphic.h"

// Class definition include
#include "OXML_Image.h"

// Internal includes
#include "OXML_Types.h"
#include "OXML_Document.h"



OXML_Image::OXML_Image()
{

}

OXML_Image::~OXML_Image()
{
}

void OXML_Image::setId(const std::string & imageId)
{
	m_id = imageId;
}

void OXML_Image::setMimeType(const std::string & imageMimeType)
{
	m_mimeType = imageMimeType;
}


void OXML_Image::setData(const UT_ConstByteBufPtr & imageData)
{
	m_graphic.reset();
	m_data = imageData;
}

void OXML_Image::setGraphic(FG_ConstGraphicPtr && graphic)
{
	m_data.reset();
	m_graphic = std::move(graphic);
}


UT_Error OXML_Image::serialize(IE_Exp_OpenXML* exporter)
{
	std::string filename = m_id;
	std::string mimeType;
	if(m_graphic) {
		mimeType = m_graphic->getMimeType();
	}
	else {
		mimeType = m_mimeType;
	}

	if(mimeType.empty() || (mimeType == "image/png"))
	{
		filename += ".png";
	}
	else if(mimeType == "image/jpeg")
	{
		filename += ".jpg";
	}
	else if(mimeType == "image/svg+xml")
	{
		filename += ".svg";
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	return exporter->writeImage(filename.c_str(), m_graphic ? m_graphic->getBuffer() : m_data);
}

UT_Error OXML_Image::addToPT(PD_Document * pDocument)
{
	if (!pDocument->createDataItem(m_id.c_str(), false, m_graphic ? m_graphic->getBuffer() : m_data, 
                                   m_graphic ? m_graphic->getMimeType().c_str() : m_mimeType, 
                                   NULL))
	{            
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return UT_ERROR;
    }    
	return UT_OK;
}

