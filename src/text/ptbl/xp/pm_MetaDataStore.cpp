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



#include "pm_MetaDataStore.h"



pm_MetaDataStore::pm_MetaDataStore()
    : m_lastid(0)
{
}



pm_MetaData * pm_MetaDataStore::findMetaData(PMMetaDataId id) const
{
    MetaIdMap::const_iterator iter = m_mapstore.find(id);
    if(iter == m_mapstore.end()) {
        return NULL;
    }
    return iter->second;
}



PMMetaDataId  pm_MetaDataStore::addMetaData(pm_MetaData *meta)
{
    m_lastid++;
    m_mapstore.insert(std::make_pair(m_lastid, meta));
    return m_lastid;
}
