/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* AbiSource
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
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
#include "ODi_Style_Style.h"
 
// Internal includes
#include "ODi_FontFaceDecls.h"
#include "ODi_ListenerStateAction.h"
#include "ODi_ElementStack.h"
#include "ODi_StartTag.h"
#include "ODi_Abi_Data.h"

// AbiWord includes
#include <pd_Document.h>
#include <ut_math.h>
#include <ut_locale.h>
#include <ut_std_string.h>

// External includes
#include <ctype.h>


/**
 * Constructor
 */
ODi_Style_Style::ODi_Style_Style(ODi_ElementStack& rElementStack,
				  ODi_Abi_Data & rAbiData) :
                       ODi_ListenerState("StyleStyle", rElementStack),
                       m_pParentStyle(NULL),
                       m_pNextStyle(NULL),
                       m_haveTopBorder(HAVE_BORDER_UNSPECIFIED),
                       m_haveBottomBorder(HAVE_BORDER_UNSPECIFIED),
                       m_haveLeftBorder(HAVE_BORDER_UNSPECIFIED),
                       m_haveRightBorder(HAVE_BORDER_UNSPECIFIED),
		       m_rAbiData(rAbiData)
{
    if (rElementStack.hasElement("office:automatic-styles")) {
        m_bAutomatic = true;
    } else {
        m_bAutomatic = false;
    }
}


/**
 * 
 */
void ODi_Style_Style::startElement(const gchar* pName,
                                  const gchar** ppAtts,
								   ODi_ListenerStateAction& /*rAction*/) 
{

    if (!strcmp("style:style", pName)) {
        
        _parse_style_style(ppAtts);
        
    } else if (!strcmp("style:paragraph-properties", pName)) {
        
        _parse_style_paragraphProperties(ppAtts);

    } else if (!strcmp("style:tab-stop", pName)) {

        if (m_rElementStack.getStackSize() >= 2 &&
            !strcmp(m_rElementStack.getStartTag(1)->getName(), "style:paragraph-properties") &&
            !strcmp(m_rElementStack.getStartTag(0)->getName(), "style:tab-stops")) {

            _parse_style_tabStopProperties(ppAtts);
            
        } else {

            // we only know tabstops inside paragraphs
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            
        }
        
    } else if (!strcmp("style:text-properties", pName)) {
        
        _parse_style_textProperties(ppAtts);
        
    } else if (!strcmp("style:section-properties", pName)) {
        
        _parse_style_sectionProperties(ppAtts);
        
    } else if (!strcmp("style:graphic-properties", pName)) {
        
        _parse_style_graphicProperties(ppAtts);
        
    } else if (!strcmp("style:table-properties", pName)) {
        
        _parse_style_tableProperties(ppAtts);
        
    } else if (!strcmp("style:table-column-properties", pName)) {
        
        _parse_style_tableColumnProperties(ppAtts);
        
    } else if (!strcmp("style:table-row-properties", pName)) {
        
        _parse_style_tableRowProperties(ppAtts);
        
    } else if (!strcmp("style:table-cell-properties", pName)) {
        
        _parse_style_tableCellProperties(ppAtts);
        
    } else if (!strcmp("style:background-image", pName)) {
        
        _parse_style_background_image(ppAtts);
        
    } else if (!strcmp("style:default-style", pName)) {
        
        const gchar* pAttr;
        
        pAttr = UT_getAttribute("style:family", ppAtts);
        UT_ASSERT(pAttr);
        m_family = pAttr;
        
        // In AbiWord, the default style is called "Normal"
        m_displayName = m_name = "Normal";
        m_parentStyleName = "None";
        
    } else if (!strcmp("style:columns", pName)) {
        
        const gchar* pVal;
        
        pVal = UT_getAttribute("fo:column-count", ppAtts);
        if (pVal) {
            // A column count of "0" (zero) crashes AbiWord.
            // Instead we just leave the column count empty.
            if (atoi(pVal) > 0) {
                m_columns = pVal;
            }
        }
        
        pVal = UT_getAttribute("fo:column-gap", ppAtts);
        if (pVal) {
            m_columnGap = pVal;
        }
    }
    
}





/**
 * 
 */
void ODi_Style_Style::endElement(const gchar* pName,
                                ODi_ListenerStateAction& rAction) {
                                    
    if (!strcmp("style:style", pName)) {
        rAction.popState();
        
    } else if (!strcmp("style:default-style", pName)) {
        // I'm a default style.
        rAction.popState();
    }
}





/**
 * 
 */
