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

#pragma once

#include <gsf/gsf.h>

// Internal includes
#include "ODi_ListenerState.h"
#include "ODi_ElementStack.h"

// Internal classes
class ODi_FontFaceDecls;
class ODi_Office_Styles;

// AbiWord classes
class PD_Document;
class ODi_Abi_Data;
/**
 * Class to handle the content stream.
 */
class ODi_ContentStream_ListenerState : public ODi_ListenerState {

public:

    ODi_ContentStream_ListenerState (PD_Document* pDocument,
                                    ODi_Office_Styles* pStyles,
                                    ODi_FontFaceDecls& rFontFaceDecls,
				     ODi_ElementStack& rElementStack,
				     ODi_Abi_Data & rAbiData);

    virtual ~ODi_ContentStream_ListenerState();

    virtual void startElement(const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction) override;

    virtual void endElement(const gchar* pName, ODi_ListenerStateAction& rAction) override;

    virtual void charData(const gchar* pBuffer, int length) override;

private:

    PD_Document* m_pAbiDocument;
    ODi_Office_Styles* m_pStyles;
    ODi_FontFaceDecls& m_rFontFaceDecls;
    ODi_Abi_Data& m_rAbiData;
};
