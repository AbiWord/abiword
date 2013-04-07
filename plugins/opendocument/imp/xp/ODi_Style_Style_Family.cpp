/* AbiSource
 * 
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
 * Copryight (C) 2009 Hubert Figuiere
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
#include "ODi_Style_Style_Family.h"

// Internal includes
#include "ODi_ElementStack.h"
#include "ODi_Abi_Data.h"

// AbiWord includes
#include "ut_misc.h"
#include "ut_std_map.h"

/**
 * Destructor
 */
ODi_Style_Style_Family::~ODi_Style_Style_Family() 
{
    UT_map_delete_all_second(m_styles);
    UT_map_delete_all_second(m_styles_contentStream);
    
    DELETEP(m_pDefaultStyle);
}


/**
 * @param pReplacementName Name to replace the actual name of the style that is
 *                         being added. NULL if the name shouldn't be replaced.
 */
ODi_Style_Style* ODi_Style_Style_Family::addStyle(const gchar** ppAtts,
                             ODi_ElementStack& rElementStack,
			     ODi_Abi_Data & rAbiData,
                             std::string* pReplacementName,
                             std::string* pReplacementDisplayName) {
                                
    ODi_Style_Style* pStyle = NULL;
    bool bOnContentStream;
    const gchar* pName;
    
    bOnContentStream = rElementStack.hasElement("office:document-content");
    
    pName = UT_getAttribute("style:name", ppAtts);
    UT_ASSERT(pName);

    
    if (bOnContentStream) {
        
        if (pReplacementName) {
            StyleMap::const_iterator iter = m_styles_contentStream.find(pReplacementName->c_str());

            if (iter == m_styles_contentStream.end()) {
	      pStyle = new ODi_Style_Style(rElementStack,rAbiData);
                
                m_styles_contentStream.insert(std::make_pair(pReplacementName->c_str(),
                                                                  pStyle));
                                                   
                pStyle->setName(*pReplacementName);
                pStyle->setDisplayName(*pReplacementDisplayName);
            }
            
        } else {
            StyleMap::const_iterator iter = m_styles_contentStream.find(pName);

            if (iter == m_styles_contentStream.end()) {
	      pStyle = new ODi_Style_Style(rElementStack,rAbiData);
                
                m_styles_contentStream.insert(std::make_pair(pName, pStyle));
            }
        }
        
        
        
    } else {
        
        if (pReplacementName) {
            StyleMap::const_iterator iter = m_styles.find(pReplacementName->c_str());
            
            if (iter == m_styles.end()) {
	      pStyle = new ODi_Style_Style(rElementStack,rAbiData);
                
                m_styles.insert(std::make_pair(pReplacementName->c_str(), pStyle));
                                                   
                pStyle->setName(*pReplacementName);
                pStyle->setDisplayName(*pReplacementDisplayName);
            }
            
        } else {
            StyleMap::const_iterator iter = m_styles.find(pName);
            
            if (iter == m_styles.end()) {
	      pStyle = new ODi_Style_Style(rElementStack,rAbiData);
                
                m_styles.insert(std::make_pair(pName, pStyle));
            }
        }
    }


    if (pReplacementName != NULL) {
        std::string originalName = pName;
        
        if (bOnContentStream) {
            m_removedStyleStyles_contentStream[pName]
				= pReplacementName->c_str();
        } else {
            m_removedStyleStyles[pName] = pReplacementName->c_str();
        }
	}
    
    return pStyle;
}



/** Remove any style that doesn't have properties */
void ODi_Style_Style_Family::_removeEmptyStyles(const StyleMap & map, bool bOnContentStream)
{
    ODi_Style_Style* pStyle = NULL;
    bool foundNone;
    

    if(map.empty()) {
        return;
    }
    // TODO still very inefficient... but more than it used
    // to be.
    do {

        foundNone = true;

        for (StyleMap::const_iterator iter = map.begin();
             iter != map.end(); ++iter) {
            if ( !iter->second->hasProperties() ) {
                pStyle = iter->second;
                foundNone = false;
                break;
            }
        }


        if (pStyle) {
            removeStyleStyle(pStyle, bOnContentStream);
            DELETEP(pStyle);
        }
    } while(!foundNone);
}


/**
 * Fix any problems encountered on the added styles.
 */
