/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* AbiSource Program Utilities
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
#include "ODi_Style_MasterPage.h"
#include "ODi_ListenerStateAction.h"

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>
#include <pd_Document.h>


/**
 * Constructor
 */
ODi_Style_MasterPage::ODi_Style_MasterPage(PD_Document* pDocument,
										 ODi_ElementStack& rElementStack) :
                            ODi_ListenerState("StyleMasterPage", rElementStack),
                            m_pAbiDocument(pDocument),
                            m_pPageLayoutStyle(NULL),
                            m_parsingState(ODI_FIRST_PASS)
{
}


/**
 * 
 */
void ODi_Style_MasterPage::startElement(const gchar* pName,
                                       const gchar** ppAtts,
                                       ODi_ListenerStateAction& rAction) {

    if (!strcmp("style:master-page", pName)) {
        if (m_parsingState == ODI_FIRST_PASS) {
            const gchar* pVal;
    
            pVal = UT_getAttribute ("style:name", ppAtts);
            UT_ASSERT(pVal);
            m_name = pVal;
            
            pVal = UT_getAttribute ("style:page-layout-name", ppAtts);
            UT_ASSERT(pVal);
            m_layoutName = pVal;
            
            // We want a second pass after this one.
            rAction.repeatElement();
            
        } else if (m_parsingState == ODI_SECOND_PASS) {
            // We want to postpone the second pass.
            rAction.postponeElementParsing(this, false);
            
            m_parsingState = ODI_POSTPONING;
            
        } else if (m_parsingState == ODI_POSTPONED_SECOND_PASS) {
            // Do nothing
        } else {
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        }
        
    } else if (!strcmp("style:header", pName)) {
        
        if (m_parsingState == ODI_FIRST_PASS) {
            UT_uint32 id;
            char buffer[500];
            
            id = m_pAbiDocument->getUID(UT_UniqueId::HeaderFtr);
            sprintf(buffer, "%u", id);
            
            if (m_AW_headerSectionID.empty()) {
                m_AW_headerSectionID = buffer;
            } else {
                m_AW_evenHeaderSectionID = buffer;
            }
            
        } else if (m_parsingState == ODI_POSTPONED_SECOND_PASS) {
            PP_PropertyVector ppSecAttr = {
                "id", ""
                "type", ""
            };

            if (m_AW_evenHeaderSectionID.empty()) {
                ppSecAttr[1] = m_AW_headerSectionID.c_str();
                ppSecAttr[3] = "header";
            } else {
                ppSecAttr[1] = m_AW_evenHeaderSectionID.c_str();
                ppSecAttr[3] = "header-even";
            }

            m_pAbiDocument->appendStrux(PTX_Section, ppSecAttr);
            
            rAction.pushState("TextContent");
        } else {
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        }
        
    } else if (!strcmp("style:footer", pName)) {
        
        if (m_parsingState == ODI_FIRST_PASS) {
            UT_uint32 id;
            char buffer[500];
            
            id = m_pAbiDocument->getUID(UT_UniqueId::HeaderFtr);
            sprintf(buffer, "%u", id);
            
            if (m_AW_footerSectionID.empty()) {
                m_AW_footerSectionID = buffer;
            } else {
                m_AW_evenFooterSectionID = buffer;
            }
            
        } else if (m_parsingState == ODI_POSTPONED_SECOND_PASS) {
            PP_PropertyVector ppSecAttr = {
                "id", "",
                "type", ""
            };

            if (m_AW_evenFooterSectionID.empty()) {
                ppSecAttr[1] = m_AW_footerSectionID;
                ppSecAttr[3] = "footer";
            } else {
                ppSecAttr[1] = m_AW_evenFooterSectionID;
                ppSecAttr[3] = "footer-even";
            }

            m_pAbiDocument->appendStrux(PTX_Section, ppSecAttr);

            rAction.pushState("TextContent");
        } else {
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        }
        
    } else if (!strcmp("style:header-left", pName)) {
        
        if (m_parsingState == ODI_FIRST_PASS) {
            UT_uint32 id;
            char buffer[500];
            
            id = m_pAbiDocument->getUID(UT_UniqueId::HeaderFtr);
            sprintf(buffer, "%u", id);
            
            if (!m_AW_headerSectionID.empty()) {
                m_AW_evenHeaderSectionID = m_AW_headerSectionID;
            }
            
            m_AW_headerSectionID = buffer;
            
        } else if (m_parsingState == ODI_POSTPONED_SECOND_PASS) {
            PP_PropertyVector ppSecAttr = {
                "id", m_AW_headerSectionID,
                "type", "header"
            };

            m_pAbiDocument->appendStrux(PTX_Section, ppSecAttr);
            
            rAction.pushState("TextContent");
        } else {
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        }
        
    } else if (!strcmp("style:footer-left", pName)) {
        
        if (m_parsingState == ODI_FIRST_PASS) {
            UT_uint32 id;
            char buffer[500];
            
            id = m_pAbiDocument->getUID(UT_UniqueId::HeaderFtr);
            sprintf(buffer, "%u", id);
            
            if (!m_AW_footerSectionID.empty()) {
                m_AW_evenFooterSectionID = m_AW_footerSectionID;
            }
            
            m_AW_footerSectionID = buffer;
            
        } else if (m_parsingState == ODI_POSTPONED_SECOND_PASS) {
            PP_PropertyVector ppSecAttr = {
                "id", m_AW_footerSectionID,
                "type", "footer"
            };
            m_pAbiDocument->appendStrux(PTX_Section, ppSecAttr);

            rAction.pushState("TextContent");
        } else {
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        }
    }
}


/**
 * 
 */ 
void ODi_Style_MasterPage::endElement(const gchar* pName,
                                     ODi_ListenerStateAction& rAction) {
                                        
    if (!strcmp("style:master-page", pName)) {
        
        if (m_parsingState == ODI_FIRST_PASS) {
            m_parsingState = ODI_SECOND_PASS;
            
        } else if (m_parsingState == ODI_POSTPONING) {
            m_parsingState = ODI_POSTPONED_SECOND_PASS;
            
            // The next time we're called it will be a postponed second pass.
            rAction.popState();
            
        } else if (m_parsingState == ODI_POSTPONED_SECOND_PASS) {
            // Now we're done for good.
            rAction.popState();
        }
    }
}
