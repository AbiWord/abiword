/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
#include "ODi_TableOfContent_ListenerState.h"

// Internal includes
#include "ODi_Style_Style.h"
#include "ODi_Office_Styles.h"
#include "ODi_ListenerStateAction.h"

// AbiWord includes
#include <ut_misc.h>
#include <pd_Document.h>

/**
 * Constructor
 */
ODi_TableOfContent_ListenerState::ODi_TableOfContent_ListenerState (
        PD_Document* pDocument,
        ODi_Office_Styles* pStyles,
        ODi_ElementStack& rElementStack)
            : ODi_ListenerState("TableOfContent", rElementStack),
              m_pAbiDocument ( pDocument ),
              m_pStyles(pStyles),
              m_pTOCStrux(NULL),
              m_acceptingText(false)
{
}


/**
 * 
 */
void ODi_TableOfContent_ListenerState::startElement (const gchar* pName,
													 const gchar** ppAtts,
													 ODi_ListenerStateAction& /*rAction*/) 
{

    if (!strcmp(pName, "text:index-title-template")) {
        const gchar* pVal;
        const ODi_Style_Style* pStyle;
        
        pVal = UT_getAttribute("text:style-name", ppAtts);
        if (pVal) {
            pStyle = m_pStyles->getParagraphStyle(pVal, true);
            UT_ASSERT(pStyle);
            
            if (pStyle) {
	            if (!props.empty()) {
	                props += "; ";
	            }
	            
	            props += "toc-heading-style:";
	            props += pStyle->getDisplayName().c_str();
            }

        }
        
        m_acceptingText = true;
        
    } else if (!strcmp(pName, "text:table-of-content-entry-template")) {
        const gchar* pOutlineLevel;
        const gchar* pStyleName;
        const ODi_Style_Style* pStyle;

        pOutlineLevel = UT_getAttribute("text:outline-level", ppAtts);
        UT_ASSERT_HARMLESS(pOutlineLevel);

        // AbiWord supports only 4 levels.
        if (pOutlineLevel && (atoi(pOutlineLevel) < 5)) {

            pStyleName = UT_getAttribute("text:style-name", ppAtts);
            UT_ASSERT(pStyleName);

            pStyle = m_pStyles->getParagraphStyle(pStyleName, true);
            // If the style isn't defined than it should be because the TOC itself
            // never uses this content level.
            if (pStyle) {
                if (!props.empty()) {
                    props += "; ";
                }
    
                props += "toc-dest-style";
                props += pOutlineLevel;
                props += ":";
                props += pStyle->getDisplayName().c_str();
            }
        }

    }
}


/**
 * 
 */
void ODi_TableOfContent_ListenerState::endElement (const gchar* pName,
                                              ODi_ListenerStateAction& rAction) {

    if (!strcmp(pName, "text:table-of-content")) {

        m_pAbiDocument->appendStrux(PTX_SectionTOC, PP_NOPROPS, &m_pTOCStrux);
        UT_ASSERT(m_pTOCStrux != NULL);

        m_pAbiDocument->appendStrux(PTX_EndTOC, PP_NOPROPS);

        rAction.popState();

    } else if (!strcmp(pName, "text:index-title-template")) {

        if (!props.empty()) {
            props += "; ";
        }

        if (!m_charData.empty()) {
            props += "toc-heading:";
            props += m_charData.utf8_str();
            props += "; toc-has-heading:1";
            
            m_charData.clear();
        } else {
            props += "toc-has-heading:0";
        }

        m_acceptingText = false;
    }
}


/**
 * 
 */
void ODi_TableOfContent_ListenerState::charData (const gchar* pBuffer,
                                                int length) {
    if (pBuffer && length && m_acceptingText) {
        m_charData.append(pBuffer, length);
    }
}