void ODi_Style_Style::_parse_style_style(const gchar** ppAtts) {
    
    const gchar* pAttr;
    
    if (m_name.empty()) {
        pAttr = UT_getAttribute("style:name", ppAtts);
        UT_ASSERT(pAttr);
        m_name = pAttr;
    }

    pAttr = UT_getAttribute("style:family", ppAtts);
    UT_ASSERT(pAttr);
    m_family = pAttr;

    if (m_displayName.empty()) {
        pAttr = UT_getAttribute("style:display-name", ppAtts);
        if (pAttr) {
            m_displayName = pAttr;
        } else {
            m_displayName = m_name;
        }
    }
    
    pAttr = UT_getAttribute("style:parent-style-name", ppAtts);
    if (pAttr) {
        m_parentStyleName = pAttr;
    } else {
        m_parentStyleName.clear();
    }
    
    pAttr = UT_getAttribute("style:next-style-name", ppAtts);
    if (pAttr) {
        m_nextStyleName = pAttr;
    } else {
        m_nextStyleName = m_name;
    }
    
    pAttr = UT_getAttribute("style:list-style-name", ppAtts);
    if (pAttr) {
        m_listStyleName = pAttr;
    } else {
        m_listStyleName.clear();
    }
    
    pAttr = UT_getAttribute("style:master-page-name", ppAtts);
    if (pAttr) {
        m_masterPageName = pAttr;
    } else {
        m_masterPageName.clear();
    }
}





/**
 * 
 */
void ODi_Style_Style::_parse_style_paragraphProperties(const gchar** ppProps) {
    
    const gchar* pVal;
    
    pVal = UT_getAttribute ("style:line-height-at-least", ppProps);
    if (pVal) {
        m_lineHeight = UT_std_string_sprintf ("%s+", pVal);
    }
    
    pVal = UT_getAttribute ("fo:line-height", ppProps);
    if (pVal) {
        if (strstr(pVal, "%") != NULL) {
            int spacing;
	    UT_LocaleTransactor lt(LC_NUMERIC, "C");            

            sscanf(pVal, "%d%%", &spacing);
            m_lineHeight = UT_std_string_sprintf ("%f",
                                              (double)spacing/100.);
        } else {
            m_lineHeight.assign(pVal);
        }
    }   
    
    pVal = UT_getAttribute ("fo:text-align", ppProps);
    if (pVal) {
        if (!strcmp(pVal, "end")) {
            m_align = "right";
        } else if (!strcmp(pVal, "center")) {
            m_align = "center";
        } else if (!strcmp(pVal, "justify")) {
            m_align = "justify";
        } else {
            m_align = "left";
        }
    }
    
    pVal = UT_getAttribute ("fo:break-after", ppProps);
    if (pVal) {
        m_breakAfter.assign(pVal);
    }
    
    pVal = UT_getAttribute("fo:widows", ppProps);
    if (pVal) {
        int widows = 0;
        sscanf(pVal, "%d", &widows);
        m_widows = UT_std_string_sprintf ("%d", widows);
    }    
    
    pVal = UT_getAttribute("fo:orphans", ppProps);
    if (pVal) {
        int orphans = 0;
        sscanf (pVal, "%d", &orphans);
        m_orphans = UT_std_string_sprintf ("%d", orphans);
    }
    
    pVal = UT_getAttribute ("fo:margin-left", ppProps);
    if(pVal) {
        m_marginLeft.assign(pVal);
    }
        
    pVal = UT_getAttribute ("fo:margin-right", ppProps);
    if(pVal) {
        m_marginRight.assign(pVal);
    }
    
    pVal = UT_getAttribute ("fo:margin-top", ppProps);
    if(pVal) {
        m_marginTop.assign(pVal);
    }
    
    pVal = UT_getAttribute ("fo:margin-bottom", ppProps);
    if(pVal) {
        m_marginBottom.assign(pVal);
    }
    
    pVal = UT_getAttribute ("fo:break-before", ppProps);
    if (pVal) {
        m_breakBefore = pVal;
    }
    
    pVal = UT_getAttribute ("fo:background-color", ppProps);
    if(pVal) {
        m_bgcolor.assign(pVal);
    }
    
    pVal = UT_getAttribute("fo:keep-with-next", ppProps);
    if (pVal) {
        if ( !strcmp(pVal, "always") ) {
            m_keepWithNext = "yes";
        } else {
            m_keepWithNext.clear();
        }
    }
    
    pVal = UT_getAttribute("fo:text-indent", ppProps);
    if (pVal) {
        m_textIndent = pVal;
    }

    pVal = UT_getAttribute("style:writing-mode", ppProps);
    if (pVal) {
        if(!strcmp(pVal,"rl") || !strcmp(pVal,"rl-tb") || !strcmp(pVal,"tb-rl")) {
            m_direction = "rtl";
        } else {
            m_direction = "ltr";
        }
    }

    // If "fo:border" is defined, its value will fill all "fo:border-*"
    pVal = UT_getAttribute("fo:border", ppProps);
    if (pVal) {
        _stripColorLength(m_borderTop_color, m_borderTop_thickness,
                          m_haveTopBorder, pVal);
        
        m_borderBottom_color = m_borderTop_color;
        m_borderBottom_thickness = m_borderTop_thickness;
        m_haveBottomBorder = m_haveTopBorder;
        
        m_borderLeft_color = m_borderTop_color;
        m_borderLeft_thickness = m_borderTop_thickness;
        m_haveLeftBorder = m_haveTopBorder;
        
        m_borderRight_color = m_borderTop_color;
        m_borderRight_thickness = m_borderTop_thickness;
        m_haveRightBorder = m_haveTopBorder;
        
    } else {
        pVal = UT_getAttribute("fo:border-top", ppProps);
        if (pVal) {
            _stripColorLength(m_borderTop_color, m_borderTop_thickness,
                              m_haveTopBorder, pVal);
        }
        
        pVal = UT_getAttribute("fo:border-bottom", ppProps);
        if (pVal) {
            _stripColorLength(m_borderBottom_color, m_borderBottom_thickness,
                              m_haveBottomBorder, pVal);
        }
        
        pVal = UT_getAttribute("fo:border-left", ppProps);
        if (pVal) {
            _stripColorLength(m_borderLeft_color, m_borderLeft_thickness,
                              m_haveLeftBorder, pVal);
        }
        
        pVal = UT_getAttribute("fo:border-right", ppProps);
        if (pVal) {
            _stripColorLength(m_borderRight_color, m_borderRight_thickness,
                              m_haveRightBorder, pVal);
        }
    }
    pVal=  UT_getAttribute("style:join-border", ppProps);
    m_mergeBorders.clear();
    if(pVal)
    {
	m_mergeBorders = pVal;
    }
    // If "fo:padding" is defined, its value will fill all "fo:padding-*"
    pVal = UT_getAttribute("fo:padding", ppProps);
    if (pVal) 
    {
         m_paddingLeft = pVal;
	 m_paddingRight = pVal;
	 m_paddingTop = pVal;
	 m_paddingBot = pVal;
    }
    else
    {
         pVal = UT_getAttribute("fo:padding-left", ppProps);
	 if(pVal)
	 {
	     m_paddingLeft = pVal;
	 }
         pVal = UT_getAttribute("fo:padding-right", ppProps);
	 if(pVal)
	 {
	     m_paddingRight = pVal;
	 }
         pVal = UT_getAttribute("fo:padding-top", ppProps);
	 if(pVal)
	 {
	     m_paddingTop = pVal;
	 }
         pVal = UT_getAttribute("fo:padding-bot", ppProps);
	 if(pVal)
	 {
	     m_paddingBot = pVal;
	 }
    }

    // style:tab-stop-distance
    pVal = UT_getAttribute("style:tab-stop-distance", ppProps);
	if (pVal) {
        m_defaultTabInterval = pVal;
    }
}




