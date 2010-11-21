/* AbiSource Program Utilities
 * 
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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

#include "ut_std_algo.h"

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
ODi_Style_List::~ODi_Style_List() 
{
    UT_std_delete_all(m_levelStyles);
}


/**
 * 
 */
void ODi_Style_List::startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction) 
{
    const gchar* pVal = NULL;
    ODi_ListLevelStyle* pLevelStyle = NULL;
    if(m_bListStyle)
    {
      //
      // We have a regular list element. We don;t need this default
      //
        pLevelStyle = m_levelStyles.back();
	delete pLevelStyle;
	m_levelStyles.pop_back();
	m_bListStyle = false;
    }
    if (!strcmp("text:list-style", pName)) {
        pVal = UT_getAttribute ("style:name", ppAtts);
        UT_ASSERT(pVal);
        m_name = pVal;
	m_bListStyle = true;
	//
	// make a default list
	//
	pLevelStyle = new ODi_Numbered_ListLevelStyle(m_rElementStack);
	m_levelStyles.push_back(pLevelStyle);
    } 
    else if (!strcmp("text:list-level-style-bullet", pName) ||
               !strcmp("text:list-level-style-image", pName)) {
        pLevelStyle = new ODi_Bullet_ListLevelStyle(m_rElementStack);
        m_levelStyles.push_back(pLevelStyle);
        rAction.pushState(pLevelStyle, false);
    } 
    else if (!strcmp("text:list-level-style-number", pName)) {
        ODi_ListLevelStyle* pLevelStyle = NULL;
        
        pLevelStyle = new ODi_Numbered_ListLevelStyle(m_rElementStack);
        m_levelStyles.push_back(pLevelStyle);
        
        rAction.pushState(pLevelStyle, false);
    }
    else if(!strcmp("text:outline-level-style", pName))
    {
        ODi_ListLevelStyle* pLevelStyle = NULL;
	UT_DEBUGMSG(("Found outline-level-style \n"));
        pVal = UT_getAttribute ("style:num-format", ppAtts);
	if(pVal)
	{
	     pLevelStyle = new ODi_Numbered_ListLevelStyle(m_rElementStack);
	     m_levelStyles.push_back(pLevelStyle);
	}
        else
	{
	     pLevelStyle = new ODi_Numbered_ListLevelStyle(m_rElementStack);
	     m_levelStyles.push_back(pLevelStyle);
	}
        rAction.pushState(pLevelStyle, false);
    }
}


/**
 * 
 */
void ODi_Style_List::endElement (const gchar* pName,
                         ODi_ListenerStateAction& rAction) {
                            
    m_bListStyle = false;
    if (!strcmp("text:list-style", pName)) {
        // We're done.
        rAction.popState();
    }
    if(!strcmp("text:outline-style",pName))
    {
        // We're done.
        rAction.popState();
    }
  }

/*!
 * Each sublist in abiword needs it own unique identifier. When we drop
 * back a level we have to redefine the ID's of the child levels
 * This method gets called when this happens
 */
void ODi_Style_List::redefine(PD_Document* pDocument, UT_sint32 iLevel)
{
    UT_uint32 level=0;
    bool foundParent;
    const UT_UTF8String* pString;
    
    // reset the appropriate the id attribute of the <l> tags.

    for(std::vector<ODi_ListLevelStyle*>::const_iterator iter = m_levelStyles.begin();
        iter != m_levelStyles.end(); ++iter) 
    {

         level++;
	 if(level >= iLevel)
	 {
             (*iter)->setAbiListID(pDocument->getUID(UT_UniqueId::List));
	 }
    }
    
    // Reset the parent ID's now

    for(std::vector<ODi_ListLevelStyle*>::const_iterator iter = m_levelStyles.begin();
        iter != m_levelStyles.end(); ++iter) 
    {

        level = (*iter)->getLevelNumber();
        
        // Let's find the ID of the parent list level
        if (level > iLevel) 
	{
            std::vector<ODi_ListLevelStyle*>::const_iterator iter2;
            for(iter2 = m_levelStyles.begin(), foundParent=false;
                iter2 != m_levelStyles.end() && !foundParent; ++iter2) 
	    {
                
                if ((*iter2)->getLevelNumber() == level-1) 
		{
                    pString = (*iter2)->getAbiListID();
                    (*iter)->setAbiListParentID(*pString);
                    foundParent = true;
                }
            }
        } 
    }
}

/**
 * 
 */
void ODi_Style_List::defineAbiList(PD_Document* pDocument) 
{
    UT_uint32 level;
    bool foundParent;
    const UT_UTF8String* pString;
    
    
    // Each style level of a <text:list-style> corresponds to a different
    // <l> on AbiWord. Those <l> of the style levels are related through the
    // parentid attributes, i.e. leven 4 has as parentid the id of the <l> from
    // level 3, level 3 has as parentid the id of the <l> from level 2 and so on.
    
    // Fill the id attribute of the <l> tags.
     for(std::vector<ODi_ListLevelStyle*>::const_iterator iter = m_levelStyles.begin();
        iter != m_levelStyles.end(); ++iter) {

        (*iter)->setAbiListID(pDocument->getUID(UT_UniqueId::List));
    }
    
    // Fill the parentid attribute of the <l> tags.
    for(std::vector<ODi_ListLevelStyle*>::const_iterator iter = m_levelStyles.begin();
        iter != m_levelStyles.end(); ++iter) {

        level = (*iter)->getLevelNumber();
        
        // Let's find the ID of the parent list level
        if (level > 1) {
            std::vector<ODi_ListLevelStyle*>::const_iterator iter2;
            for(iter2 = m_levelStyles.begin(), foundParent=false;
                iter2 != m_levelStyles.end() && !foundParent; ++iter2) {
                
                if ((*iter2)->getLevelNumber() == level-1) {
                    pString = (*iter2)->getAbiListID();
                    (*iter)->setAbiListParentID(*pString);
                    foundParent = true;
                }
            }
        } 
        else {
            (*iter)->setAbiListParentID("0");
        }
    }
    
    // Done it on separate foor loops because I consider the possibility of the
    // list level styles coming unordered (e.g.: level 1, level 5, level 3).
    
    
    
    // Example:
    // <l id="1023" parentid="0" type="5" start-value="0" list-delim="%L" list-decimal="NULL"/>
    for(std::vector<ODi_ListLevelStyle*>::const_iterator iter = m_levelStyles.begin();
        iter != m_levelStyles.end(); ++iter) {

        (*iter)->defineAbiList(pDocument);
    }
}


/**
 * 
 */
void ODi_Style_List::buildAbiPropertiesString() const
{
    for(std::vector<ODi_ListLevelStyle*>::const_iterator iter = m_levelStyles.begin();
        iter != m_levelStyles.end(); ++iter) {

        (*iter)->buildAbiPropsString();
    }
}
