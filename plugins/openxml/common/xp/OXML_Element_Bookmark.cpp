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
#include <OXML_Element_Bookmark.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_Bookmark::OXML_Element_Bookmark(const std::string & id) : 
	OXML_Element(id, BOOK_TAG, BOOKMRK)
{
}

OXML_Element_Bookmark::~OXML_Element_Bookmark()
{

}

void OXML_Element_Bookmark::setType(const std::string & type)
{
	m_type = type;
}

void OXML_Element_Bookmark::setName(const std::string & name)
{
	m_name = name;
}

UT_Error OXML_Element_Bookmark::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;
	const gchar* type;
	const gchar* name;

	err = getAttribute("name", name);
	if(err != UT_OK)
		return UT_OK;

	if(getAttribute("type", type) == UT_OK)
	{
		if(strcmp(type, "start") == 0)
		{
			err = exporter->startBookmark(getId().c_str(), name);
			if(err != UT_OK)
				return err;
		}
		else if(strcmp(type, "end") == 0)
		{
			err = exporter->finishBookmark(getId().c_str());
			if(err != UT_OK)
				return err;
		}
		else
		{
			UT_DEBUGMSG(("FRT: Unknown Bookmark type\n"));
		}
	}
	
	return UT_OK;
}

UT_Error OXML_Element_Bookmark::addToPT(PD_Document* pDocument)
{
	UT_Error err = UT_OK;

	const gchar *field_fmt[5];
	field_fmt[0] = "type";
	field_fmt[1] = m_type.c_str();
	field_fmt[2] = "name";
	field_fmt[3] = m_name.c_str();
	field_fmt[4] = 0;

	if(!pDocument->appendObject(PTO_Bookmark, field_fmt))
		return UT_ERROR;

	err = addChildrenToPT(pDocument);
	if(err != UT_OK)
		return err;

	return UT_OK;
}