/**
 * <style:tab-stop />
 */
void ODi_Style_Style::_parse_style_tabStopProperties(const gchar** ppProps) {
    
    const gchar* pVal = NULL;
    std::string type;
    std::string position;
    std::string leaderStyle;
    std::string leaderText;
    
    pVal = UT_getAttribute("style:type", ppProps);
    if (pVal) {
        type = pVal;
    }

    pVal = UT_getAttribute("style:position", ppProps);
    if (pVal) {
        position = pVal;
    }

    pVal = UT_getAttribute("style:leader-style", ppProps);
    if (pVal) {
        leaderStyle = pVal;
    }

    pVal = UT_getAttribute("style:leader-text", ppProps);
    if (pVal) {
        leaderText = pVal;
    }

    pVal = UT_getAttribute("style:char", ppProps);
    if (pVal) {
        // We ignore the "style:char" attribute if it exists, as abiword has no
        // equivalent: it will always use the locale-defined decimal point as the
        // decimal tab character. See fp_Line::_calculateWidthOfRun() for details.
    }

    // convert the tab information into an AbiWord property value
    
    UT_return_if_fail(!position.empty()); // a tab position is required (at least for AbiWord)

    if (!m_tabStops.empty())
        m_tabStops += ",";

    // tab position
    m_tabStops += position;
    m_tabStops += "/";

    // tab type
    if (type == "left") {
        m_tabStops += "L";
    } else if (type == "center") {
        m_tabStops += "C";
    } else if (type == "right") {
        m_tabStops += "R";
    } else if (type == "char") {
        m_tabStops += "D";
    } else {
        m_tabStops += "L";
    }

    // tab leader style: AbiWord's 4 tab styles map not to ODF's leader-styles but 
    // to leader-text's, with style 1 mapping to character ".", 2 to "-" and 3 to "_".
    // AbiWord tab style 0 means no leader style. ODF's leader-styles denoting a
    // line style are not supported.
    //
    // NOTE: in ODF, leader text (if present) *always* has a higher priority than 
    // leader styles, even if the text character is not supported.
    if (!leaderText.empty()) {
        UT_UCS4String leaderTextUCS4 = leaderText;
        UT_UCS4Char ucs4char = leaderTextUCS4[0];

        switch (ucs4char) {
            case '.':
                m_tabStops += "1";
                break;
            case '-':
                m_tabStops += "2";
                break;
            case '_':
                m_tabStops += "3";
                break;
            default:
                m_tabStops += "0";
                break;
        }
    } else if (!leaderStyle.empty()) {
        
        // AbiWord does not really support leader-styles, so do a best effort conversion.
        // Note: leader-styles describe the *underlining* line style. This means that we
        //       won't map "dash" for example to AbiWord's tab style "2" (dashed), as that
        //       does not represent underlining.

        if (leaderStyle == "none") {
		     m_tabStops += "0";
        } else if (leaderStyle == "solid") {
		     m_tabStops += "3";
        } else if (leaderStyle == "dotted") {
	    	m_tabStops += "1";
        } else if (leaderStyle == "dash" || leaderStyle == "long-dash" ||
                   leaderStyle == "dot-dash" || leaderStyle == "dot-dot-dash" ||
                   leaderStyle == "wave") {
            // 
	    	m_tabStops += "3";
        } else {
             m_tabStops += "0";
        }
    } else {
        // fall back to style "none"
        m_tabStops += "0";
    }
}





