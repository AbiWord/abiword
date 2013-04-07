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

#ifndef _OXMLI_LISTENERSTATE_COMMON_H_
#define _OXMLI_LISTENERSTATE_COMMON_H_

// Internal includes
#include <OXMLi_ListenerState.h>
#include <OXMLi_Types.h>

/* \class OXMLi_ListenerState_Common
 * \brief This ListenerState parses all the tags that are common to multiple parts.
*/
class OXMLi_ListenerState_Common : public OXMLi_ListenerState
{
public:
	OXMLi_ListenerState_Common();
	~OXMLi_ListenerState_Common();

	void startElement (OXMLi_StartElementRequest * rqst);
	void endElement (OXMLi_EndElementRequest * rqst);
	void charData (OXMLi_CharDataRequest * rqst);

private:
	bool m_pendingSectBreak;
	OXML_SectionBreakType m_pendingSectBreakType;
	bool m_eqField;
	bool m_pageNumberField;
	bool m_fldChar;
};

#endif //_OXMLI_LISTENERSTATE_COMMON_H_

