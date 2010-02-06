/* AbiSource
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
 
#include "ut_std_map.h"

// Class definition include
#include "ODi_Office_Styles.h"

// Internal inlcudes
#include "ODi_Style_Style.h"
#include "ODi_Style_PageLayout.h"
#include "ODi_Style_MasterPage.h"
#include "ODi_Style_List.h"
#include "ODi_NotesConfiguration.h"
#include "ODi_ListLevelStyle.h"
#include "ODi_ElementStack.h"
#include "ODi_Abi_Data.h" 

// AbiWord includes
#include <ut_misc.h>
#include <pd_Document.h>
#include <ut_debugmsg.h>


/**
 * Destructor
 */
ODi_Office_Styles::~ODi_Office_Styles() 
{
    UT_map_delete_all_second(m_listStyles);
    UT_map_delete_all_second(m_pageLayoutStyles);
    UT_map_delete_all_second(m_masterPageStyles);
    UT_map_delete_all_second(m_notesConfigurations);
}


/**
 * Adds a <style:style> (ODi_Style_Style class).
 * 
 * @param bAutomatic true if the style is an OpenDocument "automatic style".
 * 
 * @return The address of the newly created ODi_Style_Style or NULL, if the
 *         specified style is not currently supported (like graphic styles).
 */
ODi_Style_Style* ODi_Office_Styles::addStyle(const gchar** ppAtts,
					     ODi_ElementStack& rElementStack,
					     ODi_Abi_Data & rAbiData) 
{

    const gchar* pFamily;
    const gchar* pName;
    const gchar* pDisplayName;
    ODi_Style_Style* pStyle = NULL;
    const ODi_Style_Style* pConstStyle;
    UT_UTF8String replacementName;
    UT_UTF8String replacementDisplayName;
    
    pFamily = UT_getAttribute("style:family", ppAtts);
    pName = UT_getAttribute("style:name", ppAtts);
    pDisplayName = UT_getAttribute("style:display-name", ppAtts);
    UT_ASSERT(pName);

    UT_return_val_if_fail(pFamily, pStyle);
    
    if(!strcmp(pFamily, "text")) {
        // AbiWord doesn't support two styles with the same name, differing only
        // on its type (eg: A "Example" character style and a "Example"
        // paragraph style).

        // Do I already have a paragraph style with this name?
        pConstStyle = m_paragraphStyleStyles.getStyle(pName, true);
        if (pConstStyle) {
            replacementName = pName;
            replacementName += "_text";
            
            if (pDisplayName) {
                replacementDisplayName = pDisplayName;
                replacementDisplayName += "_text";
            }
            
            pStyle = m_textStyleStyles.addStyle(ppAtts, rElementStack,
						rAbiData,
                                                &replacementName,
                                                &replacementDisplayName);
        } else {
	  pStyle = m_textStyleStyles.addStyle(ppAtts, rElementStack,rAbiData);
        }
        
    } else if(!strcmp(pFamily, "paragraph")) {
        // AbiWord doesn't support two styles with the same name, differing only
        // on its type (eg: A "Example" character style and a "Example"
        // paragraph style).

        // Do I already have a text style with this name?
        pConstStyle = m_textStyleStyles.getStyle(pName, true);
        if (pConstStyle) {
            replacementName = pName;
            replacementName += "_paragraph";
            
            if (pDisplayName) {
                replacementDisplayName = pDisplayName;
                replacementDisplayName += "_paragraph";
            }
            
            pStyle = m_paragraphStyleStyles.addStyle(ppAtts, 
						     rElementStack,
						     rAbiData,
                                                     &replacementName,
                                                     &replacementDisplayName);
        } else {
	  pStyle = m_paragraphStyleStyles.addStyle(ppAtts, rElementStack,rAbiData);
        }
        
    } else if(!strcmp(pFamily, "section")) {
      pStyle = m_sectionStyleStyles.addStyle(ppAtts, rElementStack,rAbiData);
        
    } else if(!strcmp(pFamily, "graphic")) {
      pStyle = m_graphicStyleStyles.addStyle(ppAtts, rElementStack,rAbiData);
        
    } else if(!strcmp(pFamily, "table")) {
      pStyle = m_tableStyleStyles.addStyle(ppAtts, rElementStack,rAbiData);
        
    } else if(!strcmp(pFamily, "table-column")) {
      pStyle = m_tableColumnStyleStyles.addStyle(ppAtts, rElementStack,rAbiData);
        
    } else if(!strcmp(pFamily, "table-row")) {
      pStyle = m_tableRowStyleStyles.addStyle(ppAtts, rElementStack,rAbiData);
        
    } else if(!strcmp(pFamily, "table-cell")) {
      pStyle = m_tableCellStyleStyles.addStyle(ppAtts, rElementStack,rAbiData);
    }
    
    return pStyle;   
}