/**
 * <style:text-properties />
 */
void ODi_Style_Style::_parse_style_textProperties(const gchar** ppProps) {
    
    const gchar* pVal = NULL;
    const gchar* pVal2 = NULL;
    
    pVal = UT_getAttribute("fo:color", ppProps);
    if (pVal) {
        m_color.assign(pVal);
    }
    
    const gchar* undrStyle = UT_getAttribute("style:text-underline-style", ppProps);
    const gchar* undrType = UT_getAttribute("style:text-underline-type", ppProps);

    if ((undrStyle && (strcmp(undrStyle, "none") != 0)) ||
        (undrType && (strcmp(undrType, "none") != 0))) {

        m_textDecoration += "underline";
    }

    const gchar* ovrStyle = UT_getAttribute("style:text-overline-style", ppProps);
    const gchar* ovrType = UT_getAttribute("style:text-overline-type", ppProps);

    if ((ovrStyle && (strcmp(ovrStyle, "none") != 0)) ||
        (ovrType && (strcmp(ovrType, "none") != 0))) {

        if(!m_textDecoration.empty())
            m_textDecoration += " "; //separate the props with a space, not a comma

        m_textDecoration += "overline";
    }

    const gchar* strkStyle = UT_getAttribute("style:text-line-through-style", ppProps);
    const gchar* strkType = UT_getAttribute("style:text-line-through-type", ppProps);

    if ((strkStyle && (strcmp(strkStyle, "none") != 0)) ||
        (strkType && (strcmp(strkType, "none") != 0))) {

        if(!m_textDecoration.empty())
            m_textDecoration += " "; //separate the props with a space, not a comma

        m_textDecoration += "line-through";
    }

    pVal = UT_getAttribute("style:text-position", ppProps);
    if(pVal) {
        int position = 0;

        if (strstr(pVal, "sub") || strstr(pVal, "-"))
            m_textPos = "subscript";
        else if (strstr(pVal, "super") || ((sscanf(pVal, "%d%%", &position) == 1) && (position > 0)))
            m_textPos = "superscript";
        else 
            m_textPos = "normal";
    }
    
    pVal = UT_getAttribute("style:font-name", ppProps);
    if(!pVal) {
      pVal = UT_getAttribute("fo:font-family", ppProps);
    }

    if(pVal) {
        m_fontName.assign(pVal);
    }
    
    
    pVal = UT_getAttribute("fo:font-size", ppProps);
    if(pVal) {
        m_fontSize.assign(pVal);
    }
    
    pVal = UT_getAttribute("fo:language", ppProps);
    pVal2 = UT_getAttribute("fo:country", ppProps);
    if ( pVal && pVal2 ) {
            
        if (!strcmp(pVal, "none") && !strcmp(pVal2, "none")) {
            // AbiWord uses "-none-" instead of "none-none";
            m_lang = "-none-";
        } else {
            m_lang = UT_std_string_sprintf ("%s-%s", pVal, pVal2);
        }
    }
    
    
    pVal = UT_getAttribute("fo:font-style", ppProps);
    if (pVal) {
        if (!strcmp(pVal, "italic") || !strcmp(pVal, "normal")) {
            m_fontStyle = pVal;
        }
    }
    
    
    pVal = UT_getAttribute ("fo:font-weight", ppProps);
    if(pVal) {
        if (!strcmp(pVal, "bold")) {
            m_fontWeight = "bold";
        } else {
            m_fontWeight = "normal";
        }
    }

    // Note (for testing interoperability): OO.org doesn't correctly support hidden text:
    // http://qa.openoffice.org/issues/show_bug.cgi?id=64237 [Fixed in OO.org 3.0]

    pVal = UT_getAttribute ("text:display", ppProps);
    if(pVal) {
        if (!strcmp(pVal, "none")) {
            m_display = pVal;
        }
    }

    pVal = UT_getAttribute ("fo:background-color", ppProps);
    if(pVal) {
        m_bgcolor.assign(pVal);
    }

    pVal = UT_getAttribute ("fo:text-transform", ppProps);
    if(pVal && *pVal && 
       (!strcmp(pVal, "none") || !strcmp(pVal, "lowercase") ||
        !strcmp(pVal, "uppercase") || !strcmp(pVal, "capitalize"))) {

        m_transform = pVal;
    }
}





/**
 * 
 */
void ODi_Style_Style::_parse_style_sectionProperties(const gchar** ppProps) {
    
    const gchar* pVal;
    
    pVal = UT_getAttribute("fo:column-count", ppProps);
    if (pVal) {
        int columns = 0;
        sscanf (pVal, "%d", &columns);

        m_columns = UT_std_string_sprintf ("%d", columns);
    }
}


