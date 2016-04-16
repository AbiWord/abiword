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
#include <OXML_Element_Hyperlink.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_Hyperlink::OXML_Element_Hyperlink(const std::string & id) : 
	OXML_Element(id, HYPR_TAG, HYPRLNK)
{
}

OXML_Element_Hyperlink::~OXML_Element_Hyperlink()
{

}

void OXML_Element_Hyperlink::setHyperlinkTarget(const std::string & target)
{
	m_target = target;
}

UT_Error OXML_Element_Hyperlink::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error err = UT_OK;
	const gchar* szValue;

	if(getAttribute("xlink:href", szValue) == UT_OK)
	{	
		if(*szValue != '#')
		{
			//external reference
			std::string relId("rId");
			relId += getId();

			err = exporter->setHyperlinkRelation(TARGET_DOCUMENT_RELATION, relId.c_str(), szValue, "External");
			if(err != UT_OK)
				return err;

			err = exporter->startExternalHyperlink(relId.c_str());
			if(err != UT_OK)
				return err;
		}
		else
		{
			//hyperlink to a bookmark, internal reference
			err = exporter->startInternalHyperlink(szValue+1);
			if(err != UT_OK)
				return err;
		}

		err = this->serializeChildren(exporter);
		if(err != UT_OK)
			return err;
	
		return exporter->finishHyperlink();
	}

	return UT_OK;
}

UT_Error OXML_Element_Hyperlink::addToPT(PD_Document* pDocument)
{
	UT_Error err = UT_OK;

	const PP_PropertyVector field_fmt = {
		"xlink:href", m_target
	};
	if(!pDocument->appendObject(PTO_Hyperlink, field_fmt))
		return UT_ERROR;

	err = addChildrenToPT(pDocument);
	if(err != UT_OK)
		return err;

	if(!pDocument->appendObject(PTO_Hyperlink, PP_NOPROPS))
		return UT_ERROR;

	return UT_OK;
}
