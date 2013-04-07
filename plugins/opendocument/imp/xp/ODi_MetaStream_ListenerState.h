/* AbiSource
 *
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
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

#ifndef _ODI_METASTREAM_LISTENERSTATE_H_
#define _ODI_METASTREAM_LISTENERSTATE_H_

#include <string>

// Internal includes
#include "ODi_ListenerState.h"

// AbiWord includes
#include "ut_string_class.h"

// AbiWord classes
class PD_Document;

/**
 * Class to handle meta-streams
 */
class ODi_MetaStream_ListenerState : public ODi_ListenerState {

public:

    ODi_MetaStream_ListenerState(PD_Document* pDocument,
    							ODi_ElementStack& rElementStack);

    virtual ~ODi_MetaStream_ListenerState() {}

    void startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction);

    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* pBuffer, int length);

private:

    /**
     * Holds the character data of the element being parsed.
     */
    std::string m_charData;

    PD_Document* m_pDocument;

    /**
     * Holds all keywords defined on the meta stream.
     *
     * This is necessary because OpenDocument defines one keyword per
     * meta:keyword element and AbiWord holds all the keywords on a single
     * abiword:keywords element.
     */
    std::string m_keywords;
};

#endif //_ODI_METASTREAM_LISTENERSTATE_H_
