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
#include "ODi_StreamListener.h"

// Internal includes
#include "ODi_ListenerState.h"
#include "ODi_ContentStream_ListenerState.h"
#include "ODi_ContentStreamAnnotationMatcher_ListenerState.h"
#include "ODi_Frame_ListenerState.h"
#include "ODi_MetaStream_ListenerState.h"
#include "ODi_Postpone_ListenerState.h"
#include "ODi_SettingsStream_ListenerState.h"
#include "ODi_StylesStream_ListenerState.h"
#include "ODi_Table_ListenerState.h"
#include "ODi_TextContent_ListenerState.h"

// AbiWord includes
#include "ut_string.h"


/**
 * Constructor
 */
ODi_StreamListener::ODi_StreamListener(PD_Document* pAbiDocument,
                                     GsfInfile* pGsfInfile,
                                     ODi_Office_Styles* pStyles,
                                     ODi_Abi_Data& rAbiData,
                                     ODi_ElementStack* pElementStack)
    : m_pAbiDocument(pAbiDocument),
      m_pGsfInfile(pGsfInfile),
      m_pStyles(pStyles),
      m_rAbiData(rAbiData),
      m_fontFaceDecls(*pElementStack),
      m_currentAction(ODI_NONE),
      m_pCurrentState(NULL),
      m_deleteCurrentWhenPop(false),
      m_ownStack(false)
{
    UT_ASSERT_HARMLESS(m_pAbiDocument);
    UT_ASSERT_HARMLESS(m_pGsfInfile);
    UT_ASSERT_HARMLESS(m_pStyles);


    // This is done for supporting nested StreamListeners, used when we are
    // resuming postponed elements.    
    if (pElementStack == NULL) {
        m_pElementStack = new ODi_ElementStack;
        m_ownStack = true;
    } 
    else {
        m_pElementStack = pElementStack;
    }
}


/**
 * Destructor
 */
ODi_StreamListener::~ODi_StreamListener()
{
    UT_ASSERT(m_currentAction == ODI_NONE);
#if DEBUG
    if(m_postponedParsing.getItemCount()) {
        UT_DEBUGMSG(("ERROR ODTi: postponedParsing not empty\n"));
    }
    if(m_stateStack.getItemCount()) {
        UT_DEBUGMSG(("ERROR ODTi: stateStack not empty\n"));
    }
    if(m_pCurrentState) {
        UT_DEBUGMSG(("ERROR ODTi: current state exist\n"));
    }
#endif
    UT_VECTOR_PURGEALL(ODi_Postpone_ListenerState*, m_postponedParsing);
    if(m_ownStack) {
        DELETEP(m_pElementStack);
    }
    _clear();
}


/**
 * 
 */
void ODi_StreamListener::_startElement (const gchar* pName,
                                        const gchar** ppAtts,
                                        bool doingRecursion)
{
    UT_ASSERT(m_pCurrentState);
    
    if (m_currentAction != ODI_IGNORING) {
        m_stateAction.reset();
        if (m_pCurrentState) {
            m_pCurrentState->startElement(pName, ppAtts, m_stateAction);
        }
        
        if (m_stateAction.getAction() != m_stateAction.ACTION_NONE) {
            ODi_ListenerState* pState;
            
            pState = m_pCurrentState;
            _handleStateAction();
            
            if (m_pCurrentState != NULL && pState != m_pCurrentState) {
                // The state has changed.
                this->_startElement(pName, ppAtts, true);
            }
        }
    }


    // A check to avoid calling it more than once for a single, actual, XML
    // start element tag.
    if (!doingRecursion) {
    
        if (m_currentAction == ODI_RECORDING) {
            m_xmlRecorder.startElement(pName, ppAtts);
        }

        m_pElementStack->startElement(pName, ppAtts);
    }
}


/**
 * 
 */
void ODi_StreamListener::_endElement (const gchar* pName, bool doingRecursion)
{
    UT_return_if_fail(m_pCurrentState != NULL);
    
    if (m_currentAction != ODI_IGNORING) {
        m_stateAction.reset();
        m_pCurrentState->endElement(pName, m_stateAction);
        
        if (m_stateAction.getAction() != m_stateAction.ACTION_NONE) {
            ODi_ListenerState* pState;
    
            pState = m_pCurrentState;
            _handleStateAction();
            
            if (m_pCurrentState != NULL && pState != m_pCurrentState) {
                // The state has changed.
                this->_endElement(pName, true);
            }
        }
    }
    
    
    // A check to avoid calling it more than once for a single, actual, XML
    // start element tag.
    if (!doingRecursion) {

        m_pElementStack->endElement(pName);
    
        if (m_currentAction == ODI_RECORDING) {
            m_xmlRecorder.endElement(pName);
            
            if (m_pElementStack->getStackSize() == m_elemenStackSize) {
                _playRecordedElement();
            }
        } else if (m_currentAction == ODI_IGNORING &&
                   m_pElementStack->getStackSize() == m_elemenStackSize) {
            // Stop ignoring
            m_currentAction = ODI_NONE;
            // Signal that the end tag of the ignored element has been reached.
            this->_endElement(pName, true);
        }
    }
}


