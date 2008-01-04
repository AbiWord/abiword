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

#ifndef _OXMLI_LISTENERSTATE_MAINDOCUMENT_H_
#define _OXMLI_LISTENERSTATE_MAINDOCUMENT_H_

// Internal includes
#include <OXMLi_ListenerState.h>
#include <OXML_Section.h>

/* \class OXMLi_ListenerState_MainDocument
 * \brief This ListenerState parses the Main Document part.
*/
class OXMLi_ListenerState_MainDocument : public OXMLi_ListenerState
{
public:
	OXMLi_ListenerState_MainDocument();
	~OXMLi_ListenerState_MainDocument();

	void startElement (OXMLi_StartElementRequest * rqst);
	void endElement (OXMLi_EndElementRequest * rqst);
	void charData (OXMLi_CharDataRequest * rqst);
};

#endif //_OXMLI_LISTENERSTATE_MAINDOCUMENT_H_

