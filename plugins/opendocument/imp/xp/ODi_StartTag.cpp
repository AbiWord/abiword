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
 
// Class definition include
#include "ODi_StartTag.h"

// AbiWord includes
#include <ut_string.h> 





/**
 * Constructor
 */
ODi_StartTag::ODi_StartTag(UT_uint32 attributeGrowStep) :
    m_pAttributes(NULL),
    m_attributeSize(0),
    m_attributeMemSize(0),
    m_attributeGrowStep(attributeGrowStep) {
}
 
 
 


/**
 * Destructor
 */
ODi_StartTag::~ODi_StartTag() {
    DELETEPV(m_pAttributes);
}
 
 
 


/**
 * 
 */
void ODi_StartTag::set(const gchar* pName, const gchar** ppAtts) {
    UT_uint32 i;
    
    
    m_name.assign(pName);
    

    m_attributeSize = 0; 
    i=0;
    
    
    while (ppAtts[i] != 0) {
        
        if (i >= m_attributeMemSize) {
            _growAttributes();
        }
        
        // Attribute name
        m_pAttributes[i].assign(ppAtts[i]);
        
        // Attribute value
        m_pAttributes[i+1].assign(ppAtts[i+1]);
        
        m_attributeSize += 2;
        i += 2;
    }
}





/**
 * 
 */
void ODi_StartTag::_growAttributes() {
    
    if (m_pAttributes == NULL) {
        
        m_pAttributes = new UT_UTF8Stringbuf[m_attributeGrowStep];
        m_attributeMemSize = m_attributeGrowStep;
        
    } else {
        
        UT_UTF8Stringbuf* pTemp;
        UT_uint32 i;
        
        pTemp = m_pAttributes;
        
        m_pAttributes =
            new UT_UTF8Stringbuf[m_attributeMemSize + m_attributeGrowStep];
            
        m_attributeMemSize += m_attributeGrowStep;
        
        for (i=0; i<m_attributeSize; i++) {
            m_pAttributes[i] = pTemp[i];
        }
        
        DELETEPV(pTemp);
    }
}





/**
 * @param rName An UTF-8 string, conataining the attribute name.
 * @return An UTF-8 string, containing its value.
 */    
const char* ODi_StartTag::getAttributeValue(const char* rName ) const {
    UT_uint32 i;
    
    for (i=0; i<m_attributeSize; i+=2) {
        
        if (!strcmp (rName, m_pAttributes[i].data())) {
            return m_pAttributes[i+1].data();
        }
    }
    
    // If we reached this point it's because we haven't found the specified
    // attribute.
    return NULL;
}