/**
 * <style:graphic-properties />
 */
void ODi_Style_Style::_parse_style_graphicProperties(const gchar** ppProps) {
    const gchar* pVal;
    
    pVal = UT_getAttribute("style:wrap", ppProps);
    if (pVal) {
        m_wrap = pVal;
    }

    pVal = UT_getAttribute("style:horizontal-rel", ppProps);
    if (pVal) {
        m_HorizRel = pVal;
    }


    pVal = UT_getAttribute("style:horizontal-pos", ppProps);
    if (pVal) {
        m_HorizPos = pVal;
    }


    pVal = UT_getAttribute("style:vertical-rel", ppProps);
    if (pVal) {
        m_VerticalRel = pVal;
    }


    pVal = UT_getAttribute("style:vertical-pos", ppProps);
    if (pVal) {
        m_VerticalPos = pVal;
    }

    pVal = UT_getAttribute("style:parent-style-name", ppProps);
    if (pVal && *pVal) {
        m_parentStyleName = pVal;
    }

    pVal = UT_getAttribute("fo:border-top", ppProps);
    if (pVal) {
        _stripColorLength(m_borderTop_color, m_borderTop_thickness, m_haveTopBorder, pVal);
    }

    pVal = UT_getAttribute("fo:border-bottom", ppProps);
    if (pVal) {
        _stripColorLength(m_borderBottom_color, m_borderBottom_thickness, m_haveBottomBorder, pVal);
    }

    pVal = UT_getAttribute("fo:border-left", ppProps);
    if (pVal) {
        _stripColorLength(m_borderLeft_color, m_borderLeft_thickness, m_haveLeftBorder, pVal);
    }

    pVal = UT_getAttribute("fo:border-right", ppProps);
    if (pVal) {
        _stripColorLength(m_borderRight_color, m_borderRight_thickness, m_haveRightBorder, pVal);
    }

    pVal = UT_getAttribute("fo:background-color", ppProps);
    if (pVal) {
        m_backgroundColor = pVal;
    }
}


/**
 * <style:table-properties />
 */
void ODi_Style_Style::_parse_style_tableProperties(const gchar** ppProps) {
    const gchar* pVal;
    
    pVal = UT_getAttribute("fo:background-color", ppProps);
    if (pVal) {
        m_backgroundColor = pVal;
    }

    pVal = UT_getAttribute("fo:margin-left",ppProps);
    if (pVal) {
        m_TableMarginLeft = pVal;
    }

    pVal = UT_getAttribute("fo:margin-right",ppProps);
    if (pVal) {
        m_TableMarginRight = pVal;
    }

    pVal = UT_getAttribute("style:width",ppProps);
    if (pVal) {
        m_TableWidth = pVal;
    }

    pVal = UT_getAttribute("style:rel-width",ppProps);
    if (pVal) {
        m_TableRelWidth = pVal;
    }
}


/**
 * <style:table-column-properties />
 */
void ODi_Style_Style::_parse_style_tableColumnProperties(const gchar** ppProps) {
    const gchar* pVal;
    
    pVal = UT_getAttribute("style:column-width", ppProps);
    if (pVal) {
        m_columnWidth = pVal;
    }

    pVal = UT_getAttribute("style:rel-column-width", ppProps);
    if (pVal) {
        m_columnRelWidth = pVal;
	UT_DEBUGMSG(("style %p name %s style:rel-column-width found with %s \n",this, m_name.c_str(), pVal));
    }
}


/**
 * <style:table-row-properties />
 */
void ODi_Style_Style::_parse_style_tableRowProperties(const gchar** ppProps) {
    const gchar* pVal;
    
    pVal = UT_getAttribute("style:min-row-height", ppProps);
    if (pVal) {
        m_minRowHeight = pVal;
    }
    
    pVal = UT_getAttribute("style:row-height", ppProps);
    if (pVal) {
        m_rowHeight = pVal;
    }
}


/**
 * <style:background-image />
 */
void ODi_Style_Style::_parse_style_background_image(const gchar** ppProps) 
{
    const gchar* pVal;
    
    // only implement link:href for now
    pVal = UT_getAttribute("xlink:href", ppProps);
    if (pVal) 
    {
         UT_String dataId; // id of the data item that contains the image.
	 if(!m_rAbiData.addImageDataItem(dataId, ppProps)) 
	 {
	     UT_DEBUGMSG(("ODT import: no suitable image importer found\n"));
	     return;
	 }
	 m_backgroundImageID = dataId.c_str();
    }
}


/**
 * <style:table-cell-properties />
 */
