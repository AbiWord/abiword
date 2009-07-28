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


#include <string.h>

#include <glib.h>

#include "ut_bytebuf.h"
#include "pm_MetaData.h"


pm_MetaData::pm_MetaData()
    : m_cdata(NULL)
{
    
}

pm_MetaData::~pm_MetaData()
{
    FREEP(m_cdata);
}


bool pm_MetaData::empty() const
{
    return !m_cdata && m_subject.empty();
}

void pm_MetaData::setRawData(const UT_ByteBuf & buf)
{
    FREEP(m_cdata);
    UT_uint32 len = buf.getLength();
    m_cdata = (char *)g_try_malloc(len + 1);
    if(m_cdata) 
    {
        memcpy((void*)m_cdata, buf.getPointer(0), len);
        m_cdata[len] = 0;
    }
}

void pm_MetaData::insertData(const char *key, const char *value)
{
    m_data.insert(std::make_pair(key, value));
}

