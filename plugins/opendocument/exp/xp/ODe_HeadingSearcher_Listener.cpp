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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 
// Class definition include
#include "ODe_HeadingSearcher_Listener.h"

// Internal includes
#include "ODe_AuxiliaryData.h"
#include "ODe_Styles.h"

// AbiWord includes
#include <pp_AttrProp.h>
#include <fl_TOCLayout.h>


/**
 * Constructor
 */
ODe_HeadingSearcher_Listener::ODe_HeadingSearcher_Listener(
                                    ODe_Styles& rStyles,
                                    ODe_AuxiliaryData& rAuxiliaryData)
                                    :
                                    m_rStyles(rStyles),
                                    m_rAuxiliaryData(rAuxiliaryData) {
}


/**
 * 
 */
void ODe_HeadingSearcher_Listener::openTOC(const PP_AttrProp* pAP) {
    
    if (!m_rAuxiliaryData.m_pTOCContents ) {
        m_rAuxiliaryData.m_pTOCContents = gsf_output_memory_new();
    }

    for (UT_sint32 iLevel = 1; iLevel <= 4; iLevel++) {
        bool ok = FALSE;
        const gchar* pValue = NULL;

        // gather the source style names for all levels
        UT_UTF8String sSourceStyle = UT_UTF8String_sprintf("toc-source-style%d", iLevel);
        ok = pAP->getProperty(sSourceStyle.utf8_str(), pValue);
        if (ok && pValue != NULL) {
            m_rAuxiliaryData.m_headingStyles.addStyleName(pValue, iLevel);
        } else {
            const PP_Property* pProp = PP_lookupProperty(sSourceStyle.utf8_str());
            UT_continue_if_fail(pProp);
            m_rAuxiliaryData.m_headingStyles.addStyleName(pProp->getInitial(), iLevel);
        }

        // gather the destination style names for all levels
        UT_UTF8String sDestStyle = UT_UTF8String_sprintf("toc-dest-style%u", iLevel);
        ok = pAP->getProperty(sDestStyle.utf8_str(), pValue);
        UT_UTF8String destStyle;
        if (ok && pValue)
            destStyle = pValue;
        else
            destStyle = fl_TOCLayout::getDefaultDestStyle(iLevel);
        m_rAuxiliaryData.m_mDestStyles[iLevel] = destStyle;

        // make sure this destination style is exported to the ODT style list
        m_rStyles.addStyle(destStyle);
    }
}
