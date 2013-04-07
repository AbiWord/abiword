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
 
// Class definition include
#include "ODi_MetaStream_ListenerState.h"

// Internal includes
#include "ODi_ElementStack.h"
#include "ODi_ListenerStateAction.h"
#include "ODi_StartTag.h"

// AbiWord includes
#include "pd_Document.h"

/**
 * Constructor
 */
ODi_MetaStream_ListenerState::ODi_MetaStream_ListenerState(
												PD_Document* pDocument,
												ODi_ElementStack& rElementStack)
        : ODi_ListenerState("MetaStream", rElementStack),
          m_pDocument(pDocument)
{
  pDocument->setMetaDataProp(PD_META_KEY_FORMAT, "OpenDocument::ODT");
}

/** 
 * Called to signal that the start tag of an element has been reached.
 */
void ODi_MetaStream_ListenerState::startElement (const gchar* /*pName*/,
												 const gchar** /*ppAtts*/,
												 ODi_ListenerStateAction& /*rAction*/) 
{
    m_charData.clear ();
    
}

/**
 * Called to signal that the end tag of an element has been reached.
 */
void ODi_MetaStream_ListenerState::endElement (const gchar* pName,
                                              ODi_ListenerStateAction& rAction)
{
    if (m_charData.size()) {

        if (!strcmp (pName, "meta:generator")) {
            
            m_pDocument->setMetaDataProp (PD_META_KEY_GENERATOR, m_charData);
            
        } else if (!strcmp (pName, "dc:title")) {
            
            m_pDocument->setMetaDataProp (PD_META_KEY_TITLE, m_charData);
            
        } else if (!strcmp (pName, "dc:description")) {
            
            m_pDocument->setMetaDataProp (PD_META_KEY_DESCRIPTION, m_charData);
            
        } else if (!strcmp (pName, "dc:subject")) {
            
            m_pDocument->setMetaDataProp (PD_META_KEY_SUBJECT, m_charData);
            
        } else if (!strcmp (pName, "meta:keyword")) {

            if (!m_keywords.empty()) {
                m_keywords.append(" ");
            }
            
            m_keywords += m_charData;
                                    
        } else if (!strcmp (pName, "meta:initial-creator")) {
            
            // Should have a PD_META_KEY_INITIAL_CREATOR macro for this one
            m_pDocument->setMetaDataProp("meta:initial-creator", m_charData);
            
        } else if (!strcmp (pName, "dc:creator")) {
            
            m_pDocument->setMetaDataProp (PD_META_KEY_CREATOR, m_charData);

        } else if (!strcmp (pName, "meta:printed-by")) {
            
            // Should have a PD_META_KEY_PRINTED_BY macro for this one
            m_pDocument->setMetaDataProp ("meta:printed-by", m_charData);

        } else if (!strcmp (pName, "meta:creation-date")) {
            
            // ATTENTION: I'm assuming that dc.date is used by AbiWord as
            // the document creation date & time.
            m_pDocument->setMetaDataProp (PD_META_KEY_DATE, m_charData);

        } else if (!strcmp (pName, "dc:date")) {
            
            // Note that, for the OpenDocument standard, dc.date
            // is the last modification date & time.
            m_pDocument->setMetaDataProp (PD_META_KEY_DATE_LAST_CHANGED,
                m_charData);

        } else if (!strcmp (pName, "meta:print-date")) {
            
            // Should have a PD_META_KEY_PRINT_DATE macro for this one
            m_pDocument->setMetaDataProp ("meta:print-date", m_charData);

        } else if (!strcmp (pName, "meta:template")) {
            
            // AbiWord can't handle this kind of meta-data.
            // So, I'm ignoring it.

        } else if (!strcmp (pName, "meta:auto-reload")) {
            
            // AbiWord can't handle this kind of meta-data.
            // So, I'm ignoring it.

        } else if (!strcmp (pName, "meta:hyperlink-behaviour")) {
            
            // AbiWord can't handle this kind of meta-data.
            // So, I'm ignoring it.

        } else if (!strcmp (pName, "dc:language")) {
            
            m_pDocument->setMetaDataProp(PD_META_KEY_LANGUAGE, m_charData);

        } else if (!strcmp (pName, "meta:editing-cycles")) {
            
            // Should have a PD_META_KEY_EDITING_CYCLES macro for this one
            m_pDocument->setMetaDataProp("meta:editing-cycles", m_charData);

        } else if (!strcmp (pName, "meta:editing-duration")) {
            
            // Should have a PD_META_KEY_EDITING_DURATION macro for this one
            m_pDocument->setMetaDataProp("meta:editing-duration", m_charData);

        } else if (!strcmp (pName, "meta:document-statistic")) {
            
            // AbiWord can't handle this kind of meta-data.
            // So, I'm ignoring it.

        } else if (!strcmp (pName, "meta:user-defined")) {

            const gchar* pMetaName = NULL;
            
            pMetaName = m_rElementStack.getStartTag(0)->getAttributeValue("meta:name");
            
            UT_ASSERT(pMetaName != NULL);

            m_pDocument->setMetaDataProp(pMetaName, m_charData);

        } else if (!strcmp (pName, "office:meta")) {
            
            m_pDocument->setMetaDataProp(PD_META_KEY_KEYWORDS, m_keywords);

        }
    }
    
    if (!strcmp(pName, "office:document-meta")) {
        // We're done.
        rAction.popState();
    }

    /*
     * 
     * The following AbiWord meta-data have no counterpart on the OpenDocument
     * standard:
     * 
     * - PD_META_KEY_CONTRIBUTOR
     * - PD_META_KEY_COVERAGE
     * - PD_META_KEY_PUBLISHER
     * - PD_META_KEY_RELATION
     * - PD_META_KEY_RIGHTS
     * - PD_META_KEY_SOURCE
     * - PD_META_KEY_TYPE
     * 
     */
     
    m_charData.clear ();
}

/** 
 * Called when parsing the content between the start and end element tags.
 */
void ODi_MetaStream_ListenerState::charData (const gchar* pBuffer, int length)
{
    if (pBuffer && length) {
        m_charData += UT_String (pBuffer, length).c_str();
    }
}