/**
 * Adds a <style:page-layout> (ODi_Style_PageLayout class)
 * 
 * @return The address of the newly created ODi_Style_PageLayout.
 */
ODi_Style_PageLayout* ODi_Office_Styles::addPageLayout(const gchar** ppAtts,
                                                     ODi_ElementStack& rElementStack,
                                                     ODi_Abi_Data& rAbiData) {
                               
    const gchar* pAttrValue;
    ODi_Style_PageLayout* pStyle;
                             
    pStyle = new ODi_Style_PageLayout(rElementStack, rAbiData);
    pAttrValue = UT_getAttribute("style:name", ppAtts);
    m_pageLayoutStyles.insert(std::make_pair(pAttrValue, pStyle));
    
    return pStyle;
}


/**
 * Adds a <style:master-page> (ODi_Style_MasterPage class)
 * 
 * @return The address of the newly created ODi_Style_MasterPage.
 */
ODi_Style_MasterPage* ODi_Office_Styles::addMasterPage(const gchar** ppAtts,
                                                     PD_Document* pDocument,
                                                     ODi_ElementStack& rElementStack) {
                                                        
    const gchar* pAttrValue;
    ODi_Style_MasterPage* pStyle;
                             
    pStyle = new ODi_Style_MasterPage(pDocument, rElementStack);
    pAttrValue = UT_getAttribute("style:name", ppAtts);
    m_masterPageStyles.insert(std::make_pair(pAttrValue, pStyle));
    
    return pStyle;
}


/**
 * 
 */
ODi_Style_Style* ODi_Office_Styles::addDefaultStyle(const gchar** ppAtts,
						    ODi_ElementStack& rElementStack, ODi_Abi_Data & rAbiData) {
    
    const gchar* pAttr = NULL;
    
    pAttr = UT_getAttribute("style:family", ppAtts);

    if (pAttr && !strcmp("paragraph", pAttr)) {
        
      return m_paragraphStyleStyles.addDefaultStyle(rElementStack,rAbiData);
        
    } else if (pAttr && !strcmp("table", pAttr)) {
        
      return m_tableStyleStyles.addDefaultStyle(rElementStack,rAbiData);        

    } else {
        // Not currently supported
        return NULL;
    }
}


/**
 * Links every style with its parent and next ones.
 */
void ODi_Office_Styles::_linkStyles() {
    
    m_textStyleStyles.linkStyles();
    m_paragraphStyleStyles.linkStyles();
    m_sectionStyleStyles.linkStyles();
    m_graphicStyleStyles.linkStyles();
    m_tableStyleStyles.linkStyles();
    m_tableColumnStyleStyles.linkStyles();
    m_tableRowStyleStyles.linkStyles();
    m_tableCellStyleStyles.linkStyles();
        
    _linkMasterStyles();
    _linkListStyles();
}


/**
 * Link the master styles to their page layouts
 */
void ODi_Office_Styles::_linkMasterStyles() {
    
    ODi_Style_MasterPage* pMasterStyle;

    for(MasterPageMap::const_iterator iter = m_masterPageStyles.begin();
        iter != m_masterPageStyles.end(); ++iter) {

        pMasterStyle = iter->second;
        
        PageLayoutMap::const_iterator l_iter 
            = m_pageLayoutStyles.find(pMasterStyle->getLayoutName().utf8_str());
        if(l_iter != m_pageLayoutStyles.end()) {
            pMasterStyle->setLayoutStylePointer(l_iter->second);
        }
    }
}


/**
 * Link list level styles to the text styles that they refer to.
 */
void ODi_Office_Styles::_linkListStyles() const
{
    UT_sint32 count2;
    ODi_ListLevelStyle* pLevelStyle;
    ODi_Style_List* pListStyle;
    const ODi_Style_Style* pStyle;
    
    for(ListMap::const_iterator iter = m_listStyles.begin();
        iter != m_listStyles.end(); ++iter) {

        pListStyle = iter->second;
        UT_continue_if_fail(pListStyle);
        
        count2 = pListStyle->getLevelCount();

        // List levels start from 1.        
        for (UT_sint32 j = 1; j <= count2; j++) {
            pLevelStyle = pListStyle->getLevelStyle(j);
            
            pStyle = getTextStyle(
                pLevelStyle->getTextStyleName()->utf8_str(), false);
            pLevelStyle->setTextStyle(pStyle);
        }
    }
}


