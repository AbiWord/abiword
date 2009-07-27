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


#ifndef __PM_METADATA_H_
#define __PM_METADATA_H_

#include <map>
#include <string>


class pm_MetaData
{
public:
    pm_MetaData();

    bool empty() const;

    void setSubject(const std::string & id);
    const std::string & getSubject() const
        {
            return m_subject;
        }
    
    void insertData(const char *key, const char *value);
    const std::map<std::string, std::string> & getData() const
        {
            return m_data;
        }
    const std::map<std::string, std::string> & getPrefixes() const
        {
            return m_prefixes;
        }

private:
    // the node id. informational.
    std::string m_subject;
    // key/value pairs from RDF (or predicate / object)
    std::map<std::string, std::string> m_data;
    // prefix/uri map for the RDF
    std::map<std::string, std::string> m_prefixes;
};


#endif
