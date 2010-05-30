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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef _ODI_TABLE_LISTENERSTATE_H_
#define _ODI_TABLE_LISTENERSTATE_H_

// Internal includes
#include "ODi_ListenerState.h"

// AbiWord includes
#include <ut_types.h>

// Internal classes
class ODi_Office_Styles;

// AbiWord classes
class PD_Document;


/**
 * Used to parse <table:table> elements.
 * 
 * It uses a two-pass approach.
 * 
 * The first pass gathers info about its rows and collumns to fill the
 * properties of the AbiWord <table> tag (eg: Rows and collumns sizes).
 * When the first pass is finished, the Abi <table> tag is appended.
 * 
 * The second pass defines all <cell> tags with their paragraph contents (ie:
 * the table content itself).
 */
class ODi_Table_ListenerState : public ODi_ListenerState {

public:

    ODi_Table_ListenerState (PD_Document* pDocument,
                            ODi_Office_Styles* pStyles,
                            ODi_ElementStack& rElementStack);
                            
    virtual ~ODi_Table_ListenerState() {}

    void startElement (const gchar* pName, const gchar** ppAtts,
                       ODi_ListenerStateAction& rAction);
                       
    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);
    
    void charData (const gchar* /*pBuffer*/, int /*length*/) {}
    
private:

    void _parseTableStart(const gchar** ppAtts,
                          ODi_ListenerStateAction& rAction);

    void _parseColumnStart(const gchar** ppAtts,
                           ODi_ListenerStateAction& rAction);

    void _parseRowStart(const gchar** ppAtts,
                           ODi_ListenerStateAction& rAction);

    void _parseCellStart(const gchar** ppAtts,
                         ODi_ListenerStateAction& rAction);
    
    bool m_onContentStream;
    
    bool m_onFirstPass;
    UT_sint16 m_elementLevel;

    PD_Document* m_pAbiDocument;
    ODi_Office_Styles* m_pStyles;

    UT_sint16 m_row;
    UT_sint16 m_col;

    UT_sint32 m_rowsLeftToRepeat;
    
    UT_UTF8String m_columnWidths;
    UT_UTF8String m_rowHeights;
    UT_UTF8String m_columnRelWidths;

    bool m_gotAllColumnWidths;
    
    UT_UTF8String m_waitingEndElement;
};

#endif //_ODI_TABLE_LISTENERSTATE_H_
