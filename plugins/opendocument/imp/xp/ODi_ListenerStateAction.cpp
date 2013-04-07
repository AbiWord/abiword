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
#include "ODi_ListenerStateAction.h"


/**
 * Push a state on the top of the stack.
 * 
 * @param pListenerState The state to be pushed.
 *                       It isn't deleted when popped afterwards.
 */
void ODi_ListenerStateAction::pushState(ODi_ListenerState* pListenerState,
                                       bool deleteWhenPop)
{
    UT_ASSERT(pListenerState);
    
    m_action = this->ACTION_PUSH;
    m_deleteWhenPop = deleteWhenPop;
    m_pState = pListenerState;
    m_stateName.clear();
}


/**
 * Push a state on the top of the stack.
 * 
 * @param pStateName The name of the state to be pushed onto the stack.
 */
void ODi_ListenerStateAction::pushState(const char* pStateName) {
    m_action = this->ACTION_PUSH;
    m_deleteWhenPop = true;
    m_stateName = pStateName;
    m_pState = NULL;
}


/**
 * Release the current state and pops another from the top of the stack
 * (if there is one) to take its place.
 */
void ODi_ListenerStateAction::popState()
{
    m_action=this->ACTION_POP;
    m_stateName.clear();
    m_pState = NULL;
}


/**
 * Postpone the parsing of the current element.
 * 
 * @param pState The state that should be used to parse this element.
 * @param deleteWhenPop If the state should be deleted after use (when popped).
 */
void ODi_ListenerStateAction::postponeElementParsing(ODi_ListenerState* pState,
                                                    bool deleteWhenPop) {
    m_action=this->ACTION_POSTPONE;
    m_deleteWhenPop = deleteWhenPop;
    m_pState = pState;
    m_stateName.clear();
}


/**
 * Postpone the parsing of the current element.
 * 
 * @param pStateName The name of the state that should be used to parse this
 *                   element.
 */
void ODi_ListenerStateAction::postponeElementParsing(const gchar* pStateName) {
    m_action=this->ACTION_POSTPONE;
    m_deleteWhenPop = true;
    m_pState = NULL;
    m_stateName = pStateName;
}


/**
 * Brings up all the postponed element parsing.
 */
void ODi_ListenerStateAction::bringUpPostponedElements(bool comeBackAfter) {
    m_action=this->ACTION_BRINGUPALL;
    m_comeBackAfter = comeBackAfter;
    m_pState = NULL;
    m_stateName.clear();
}


/**
 * Reset/clear the action
 */
void ODi_ListenerStateAction::reset()
{
    m_action=this->ACTION_NONE;
    m_pState=NULL;
    m_stateName.clear();
    m_elementLevel=-999;
}


/**
 * Brings up the most recently postponed element (top of the stack)
 * if it has the specified name.
 */
void ODi_ListenerStateAction::bringUpMostRecentlyPostponedElement(
                                         const gchar* pStateName,
                                         bool comeBackAfter) {
    m_action=this->ACTION_BRINGUP;
    m_comeBackAfter = comeBackAfter;
    m_pState = NULL;
    m_stateName = pStateName;
}


/**
 * Tells the StreamListener to repeat the current element parsing calls
 * (startElement, endElement and charData) after having called then all once.
 * 
 * Useful for ListenerStates that have to pass twice through a XML element.
 */
void ODi_ListenerStateAction::repeatElement() {
    m_action=this->ACTION_REPEAT;
    m_pState = NULL;
    m_stateName.clear();
}


/**
 * Tells the StreamListener to ignore an element that is being parsed
 * 
 * OBS: The next call received by the ListenerState will be the endElement()
 * of the ignored element.
 * 
 * @param elementLevel The level of the element that will be ignored, as
 *                     defined by ODi_ElementStack::getStartTag(). -1 if
 *                     it's the element that has just been opened (the one
 *                     specified by startElement()).
 */
void ODi_ListenerStateAction::ignoreElement(UT_sint32 elementLevel) {
    m_action=this->ACTION_IGNORE;
    m_elementLevel = elementLevel;
    m_pState = NULL;
    m_stateName.clear();
}
