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
#include "ODe_Style_MasterPage.h"

// Internal includes
#include "ODe_Common.h"

// AbiWord includes
#include <pp_AttrProp.h>
#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-input-memory.h>


/**
 * Constructor
 */
ODe_Style_MasterPage::ODe_Style_MasterPage (const gchar* pName,
                                            const gchar* pPageLayoutName) {

    m_name = pName;
    m_pageLayoutName = pPageLayoutName;

    m_pFooterContentTemp = gsf_output_memory_new ();
    m_pFooterEvenContentTemp = gsf_output_memory_new ();
    m_pHeaderContentTemp = gsf_output_memory_new ();
    m_pHeaderEvenContentTemp = gsf_output_memory_new ();
}


/**
 * Destructor
 */
ODe_Style_MasterPage::~ODe_Style_MasterPage() {
    if (m_pHeaderContentTemp != NULL) {
        ODe_gsf_output_close(m_pHeaderContentTemp);
    }
 
    if (m_pHeaderEvenContentTemp != NULL) {
        ODe_gsf_output_close(m_pHeaderEvenContentTemp);
    }

    if (m_pFooterContentTemp != NULL) {
        ODe_gsf_output_close(m_pFooterContentTemp);
    }

    if (m_pFooterEvenContentTemp != NULL) {
        ODe_gsf_output_close(m_pFooterEvenContentTemp);
    }
}


/**
 * Fetches info from an AbiWord <section> tag (from its attributes as properties).
 */
void ODe_Style_MasterPage::fetchAttributesFromAbiSection(const PP_AttrProp* pAP) {
    const gchar* pValue;
    bool ok;
    
    ok = pAP->getAttribute("header", pValue);
    if (ok && pValue != NULL) {
        m_abiHeaderId = pValue;
    }

    ok = pAP->getAttribute("header-even", pValue);
    if (ok && pValue != NULL) {
        m_abiHeaderEvenId = pValue;
    }

    ok = pAP->getAttribute("footer", pValue);
    if (ok && pValue != NULL) {
        m_abiFooterId = pValue;
    }

    ok = pAP->getAttribute("footer-even", pValue);
    if (ok && pValue != NULL) {
        m_abiFooterEvenId = pValue;
    }
}


/**
 * 
 */
bool ODe_Style_MasterPage::write(GsfOutput* pODT) const {
    
    UT_UTF8String output;
    
    UT_UTF8String_sprintf(output,
        "  <style:master-page style:name=\"%s\" style:page-layout-name=\"%s\">\n",
        m_name.utf8_str(), m_pageLayoutName.utf8_str());
        
    ODe_writeUTF8String(pODT, output);
    
    /*
    We have to deal with two confusion things when writing out header/footers:
    
    1. Oddly enough AbiWord uses "header-even" and "footer-even" for page 1, 3, 5, etc :)
    
    2. In OpenDocument you can specify an alternative header/footer for "left" pages.
       Oddly enough OpenOffice.org seems to interpret "left" pages as page 2, 4, 6, etc.
    */

    if (!m_abiHeaderId.empty()) {
        // It has a header
        ODe_writeUTF8String(pODT, "   <style:header>\n");

        // Swap even/uneven when there is an alternative header for uneven pages to
        // match what OpenOffice expects
        if (m_abiHeaderEvenId.empty()) {
            ODe_gsf_output_write(pODT, gsf_output_size (m_pHeaderContentTemp), 
                   gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (m_pHeaderContentTemp)));
        } else {
            ODe_gsf_output_write(pODT, gsf_output_size (m_pHeaderEvenContentTemp), 
                   gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (m_pHeaderEvenContentTemp)));
        }
        
        ODe_writeUTF8String(pODT, "   </style:header>\n");
    }

    if (!m_abiHeaderEvenId.empty()) {
        // It has a different header for uneven pages
        ODe_writeUTF8String(pODT, "   <style:header-left>\n");

        ODe_gsf_output_write(pODT, gsf_output_size (m_pHeaderContentTemp), 
                gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (m_pHeaderContentTemp)));
        
        ODe_writeUTF8String(pODT, "   </style:header-left>\n");
    }

    if (!m_abiFooterId.empty()) {
        // It has a footer
        ODe_writeUTF8String(pODT, "   <style:footer>\n");

        // Swap even/uneven when there is an alternative footer for uneven pages to
        // match what OpenOffice expects
        if (m_abiFooterEvenId.empty()) {
            ODe_gsf_output_write(pODT, gsf_output_size (m_pFooterContentTemp), 
                   gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (m_pFooterContentTemp)));
        } else {
            ODe_gsf_output_write(pODT, gsf_output_size (m_pFooterEvenContentTemp), 
                   gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (m_pFooterEvenContentTemp)));
        }

        ODe_writeUTF8String(pODT, "   </style:footer>\n");
    }

    if (!m_abiFooterEvenId.empty()) {
        // It has a footer for uneven pages
        ODe_writeUTF8String(pODT, "   <style:footer-left>\n");

        ODe_gsf_output_write(pODT, gsf_output_size (m_pFooterContentTemp), 
                gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (m_pFooterContentTemp)));

        ODe_writeUTF8String(pODT, "   </style:footer-left>\n");
    }

    ODe_writeUTF8String(pODT, "  </style:master-page>\n");

    return true;
}
