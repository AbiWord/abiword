/* AbiSource
 *
 * Copyright (C) 2005 INdT
 * Author: Daniel d'Andrada T. de Carvalho <daniel.carvalho@indt.org.br>
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

#ifndef ODI_FONTFACEDECLS_H_
#define ODI_FONTFACEDECLS_H_

#include <map>
#include <string>

// Internal includes
#include "ODi_ListenerState.h"


/**
 * Holds all relevant info contained in the <office:font-face-decls>
 *
 * For now it's used just to map a given font face style name (style:font-name)
 * into the name of its font face family (svg:font-family).
 */
class ODi_FontFaceDecls : public ODi_ListenerState {

public:

    ODi_FontFaceDecls(ODi_ElementStack& rElementStack);

    void startElement (const gchar* pName, const gchar** ppAtts,
                       ODi_ListenerStateAction& rAction);

    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* /*pBuffer*/, int /*length*/) {}

    const std::string & getFontFamily(const std::string& rStyleName);

    void clear() {m_fontFamilies.clear();}

private:

    // Maps a font face style:name into its svg:font-family
    std::map<std::string, std::string> m_fontFamilies;
};

#endif /*ODI_FONTFACEDECLS_H_*/
