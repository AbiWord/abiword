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

#ifndef _ODI_STYLE_STYLE_FAMILY_H_
#define _ODI_STYLE_STYLE_FAMILY_H_

#include <map>
#include <string>

// Internal includes
#include "ODi_Style_Style.h"

// AbiWord includes
#include <ut_types.h>
#include <ut_string_class.h>

// Internal classes
class ODi_ElementStack;
class ODi_FontFaceDecls;

// AbiWord classes
class PD_Document;
class ODi_Abi_Data;
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
			     ODi_Abi_Data & rAbiData,
                             std::string* pReplacementName = NULL,
                             std::string* pReplacementDisplayName = NULL);

    ODi_Style_Style* addDefaultStyle(ODi_ElementStack& rElementStack,
				     ODi_Abi_Data & rAbiData) {
      m_pDefaultStyle = new ODi_Style_Style(rElementStack,rAbiData);
        return m_pDefaultStyle;
    }

    void fixStyles();
    void linkStyles();

    const ODi_Style_Style* getStyle(const gchar* pStyleName,
                                   bool bOnContentStream) const;

    const ODi_Style_Style* getDefaultStyle() const {
        return m_pDefaultStyle;
    }

    void removeStyleStyle(ODi_Style_Style* pRemovedStyle, bool bOnContentStream);

    void defineAbiStyles(PD_Document* pDocument) const;
    void buildAbiPropsAttrString(ODi_FontFaceDecls& rFontFaceDecls);

private:

    typedef std::map<std::string, ODi_Style_Style*> StyleMap;

    void _buildAbiPropsAttrString(ODi_FontFaceDecls& rFontFaceDecls,
                                  const StyleMap & map);
    void _findSuitableReplacement(std::string& rReplacementName,
                    const ODi_Style_Style* pRemovedStyle,
                    bool bOnContentStream);
    void _reparentStyles(const StyleMap & map, const std::string & removedName,
                         const std::string & replacementName);

    void _linkStyles(const StyleMap & map, bool onContentStream);
    void _removeEmptyStyles(const StyleMap & map, bool bOnContentStream);

    // Styles define inside the styles stream (<office:document-styles>).
    StyleMap m_styles;

    // Styles defined inside the content stream, only automatic styles.
    // (<office:document-content> element)
    StyleMap m_styles_contentStream;

    // <style:default-style style:family="...">
    ODi_Style_Style* m_pDefaultStyle;

    // The styles (<style:style> kind) that get removed due to lack of
    // properties have their names stored here.
    //
    // The key is the name of the removed style and its value is the name of
    // the style to replace him.
    std::map<std::string, std::string> m_removedStyleStyles;
    std::map<std::string, std::string> m_removedStyleStyles_contentStream;
};

#endif //_ODI_STYLE_STYLE_FAMILY_H_