void ODi_Style_Style_Family::fixStyles()
{
    // Problem 1: We can't have styles without properties
    //
    // The "Standard" paragraph style usually comes empty
    // (I have never seen otherwise)
    
    _removeEmptyStyles(m_styles, false);
    _removeEmptyStyles(m_styles_contentStream, true);
}


void ODi_Style_Style_Family::_buildAbiPropsAttrString(ODi_FontFaceDecls& rFontFaceDecls,
                                                      const StyleMap & map)
{
    for(StyleMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
        iter->second->buildAbiPropsAttrString(rFontFaceDecls);
    }
}

/**
 * 
 */
void ODi_Style_Style_Family::buildAbiPropsAttrString(
                                            ODi_FontFaceDecls& rFontFaceDecls)
{
    if (m_pDefaultStyle != NULL) {
        m_pDefaultStyle->buildAbiPropsAttrString(rFontFaceDecls);
    }
    
    _buildAbiPropsAttrString(rFontFaceDecls, m_styles);
    _buildAbiPropsAttrString(rFontFaceDecls, m_styles_contentStream);
}



void ODi_Style_Style_Family::_reparentStyles(const StyleMap & map, const std::string & removedName,
                                             const std::string & replacementName)
{
    for(StyleMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
        
        ODi_Style_Style* pStyle = iter->second;
        
        if (pStyle->getParentName() == removedName) {
            pStyle->setParentName(replacementName);
        }
        
        if (pStyle->getNextStyleName() == removedName) {
            pStyle->setNextStyleName(replacementName);
        }
    }
}


/**
 * 
 */
void ODi_Style_Style_Family::removeStyleStyle(ODi_Style_Style* pRemovedStyle,
                                             bool bOnContentStream) 
{
    std::string replacementName;


    _findSuitableReplacement(replacementName, pRemovedStyle, bOnContentStream);
    
    // Remove the style itself
    if (bOnContentStream) {
        m_styles_contentStream.erase(pRemovedStyle->getName().c_str());
            
        m_removedStyleStyles_contentStream[pRemovedStyle->getName().c_str()]
			= replacementName.c_str();
    } else {
        m_styles.erase(pRemovedStyle->getName().c_str());
        m_removedStyleStyles[pRemovedStyle->getName().c_str()] = replacementName.c_str();
    }
    


    // Fix all references to it.
    // Note that automatic styles can't refer to each other.
    
    if (pRemovedStyle->isAutomatic()) {
        // It's an automatic style, nobody have references him.
        return;
    }
    
    if (replacementName == "<NULL>") {
        replacementName.clear();
    }
    
    // Some automatic styles defined on the content stream may have
    // references to this common style.
    _reparentStyles(m_styles_contentStream, pRemovedStyle->getName(),
                    replacementName);
    // Now fix references from the styles defined on the styles stream.
    _reparentStyles(m_styles, pRemovedStyle->getName(),
                    replacementName);    
}


/**
 * 
 */
