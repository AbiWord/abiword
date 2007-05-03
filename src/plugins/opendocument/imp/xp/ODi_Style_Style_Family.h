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

#ifndef _ODI_STYLE_STYLE_FAMILY_H_
#define _ODI_STYLE_STYLE_FAMILY_H_

// Internal includes
#include "ODi_Style_Style.h"

// AbiWord includes
#include <ut_types.h>
#include <ut_string_class.h>
#include <ut_hash.h>
#include <ut_IntStrMap.h>

// Internal classes
class ODi_ElementStack;
class ODi_FontFaceDecls;

// AbiWord classes
class PD_Document;

/**
 * Used by ODi_Office_Styles to handle a family of "style:style" styles.
 * Some family names: "text", "paragraph", "section", "graphic", "table", etc.
 */
class ODi_Style_Style_Family {
public:
    
    ODi_Style_Style_Family() : m_pDefaultStyle(NULL) {}
    virtual ~ODi_Style_Style_Family();
    
    ODi_Style_Style* addStyle(const gchar** ppAtts,
                             ODi_ElementStack& rElementStack,
                             UT_UTF8String* pReplacementName = NULL,
                             UT_UTF8String* pReplacementDisplayName = NULL);
                             
    ODi_Style_Style* addDefaultStyle(ODi_ElementStack& rElementStack) {
        m_pDefaultStyle = new ODi_Style_Style(rElementStack);
        return m_pDefaultStyle;
    }
    
    void fixStyles();
    void linkStyles();
    
    const ODi_Style_Style* getStyle(const gchar* pStyleName,
                                   bool bOnContentStream);
                                   
    const ODi_Style_Style* getDefaultStyle() const {
        return m_pDefaultStyle;
    }
    
    void removeStyleStyle(ODi_Style_Style* pRemovedStyle, bool bOnContentStream);
    
    void defineAbiStyles(PD_Document* pDocument) const;
    void buildAbiPropsAttrString(ODi_FontFaceDecls& rFontFaceDecls);
    
private:

    void _findSuitableReplacement(UT_UTF8String& rReplacementName,
                    const ODi_Style_Style* pRemovedStyle,
                    bool bOnContentStream);
                    
    void _linkStyles(bool onContentStream);
    
    // Styles define inside the styles stream (<office:document-styles>).
    UT_GenericStringMap<ODi_Style_Style*> m_styles;
    
    // Styles defined inside the content stream, only automatic styles.
    // (<office:document-content> element)
    UT_GenericStringMap<ODi_Style_Style*> m_styles_contentStream;
    
    // <style:default-style style:family="...">
    ODi_Style_Style* m_pDefaultStyle;
    
    // The styles (<style:style> kind) that get removed due to lack of
    // properties have their names stored here.
    //
    // The key is the name of the removed style and its value is the name of
    // the style to replace him.
    UT_UTF8Hash m_removedStyleStyles;
    UT_UTF8Hash m_removedStyleStyles_contentStream;
};

#endif //_ODI_STYLE_STYLE_FAMILY_H_
