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

#ifndef _ODI_ABI_DATA_H_
#define _ODI_ABI_DATA_H_

#include <map>
#include <set>
#include <string>

// External includes
#include <gsf/gsf.h>

#include "ut_types.h"
#include "ut_bytebuf.h"
// AbiWord classes
class PD_Document;
class UT_String;

/**
 * Represents the <data> section of the resulting AbiWord document from an
 * OpenDocument file import.
 */
class ODi_Abi_Data {

public:

    ODi_Abi_Data(PD_Document* pDocument, GsfInfile* pGsfInfile);

    /**
     * Adds an data item (<d> tag) in the AbiWord document for the specified image.
     *
     * @param rDataId Receives the id that has been given to the added data item.
     * @param ppAtts The attributes of a <draw:image> element.
     */
    bool addImageDataItem(UT_String& rDataId, const gchar** ppAtts);

    bool addObjectDataItem(UT_String& rDataId, const gchar** ppAtts, int& pto_Type);

private:

    UT_Error _loadStream(GsfInfile* oo, const char* stream, const UT_ByteBufPtr& buf);
    void _splitDirectoryAndFileName(const gchar* pHRef, UT_String& dirName, UT_String& fileName) const;

    PD_Document* m_pAbiDocument;
    GsfInfile* m_pGsfInfile;

    // Stores all added data items id's, given its hrefs.
    // Used to avoid adding multiple data items for the same picture.
	typedef std::map<std::string, std::string> href_id_map_t;
    href_id_map_t m_href_to_id;

  public:
    std::set< std::string > m_openAnnotationNames;
    std::set< std::string > m_rangedAnnotationNames;
};

#endif //_ODI_ABI_DATA_H_
