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


#ifndef _ODI_POSTPONE_LISTENERSTATE_H_
#define _ODI_POSTPONE_LISTENERSTATE_H_

// Internal includes
#include "ODi_ListenerState.h"
#include "ODi_XMLRecorder.h"


/**
 * It stores the XML data and, when wanted, does its parsing.
 *
 * This class was created due to a issue with headers/footers parsing. They are
 * defined on the styles stream but their corresponding AbiWord sections can only
 * be added after the definition of all styles, page size and (I think) the
 * sections of the text content.
 */
class ODi_Postpone_ListenerState : public ODi_ListenerState {

public:
    ODi_Postpone_ListenerState(ODi_ListenerState* pParserState,
                              bool bDeleteWhenPop,
    						  ODi_ElementStack& rElementStack);

    virtual ~ODi_Postpone_ListenerState();

    void startElement (const gchar* pName, const gchar** ppAtts,
                       ODi_ListenerStateAction& rAction);

    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* pBuffer, int length);

    ODi_ListenerState* getParserState() {return m_pParserState;}
    bool getDeleteParserStateWhenPop() const {return m_bDeleteParserStateWhenPop;}
    const ODi_XMLRecorder* getXMLRecorder() const {return &m_xmlRecorder;}

private:

    ODi_XMLRecorder m_xmlRecorder;

    ODi_ListenerState* m_pParserState;
    bool m_bDeleteParserStateWhenPop;

    // Auxiliary variable to make us know when we reached the corresponding
    // end element tag of a given element.
    UT_uint32 m_elementStackCount;
};

#endif //_ODI_POSTPONE_LISTENERSTATE_H_