/**
 * 
 */
void ODi_StreamListener::charData (const gchar* pBuffer, int length)
{
    UT_return_if_fail(m_pCurrentState);
    
    m_pCurrentState->charData(pBuffer, length);
    
    if (m_currentAction == ODI_RECORDING) {
        m_xmlRecorder.charData(pBuffer, length);
    }
}


/**
 * Sets the current state of the stream listener.
 * 
 * @param pStateName The name of the state
 * @return An error if the state name is not recognized.
 */
UT_Error ODi_StreamListener::setState(const char* pStateName)
{
    
    UT_ASSERT(m_stateStack.getItemCount() == 0);
    UT_ASSERT(m_pCurrentState == NULL);
    _clear();
    
    m_pCurrentState = _createState(pStateName);
    m_deleteCurrentWhenPop = true;
    
    if (m_pCurrentState) {
        return UT_OK;
    } else {
        return UT_ERROR;
    }
}


/**
 * 
 */
void ODi_StreamListener::setState(ODi_ListenerState* pState, bool deleteWhenPop) {
    
    UT_ASSERT(m_stateStack.getItemCount() == 0);
    UT_ASSERT(m_pCurrentState == NULL);
    _clear();
    
    m_pCurrentState = pState;
    m_deleteCurrentWhenPop = deleteWhenPop;
}


/**
 * Push or pop the stack according to the action stated by the current state.
 */