void ODi_Style_Style_Family::defineAbiStyles(PD_Document* pDocument) const 
{
    if (m_pDefaultStyle) {
        m_pDefaultStyle->defineAbiStyle(pDocument);
    }
    
    for(StyleMap::const_iterator iter = m_styles.begin();
        iter != m_styles.end(); ++iter) {

        iter->second->defineAbiStyle(pDocument);
    }
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
                                            std::string& rReplacementName,
                                            const ODi_Style_Style* pRemovedStyle,
                                            bool bOnContentStream)
{
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

    ODi_Style_Style* pStyle = NULL;
    
    if (bOnContentStream) {
        StyleMap::const_iterator iter = m_styles_contentStream.find(pRemovedStyle->getParentName().c_str());
        if(iter !=  m_styles_contentStream.end()) {
            pStyle = iter->second;
        }
    }

    if (!pStyle) {
        // Must be a regular style, defined on the Styles stream.
        StyleMap::const_iterator iter = m_styles.find(pRemovedStyle->getParentName().c_str());
        if(iter !=  m_styles.end()) {
            pStyle = iter->second;
        }
    }
    
    
    if (pStyle) {
        if (pStyle->hasProperties()) {
            // Alright, we've found it.
            rReplacementName = pStyle->getName();
        } 
        else {
            // Let's look deeper
            _findSuitableReplacement(rReplacementName, pStyle, bOnContentStream);
        }
    } else {
        std::string aString;
        // Was this parent already removed?
        if (bOnContentStream) {
            aString = m_removedStyleStyles_contentStream[pRemovedStyle->getParentName().c_str()];
        }
        
        if (!pStyle) {
            aString = m_removedStyleStyles[pRemovedStyle->getParentName().c_str()];
        }
        
        if(!aString.empty()) {
            rReplacementName = aString;
        }
        else {
            // I give up...
            if (m_pDefaultStyle) {
                // Pretty simple. We use the default style.
                if (*(pRemovedStyle->getFamily()) == "paragraph") {
                    // AbiWord uses "Normal" as the name of its default style.
                    rReplacementName = "Normal";
                } 
                else {
                    rReplacementName = m_pDefaultStyle->getName();
                }
            } 
            else {
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
void ODi_Style_Style_Family::linkStyles() 
{
    _linkStyles(m_styles, false);
    _linkStyles(m_styles_contentStream, true);
}


/**
 * 
 */
const ODi_Style_Style* ODi_Style_Style_Family::getStyle(const gchar* pStyleName,
                                                      bool bOnContentStream) const
{
    UT_return_val_if_fail(pStyleName, NULL);

    const ODi_Style_Style* pStyle = NULL;
    
    // Is it the default style?
    if (m_pDefaultStyle && (m_pDefaultStyle->getName() == pStyleName)) {
        pStyle = m_pDefaultStyle;
    }
    
    if (!pStyle) {
        // It's not the default style. Let's search our style lists.
        
        if (bOnContentStream) {
            StyleMap::const_iterator iter = m_styles_contentStream.find(pStyleName);
            if(iter !=  m_styles_contentStream.end()) {
                pStyle = iter->second;
            }
        }

        if (!pStyle) {
            // Must be a regular style, defined on the Styles stream.
            StyleMap::const_iterator iter = m_styles.find(pStyleName);
            if(iter !=  m_styles.end()) {
                pStyle = iter->second;
            }
        }
    }
    
    if (!pStyle) {
        // We haven't found it. Have we removed it (done on _fixStyles())?
        
        std::string replacementName;
        
        if (bOnContentStream) {
            std::map<std::string, std::string>::const_iterator iter
                = m_removedStyleStyles_contentStream.find(pStyleName);
            if(iter != m_removedStyleStyles_contentStream.end()) {
                replacementName = iter->second;
            }
        }

        if (replacementName.empty()) {
            std::map<std::string, std::string>::const_iterator iter
                = m_removedStyleStyles.find(pStyleName);
            if(iter != m_removedStyleStyles.end()) {
                replacementName = iter->second;
            }
        }
        
        if (!replacementName.empty()) {
            // We will send back its replacement.
            return getStyle(replacementName.c_str(),
                            bOnContentStream);
        } else {
            // This style never existed.
            // Let's return the default one instead, if there is one.
            if (m_pDefaultStyle != NULL) {
                pStyle = m_pDefaultStyle;
            } 
            else {
                pStyle = NULL;
            }
        }
        
    }
    
    return pStyle;
}


/**
 * Helper function for linkStyles()
 */   
void ODi_Style_Style_Family::_linkStyles(const StyleMap & map, bool onContentStream)
{
    ODi_Style_Style* pStyle;
    const ODi_Style_Style* pOtherStyle;
    
    for(StyleMap::const_iterator iter = map.begin();
        iter != map.end(); ++iter) {

        pStyle = iter->second;
        
        // Link to its parent style, if there is one.
        if (!pStyle->getParentName().empty()) {
            
            pOtherStyle = getStyle(pStyle->getParentName().c_str(),
                                         onContentStream);

            if (pOtherStyle) {
                pStyle->setParentStylePointer(pOtherStyle);
            } 
            else {
                // We don't have this style!
                // Let's pretend that it never existed.
                UT_ASSERT_HARMLESS(pOtherStyle);
                pStyle->setParentName(NULL);
            }
        }
        
        // Link to its next style, if there is one.
        if (!pStyle->getNextStyleName().empty()) {
            
            pOtherStyle = getStyle(pStyle->getNextStyleName().c_str(),
                                         onContentStream);

            if (pOtherStyle) {
                pStyle->setNextStylePointer(pOtherStyle);
            } 
            else {
                // We don't have this style!
                // Let's pretend that it never existed.
                UT_ASSERT_HARMLESS(pOtherStyle);
                pStyle->setNextStyleName(NULL);
            }
        }
    }
}
