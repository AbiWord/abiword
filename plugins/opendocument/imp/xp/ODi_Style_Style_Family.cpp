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
#include "ODi_Style_Style_Family.h"

// Internal includes
#include "ODi_ElementStack.h"

// AbiWord includes
#include "ut_misc.h"


/**
 * Destructor
 */
ODi_Style_Style_Family::~ODi_Style_Style_Family() {
    
    UT_GenericVector<ODi_Style_Style*>* pStyleVector;
    UT_uint32 i, count;
    
    pStyleVector = m_styles.enumerate();
    count = pStyleVector->getItemCount();
    for (i=0; i<count; i++) {
        delete (*pStyleVector)[i];
    }
	DELETEP(pStyleVector);
    
    pStyleVector = m_styles_contentStream.enumerate();
    count = pStyleVector->getItemCount();
    for (i=0; i<count; i++) {
        delete (*pStyleVector)[i];
    }
	DELETEP(pStyleVector);
    
    DELETEP(m_pDefaultStyle);
}


/**
 * @param pReplacementName Name to replace the actual name of the style that is
 *                         being added. NULL if the name shouldn't be replaced.
 */
ODi_Style_Style* ODi_Style_Style_Family::addStyle(const gchar** ppAtts,
                             ODi_ElementStack& rElementStack,
                             UT_UTF8String* pReplacementName,
                             UT_UTF8String* pReplacementDisplayName) {
                                
    ODi_Style_Style* pStyle;
    bool bOnContentStream;
    bool ok = true;
    const gchar* pName;
    
    bOnContentStream = rElementStack.hasElement("office:document-content");
    
    pName = UT_getAttribute("style:name", ppAtts);
    UT_ASSERT(pName);
    
    if (bOnContentStream) {
        
        if (pReplacementName) {
            pStyle = m_styles_contentStream.pick(pReplacementName->utf8_str());
            
            if (pStyle == NULL) {
                pStyle = new ODi_Style_Style(rElementStack);
                
                ok = m_styles_contentStream.insert(pReplacementName->utf8_str(),
                                                   pStyle);
                                                   
                pStyle->setName(*pReplacementName);
                pStyle->setDisplayName(*pReplacementDisplayName);
            }
            
        } else {
            pStyle = m_styles_contentStream.pick(pName);
            
            if (pStyle == NULL) {
                pStyle = new ODi_Style_Style(rElementStack);
                
                ok = m_styles_contentStream.insert(pName, pStyle);
            }
        }
        
        
        
    } else {
        
        if (pReplacementName) {
            pStyle = m_styles.pick(pReplacementName->utf8_str());
            
            if (pStyle == NULL) {
                pStyle = new ODi_Style_Style(rElementStack);
                
                ok = m_styles.insert(pReplacementName->utf8_str(),
                                                   pStyle);
                                                   
                pStyle->setName(*pReplacementName);
                pStyle->setDisplayName(*pReplacementDisplayName);
            }
            
        } else {
            pStyle = m_styles.pick(pName);
            
            if (pStyle == NULL) {
                pStyle = new ODi_Style_Style(rElementStack);
                
                ok = m_styles.insert(pName, pStyle);
            }
        }
    }
    UT_ASSERT(ok);


    if (pReplacementName != NULL) {
        UT_UTF8String originalName = pName;
        
        if (bOnContentStream) {
            m_removedStyleStyles_contentStream[pName]
				= pReplacementName->utf8_str();
        } else {
            m_removedStyleStyles[pName] = pReplacementName->utf8_str();
        }
	}
    
    return pStyle;
}


/**
 * Fix any problems encountered on the added styles.
 */
