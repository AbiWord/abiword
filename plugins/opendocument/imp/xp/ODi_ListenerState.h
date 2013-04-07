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

#ifndef _ODI_LISTENERSTATE_H_
#define _ODI_LISTENERSTATE_H_

// AbiWord includes
#include <ut_types.h>
#include <ut_string_class.h>

// Internal classes
class ODi_ListenerStateAction;
class ODi_ElementStack;


/**
 * Base class for all ODi_*_ListenerState classes.
 */
class ODi_ListenerState {

public:

    ODi_ListenerState(const char* pStateName, ODi_ElementStack& rElementStack)
    	: m_stateName(pStateName), m_rElementStack(rElementStack) {}

    virtual ~ODi_ListenerState() {}

    virtual void startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction) = 0;

    virtual void endElement (const gchar* pName,
                             ODi_ListenerStateAction& rAction) = 0;

    virtual void charData (const gchar* pBuffer, int length) = 0;

    const UT_String& getStateName() const {return m_stateName;}

protected:

    UT_String m_stateName;
    ODi_ElementStack& m_rElementStack;
};

#endif //_ODI_LISTENERSTATE_H_