void ODi_Style_Style::_parse_style_tableCellProperties(const gchar** ppProps) {
    const gchar* pVal;
    
    // If "fo:border" is defined, its value will fill all "fo:border-*"
    pVal = UT_getAttribute("fo:border", ppProps);
    if (pVal) {
        _stripColorLength(m_borderTop_color, m_borderTop_thickness,
                          m_haveTopBorder, pVal);
        
        m_borderBottom_color = m_borderTop_color;
        m_borderBottom_thickness = m_borderTop_thickness;
        m_haveBottomBorder = m_haveTopBorder;
        
        m_borderLeft_color = m_borderTop_color;
        m_borderLeft_thickness = m_borderTop_thickness;
        m_haveLeftBorder = m_haveTopBorder;
        
        m_borderRight_color = m_borderTop_color;
        m_borderRight_thickness = m_borderTop_thickness;
        m_haveRightBorder = m_haveTopBorder;
        
    } else {
        pVal = UT_getAttribute("fo:border-top", ppProps);
        if (pVal) {
            _stripColorLength(m_borderTop_color, m_borderTop_thickness,
                              m_haveTopBorder, pVal);
        }
        
        pVal = UT_getAttribute("fo:border-bottom", ppProps);
        if (pVal) {
            _stripColorLength(m_borderBottom_color, m_borderBottom_thickness,
                              m_haveBottomBorder, pVal);
        }
        
        pVal = UT_getAttribute("fo:border-left", ppProps);
        if (pVal) {
            _stripColorLength(m_borderLeft_color, m_borderLeft_thickness,
                              m_haveLeftBorder, pVal);
        }
        
        pVal = UT_getAttribute("fo:border-right", ppProps);
        if (pVal) {
            _stripColorLength(m_borderRight_color, m_borderRight_thickness,
                              m_haveRightBorder, pVal);
        }
    }
    
    
    pVal = UT_getAttribute("fo:background-color", ppProps);
    if (pVal) {
        m_backgroundColor = pVal;
    }

    pVal = UT_getAttribute("style:vertical-align", ppProps);
    if(pVal) {
	m_VerticalAlign = pVal;
    }
}


/**
 * Defines an AbiWord style that is equivalent to this
 * OpenDocument style.
 * 
 * @param pDocument The AbiWord document on which the style will be defined.
 */
void ODi_Style_Style::defineAbiStyle(PD_Document* pDocument) {
    
    if (m_bAutomatic) {
        // Automatic styles must be invisible to the user.
        // That's (in other words) is what the OpenDocument standard says.
        // They are created for the sake of organization on the OpenDocument file.
        //
        // When they are referenced by the OpenDocument content, on AbiWord
        // their properties are just pasted into the text element.
        //
        // So, invisibility means that the user can't see this style
        // on the styles list. In fact, on the AbiWord document, it doesn't
        // even exist.
        return;
    }
    
    if (m_family == "graphic") {
        // AbiWord don't have graphic styles.
        return;
    }
    
    /*   
     *   An array of strings (array of array of chars)
     * 
     *         e.g.:
     *           a[0] = "type"
     *           a[1] = "P"
     *           a[2] = "name"
     *           a[3] = "Subtitle"
     *           a[4] = "props"
     *           a[5] = "text-indent:0in; margin-top:0pt; margin-left:0pt; ..."
     *           ...
     *           a[n] = 0 (NULL character)
     */
    bool ok;

    PP_PropertyVector pAttr;
    pAttr.push_back("type");
    if (!strcmp("paragraph", m_family.c_str())) {
        pAttr.push_back("P");
    } else if (!strcmp("text", m_family.c_str())) {
        pAttr.push_back("C");
    } else {
        // Really shouldn't happen
        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    }

    // AbiWord uses the display name
    pAttr.push_back("name");
    pAttr.push_back(m_displayName);

    if (m_pParentStyle) {
        pAttr.push_back("basedon");
        pAttr.push_back(m_pParentStyle->getDisplayName());
    }

    if (m_pNextStyle) {
        pAttr.push_back("followedby");
        pAttr.push_back(m_pNextStyle->getDisplayName());
    }


    pAttr.push_back("props");
    pAttr.push_back(m_abiPropsAttr.c_str());

    ok = pDocument->appendStyle(pAttr);
    UT_ASSERT_HARMLESS(ok);
}





/**
 * Builds the AbiWord "props" attribute value that describes this
 * Style.
 */
