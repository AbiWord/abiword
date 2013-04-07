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

#ifndef _ODI_SETTINGSSTREAM_LISTENERSTATE_H_
#define _ODI_SETTINGSSTREAM_LISTENERSTATE_H_

// Internal includes
#include "ODi_ListenerState.h"


/**
 * Class to handle the settings stream
 */
class ODi_SettingsStream_ListenerState : public ODi_ListenerState {

public:

    ODi_SettingsStream_ListenerState(ODi_ElementStack& rElementStack) :
    	ODi_ListenerState("SettingsStream", rElementStack) {}

    virtual ~ODi_SettingsStream_ListenerState() {}

    void startElement (const gchar* /*pName*/, const gchar** /*ppAtts*/,
					   ODi_ListenerStateAction& /*rAction*/) {}

    void endElement (const gchar* /*pName*/, ODi_ListenerStateAction& /*rAction*/) {}

    void charData (const gchar* /*pBuffer*/, int /*length*/) {}
};

#endif //_ODI_SETTINGSSTREAM_LISTENERSTATE_H_
