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

#ifndef _OXMLI_LISTENERSTATE_H_
#define _OXMLI_LISTENERSTATE_H_

// Internal includes
#include <OXMLi_Types.h>
#include <OXMLi_StreamListener.h>

// AbiWord includes
#include <ut_types.h>

// External includes
#include <stack>
#include <string>

// Forward declarations
class OXMLi_StreamListener;

/* \class OXMLi_ListenerState
 * \brief Base class for all OXMLi_StreamListener states.
*/
class OXMLi_ListenerState
{
public:
	OXMLi_ListenerState();
	virtual ~OXMLi_ListenerState();

	inline void setListener(OXMLi_StreamListener * pListener) { m_pListener = pListener; }
	inline OXMLi_StreamListener * getListener() { return m_pListener; }

	virtual void startElement (OXMLi_StartElementRequest * rqst) = 0;
	virtual void endElement (OXMLi_EndElementRequest * rqst) = 0;
	virtual void charData (OXMLi_CharDataRequest * rqst) = 0;

	bool nameMatches(const std::string& name, const char* ns, const char* tag);
	const gchar* attrMatches(const char* ns, const gchar* attr, std::map<std::string, std::string>* atts);
	bool contextMatches(const std::string& name, const char* ns, const char* tag);

protected:
	bool _error_if_fail(bool val);
	UT_Error _flushTopLevel(OXMLi_ElementStack * stck, OXMLi_SectionStack * sect_stck);
	const gchar * _TwipsToPoints(const gchar * twips);
	const gchar * _TwipsToInches(const gchar * twips);
	const gchar * _EighthPointsToPoints(const gchar * eights);
	const gchar * _EmusToInches(const gchar * emus);
	void getFontLevelRange(const gchar * val, OXML_FontLevel& level, OXML_CharRange& range);

private:
	//StreamListener backreference
	OXMLi_StreamListener * m_pListener;
};
#endif //_OXMLI_LISTENERSTATE_H_