void ODi_Style_Style::buildAbiPropsAttrString(ODi_FontFaceDecls& rFontFaceDecls) {
    
    if (!m_fontSize.empty()) {
        UT_Dimension dim = UT_determineDimension(m_fontSize.c_str(), DIM_none);
        
        if (dim == DIM_PERCENT && !m_pParentStyle) {
            
            UT_DEBUGMSG(("*** [OpenDocument] no parent style to resolve '%s'\n",
                m_fontSize.c_str()));
                
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            
            // Ignore its value then
            m_fontSize.clear();
            
        } else if (dim == DIM_PERCENT && m_pParentStyle) {
            // calculate font-size based on parent's
            double fontSize = 12;
	    UT_LocaleTransactor lt(LC_NUMERIC, "C");
            
            if (m_pParentStyle->m_fontSize.size()) {
                fontSize = atoi(m_pParentStyle->m_fontSize.c_str()) *
                           atoi(m_fontSize.c_str()) / 100.0;
            } else {
                UT_DEBUGMSG(
                    ("*** [OpenDocument] using fallback font-size '%f'\n",
                     fontSize));
            }
            m_fontSize = UT_std_string_sprintf ("%gpt", rint(fontSize));
        }
    }

    m_abiPropsAttr.clear();

#define APPEND_STYLE(styName, styValue) if (styValue.size()) { \
                                            if(m_abiPropsAttr.size()) { \
                                                m_abiPropsAttr += ";"; \
                                            } \
                                            m_abiPropsAttr += styName; \
                                            m_abiPropsAttr += styValue; \
                                        }

    // <style:paragraph-properties>
    APPEND_STYLE("line-height: ", m_lineHeight);
    APPEND_STYLE("text-align: ", m_align);
    APPEND_STYLE("widows: ", m_widows);    
    APPEND_STYLE("orphans: ", m_orphans); 
    APPEND_STYLE("margin-left: ", m_marginLeft);
    APPEND_STYLE("margin-right: ", m_marginRight);
    APPEND_STYLE("margin-top: ", m_marginTop);
    APPEND_STYLE("margin-bottom: ", m_marginBottom);
    if(m_bgcolor.size() > 0)
    {
        std::string sPat("1");
        APPEND_STYLE("shading-pattern: ",sPat);
        APPEND_STYLE("shading-foreground-color: ",m_bgcolor);
    }
    APPEND_STYLE("bgcolor: ", m_bgcolor);
    APPEND_STYLE("keep-with-next: ", m_keepWithNext);
    APPEND_STYLE("text-indent: ", m_textIndent);
    APPEND_STYLE("dom-dir: ", m_direction);
    APPEND_STYLE("default-tab-interval: ", m_defaultTabInterval);
    APPEND_STYLE("tabstops: ", m_tabStops)

    APPEND_STYLE("bot-space: ", m_paddingBot);
    if(m_haveBottomBorder == HAVE_BORDER_YES)
    {
        std::string solid("1");
        APPEND_STYLE("bot-style: ", solid);
    }
    APPEND_STYLE("bot-thickness: ", m_borderBottom_thickness);
    APPEND_STYLE("bot-color: ", m_borderBottom_color);

    APPEND_STYLE("left-space: ", m_paddingLeft);
    if(m_haveLeftBorder == HAVE_BORDER_YES)
    {
        std::string solid("1");
        APPEND_STYLE("left-style: ", solid);
    }
    APPEND_STYLE("left-thickness: ", m_borderLeft_thickness);
    APPEND_STYLE("left-color: ", m_borderLeft_color);

    APPEND_STYLE("right-space: ", m_paddingRight);
    if(m_haveRightBorder == HAVE_BORDER_YES)
    {
        std::string solid("1");
        APPEND_STYLE("right-style: ", solid);
    }
    APPEND_STYLE("right-thickness: ", m_borderRight_thickness);
    APPEND_STYLE("right-color: ", m_borderRight_color);

    APPEND_STYLE("top-space: ", m_paddingTop);
    if(m_haveTopBorder == HAVE_BORDER_YES)
    {
        std::string solid("1");
        APPEND_STYLE("top-style: ", solid);
    }
    APPEND_STYLE("top-thickness: ", m_borderTop_thickness);
    APPEND_STYLE("top-color: ", m_borderTop_color);
    APPEND_STYLE("border-merge: ", m_mergeBorders);
    
    // <style:text-properties />
    APPEND_STYLE("color: ", m_color);
    APPEND_STYLE("text-decoration: ", m_textDecoration);
    APPEND_STYLE("text-position: ", m_textPos);
    
    if (!m_fontName.empty()) {
        const std::string & fontFamily = rFontFaceDecls.getFontFamily(m_fontName.c_str());
        
        UT_ASSERT_HARMLESS(!fontFamily.empty());
        if (!fontFamily.empty()) {
            APPEND_STYLE("font-family: ", fontFamily);
        }
    }
    
    APPEND_STYLE("font-size: ", m_fontSize);
    APPEND_STYLE("lang: ", m_lang);
    APPEND_STYLE("font-style: ", m_fontStyle);
    APPEND_STYLE("font-weight: ", m_fontWeight);

    // AbiWord hangs when a paragraph has a "display:none" property
    if (m_family.length() && !strcmp("text", m_family.c_str())) {
        APPEND_STYLE("display: ", m_display);
    }

    APPEND_STYLE("text-transform: ", m_transform);
    
    // <style:section-properties />
    APPEND_STYLE("columns: ", m_columns);
    APPEND_STYLE("column-gap: ", m_columnGap);
    
#undef APPEND_STYLE

}


/**
 * @param rProps The string that will have appended to it the properties of this
 *               style.
 */
void ODi_Style_Style::getAbiPropsAttrString(std::string& rProps, bool appendParentProps) const {

        if (appendParentProps && m_pParentStyle) {
            m_pParentStyle->getAbiPropsAttrString(rProps);
        }
    
        if (!m_abiPropsAttr.empty()) {
            
            if (!rProps.empty()) {
                rProps += "; ";
            }
            
            rProps += m_abiPropsAttr;
        }
}


/**
 * @param local If "true", It returns the plain value of the corresponding
 *              variable. Otherwise, it considers the final value of this
 *              property, taking into account its value on the parent styles.
 */
