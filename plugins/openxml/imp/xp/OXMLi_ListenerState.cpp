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
#include <OXMLi_StreamListener.h>

// AbiWord includes
#include <ut_types.h>


OXMLi_ListenerState::OXMLi_ListenerState() : 
	m_pListener(NULL)
{

}

OXMLi_ListenerState::~OXMLi_ListenerState()
{
}

bool OXMLi_ListenerState::contextMatches(const char* name, const char* ns, const char* tag)
{
	return nameMatches(name, ns, tag);
}

bool OXMLi_ListenerState::nameMatches(const char* name, const char* ns, const char* tag)
{
	std::string str(ns);
	str += ":";
	str += tag;

	return str.compare(name) == 0;
}

const gchar* OXMLi_ListenerState::attrMatches(const char* ns, const gchar* attr, const gchar** atts)
{
	UT_return_val_if_fail( ns && attr && atts, NULL );

	std::string str(ns);
	str += ":";
	str += attr;

	const gchar** p = atts;

	while (*p)
	{
		if (str.compare(p[0]) == 0)
			break;
		p += 2;
	}

	if (*p)
		return p[1];
	else
		return NULL;
}

bool OXMLi_ListenerState::_error_if_fail(bool val)
{
	if (val != true && m_pListener != NULL) {
		m_pListener->setStatus(UT_ERROR);
	}
	return val;
}

UT_Error OXMLi_ListenerState::_flushTopLevel(OXMLi_ElementStack * stck)
{
	UT_return_val_if_fail( stck != NULL, UT_ERROR );
	OXML_SharedElement elem = stck->top();
	UT_return_val_if_fail( elem != NULL, UT_ERROR );
	stck->pop();
	OXML_SharedElement newTop = stck->top();
	UT_return_val_if_fail( newTop != NULL, UT_ERROR );
	return newTop->appendElement(elem);
}

const gchar * OXMLi_ListenerState::_TwipsToPoints(const gchar * twips)
{
	double pt = UT_convertDimensionless(twips) / 20;
	return UT_convertToDimensionlessString(pt);
}

void OXMLi_ListenerState::getFontLevelRange(const gchar * val, OXML_FontLevel& level, OXML_CharRange& range)
{
	if (NULL != strstr(val, "major")) {
		level = MAJOR_FONT;
	} else {
		level = MINOR_FONT;
	}
	if (NULL != strstr(val, "Bidi")) {
		range = COMPLEX_RANGE;
	} else if (NULL != strstr(val, "EastAsia")) {
		range = EASTASIAN_RANGE;
	} else {
		range = ASCII_RANGE;
	}
}

