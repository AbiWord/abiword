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

#ifndef _OXMLI_STREAMLISTENER_H_
#define _OXMLI_STREAMLISTENER_H_

// AbiWord includes
#include <ut_types.h>
#include <ut_xml.h>

// Internal includes
#include <OXMLi_Types.h>
#include <OXMLi_ListenerState.h>
#include <OXMLi_Namespace_Common.h>

// External includes
#include <list>

// Forward declarations
class OXMLi_ListenerState;

/* \class OXMLi_StreamListener
 * \brief This is a listener for a UT_XML object.
 * OXMLi_StreamListener can parse any XML part of an OpenXML document thanks to its
 * ListenerStates.  In order to be able to parse a previously-unsupported part, all
 * that is needed is to write a new ListenerState class and include it in this class'
 * setupStates() method.
*/
class OXMLi_StreamListener : public virtual UT_XML::Listener
{
public:
	OXMLi_StreamListener();
	virtual ~OXMLi_StreamListener();

	inline void setStatus(UT_Error sts) { m_parseStatus = sts; }
	inline UT_Error getStatus() { return m_parseStatus; }

	void setupStates(OXML_PartType type, const char * partId = "");

	virtual void startElement (const gchar* pName, const gchar** ppAtts);
	virtual void endElement (const gchar* pName);
	virtual void charData (const gchar* pBuffer, int length);

private:
	OXMLi_ElementStack* m_pElemStack;
	OXMLi_SectionStack* m_pSectStack;
	OXMLi_ContextVector* m_context;
	std::list<OXMLi_ListenerState*> m_states;
	UT_Error m_parseStatus;
	OXMLi_Namespace_Common* m_namespaces;

	void pushState(OXMLi_ListenerState * s);
	void popState();
	void clearStates();

	void verifyStatus();
};

#endif //_OXMLI_STREAMLISTENER_H_