void ODi_StreamListener::_handleStateAction ()
{
    ODi_StreamListener::StackCell stackCell;
    
    switch (m_stateAction.getAction()) {
        
        case ODi_ListenerStateAction::ACTION_PUSH:
        
            m_stateStack.push_back(
                ODi_StreamListener::StackCell(m_pCurrentState, m_deleteCurrentWhenPop));
                
            if (m_stateAction.getState() != NULL) {
                m_pCurrentState = m_stateAction.getState();
                m_deleteCurrentWhenPop = m_stateAction.getDeleteWhenPop();
            } else {
                
                if (!strcmp(m_stateAction.getStateName().c_str(),
                               "FontFaceDecls")) {
                                
                    m_pCurrentState = &m_fontFaceDecls;
                    m_deleteCurrentWhenPop = false;
                    
                } else {
                
                    m_pCurrentState = _createState(
                        m_stateAction.getStateName().c_str());
                        
                    m_deleteCurrentWhenPop = true;
                }
            }
            
            UT_ASSERT(m_pCurrentState);
            
            break;
            
            
        case ODi_ListenerStateAction::ACTION_POP:
            
            if (m_deleteCurrentWhenPop) {
                DELETEP(m_pCurrentState);
            } else {
                m_pCurrentState = NULL;
            }

            if (m_stateStack.getItemCount() > 0) {
                stackCell = m_stateStack.getLastItem();            
                m_pCurrentState = stackCell.m_pState;
                m_deleteCurrentWhenPop = stackCell.m_deleteWhenPop;
                
                m_stateStack.pop_back();
            }

            break;


        case ODi_ListenerStateAction::ACTION_POSTPONE:
            // If the state wants to come back later he shouldn't be deleted.
            //UT_ASSERT(!m_deleteCurrentWhenPop);
            
            ODi_Postpone_ListenerState* pPostponeState;
            
            if (m_stateAction.getState() != NULL) {
                pPostponeState = new ODi_Postpone_ListenerState(
                                                  m_stateAction.getState(),
                                                  m_stateAction.getDeleteWhenPop(),
                                                  *m_pElementStack);
            } else {
                ODi_ListenerState* pNewState;
                
                UT_ASSERT(!m_stateAction.getStateName().empty());
                
                pNewState = _createState(m_stateAction.getStateName().c_str());
                
                pPostponeState = new ODi_Postpone_ListenerState(
                                                  pNewState,
                                                  m_stateAction.getDeleteWhenPop(),
                                                  *m_pElementStack);
            }
            m_postponedParsing.addItem(pPostponeState);
            
            m_stateStack.push_back(
                ODi_StreamListener::StackCell(m_pCurrentState, m_deleteCurrentWhenPop));
                
            m_pCurrentState = pPostponeState;
            m_deleteCurrentWhenPop = false;
            
            UT_ASSERT(m_pCurrentState);
            
            break;
            
        case ODi_ListenerStateAction::ACTION_BRINGUPALL:
            
            {
                UT_sint32 i;
                bool comeBackAfter = m_stateAction.getComeBackAfter();
                            
                for (i=0; i<m_postponedParsing.getItemCount(); i++) {
                    _resumeParsing(m_postponedParsing[i]);
                }
                
                UT_VECTOR_PURGEALL(ODi_Postpone_ListenerState*, m_postponedParsing);
                m_postponedParsing.clear();
                
                if (!comeBackAfter) {
                    m_stateAction.popState();
                    this->_handleStateAction();
                }
            }
            
            break;
        
            
        case ODi_ListenerStateAction::ACTION_BRINGUP:
        
            if (m_postponedParsing.getItemCount() > 0) {
                    
                ODi_Postpone_ListenerState* pPostponedState;
                
                pPostponedState =
                    m_postponedParsing.getLastItem();
                    
                const UT_String& rStateName =
                    pPostponedState->getParserState()->getStateName();
                    
                if (rStateName == m_stateAction.getStateName()) {
                    
                    bool comeBackAfter = m_stateAction.getComeBackAfter();
                    
                    _resumeParsing(pPostponedState);
                    DELETEP(pPostponedState);
                    m_postponedParsing.pop_back();
                    
                    if (!comeBackAfter) {
                        m_stateAction.popState();
                        this->_handleStateAction();
                    }
                }
            }
            break;


        case ODi_ListenerStateAction::ACTION_REPEAT:
            UT_ASSERT(m_currentAction == ODI_NONE);
            
            m_currentAction = ODI_RECORDING;
            m_xmlRecorder.clear();
            m_elemenStackSize = m_pElementStack->getStackSize();
            break;


        case ODi_ListenerStateAction::ACTION_IGNORE:
            UT_ASSERT(m_currentAction == ODI_NONE);
            
            m_currentAction = ODI_IGNORING;
            
            UT_ASSERT(m_stateAction.getElementLevel() >= -1);
            UT_ASSERT((int)m_pElementStack->getStackSize() -
                      (m_stateAction.getElementLevel()+1) >= 0);
            
            m_elemenStackSize = m_pElementStack->getStackSize() -
                                (m_stateAction.getElementLevel()+1);
            break;
    };
}


/**
 * Clear the state stack.
 */
void ODi_StreamListener::_clear ()
{
    if (m_pCurrentState && m_deleteCurrentWhenPop) {
        DELETEP(m_pCurrentState);
    } else {
        m_pCurrentState = NULL;
    }
    
    UT_sint32 i;
    ODi_StreamListener::StackCell cell;
    for (i=0; i < m_stateStack.getItemCount(); i++) {
        cell = m_stateStack.getNthItem(i);
        if (cell.m_deleteWhenPop) {
            DELETEP(cell.m_pState);
        }
    }
    m_stateStack.clear();
}


/**
 * Create a state given its name.
 * 
 * @param pStateName Tha name of the state to be created.
 */
ODi_ListenerState* ODi_StreamListener::_createState(const char* pStateName) {
    
    ODi_ListenerState* pState = NULL;
    UT_DEBUGMSG(("ODi ListenerState name %s \n",pStateName));
    if (!strcmp("StylesStream", pStateName)) {
        
        pState = new ODi_StylesStream_ListenerState(m_pAbiDocument, m_pGsfInfile,
                                                   m_pStyles, *m_pElementStack, m_rAbiData);
        
    } else if (!strcmp("MetaStream", pStateName)) {
        
        pState = new ODi_MetaStream_ListenerState(m_pAbiDocument, *m_pElementStack);
        
    } else if (!strcmp("SettingsStream", pStateName)) {
        
        pState = new ODi_SettingsStream_ListenerState(*m_pElementStack);
        
    } else if (!strcmp("ContentStream", pStateName)) {
        
        pState = new ODi_ContentStream_ListenerState(m_pAbiDocument, m_pGsfInfile,
                                                    m_pStyles,
                                                    m_fontFaceDecls,
						     *m_pElementStack,
						     m_rAbiData);

    } else if (!strcmp("ContentStreamAnnotationMatcher", pStateName)) {
        
        pState = new ODi_ContentStreamAnnotationMatcher_ListenerState(m_pAbiDocument, m_pGsfInfile,
                                                                      m_pStyles,
                                                                      *m_pElementStack,
                                                                      m_rAbiData);
        
    } else if (!strcmp("TextContent", pStateName)) {
        
        pState = new ODi_TextContent_ListenerState(m_pAbiDocument, m_pStyles,
						   *m_pElementStack,
						   m_rAbiData);
    } else if (!strcmp("Frame", pStateName)) {
        
        pState = new ODi_Frame_ListenerState(m_pAbiDocument, m_pStyles,
                                            m_rAbiData,
                                            *m_pElementStack);
                                            
    } else if (!strcmp("Table", pStateName)) {
        
        pState = new ODi_Table_ListenerState(m_pAbiDocument, m_pStyles,
                                            *m_pElementStack);
                                            
    }
    
    return pState;
}


