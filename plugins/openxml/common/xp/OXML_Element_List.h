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

#ifndef _OXML_ELEMENT_LIST_H_
#define _OXML_ELEMENT_LIST_H_

// Internal includes
#include "OXML_Element.h"
#include "ie_exp_OpenXML.h"
#include "OXML_Element_Paragraph.h"

// AbiWord includes
#include "ut_types.h"
#include "ut_string.h"
#include "pd_Document.h"

class OXML_Element_Paragraph;

class OXML_Element_List : public OXML_Element
{
public:
	OXML_Element_List(const std::string & id, OXML_Element_Paragraph* paragraph);
	virtual ~OXML_Element_List();

	virtual UT_Error serialize(IE_Exp_OpenXML* exporter) override;
	virtual UT_Error addToPT(PD_Document * pDocument) override;

private:
	virtual UT_Error serializeProperties(IE_Exp_OpenXML* exporter);
	OXML_Element_Paragraph* parent;
};

#endif //_OXML_ELEMENT_LIST_H_

