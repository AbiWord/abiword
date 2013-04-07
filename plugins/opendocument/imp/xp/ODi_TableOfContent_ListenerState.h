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

#ifndef _ODI_TABLEOFCONTENT_LISTENERSTATE_H_
#define _ODI_TABLEOFCONTENT_LISTENERSTATE_H_

// Internal includes
#include "ODi_ListenerState.h"

// Internal classes
class ODi_Office_Styles;

// AbiWord classes
class pf_Frag_Strux;
class PD_Document;

/**
 * Used to parse the <text:table-of-content> element.
 */
class ODi_TableOfContent_ListenerState : public ODi_ListenerState {

public:

    ODi_TableOfContent_ListenerState (
        PD_Document* pDocument,
        ODi_Office_Styles* pStyles,
        ODi_ElementStack& rElementStack);

    virtual ~ODi_TableOfContent_ListenerState() {}

    void startElement (const gchar* pName, const gchar** ppAtts,
                       ODi_ListenerStateAction& rAction);

    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* pBuffer, int length);

    pf_Frag_Strux* getTOCStrux() {return m_pTOCStrux;}
    const UT_UTF8String& getProps() const {return props;}

private:

    PD_Document* m_pAbiDocument;
    ODi_Office_Styles* m_pStyles;

    pf_Frag_Strux* m_pTOCStrux;

    // The properties for the corresponding AbiWord <toc> strux.
    UT_UTF8String props;

    // Buffer that stores character data defined between start and end element
    // tags. e.g.: <bla>some char data</bla>
    UT_UTF8String m_charData;
    bool m_acceptingText;
};

#endif //_ODI_TABLEOFCONTENT_LISTENERSTATE_H_
