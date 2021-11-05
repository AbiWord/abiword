/* AbiSource
 *
 * Copyright (C) 2011 Ben Martin
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

#pragma once

#include <gsf/gsf.h>

// Internal includes
#include "ODi_ListenerState.h"
#include "ODi_ElementStack.h"

#include <set>

// Internal classes
class ODi_FontFaceDecls;
class ODi_Office_Styles;

// AbiWord classes
class PD_Document;
class ODi_Abi_Data;
/**
 * Class to handle the content stream.
 */
class ODi_ContentStreamAnnotationMatcher_ListenerState : public ODi_ListenerState {

public:

    ODi_ContentStreamAnnotationMatcher_ListenerState ( ODi_ElementStack& rElementStack,
                                                       ODi_Abi_Data & rAbiData );

    virtual ~ODi_ContentStreamAnnotationMatcher_ListenerState();

    void startElement(const gchar* pName, const gchar** ppAtts,
                       ODi_ListenerStateAction& rAction) override;
    void endElement(const gchar* pName, ODi_ListenerStateAction& rAction) override;
    void charData(const gchar* pBuffer, int length) override;

    const std::set< std::string >& getRangedAnnotationNames() const;

private:

    ODi_Abi_Data& m_rAbiData;


};
