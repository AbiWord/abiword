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
#include "ODe_TOC_Listener.h"

// Internal includes
#include "ODe_AuxiliaryData.h"
#include "ODe_Common.h"
#include "ODe_Style_Style.h"

// AbiWord includes
#include <pp_AttrProp.h>


/**
 * Constructor
 */
ODe_TOC_Listener::ODe_TOC_Listener(
                                    ODe_AuxiliaryData& rAuxiliaryData
                                    )
                                    :
                                    m_bInTOCBlock(false),
                                    m_rAuxiliaryData(rAuxiliaryData) {
}


void ODe_TOC_Listener::insertText(const UT_UTF8String& rText) {
    if (!m_bInTOCBlock)
        return;

    if (rText.length() == 0)
        return;
    
    UT_return_if_fail(m_rAuxiliaryData.m_pTOCContents);
    ODe_writeUTF8String(m_rAuxiliaryData.m_pTOCContents, rText);
}

void ODe_TOC_Listener::insertTabChar() {
    if (!m_bInTOCBlock)
        return;

    UT_return_if_fail(m_rAuxiliaryData.m_pTOCContents);
    ODe_writeUTF8String(m_rAuxiliaryData.m_pTOCContents, "<text:tab/>");
}

void ODe_TOC_Listener::openBlock(const PP_AttrProp* pAP, ODe_ListenerAction& /*rAction*/) {
    UT_sint32 iLevel = 0;

    // check if this block should appear in the TOC
    const gchar* pValue = NULL;
    bool ok = pAP->getAttribute("style", pValue);
    if (ok && pValue)
        iLevel = m_rAuxiliaryData.m_headingStyles.getHeadingOutlineLevel(pValue);

    if (iLevel == 0)
        return;

    m_bInTOCBlock = true;

    UT_return_if_fail(m_rAuxiliaryData.m_pTOCContents);

    UT_UTF8String sDestStyle = m_rAuxiliaryData.m_mDestStyles[iLevel];
    UT_ASSERT_HARMLESS(sDestStyle != "");
    
    UT_UTF8String output;
    _printSpacesOffset(output);
    output += "<text:p text:style-name=\"" + ODe_Style_Style::convertStyleToNCName(sDestStyle).escapeXML();
    output += "\">";
        
    ODe_writeUTF8String(m_rAuxiliaryData.m_pTOCContents, output);
}

void ODe_TOC_Listener::closeBlock() {
    if (!m_bInTOCBlock)
        return;

    m_bInTOCBlock = false;

    UT_return_if_fail(m_rAuxiliaryData.m_pTOCContents);
    ODe_writeUTF8String(m_rAuxiliaryData.m_pTOCContents, "</text:p>\n");
}