/**
 * Resumes the parsing of a XML element that was postponed.
 */
void ODi_StreamListener::_resumeParsing(ODi_Postpone_ListenerState* pPostponeState){
    UT_uint32 i, count;
    const ODi_XMLRecorder::StartElementCall* pStartCall = NULL;
    const ODi_XMLRecorder::EndElementCall* pEndCall = NULL;
    const ODi_XMLRecorder::CharDataCall* pCharDataCall = NULL;
    const ODi_XMLRecorder* pXMLRecorder;
    
    pXMLRecorder = pPostponeState->getXMLRecorder();    
    
    ODi_StreamListener streamListener(m_pAbiDocument, m_pGsfInfile,
                                     m_pStyles, m_rAbiData,
                                     m_pElementStack);
                                     
    streamListener.setState(pPostponeState->getParserState(),
                            pPostponeState->getDeleteParserStateWhenPop());
    

    count = pXMLRecorder->getCallCount();
    for (i=0; i<count; i++) {
        switch ( pXMLRecorder->getCall(i)->getType() ) {
            
            case ODi_XMLRecorder::XMLCallType_StartElement:
                pStartCall = (ODi_XMLRecorder::StartElementCall*)
                                pXMLRecorder->getCall(i);
                                
                streamListener.startElement(
                                   pStartCall->m_pName,
                                   (const gchar**) pStartCall->m_ppAtts);
                break;
                
            case ODi_XMLRecorder::XMLCallType_EndElement:
                pEndCall = (ODi_XMLRecorder::EndElementCall*)
                                pXMLRecorder->getCall(i);
                                
                streamListener.endElement(pEndCall->m_pName);
                break;
                
            case ODi_XMLRecorder::XMLCallType_CharData:
                pCharDataCall = (ODi_XMLRecorder::CharDataCall*)
                                pXMLRecorder->getCall(i);
                                
                streamListener.charData(pCharDataCall->m_pBuffer,
                                         pCharDataCall->m_length);
                break;
        }
    }
    
}


/**
 * 
 */
void ODi_StreamListener::_playRecordedElement() {
    UT_uint32 i, count;
    const ODi_XMLRecorder::StartElementCall* pStartCall = NULL;
    const ODi_XMLRecorder::EndElementCall* pEndCall = NULL;
    const ODi_XMLRecorder::CharDataCall* pCharDataCall = NULL;
    ODi_XMLRecorder xmlRecorder;
    
    xmlRecorder = m_xmlRecorder;
    
    m_xmlRecorder.clear();
    m_currentAction = ODI_NONE;
    
    count = xmlRecorder.getCallCount();
    for (i=0; i<count; i++) {
        switch ( xmlRecorder.getCall(i)->getType() ) {
            
            case ODi_XMLRecorder::XMLCallType_StartElement:
                pStartCall = (ODi_XMLRecorder::StartElementCall*)
                                xmlRecorder.getCall(i);
                                
                this->startElement(pStartCall->m_pName,
                                   (const gchar**) pStartCall->m_ppAtts);
                break;
                
            case ODi_XMLRecorder::XMLCallType_EndElement:
                pEndCall = (ODi_XMLRecorder::EndElementCall*)
                                xmlRecorder.getCall(i);
                                
                this->endElement(pEndCall->m_pName);
                break;
                
            case ODi_XMLRecorder::XMLCallType_CharData:
                pCharDataCall = (ODi_XMLRecorder::CharDataCall*)
                                xmlRecorder.getCall(i);
                                
                this->charData(pCharDataCall->m_pBuffer, pCharDataCall->m_length);
                break;
        }
    }
}
