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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef _OXML_ELEMENT_TEXT_H_
#define _OXML_ELEMENT_TEXT_H_

// Internal includes
#include <OXML_Element.h>
#include <ie_exp_OpenXML.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

class OXML_Element_Text : public OXML_Element
{
public:
	OXML_Element_Text();
	OXML_Element_Text(const gchar * text, int length);
	virtual ~OXML_Element_Text();

	inline void setCharRange(OXML_CharRange range) { m_range = range; }
	inline OXML_CharRange getCharRange() { return m_range; }

	void setText(const gchar * text, int length);
	const UT_UCS4Char * getText_UCS4String();
	const char* getText();

	virtual UT_Error serialize(IE_Exp_OpenXML* exporter);
	virtual UT_Error addToPT(PD_Document * pDocument);
private:
	UT_UCS4String * m_pString;
	OXML_CharRange m_range;
};

#endif //_OXML_ELEMENT_TEXT_H_