void ODi_Style_Style_Family::fixStyles() {
    // Problem 1: We can't have styles without properties
    //
    // The "Standard" paragraph style usually comes empty
    // (I have never seen otherwise)
    
    UT_uint32 i, count;
    UT_GenericVector<ODi_Style_Style*>* pStylesVec;
    ODi_Style_Style* pStyle = NULL;
    bool noneFound;
    
    do {
        pStylesVec = m_styles.enumerate();
        if(!pStylesVec) {
           UT_ASSERT_HARMLESS(pStylesVec);
           break; //using continue here seems like it could cause an infinite loop?
        }
        
        noneFound = true;
        count = pStylesVec->getItemCount();
        for (i=0; i<count; i++) {
            if ( !((*pStylesVec)[i]->hasProperties()) ) {
                pStyle = (*pStylesVec)[i];
                i=count;
                noneFound = false;
            }
        }

        DELETEP(pStylesVec);

        if (!noneFound) {
            removeStyleStyle(pStyle, false);
        }
    } while (!noneFound);
    
    
    
    do {
        pStylesVec = m_styles_contentStream.enumerate();
        if(!pStylesVec) {
           UT_ASSERT_HARMLESS(pStylesVec);
           break; //using continue here seems like it could cause an infinite loop?
        }
        
        noneFound = true;
        count = pStylesVec->getItemCount();
        for (i=0; i<count; i++) {
            if ( !((*pStylesVec)[i]->hasProperties()) ) {
                pStyle = (*pStylesVec)[i];
                i=count;
                noneFound = false;
            }
        }
        DELETEP(pStylesVec);
        if (!noneFound) {
            removeStyleStyle(pStyle, true);
        }
    } while (!noneFound);
}


/**
 * 
 */
void ODi_Style_Style_Family::buildAbiPropsAttrString(
                                            ODi_FontFaceDecls& rFontFaceDecls) {
    
    UT_uint32 i, count;
    UT_GenericVector<ODi_Style_Style*>* pStylesVec;
    
    if (m_pDefaultStyle != NULL) {
        m_pDefaultStyle->buildAbiPropsAttrString(rFontFaceDecls);
    }
    
    pStylesVec = m_styles.enumerate();
    UT_return_if_fail(pStylesVec);
    count = pStylesVec->getItemCount();
    for (i=0; i<count; i++) {
        (*pStylesVec)[i]->buildAbiPropsAttrString(rFontFaceDecls);
    }
	DELETEP(pStylesVec);
    
    pStylesVec = m_styles_contentStream.enumerate();
    UT_return_if_fail(pStylesVec);
    count = pStylesVec->getItemCount();
    for (i=0; i<count; i++) {
        (*pStylesVec)[i]->buildAbiPropsAttrString(rFontFaceDecls);
    }
	DELETEP(pStylesVec);

}


/**
 * 
 */
void ODi_Style_Style_Family::removeStyleStyle(ODi_Style_Style* pRemovedStyle,
                                             bool bOnContentStream) {
    UT_uint32 i, count;
    UT_GenericVector<ODi_Style_Style*>* pStylesVec;
    UT_UTF8String styleName;
    UT_UTF8String replacementName;


    _findSuitableReplacement(replacementName, pRemovedStyle, bOnContentStream);
    
    // Remove the style itself
    if (bOnContentStream) {
        m_styles_contentStream.remove(
            pRemovedStyle->getName().utf8_str(), NULL);
            
        m_removedStyleStyles_contentStream[pRemovedStyle->getName().utf8_str()]
			= replacementName.utf8_str();
    } else {
        m_styles.remove(pRemovedStyle->getName().utf8_str(), NULL);
        m_removedStyleStyles[pRemovedStyle->getName().utf8_str()] = replacementName.utf8_str();
    }
    


    // Fix all references to it.
    // Note that automatic styles can't refer to each other.
    
    if (pRemovedStyle->isAutomatic()) {
        // It's an automatic style, nobody have references him.
        return;
    }
    
    if (!strcmp(replacementName.utf8_str(), "<NULL>")) {
        replacementName.clear();
    }
    
    // Some automatic styles defined on the content stream may have
    // references to this common style.
    pStylesVec = m_styles_contentStream.enumerate();

    UT_return_if_fail(pStylesVec);
    
    count = pStylesVec->getItemCount();
    for (i=0; i<count; i++) {
        if ((*pStylesVec)[i]->getParentName() == pRemovedStyle->getName()) {
            (*pStylesVec)[i]->setParentName(replacementName);
        }
        
        if ((*pStylesVec)[i]->getNextStyleName() == pRemovedStyle->getName()) {
            (*pStylesVec)[i]->setNextStyleName(replacementName);
        }
    }
    DELETEP(pStylesVec);

    // Now fix references from the styles defined on the styles stream.
    pStylesVec = m_styles.enumerate();

    UT_return_if_fail(pStylesVec);
    
    count = pStylesVec->getItemCount();
    for (i=0; i<count; i++) {
        if ((*pStylesVec)[i]->getParentName() == pRemovedStyle->getName()) {
            (*pStylesVec)[i]->setParentName(replacementName);
        }
        
        if ((*pStylesVec)[i]->getNextStyleName() == pRemovedStyle->getName()) {
            (*pStylesVec)[i]->setNextStyleName(replacementName);
        }
    }
    DELETEP(pStylesVec);
}


