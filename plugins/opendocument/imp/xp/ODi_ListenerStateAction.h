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

#ifndef _ODI_LISTENERSTATEACTION_H_
#define _ODI_LISTENERSTATEACTION_H_

// AbiWord includes
#include <ut_assert.h>
#include <ut_string_class.h>
#include <ut_types.h>

// Internal classes
class ODi_ListenerState;


/**
 *
 */
class ODi_ListenerStateAction {

public:

    enum {
      ACTION_NONE       = 0,
      ACTION_PUSH       = 1,
      ACTION_POP        = 2,
      ACTION_POSTPONE   = 3,
      ACTION_BRINGUP    = 4,
      ACTION_BRINGUPALL = 5,
      ACTION_REPEAT     = 6,
      ACTION_IGNORE     = 7
    };

    /**
     * Push a state on the top of the stack.
     *
     * @param pListenerState The state to be pushed.
     *                       It isn't deleted when popped afterwards.
     *
     * @param deleteWhenPop If the state should be deleted after use (when popped).
     */
    void pushState(ODi_ListenerState* pListenerState, bool deleteWhenPop);

    /**
     * Push a state on the top of the stack.
     *
     * @param pStateName The name of the state to be pushed onto the stack.
     */
    void pushState(const char* pStateName);

    /**
     * Release the current state and pops another from the top of the stack
     * (if there is one) to take its place.
     */
    void popState();

    /**
     * Postpone the parsing of the current element.
     *
     * @param pState     The state that should be used to parse this element.
     * @param deleteWhenPop If the state should be deleted after use (when popped).
     */
    void postponeElementParsing(ODi_ListenerState* pState, bool deleteWhenPop);

    /**
     * Postpone the parsing of the current element.
     *
     * @param pStateName The name of the state that should be used to parse this
     *                   element.
     */
    void postponeElementParsing(const gchar* pStateName);

    /**
     * Brings up all the postponed element parsing.
     */
    void bringUpPostponedElements(bool comeBackAfter);

    /**
     * Brings up the most recently postponed element (top of the stack)
     * if it has the specified name.
     */
    void bringUpMostRecentlyPostponedElement(const gchar* pStateName,
                                             bool comeBackAfter);

    /**
     * Tells the StreamListener to repeat the current element parsing calls
     * (startElement, endElement and charData) after having called then all once.
     *
     * Useful for ListenerStates that have to pass twice through a XML element.
     */
    void repeatElement();

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
    void ignoreElement(UT_sint32 elementLevel = -1);

    /**
     * Reset/clear the action
     */
    void reset();


    UT_uint32 getAction() const {return m_action;}
    ODi_ListenerState* getState() {return m_pState;}
    bool getDeleteWhenPop() const {return m_deleteWhenPop;}
    const UT_String& getStateName() const {return m_stateName;}
    bool getComeBackAfter() const {return m_comeBackAfter;}
    UT_sint32 getElementLevel() const {return m_elementLevel;}

private:

    UT_uint8 m_action;
    ODi_ListenerState* m_pState;
    UT_String m_stateName;
    bool m_deleteWhenPop;
    bool m_comeBackAfter;
    UT_sint32 m_elementLevel;
};

#endif //_ODI_LISTENERSTATEACTION_H_
