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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 
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
 
// AbiWord includes
#include <ut_misc.h>
#include <pd_Document.h>
#include <ut_debugmsg.h>


/**
 * Destructor
 */
ODi_Office_Styles::~ODi_Office_Styles() {
    
    UT_GenericVector<ODi_Style_PageLayout*>* pPageLayoutVector;
    UT_GenericVector<ODi_Style_MasterPage*>* pMasterPageVector;
    UT_GenericVector<ODi_Style_List*>* pListStyleVector;
    UT_GenericVector<ODi_NotesConfiguration*>* pNotesConfigVector;
    UT_uint32 i, count;
    
  
    
    pListStyleVector = m_listStyles.enumerate();
    count = pListStyleVector->getItemCount();
    for (i=0; i<count; i++) {
        delete (*pListStyleVector)[i];
    }
	DELETEP(pListStyleVector);
    
    
    pPageLayoutVector = m_pageLayoutStyles.enumerate();
    count = pPageLayoutVector->getItemCount();
    for (i=0; i<count; i++) {
        delete (*pPageLayoutVector)[i];
    }
	DELETEP(pPageLayoutVector);
    
    
    pMasterPageVector = m_masterPageStyles.enumerate();
    count = pMasterPageVector->getItemCount();
    for (i=0; i<count; i++) {
        delete (*pMasterPageVector)[i];
    }
	DELETEP(pMasterPageVector);
    
    
    pNotesConfigVector = m_notesConfigurations.enumerate();
    count = pNotesConfigVector->getItemCount();
    for (i=0; i<count; i++) {
        delete (*pNotesConfigVector)[i];
    }
	DELETEP(pNotesConfigVector);
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
                                           ODi_ElementStack& rElementStack) {

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
                                                &replacementName,
                                                &replacementDisplayName);
        } else {
            pStyle = m_textStyleStyles.addStyle(ppAtts, rElementStack);
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
            
            pStyle = m_paragraphStyleStyles.addStyle(ppAtts, rElementStack,
                                                     &replacementName,
                                                     &replacementDisplayName);
        } else {
            pStyle = m_paragraphStyleStyles.addStyle(ppAtts, rElementStack);
        }
        
    } else if(!strcmp(pFamily, "section")) {
        pStyle = m_sectionStyleStyles.addStyle(ppAtts, rElementStack);
        
    } else if(!strcmp(pFamily, "graphic")) {
        pStyle = m_graphicStyleStyles.addStyle(ppAtts, rElementStack);
        
    } else if(!strcmp(pFamily, "table")) {
        pStyle = m_tableStyleStyles.addStyle(ppAtts, rElementStack);
        
    } else if(!strcmp(pFamily, "table-column")) {
        pStyle = m_tableColumnStyleStyles.addStyle(ppAtts, rElementStack);
        
    } else if(!strcmp(pFamily, "table-row")) {
        pStyle = m_tableRowStyleStyles.addStyle(ppAtts, rElementStack);
        
    } else if(!strcmp(pFamily, "table-cell")) {
        pStyle = m_tableCellStyleStyles.addStyle(ppAtts, rElementStack);
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
    bool ok;
                             
    pStyle = new ODi_Style_PageLayout(rElementStack, rAbiData);
    pAttrValue = UT_getAttribute("style:name", ppAtts);
    ok = m_pageLayoutStyles.insert(pAttrValue, pStyle);
    
    UT_ASSERT(ok);
    
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
    bool ok;
                             
    pStyle = new ODi_Style_MasterPage(pDocument, rElementStack);
    pAttrValue = UT_getAttribute("style:name", ppAtts);
    ok = m_masterPageStyles.insert(pAttrValue, pStyle);
    
    UT_ASSERT(ok);
    
    return pStyle;
}


/**
 * 
 */
