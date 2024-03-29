/* AbiSource
 * 
 * Copyright (C) 2010 Marc Maurer <uwog@uwog.net>
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
#include "ODe_DefaultStyles.h"

// Interal includes
#include "ODe_Style_Style.h"

UT_GenericVector<ODe_Style_Style*>* ODe_DefaultStyles::enumerate() const {
    UT_GenericVector<ODe_Style_Style*>* pVec = new UT_GenericVector<ODe_Style_Style*>(m_styles.size());
    
    std::map<std::string, ODe_Style_Style*>::const_iterator pos = m_styles.begin();
    for (; pos != m_styles.end(); pos++) {
        pVec->addItem((*pos).second);
    }

    return pVec;
}

ODe_Style_Style* ODe_DefaultStyles::getStyle(std::string family) {
    std::map<std::string, ODe_Style_Style*>::iterator pos = m_styles.find(family);
    if (pos == m_styles.end())
        return nullptr;

    return (*pos).second;
}

void ODe_DefaultStyles::storeStyle(std::string family, ODe_Style_Style* pStyle) {

    // there can only be one default style for a given family, so let's check
    // for that to catch possible errors
    std::map<std::string, ODe_Style_Style*>::iterator pos = m_styles.find(family);
    UT_return_if_fail(pos == m_styles.end());
    
    m_styles[family] = pStyle;
}
