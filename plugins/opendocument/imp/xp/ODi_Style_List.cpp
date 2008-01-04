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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// Class definition include
#include "ODi_Style_List.h"

// Internal includes
#include "ODi_ListLevelStyle.h"
#include "ODi_ListenerStateAction.h"

// AbiWord includes
#include <pd_Document.h>


/**
 * Destructor
 */
ODi_Style_List::~ODi_Style_List() {
    UT_VECTOR_PURGEALL(ODi_ListLevelStyle*, m_levelStyles);
    m_levelStyles.clear();
}


/**
 * 
 */
void ODi_Style_List::startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction) {

    if (!strcmp("text:list-style", pName)) {
        const gchar* pVal;

        pVal = UT_getAttribute ("style:name", ppAtts);
        UT_ASSERT(pVal);
        m_name = pVal;

    } else if (!strcmp("text:list-level-style-bullet", pName) ||
               !strcmp("text:list-level-style-image", pName)) {
        ODi_ListLevelStyle* pLevelStyle = NULL;
        
        pLevelStyle = new ODi_Bullet_ListLevelStyle(m_rElementStack);
        m_levelStyles.addItem(pLevelStyle);
        
        rAction.pushState(pLevelStyle, false);
        
    } else if (!strcmp("text:list-level-style-number", pName)) {
        ODi_ListLevelStyle* pLevelStyle = NULL;
        
        pLevelStyle = new ODi_Numbered_ListLevelStyle(m_rElementStack);
        m_levelStyles.addItem(pLevelStyle);
        
        rAction.pushState(pLevelStyle, false);
    }
}


/**
 * 
 */
void ODi_Style_List::endElement (const gchar* pName,
                         ODi_ListenerStateAction& rAction) {
                            
    if (!strcmp("text:list-style", pName)) {
        // We're done.
        rAction.popState();
    }
}


/**
 * 
 */
void ODi_Style_List::charData (const gchar* pBuffer, int length) {
}


/**
 * 
 */
void ODi_Style_List::defineAbiList(PD_Document* pDocument) {
    UT_uint32 id;
    UT_uint32 i, j, count;
    UT_uint32 level;
    bool foundParent;
    const UT_UTF8String* pString;
    
    
    // Each style level of a <text:list-style> corresponds to a different
    // <l> on AbiWord. Those <l> of the style levels are related through the
    // parentid attributes, i.e. leven 4 has as parentid the id of the <l> from
    // level 3, level 3 has as parentid the id of the <l> from level 2 and so on.
    
    // Fill the id attribute of the <l> tags.
    count = m_levelStyles.getItemCount();
    for (i=0; i<count; i++) {
        id = pDocument->getUID(UT_UniqueId::List);
        m_levelStyles[i]->setAbiListID(id);
    }
    
    // Fill the parentid attribute of the <l> tags.
    for (i=0; i<count; i++) {
        level = m_levelStyles[i]->getLevelNumber();
        
        // Let's find the ID of the parent list level
        if (level > 1) {
            for (j=0, foundParent=false; j<count && !foundParent; j++) {
                
                if (m_levelStyles[j]->getLevelNumber() == level-1) {
                    pString = m_levelStyles[j]->getAbiListID();
                    m_levelStyles[i]->setAbiListParentID(*pString);
                    foundParent = true;
                }
                
            }
        } else {
            m_levelStyles[i]->setAbiListParentID("0");
        }
    }
    
    // Done it on separate foor loops because I consider the possibility of the
    // list level styles coming unordered (e.g.: level 1, level 5, level 3).
    
    
    
    // Example:
    // <l id="1023" parentid="0" type="5" start-value="0" list-delim="%L" list-decimal="NULL"/>
    
    for (i=0; i<count; i++) {
        m_levelStyles[i]->defineAbiList(pDocument);
    }
}


/**
 * 
 */
void ODi_Style_List::buildAbiPropertiesString() {
    UT_uint32 i, count;
    
    count = m_levelStyles.getItemCount();
    
    for (i=0; i<count; i++) {
        m_levelStyles[i]->buildAbiPropsString();
    }
}
