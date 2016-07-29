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

#ifndef ODI_MANIFESTSTREAM_LISTENERSTATE_H_
#define ODI_MANIFESTSTREAM_LISTENERSTATE_H_

#include <map>
#include <string>

// Internal includes
#include "../../common/xp/ODc_Crypto.h"
#include "ODi_ListenerState.h"

// AbiWord classes
class PD_Document;


/**
 * Class to handle the manifest.xml file.
 */
class ODi_ManifestStream_ListenerState : public ODi_ListenerState {
public:

    ODi_ManifestStream_ListenerState(ODi_ElementStack& rElementStack,
                                std::map<std::string, ODc_CryptoInfo>& cryptoInfo);

    virtual ~ODi_ManifestStream_ListenerState();

    void startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction);

    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* /*pBuffer*/, int /*length*/) {}

private:

    std::string m_sFullPath;
	UT_sint64 m_iSize;
    ODc_CryptoInfo* m_pCryptoInfo;
	std::map<std::string, ODc_CryptoInfo>& m_cryptoInfo;
};

#endif /*ODI_MANIFESTSTREAM_LISTENERSTATE_H_*/
