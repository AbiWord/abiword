/* AbiSource
 * 
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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
#include "ODi_Postpone_ListenerState.h"

// Internal includes
#include "ODi_ListenerStateAction.h"

// AbiWord includes
#include <ut_assert.h>


/**
 * Constructor
 */
ODi_Postpone_ListenerState::ODi_Postpone_ListenerState(ODi_ListenerState* pParserState,
                                                     bool bDeleteWhenPop,
													 ODi_ElementStack& rElementStack)
                            : ODi_ListenerState("Postpone", rElementStack),
                              m_pParserState(pParserState),
                              m_elementStackCount(0)
{
    UT_ASSERT(m_pParserState);
    m_bDeleteParserStateWhenPop = bDeleteWhenPop;
}


/**
 * Destructor
 */
ODi_Postpone_ListenerState::~ODi_Postpone_ListenerState()
{
}


/**
 * 
 */
void ODi_Postpone_ListenerState::startElement (const gchar* pName,
											   const gchar** ppAtts,
											   ODi_ListenerStateAction& /*rAction*/) 
{

    m_xmlRecorder.startElement(pName, ppAtts);
    m_elementStackCount++;
}


/**
 * 
 */                        
void ODi_Postpone_ListenerState::endElement (const gchar* pName,
                                            ODi_ListenerStateAction& rAction) {
    
    m_xmlRecorder.endElement(pName);
    
    m_elementStackCount--;
    
    if (m_elementStackCount == 0) {
        rAction.popState();
    }
}


/**
 * 
 */                      
void ODi_Postpone_ListenerState::charData (const gchar* pBuffer, int length) {
    m_xmlRecorder.charData(pBuffer, length);
}
