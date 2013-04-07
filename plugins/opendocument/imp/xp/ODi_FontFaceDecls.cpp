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

// Class definition include
#include "ODi_FontFaceDecls.h"

// Internal includes
#include "ODi_ListenerStateAction.h"

// AbiWord includes
#include <ut_misc.h>


/**
 * Constructor
 */
ODi_FontFaceDecls::ODi_FontFaceDecls(ODi_ElementStack& rElementStack)
                        : ODi_ListenerState("FontFaceDecls", rElementStack)
{
}


/**
 * 
 */
void ODi_FontFaceDecls::startElement (const gchar* pName,
                                      const gchar** ppAtts,
                                      ODi_ListenerStateAction& /*rAction*/) 
{
    if (!strcmp(pName, "style:font-face")) {
        const gchar* pStyleName;
        const gchar* pFontFamily;
        std::string fontFamily;
        
        pStyleName = UT_getAttribute("style:name", ppAtts);
        UT_ASSERT(pStyleName);
        
        pFontFamily = UT_getAttribute("svg:font-family", ppAtts);
        UT_ASSERT_HARMLESS(pFontFamily);

        fontFamily = pFontFamily;

        if (pFontFamily && (pFontFamily[0] == '\'') && (pFontFamily[strlen(pFontFamily)-1] == '\'')) {
            // e.g.:  Turns a "'Times New Roman'" into a "Times New Roman".
            // OpenOffice.org sometimes adds those extra "'" surrounding the
            // font family name if it's composed by more than one word.
            m_fontFamilies[pStyleName] =
				fontFamily.substr(1, fontFamily.length()-2).c_str();
        } else {
            m_fontFamilies[pStyleName] = pFontFamily;
        }
    }
}


/**
 * 
 */                   
void ODi_FontFaceDecls::endElement (const gchar* pName,
                                    ODi_ListenerStateAction& rAction) {
    if (!strcmp(pName, "office:font-face-decls")) {
        rAction.popState();
    }
}


/**
 * Returns the font family of a font face given its style name.
 */
const std::string & ODi_FontFaceDecls::getFontFamily(
                                        const std::string& rStyleName) {
    return m_fontFamilies[rStyleName];
}
