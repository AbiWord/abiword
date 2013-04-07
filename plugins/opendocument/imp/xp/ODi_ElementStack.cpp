/* AbiSource Program Utilities
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
#include "ODi_ElementStack.h"
 
// Internal includes
#include "ODi_StartTag.h"

// AbiWord includes
#include <ut_string.h>


/**
 * Constructor
 */
ODi_ElementStack::ODi_ElementStack() :
                   m_pStartTags(NULL),
                   m_stackSize(0) {
            
}


/**
 * Destructor
 */
ODi_ElementStack::~ODi_ElementStack() {

    UT_VECTOR_PURGEALL(ODi_StartTag*, (*m_pStartTags));
    DELETEP(m_pStartTags);
}


/**
 * Must be the last command called by the starElement method of the listener
 * class.
 */
void ODi_ElementStack::startElement (const gchar* pName,
                                                 const gchar** ppAtts) {

    ODi_StartTag* pStartTag = NULL;

    if (!m_pStartTags) {
        m_pStartTags = new UT_GenericVector <ODi_StartTag*> (10, 10);
    }

    if (m_stackSize == m_pStartTags->getItemCount()) { 
        
        pStartTag = new ODi_StartTag();
        m_pStartTags->push_back(pStartTag);
        
    } else if (m_stackSize < m_pStartTags->getItemCount()) {
        
        pStartTag = (*m_pStartTags)[m_stackSize];
        
    } else {
        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    }
    
    pStartTag->set(pName, ppAtts);
    
    m_stackSize++;
}


/**
 * Must be the last command called by the endElement method of the listener
 * class.
 */
void ODi_ElementStack::endElement (const gchar* /*pName*/) {
    UT_ASSERT(m_pStartTags != NULL);
    UT_return_if_fail(m_stackSize > 0);
    m_stackSize--;
}


/**
 * @param level 0 is the immediate parent, 1 is the parent of the parent
 *              and so on.
 * 
 * On the startElement method, level 0 is the parent start tag.
 * On the endElement method, level 0 is the corresponding start tag.
 */
const ODi_StartTag* ODi_ElementStack::getStartTag(UT_sint32 level) {
    
    if (m_pStartTags) {
        if (m_stackSize > level) {
            // The level is counted from the top of the vector down to the bottom
            // so, level 0 is m_pStartTags[lastIndex] and
            // level max is m_pStartTags[0]
            return (*m_pStartTags)[m_stackSize - (level+1)];
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}



/**
 * @return True if at least one of the stack elements has the specified name.
 */
bool ODi_ElementStack::hasElement(const gchar* pName) const {
    UT_sint32 i;
    ODi_StartTag* pStartTag;
    
    for (i=0; i<m_stackSize; i++) {
        pStartTag = (*m_pStartTags)[i];
        if (!strcmp(pStartTag->getName(), pName)) {
            return true;
        }
    }
    
    // If the execution reached this line it's because no match was found.
    return false;
}


/**
 * Returns the closest parent with the given name. It returns NULL if there
 * is no parent with the given name.
 * 
 * @param pName Element name.
 * @param fromLevel The level from which the search begins.
 */
const ODi_StartTag* ODi_ElementStack::getClosestElement(
                                                  const gchar* pName,
                                                  UT_sint32 fromLevel) const {
                                                    
    if (m_pStartTags && fromLevel < m_stackSize) {
        UT_sint32 level;
        ODi_StartTag* pStartTag;
        
        for (level=fromLevel; level<m_stackSize; level++) {
            // The level is counted from the top of the vector down to the bottom
            // so, level 0 is m_pStartTags[lastIndex] and
            // level max is m_pStartTags[0]
            pStartTag = (*m_pStartTags)[m_stackSize - (level+1)];
            if (!strcmp(pStartTag->getName(), pName)) {
                return pStartTag;
            }
        }

    }
    
    // Nothing was found.
    return NULL;
}


/**
 * Returns the level of the closest element with the given name.
 */
UT_sint32 ODi_ElementStack::getElementLevel(const gchar* pName) const {
    if (m_pStartTags) {
        UT_sint32 level;
        ODi_StartTag* pStartTag;
        
        for (level=0; level<m_stackSize; level++) {
            // The level is counted from the top of the vector down to the bottom
            // so, level 0 is m_pStartTags[lastIndex] and
            // level max is m_pStartTags[0]
            pStartTag = (*m_pStartTags)[m_stackSize - (level+1)];
            if (!strcmp(pStartTag->getName(), pName)) {
                return level;
            }
        }

    }
    
    UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    return 0;
}
