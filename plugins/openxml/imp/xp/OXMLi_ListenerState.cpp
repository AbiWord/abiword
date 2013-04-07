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

bool OXMLi_ListenerState::contextMatches(const std::string & name, const char* ns, const char* tag)
{
	return nameMatches(name, ns, tag);
}

bool OXMLi_ListenerState::nameMatches(const std::string & name, const char* ns, const char* tag)
{
	std::string str(ns);
	str += ":";
	str += tag;

	return str.compare(name) == 0;
}

const char* OXMLi_ListenerState::attrMatches(const char* ns, const gchar* attr, std::map<std::string, std::string>* atts)
{
	UT_return_val_if_fail( ns && attr, NULL );

	std::string str(ns);
	str += ":";
	str += attr;

	std::map<std::string, std::string>::iterator iter = atts->find(str);

	if(iter == atts->end())
		return NULL;
	else
		return (iter->second).c_str();
}

bool OXMLi_ListenerState::_error_if_fail(bool val)
{
	if (val != true && m_pListener != NULL) {
		m_pListener->setStatus(UT_ERROR);
	}
	return val;
}

UT_Error OXMLi_ListenerState::_flushTopLevel(OXMLi_ElementStack * stck, OXMLi_SectionStack * sect_stck)
{
	if(!stck || !sect_stck || stck->empty())
		return UT_ERROR;

	//pop the top element from the stack
	OXML_SharedElement elem = stck->top();
	stck->pop();
	
	//if there isn't any parent of this element, append the element to last section
	if(stck->empty())
	{
		//this element is a child of <body>
		if(sect_stck->empty())
			return UT_ERROR;

		OXML_SharedSection sect = sect_stck->top();
		return sect->appendElement(elem);
	}
	else //append it to its parent
	{
		OXML_SharedElement newTop = stck->top();
		return newTop->appendElement(elem);
	}
}

const gchar * OXMLi_ListenerState::_TwipsToPoints(const gchar * twips)
{
	double pt = UT_convertDimensionless(twips) / 20;
	return UT_convertToDimensionlessString(pt);
}

const gchar * OXMLi_ListenerState::_TwipsToInches(const gchar * twips)
{
	double in = UT_convertDimensionless(twips) / 1440;
	return UT_convertToDimensionlessString(in);
}

const gchar * OXMLi_ListenerState::_EighthPointsToPoints(const gchar * eights)
{
	double pt = UT_convertDimensionless(eights) / 8;
	return UT_convertToDimensionlessString(pt);
}

const gchar * OXMLi_ListenerState::_EmusToInches(const gchar * emus)
{
	double in = UT_convertDimensionless(emus) / 914400;
	return UT_convertToDimensionlessString(in);
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

