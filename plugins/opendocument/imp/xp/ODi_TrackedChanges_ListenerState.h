/* AbiSource
 * 
 * Worked on 2010 Ben Martin <monkeyiq@abisource.com>
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

#ifndef _ODI_TRACKEDCHANGES_LISTENERSTATE_H_
#define _ODI_TRACKEDCHANGES_LISTENERSTATE_H_

#include <string>
#include <map>

// Internal includes
#include "ODi_ListenerState.h"

// AbiWord includes
#include <ut_types.h>
#include <ut_stack.h>

// External includes
#include <gsf/gsf.h>

// Internal classes
class ODi_Office_Styles;
class ODi_Style_List;
class ODi_TableOfContent_ListenerState;
class ODi_Abi_Data;

// AbiWord classes
class PD_Document;
class pf_Frag_Strux;
class PP_RevisionAttr;

/**
 * It parses the <delta:tracked-changes> tree.
 */
class ODi_TrackedChanges_ListenerState : public ODi_ListenerState {

public:

    ODi_TrackedChanges_ListenerState (
        PD_Document* pDocument,
        ODi_Office_Styles* pStyles,
        ODi_ElementStack& rElementStack,
	ODi_Abi_Data & rAbiData);
        
    virtual ~ODi_TrackedChanges_ListenerState();

    void startElement (const gchar* pName, const gchar** ppAtts,
                       ODi_ListenerStateAction& rAction);
                       
    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);
    
    void charData (const gchar* pBuffer, int length);
    
private:

    void _flush ();

    PD_Document* m_pAbiDocument;
    ODi_Office_Styles* m_pStyles;

    bool m_bAcceptingText;
    UT_sint8 m_elementParsingLevel;

	// Buffer that stores character data defined between start and end element
	// tags. e.g.: <bla>some char data</bla>
    UT_UCS4String m_charData;


    // ODT Change Tracking
    UT_uint32 m_ctCurrentRevision;
    UT_uint32 m_ctCurrentTransactionID;
    bool m_bPendingTransaction;
    bool m_bPendingTransactionAuthor;
    bool m_bPendingTransactionDate;
    std::string m_sTransactionAuthor;
    std::string m_sTransactionDate;
    bool m_bPendingTransactionChangeLog;
    std::string m_sTransactionChangeLog;
};

#endif //_ODI_TEXTCONTENT_LISTENERSTATE_H_