/**
 * 
 */
void ODi_Office_Styles::_defineAbiStyles(PD_Document* pDocument) const 
{
    m_textStyleStyles.defineAbiStyles(pDocument);
    m_paragraphStyleStyles.defineAbiStyles(pDocument);
    
    // AbiWord doesn't have section, graphic and table styles.

    // All styles defined on the content stream are automatic, so, I'm not
    // defining them.

    for(ListMap::const_iterator iter = m_listStyles.begin();
        iter != m_listStyles.end(); ++iter) {

        iter->second->defineAbiList(pDocument);
    }
    
    if (!m_listStyles.empty()) {
        bool ok = pDocument->fixListHierarchy();
        UT_ASSERT(ok);
    }

    // I will just use the first master page style (if there is one).
    if (!m_masterPageStyles.empty()) {

        ODi_Style_MasterPage* masterPage = m_masterPageStyles.begin()->second;
        masterPage->definePageSizeTag(pDocument);
    }
}


/**
 * 
 */
void ODi_Office_Styles::_buildAbiPropsAttrString(
                                            ODi_FontFaceDecls& rFontFaceDecls) 
{
    m_textStyleStyles.buildAbiPropsAttrString(rFontFaceDecls);
    m_paragraphStyleStyles.buildAbiPropsAttrString(rFontFaceDecls);
    m_sectionStyleStyles.buildAbiPropsAttrString(rFontFaceDecls);
    m_tableStyleStyles.buildAbiPropsAttrString(rFontFaceDecls);
    

    for(ListMap::const_iterator iter = m_listStyles.begin();
        iter != m_listStyles.end(); ++iter) {

        iter->second->buildAbiPropertiesString();
    }
}


/**
 * Fix any problems encountered on the added styles
 */
void ODi_Office_Styles::_fixStyles() {
    m_textStyleStyles.fixStyles();
    m_paragraphStyleStyles.fixStyles();
}


/**
 * Returns the specified paragraph style
 * 
 * @param pStyleName The name of the style wanted.
 */
const ODi_Style_Style* ODi_Office_Styles::getParagraphStyle(
                                             const gchar* pStyleName,
                                             bool bOnContentStream) const
{
    return m_paragraphStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTextStyle(const gchar* pStyleName,
                                              bool bOnContentStream) const
{
    return m_textStyleStyles.getStyle(pStyleName, bOnContentStream);
}
    

/**
 * 
 */ 
const ODi_Style_Style* ODi_Office_Styles::getSectionStyle(const gchar* pStyleName,
                                                 bool bOnContentStream) const
{
    return m_sectionStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getGraphicStyle(const gchar* pStyleName,
                                          bool bOnContentStream) const
{
    return m_graphicStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTableStyle(const gchar* pStyleName,
                                                  bool bOnContentStream) const
{
    return m_tableStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTableColumnStyle(
                                                  const gchar* pStyleName,
                                                  bool bOnContentStream) const
{
    return m_tableColumnStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTableRowStyle(
                                                  const gchar* pStyleName,
                                                  bool bOnContentStream) const
{
    return m_tableRowStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTableCellStyle(
                                                  const gchar* pStyleName,
                                                  bool bOnContentStream) const
{
    return m_tableCellStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * Adds a list style (<text:list-style>)
 */
ODi_Style_List* ODi_Office_Styles::addList(const gchar** ppAtts,
                                         ODi_ElementStack& rElementStack)
{
    const gchar* pAttrValue = NULL;
    ODi_Style_List* pStyle = NULL;

    pStyle = new ODi_Style_List(rElementStack);
    pAttrValue = UT_getAttribute("style:name", ppAtts);
    UT_DEBUGMSG(("Adding list |%s| to outline collection \n",pAttrValue));
    m_listStyles.insert(std::make_pair(pAttrValue, pStyle));
            
    return pStyle;
}


/**
 * Adds a notes configuration (<text:notes-configuration>)
 */
ODi_NotesConfiguration* ODi_Office_Styles::addNotesConfiguration(
                                               const gchar** ppAtts,
                                               ODi_ElementStack& rElementStack) 
{
    const gchar* pAttrValue = NULL;
    ODi_NotesConfiguration* pNotesConfig = NULL;

    pNotesConfig = new ODi_NotesConfiguration(rElementStack);
    pAttrValue = UT_getAttribute("text:note-class", ppAtts);
                                            
    m_notesConfigurations.insert(std::make_pair(pAttrValue, pNotesConfig));
    
    return pNotesConfig;
}