/**
 * 
 */
void ODi_Style_Style_Family::defineAbiStyles(PD_Document* pDocument) const {
    UT_uint32 i, count;
    UT_GenericVector<ODi_Style_Style*>* pStylesVec;
    
    if (m_pDefaultStyle) {
        m_pDefaultStyle->defineAbiStyle(pDocument);
    }
    
    pStylesVec = m_styles.enumerate();
    UT_return_if_fail(pStylesVec);
    
    count = pStylesVec->getItemCount();
    for (i=0; i<count; i++) {
        (*pStylesVec)[i]->defineAbiStyle(pDocument);
    }
	DELETEP(pStylesVec);
}


/**
 * Finds a suitable replacement for the style that will be removed.
 * 
 * A suitable replacement is a parent style that has properties or, if none is
 * found, the default paragraph style.
 * 
 * @param rReplacementName Will receive the name of the replacement style.
 * @param pRemovedStyle The style that will be removed.
 */
void ODi_Style_Style_Family::_findSuitableReplacement(
                                            UT_UTF8String& rReplacementName,
                                            const ODi_Style_Style* pRemovedStyle,
                                            bool bOnContentStream) {
    // Test for a "dead-end"
    if (pRemovedStyle->getParentName().empty()) {
        
        if (m_pDefaultStyle) {
            // Pretty simple. We use the default style.
            if (*(pRemovedStyle->getFamily()) == "paragraph") {
                // AbiWord uses "Normal" as the name of its default style.
                rReplacementName = "Normal";
            } else {
                rReplacementName = m_pDefaultStyle->getName();
            }
        } else {
            // We have no choice. There will be no substitute for this style.
            rReplacementName = "<NULL>";
        }
        
        return;
    }

    ODi_Style_Style* pStyle;
    
    if (bOnContentStream) {
        pStyle = m_styles_contentStream.pick(
                    pRemovedStyle->getParentName().utf8_str());
        
        if (!pStyle) {
            // Must be a regular style, defined on the Styles stream.
            pStyle = m_styles.pick(
                        pRemovedStyle->getParentName().utf8_str());
        }
        
    } else {
        pStyle = m_styles.pick(
                    pRemovedStyle->getParentName().utf8_str());
    }
    
    
    if (pStyle) {
        if (pStyle->hasProperties()) {
            // Alright, we've found it.
            rReplacementName = pStyle->getName();
        } else {
            // Let's look deeper
            _findSuitableReplacement(rReplacementName, pStyle, bOnContentStream);
        }
    } else {
        std::string aString;
        // Was this parent already removed?
        if (bOnContentStream) {
            aString = m_removedStyleStyles_contentStream[
                                    pRemovedStyle->getParentName().utf8_str()];
        }
        
        if (!pStyle) {
            aString = m_removedStyleStyles[pRemovedStyle->getParentName()
										   .utf8_str()];
        }
        
        if(!aString.empty()) {
            rReplacementName = aString.c_str();
        } else {
            // I give up...
            if (m_pDefaultStyle) {
                // Pretty simple. We use the default style.
                if (*(pRemovedStyle->getFamily()) == "paragraph") {
                    // AbiWord uses "Normal" as the name of its default style.
                    rReplacementName = "Normal";
                } else {
                    rReplacementName = m_pDefaultStyle->getName();
                }
            } else {
                // We have no choice. There will be no substitute for this style.
                rReplacementName = "<NULL>";
            }
        }
    }
}


