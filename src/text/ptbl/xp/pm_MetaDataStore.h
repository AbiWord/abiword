/* AbiWord
 * Copyright (C) 2009 Hubert Figuiere
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



#ifndef __PM_METADATASTORE_H_
#define __PM_METADATASTORE_H_

#include <map>

#include "pm_MetaDataTypes.h"

class pm_MetaData;



class pm_MetaDataStore
{
public:
    typedef std::map<PMMetaDataId, pm_MetaData*> MetaIdMap;

    pm_MetaDataStore();

    /** Return true is there is nothing in it. */
    bool empty() const
        {
            return m_mapstore.empty();
        }

    /** Lookup for a metadata with whose id is %id
     *  @return NULL if not found
     */
    pm_MetaData * findMetaData(PMMetaDataId id) const;
    /** Add a metadata to the store.
     *  @param meta the metadata. The pointer becomes owned by the store
     *         unless it failed.
     *  @return the id of the metadata. 0 if for some reason it couldn't.
     */
    PMMetaDataId  addMetaData(pm_MetaData *meta);
    /** Add a metadata to the store.
     *  @param meta the metadata. The pointer becomes owned by the store
     *         unless it failed.
     *  @return %id. 0 if for some reason it couldn't.
     */
    PMMetaDataId  insertMetaData(pm_MetaData *meta, PMMetaDataId id);
    const MetaIdMap & getStore() const 
        {
            return m_mapstore;
        }
private:
//    pm_MetaDataStore(const & pm_MetaDataStore);
//    pm_MetaDataStore & operator=(const pm_MetaDataStore &);

    MetaIdMap     m_mapstore;
    PMMetaDataId  m_lastid;
};


#endif
