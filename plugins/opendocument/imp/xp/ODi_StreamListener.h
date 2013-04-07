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

#ifndef _ODI_STREAMLISTENER_H_
#define _ODI_STREAMLISTENER_H_

// Internal includes
#include "ODi_ListenerStateAction.h"
#include "ODi_ElementStack.h"
#include "ODi_FontFaceDecls.h"
#include "ODi_XMLRecorder.h"

// External includes
#include <gsf/gsf.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_xml.h>
#include <ut_vector.h>

// Internal classes
class ODi_Office_Styles;
class ODi_Postpone_ListenerState;
class ODi_Abi_Data;

// AbiWord classes
class PD_Document;

/**
 * This class parses the OpenDocument XML.
 *
 * Its behaviour is distributed among several ODi_*_ListenerState classes.
 * So, the code used for its functions depends on its current state.
 *
 * It follows the State design pattern.
 */
class ODi_StreamListener : public virtual UT_XML::Listener {

public:

    ODi_StreamListener(PD_Document* pAbiDocument, GsfInfile* pGsfInfile,
                      ODi_Office_Styles* pStyles, ODi_Abi_Data& rAbiData,
                      ODi_ElementStack* pElementStack = NULL);

    virtual ~ODi_StreamListener();

    void startElement (const gchar* pName, const gchar** ppAtts) {
        _startElement(pName, ppAtts, false);
    }

    void endElement (const gchar* pName) {
        _endElement(pName, false);
    }

    void charData (const gchar* pBuffer, int length);

    UT_Error setState(const char* pStateName);
    void setState(ODi_ListenerState* pState, bool deleteWhenPop);

    void clearFontFaceDecls() {m_fontFaceDecls.clear();}

    ODi_ElementStack* getElementStack() {return m_pElementStack;}
    ODi_ListenerState* getCurrentState() { return m_pCurrentState; }


private:

    void _startElement (const gchar* pName, const gchar** ppAtts,
                       bool doingRecursion);

    void _endElement (const gchar* pName, bool doingRecursion);

    void _handleStateAction();
    void _clear();
    ODi_ListenerState* _createState(const char* pStateName);
    void _resumeParsing(ODi_Postpone_ListenerState* pPostponeState);
    void _playRecordedElement();

    PD_Document* m_pAbiDocument;
    GsfInfile* m_pGsfInfile;
    ODi_Office_Styles* m_pStyles;
    ODi_Abi_Data& m_rAbiData;
    ODi_FontFaceDecls m_fontFaceDecls;

    // Used by the current listener state to signal state changes, etc.
    ODi_ListenerStateAction m_stateAction;

    ODi_ElementStack* m_pElementStack;

    enum ODi_StreamListenerAction {
        ODI_NONE,
        ODI_RECORDING, // recording an element
        ODI_IGNORING   // ignoring an element
    } m_currentAction;

    // The stack size of the element being either recorded or ignored.
    UT_sint32 m_elemenStackSize;

    ODi_XMLRecorder m_xmlRecorder;


    ////
    // Listener state related variables:

    class StackCell {
    public:
        StackCell() {m_pState=NULL; m_deleteWhenPop=false;}
        StackCell(ODi_ListenerState* pState, bool deleteWhenPop) {
            m_deleteWhenPop = deleteWhenPop;
            m_pState = pState;
        }
        // Work around the "return 0" issue of the UT_GenericVector::getNhItem()
        StackCell(UT_uint32 /*i*/) {m_pState=NULL; m_deleteWhenPop=false;}

        StackCell& operator=(const StackCell& sc) {
            this->m_deleteWhenPop = sc.m_deleteWhenPop;
            this->m_pState = sc.m_pState;

            return *this;
        }

        bool m_deleteWhenPop;
        ODi_ListenerState* m_pState;
    };

    ODi_ListenerState* m_pCurrentState;
    bool m_deleteCurrentWhenPop;
    bool m_ownStack;

    UT_GenericVector <ODi_StreamListener::StackCell> m_stateStack;
    UT_GenericVector <ODi_Postpone_ListenerState*> m_postponedParsing;
};

#endif //_ODI_STREAMLISTENER_H_