ODi_Style_Style* ODi_Office_Styles::addDefaultStyle(const gchar** ppAtts,
                                                  ODi_ElementStack& rElementStack) {
    
    const gchar* pAttr = NULL;
    
    pAttr = UT_getAttribute("style:family", ppAtts);

    if (pAttr && !strcmp("paragraph", pAttr)) {
        
        return m_paragraphStyleStyles.addDefaultStyle(rElementStack);
        
    } else if (pAttr && !strcmp("table", pAttr)) {
        
        return m_tableStyleStyles.addDefaultStyle(rElementStack);        

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
    
    UT_GenericVector<ODi_Style_MasterPage*>* pMasterStylesVec;
    UT_uint32 i, count;
    ODi_Style_MasterPage* pMasterStyle;
    ODi_Style_PageLayout* pLayout;
    
    pMasterStylesVec = m_masterPageStyles.enumerate();
    UT_return_if_fail(pMasterStylesVec);
    
    count = pMasterStylesVec->getItemCount();
    for (i=0; i<count; i++) {
        pMasterStyle = (*pMasterStylesVec)[i];
        
        pLayout = m_pageLayoutStyles.pick(pMasterStyle->getLayoutName().utf8_str());
        UT_ASSERT(pLayout);
        
        pMasterStyle->setLayoutStylePointer(pLayout);
    }
	DELETEP(pMasterStylesVec);
}


/**
 * Link list level styles to the text styles that they refer to.
 */
void ODi_Office_Styles::_linkListStyles() {
    
    UT_uint32 i, j, count, count2;
    UT_GenericVector<ODi_Style_List*>* pStylesVec;
    ODi_ListLevelStyle* pLevelStyle;
    ODi_Style_List* pListStyle;
    const ODi_Style_Style* pStyle;
    
    pStylesVec = m_listStyles.enumerate();
    UT_return_if_fail(pStylesVec);
    
    count = pStylesVec->getItemCount();
    for (i=0; i<count; i++) {

        pListStyle = (*pStylesVec)[i];
        UT_continue_if_fail(pListStyle);
        
        count2 = pListStyle->getLevelCount();

        // List levels start from 1.        
        for (j=1; j<=count2; j++) {
            pLevelStyle = pListStyle->getLevelStyle(j);
            
            pStyle = this->getTextStyle(
                pLevelStyle->getTextStyleName()->utf8_str(), false);
            pLevelStyle->setTextStyle(pStyle);
        }
    }
	DELETEP(pStylesVec);
}


/**
 * 
 */
void ODi_Office_Styles::_defineAbiStyles(PD_Document* pDocument) const {
    
    UT_uint32 i, count;
    UT_GenericVector<ODi_Style_List*>* pListVec;
    bool ok;
    
    m_textStyleStyles.defineAbiStyles(pDocument);
    m_paragraphStyleStyles.defineAbiStyles(pDocument);
    
    // AbiWord doesn't have section, graphic and table styles.

    // All styles defined on the content stream are automatic, so, I'm not
    // defining them.


    pListVec = m_listStyles.enumerate();
    UT_return_if_fail(pListVec);
    
    count = pListVec->getItemCount();
    for (i=0; i<count; i++) {
        (*pListVec)[i]->defineAbiList(pDocument);
    }
    
    if (count > 0) {
        ok = pDocument->fixListHierarchy();
        UT_ASSERT(ok);
    }
    DELETEP(pListVec);
    


    // I will just use the first master page style (if there is one).
    if (m_masterPageStyles.size() > 0) {
        UT_GenericVector<ODi_Style_MasterPage*>* pMasterStylesVec;
	    
        pMasterStylesVec = m_masterPageStyles.enumerate();

        if ((*pMasterStylesVec)[0] && (*pMasterStylesVec)[0]->getPageLayout()) {
        // make sure m_pPageLayoutStyle is not NULL
            (*pMasterStylesVec)[0]->definePageSizeTag(pDocument);
        }
	
        DELETEP(pMasterStylesVec);
    }
}


/**
 * 
 */
void ODi_Office_Styles::_buildAbiPropsAttrString(
                                            ODi_FontFaceDecls& rFontFaceDecls) {
    
    UT_uint32 i, count;
    UT_GenericVector<ODi_Style_List*>* pListVec;
    
    m_textStyleStyles.buildAbiPropsAttrString(rFontFaceDecls);
    m_paragraphStyleStyles.buildAbiPropsAttrString(rFontFaceDecls);
    m_sectionStyleStyles.buildAbiPropsAttrString(rFontFaceDecls);
    m_tableStyleStyles.buildAbiPropsAttrString(rFontFaceDecls);
    

    pListVec = m_listStyles.enumerate();
    UT_return_if_fail(pListVec);
    
    count = pListVec->getItemCount();
    for (i=0; i<count; i++) {
        (*pListVec)[i]->buildAbiPropertiesString();
    }
	DELETEP(pListVec);
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
                                             bool bOnContentStream) {
                                                
    return m_paragraphStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTextStyle(const gchar* pStyleName,
                                              bool bOnContentStream) {
    return m_textStyleStyles.getStyle(pStyleName, bOnContentStream);
}
    

/**
 * 
 */ 
const ODi_Style_Style* ODi_Office_Styles::getSectionStyle(const gchar* pStyleName,
                                                 bool bOnContentStream) {
    return m_sectionStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getGraphicStyle(const gchar* pStyleName,
                                          bool bOnContentStream) {
    return m_graphicStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTableStyle(const gchar* pStyleName,
                                                  bool bOnContentStream) {
                                                    
    return m_tableStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTableColumnStyle(
                                                  const gchar* pStyleName,
                                                  bool bOnContentStream) {
                                                    
    return m_tableColumnStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTableRowStyle(
                                                  const gchar* pStyleName,
                                                  bool bOnContentStream) {
                                                    
    return m_tableRowStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Office_Styles::getTableCellStyle(
                                                  const gchar* pStyleName,
                                                  bool bOnContentStream) {
                                                    
    return m_tableCellStyleStyles.getStyle(pStyleName, bOnContentStream);
}


/**
 * Adds a list style (<text:list-style>)
 */
ODi_Style_List* ODi_Office_Styles::addList(const gchar** ppAtts,
                                         ODi_ElementStack& rElementStack) {
    bool ok;
    const gchar* pAttrValue = NULL;
    ODi_Style_List* pStyle = NULL;

    pStyle = new ODi_Style_List(rElementStack);
    pAttrValue = UT_getAttribute("style:name", ppAtts);
                                            
    ok = m_listStyles.insert(pAttrValue, pStyle);
            
    UT_ASSERT(ok);
    
    return pStyle;
}


/**
 * Adds a notes configuration (<text:notes-configuration>)
 */
ODi_NotesConfiguration* ODi_Office_Styles::addNotesConfiguration(
                                               const gchar** ppAtts,
                                               ODi_ElementStack& rElementStack) {
    bool ok;
    const gchar* pAttrValue = NULL;
    ODi_NotesConfiguration* pNotesConfig = NULL;

    pNotesConfig = new ODi_NotesConfiguration(rElementStack);
    pAttrValue = UT_getAttribute("text:note-class", ppAtts);
                                            
    ok = m_notesConfigurations.insert(pAttrValue, pNotesConfig);
    UT_ASSERT(ok);
    
    return pNotesConfig;
}
