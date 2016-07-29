/* AbiSource
 * 
 * Copyright (C) 2006 INdT
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
#include "ODi_ManifestStream_ListenerState.h"

// Internal includes
#include "ODi_ListenerStateAction.h"

// AbiWord includes
#include "pd_Document.h"


/**
 * Constructor
 */
ODi_ManifestStream_ListenerState::ODi_ManifestStream_ListenerState(
                                                ODi_ElementStack& rElementStack,
                                                std::map<std::string, ODc_CryptoInfo>& cryptoInfo)
        : ODi_ListenerState("ManifestStream", rElementStack),
          m_sFullPath(""),
          m_iSize(-1),
          m_pCryptoInfo(NULL),
          m_cryptoInfo(cryptoInfo)
{
}

/**
 * Destructor
 */
ODi_ManifestStream_ListenerState::~ODi_ManifestStream_ListenerState()
{
    DELETEP(m_pCryptoInfo);
}

/**
 * Called to signal that the start tag of an element has been reached.
 */
void ODi_ManifestStream_ListenerState::startElement (const gchar* pName,
													 const gchar** ppAtts,
													 ODi_ListenerStateAction& /*rAction*/) 
{
    const char* pVal;

    if (!strcmp(pName, "manifest:file-entry")) {
        pVal = UT_getAttribute ("manifest:full-path", ppAtts);
		m_sFullPath = pVal ? pVal : "";

        pVal = UT_getAttribute ("manifest:size", ppAtts);
		m_iSize = pVal ? atol(pVal) : -1;
	}
	
    if (!strcmp(pName, "manifest:encryption-data")) {
        DELETEP(m_pCryptoInfo);
        m_pCryptoInfo = new ODc_CryptoInfo();
    }

    if (!strcmp(pName, "manifest:algorithm")) {
        UT_return_if_fail(m_pCryptoInfo);

        pVal = UT_getAttribute ("manifest:algorithm-name", ppAtts);
        m_pCryptoInfo->m_algorithm = pVal ? pVal : "";

        pVal = UT_getAttribute ("manifest:initialisation-vector", ppAtts);
        m_pCryptoInfo->m_initVector = pVal ? pVal : "";
	}

	if (!strcmp(pName, "manifest:key-derivation")) {
        UT_return_if_fail(m_pCryptoInfo);

        pVal = UT_getAttribute ("manifest:key-derivation-name", ppAtts);
        m_pCryptoInfo->m_keyType = pVal ? pVal : "";

        pVal = UT_getAttribute ("manifest:iteration-count", ppAtts);
        m_pCryptoInfo->m_iterCount = pVal ? atol(pVal) : -1;

        pVal = UT_getAttribute ("manifest:salt", ppAtts);
        m_pCryptoInfo->m_salt = pVal ? pVal : "";
	}
}


/**
 * Called to signal that the end tag of an element has been reached.
 */
void ODi_ManifestStream_ListenerState::endElement (const gchar* pName,
                                              ODi_ListenerStateAction& rAction)
{
    if (!strcmp(pName, "manifest:encryption-data")) {
        UT_return_if_fail(m_pCryptoInfo);
		
        // store the encryption information
		m_pCryptoInfo->m_decryptedSize = m_iSize;
        m_cryptoInfo[m_sFullPath] = *m_pCryptoInfo;
        DELETEP(m_pCryptoInfo);
    }

	if (!strcmp(pName, "manifest:manifest")) {
        rAction.popState();
    }
}