/**
 * It links, if applicable, each style with its parent and its next style.
 * 
 * By "linking" I mean that a given style will have a pointer to its parent
 * and its next style.
 */
void ODi_Style_Style_Family::linkStyles() {
    _linkStyles(false);
    _linkStyles(true);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Style_Style_Family::getStyle(const gchar* pStyleName,
                                                      bool bOnContentStream) {
    const ODi_Style_Style* pStyle = NULL;
    
    // Is it the default style?
    if (m_pDefaultStyle != NULL) {
        if (!strcmp(m_pDefaultStyle->getName().utf8_str(), pStyleName)) {
            pStyle = m_pDefaultStyle;
        }
    }
    
    if (!pStyle) {
        // It's not the default style. Let's search our style lists.
        
        if (bOnContentStream) {
            pStyle = m_styles_contentStream.pick(pStyleName);
            if (!pStyle) {
                // Should be a regular style (not automatic).
                pStyle = m_styles.pick(pStyleName);
            }
        } else {
            pStyle = m_styles.pick(pStyleName);
        }
    }
    
    if (!pStyle) {
        // We haven't found it. Have we removed it (done on _fixStyles())?
        
        std::string replacementName;
        
        if (bOnContentStream) {
            replacementName = m_removedStyleStyles_contentStream[pStyleName];
            
            if (replacementName.empty()) {
                replacementName = m_removedStyleStyles[pStyleName];
            }
        } else {
            replacementName = m_removedStyleStyles[pStyleName];
        }
        
        if (!replacementName.empty()) {
            // We will send back its replacement.
            return this->getStyle(replacementName.c_str(),
                                           bOnContentStream);
        } else {
            // This style never existed.
            // Let's return the default one instead, if there is one.
            if (m_pDefaultStyle != NULL) {
                pStyle = m_pDefaultStyle;
            } else {
                pStyle = NULL;
            }
        }
        
    }
    
    return pStyle;
}


/**
 * Helper function for linkStyles()
 */   
void ODi_Style_Style_Family::_linkStyles(bool onContentStream) {
    UT_GenericVector<ODi_Style_Style*>* pStylesVec;
    UT_uint32 i, count;
    ODi_Style_Style* pStyle;
    const ODi_Style_Style* pOtherStyle;
    
    if (onContentStream) {
        pStylesVec = m_styles_contentStream.enumerate();
    } else {
        pStylesVec = m_styles.enumerate();
    }
    UT_return_if_fail(pStylesVec);
    
    count = pStylesVec->getItemCount();
    for (i=0; i<count; i++) {
        pStyle = (*pStylesVec)[i];
        
        // Link to its parent style, if there is one.
        if (!pStyle->getParentName().empty()) {
            
            pOtherStyle = this->getStyle(pStyle->getParentName().utf8_str(),
                                         onContentStream);
            
            UT_ASSERT_HARMLESS(pOtherStyle);
            
            if (pOtherStyle) {
                pStyle->setParentStylePointer(pOtherStyle);
            } else {
                // We don't have this style!
                // Let's pretend that it never existed.
                pStyle->setParentName(NULL);
            }
        }
        
        // Link to its next style, if there is one.
        if (!pStyle->getNextStyleName().empty()) {
            
            pOtherStyle = this->getStyle(pStyle->getNextStyleName().utf8_str(),
                                         onContentStream);
                
            UT_ASSERT_HARMLESS(pOtherStyle);
                
            if (pOtherStyle) {
                pStyle->setNextStylePointer(pOtherStyle);
            } else {
                // We don't have this style!
                // Let's pretend that it never existed.
                pStyle->setNextStyleName(NULL);
            }
        }
    }
	DELETEP(pStylesVec);
}