const std::string* ODi_Style_Style::getWrap(bool local) const {
    if (local) {
        return &m_wrap;
    } else {
        if (m_wrap.empty() && m_pParentStyle) {
            return m_pParentStyle->getWrap(false);
        } else {
            return &m_wrap;
        }
    }
}


/**
 * @param local If "true", It returns the plain value of the corresponding
 *              variable. Otherwise, it considers the final value of this
 *              property, taking into account its value on the parent styles.
 */
const std::string* ODi_Style_Style::getHorizPos(bool local) const {
    if (local) {
        return &m_HorizPos;
    } else {
        if (m_HorizPos.empty() && m_pParentStyle) {
            return m_pParentStyle->getHorizPos(false);
        } else {
            return &m_HorizPos;
        }
    }
}


/**
 * @param local If "true", It returns the plain value of the corresponding
 *              variable. Otherwise, it considers the final value of this
 *              property, taking into account its value on the parent styles.
 */
const std::string* ODi_Style_Style::getVerticalPos(bool local) const {
    if (local) {
        return &m_VerticalPos;
    } else {
        if (m_VerticalPos.empty() && m_pParentStyle) {
            return m_pParentStyle->getVerticalPos(false);
        } else {
            return &m_VerticalPos;
        }
    }
}


const std::string* ODi_Style_Style::getBackgroundColor() const
{
    if (m_backgroundColor.empty() && m_pParentStyle) {
        return m_pParentStyle->getBackgroundColor();
    }

    return &m_backgroundColor;
}


const std::string* ODi_Style_Style::getBackgroundImageID() const
{
    if (m_backgroundImageID.empty() && m_pParentStyle) {
        return m_pParentStyle->getBackgroundImageID();
    }

    return &m_backgroundImageID;
}


/**
 * If pString is "0.0556in solid #0000ff", rColor will receive "#0000ff",
 * rLength "0.0556in" and rHaveBorder "yes".
 * 
 * If pString is "none", both rColor and rLenght will be empty and
 * rHaveBorder will be "no"
 */
void ODi_Style_Style::_stripColorLength(std::string& rColor,
                                  std::string& rLength,
                                  ODi_Style_Style::HAVE_BORDER& rHaveBorder,
                                  const gchar* pString) const {
                                        
    UT_uint16 i, start;
    bool hasWord;

    rColor.clear();
    rLength.clear();
    
    if (!strcmp(pString, "none")) {
        // Color and length remain empty.
        rHaveBorder = HAVE_BORDER_NO;
        return;
    } else {
        rHaveBorder = HAVE_BORDER_YES;
    }
    
    i = 0;
    start = 0;
    hasWord = true;
    while (pString[i] != 0) {
        
        if (hasWord) {
            if (isspace(pString[i])) {
                if (_isValidDimensionString(&(pString[start]), i-start)) {
                    rLength.assign(&(pString[start]), i-start);
                } else if (pString[start] == '#') {
                    rColor.assign(&(pString[start]), i-start);
                }
                hasWord = false;
            }
        } else {
            if (!isspace(pString[i])) {
                start = i;
                hasWord = true;
            }
        }
        
        i++;
    };
    
    // Process the last word.
    if (hasWord) {
        if (_isValidDimensionString(&(pString[start]), i-start)) {
            rLength.assign(&(pString[start]), i-start);
        } else if (pString[start] == '#') {
            rColor.assign(&(pString[start]), i-start);
        }
    }
}


/**
 * This function shouldn't exist. The code should use
 * UT_isValidDimensionString instead. The problem with the UT function is
 * that it doesn't check the dimension specifier and only accepts NULL
 * terminated strings.
 * 
 * @param length 0 for NULL terminated strings.
 */
bool ODi_Style_Style::_isValidDimensionString(const gchar* pString,
                                             UT_uint32 length) const {
    UT_uint32 i;
    bool gotDecimalSeparator;
    
    gotDecimalSeparator = false;
    
    if (length == 0) {
        length = strlen(pString);
    }
    
    if (length < 3) {
        // We need at least two characters for the dimension specifier and one
        // for the number.
        return false;
    }
    
    for (i=0; i<length; i++) {
        if ( !isdigit(pString[i]) ) {
            if (gotDecimalSeparator) {
                // Already have a decimal separator.
                // It should be the start of the dimension specifier.
                break;
            } else {
                if (pString[i] == '.' || pString[i] == ',') {
                    gotDecimalSeparator = true;
                } else {
                    // Invalid decimal separator character.
                    return false;
                }
            }
        }
    }
    
    
    gchar dimStr[100];
    UT_Dimension dim;
    UT_uint32 j;
    
    if (length - i > 99)  {
        // A dimension specifier can't be that big.
        return false;
    }
    
    j = 0;
    while (i < length) {
        dimStr[j] = pString[i];
        
        i++;
        j++;
    }
    dimStr[j] = 0; // A null terminated string.
    
    dim = UT_determineDimension(dimStr, DIM_none);
    
    if (dim == DIM_none) {
        return false;
    } else {
        return true;
    }
}
