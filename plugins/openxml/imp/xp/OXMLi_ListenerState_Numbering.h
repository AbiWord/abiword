/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 *
 * Copyright (C) 2009 Firat Kiyak <firatkiyak@gmail.com>
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

#ifndef _OXMLI_LISTENERSTATE_NUMBERING_H_
#define _OXMLI_LISTENERSTATE_NUMBERING_H_

// Internal includes
#include <OXMLi_ListenerState.h>
#include <OXMLi_Types.h>
#include <OXML_Types.h>

/* \class OXMLi_ListenerState_Numbering
 * \brief This ListenerState parses the Document Numbering part.
*/
class OXMLi_ListenerState_Numbering : public OXMLi_ListenerState
{
public:
	OXMLi_ListenerState_Numbering();
	void startElement (OXMLi_StartElementRequest * rqst);
	void endElement (OXMLi_EndElementRequest * rqst);
	void charData (OXMLi_CharDataRequest * rqst);

private:
	OXML_List* m_currentList;
	std::string m_currentNumId;
	std::string m_parentListId;

	void handleLevel(const gchar* ilvl);
	void handleFormattingType(const gchar* val);

};

#endif //_OXMLI_LISTENERSTATE_NUMBERING_H_

