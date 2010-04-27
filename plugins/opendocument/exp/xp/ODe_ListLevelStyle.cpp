/* AbiSource
 * 
 * Copyright (C) 2005 INdT
 * Author: Daniel d'Andrada T. de Carvalho <daniel.carvalho@indt.org.br>
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
#include "ODe_ListLevelStyle.h"

// Internal includes
#include "ODe_Common.h"

// AbiWord includes
#include <pp_AttrProp.h>
#include <ut_units.h>
#include <ut_locale.h>


/*******************************************************************************
 * ODe_ListLevelStyle
 ******************************************************************************/


/**
 * 
 */
void ODe_ListLevelStyle::fetchAttributesFromAbiBlock(const PP_AttrProp& rAP) {
    const gchar* pValue;
    bool ok;
    
    ok = rAP.getAttribute("listid", pValue);
    UT_ASSERT(ok && pValue != NULL);
    m_AbiListId = pValue;
    
    ok = rAP.getAttribute("level", pValue);
    UT_ASSERT(ok && pValue != NULL);
    m_level = pValue;
    
    calculateListMargins(rAP, m_textIndent, m_spaceBefore, m_minLabelWidth, m_marginLeft);
}

void ODe_ListLevelStyle::calculateListMargins(const PP_AttrProp& rAP, 
    UT_UTF8String& textIndent, UT_UTF8String& spaceBefore, 
    UT_UTF8String& minLabelWidth, UT_UTF8String& marginLeft) {

    const gchar* pValue;
    bool ok;

    UT_LocaleTransactor lt(LC_NUMERIC, "C");

    // When abiword's text-indent is smaller than 0, it maps onto
    // ODF's text:min-label-width. 
    // However, when abiword's text-indent is greater than 0, it acts
    // a bit weird: the size of the tab in the listitem represents
    // the size of text:min-label-width. Since we don't have any numbers 
    // here regarding the size of that tab, we'll just hardcode it to 
    // 0.3in (0.762cm), which is abiword's default value.
    ok = rAP.getProperty("text-indent", pValue);
    double abiTextIndent = (ok && pValue != NULL ? UT_convertToDimension(pValue, DIM_CM) : 0.0);
    double dMinLabelWidth = (abiTextIndent <= 0 ? -abiTextIndent : 0.762);
    UT_UTF8String_sprintf(minLabelWidth, "%f%s", dMinLabelWidth, UT_dimensionName(DIM_CM));
    
    ok = rAP.getProperty("margin-left", pValue);
    double abiMarginLeft = (ok && pValue != NULL ? UT_convertToDimension(pValue, DIM_CM) : 0.0);

    // AbiWord's margin-left = OpenDocument paragraph property fo:margin-left +
    //                         OpenDocument text:space-before +
    //                         OpenDocument text:min-label-witdh
   
    double odfMarginLeft = abiMarginLeft - dMinLabelWidth - 0;
    UT_UTF8String_sprintf(marginLeft, "%f%s",
                          odfMarginLeft,
                          UT_dimensionName(DIM_CM));

    // OpenDocument fo:margin-left + fo:text-indent + text:space-before = AbiWord's margin-left + text-indent.
    // Since AbiWord does not support the fo:space-before feature, we will just set that to 0. We
    // then have all the variables to calculate fo:text-indent.

    spaceBefore = "0cm";
    UT_UTF8String_sprintf(textIndent, "%f%s",
                          abiMarginLeft + abiTextIndent - odfMarginLeft,
                          UT_dimensionName(DIM_CM));
}

/**
 * 
 */
void ODe_ListLevelStyle::_writeTextProperties(GsfOutput* pODT,
                     const UT_UTF8String& rSpacesOffset) const {
                        
    UT_UTF8String output;
                        
    if (!m_fontName.empty()) {
        UT_UTF8String_sprintf(output,
            "%s<style:text-properties style:font-name=\"%s\"/>\n",
            rSpacesOffset.utf8_str(), m_fontName.utf8_str());
            
        ODe_writeUTF8String(pODT, output);
    }
}


/**
 * 
 */
void ODe_ListLevelStyle::_writeListLevelProperties(GsfOutput* pODT,
                          const UT_UTF8String& rSpacesOffset) const {

    if (!m_textIndent.empty() || !m_spaceBefore.empty() || !m_minLabelWidth.empty() || !m_marginLeft.empty()) {
        UT_UTF8String output;
        
        UT_UTF8String_sprintf(output, "%s<style:list-level-properties",
            rSpacesOffset.utf8_str());

        ODe_writeAttribute(output, "fo:text-indent", m_textIndent);
        ODe_writeAttribute(output, "text:space-before", m_spaceBefore);
        ODe_writeAttribute(output, "text:min-label-width", m_minLabelWidth);
        ODe_writeAttribute(output, "fo:margin-left", m_marginLeft);
            
        output += "/>\n";
            
        ODe_writeUTF8String(pODT, output);
    }
}


/*******************************************************************************
 * ODe_Bullet_ListLevelStyle
 ******************************************************************************/


/**
 * 
 */
void ODe_Bullet_ListLevelStyle::fetchAttributesFromAbiBlock(
                                                    const PP_AttrProp& rAP) {

    // We first load the common attributes.
    ODe_ListLevelStyle::fetchAttributesFromAbiBlock(rAP);
    
    const gchar* pValue = NULL;
    bool ok = false;
    UT_UCS4Char ucs4Char = 0;

    // I'm hardcoding this font because it has all possible bullet characters and
    // it's a free font.
    m_fontName = "FreeSerif";
    
    ok = rAP.getProperty("list-style", pValue);
    
    if (!pValue || !strcmp(pValue, "Bullet List")) {
        ucs4Char = 8226; // U+2022 BULLET
    } else if (!strcmp(pValue, "Dashed List")) {
        ucs4Char = 8211; // U+2013 EN DASH
    } else if (!strcmp(pValue, "Square List")) {
        ucs4Char = 9632; // U+25A0 BLACK SQUARE
    } else if (!strcmp(pValue, "Triangle List")) {
        ucs4Char = 9650; // U+25B2 BLACK UP-POINTING TRIANGLE
    } else if (!strcmp(pValue, "Diamond List")) {
        ucs4Char = 9830; // U+2666 BLACK DIAMOND SUIT
    } else if (!strcmp(pValue, "Star List")) {
        ucs4Char = 10035; // U+2733 EIGHT SPOKED ASTERISK
    } else if (!strcmp(pValue, "Tick List")) {
        ucs4Char = 10003; // U+2713 CHECK MARK
    } else if (!strcmp(pValue, "Box List")) {
        ucs4Char = 10066; // U+2752 UPPER RIGHT SHADOWED WHITE SQUARE
    } else if (!strcmp(pValue, "Hand List")) {
        ucs4Char = 9758;// U+261E WHITE RIGHT POINTING INDEX
    } else if (!strcmp(pValue, "Heart List")) {
        ucs4Char = 9829; // U+2665 BLACK HEART SUIT
    } else if (!strcmp(pValue, "Implies List")) {
        ucs4Char = 8658; // U+21D2 RIGHTWARDS DOUBLE ARROW
    } else {
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }

    m_bulletChar.clear();
    m_bulletChar += ucs4Char;
}


/**
 * 
 */
bool ODe_Bullet_ListLevelStyle::write(GsfOutput* pODT,
                                      const UT_UTF8String& rSpacesOffset) const {
                                        
    UT_UTF8String output;
    
    UT_UTF8String_sprintf(output,
        "%s<text:list-level-style-bullet text:level=\"%s\" text:bullet-char=\"%s\">\n",
        rSpacesOffset.utf8_str(), m_level.utf8_str(), m_bulletChar.utf8_str());
    
    ODe_writeUTF8String(pODT, output);


    output = rSpacesOffset;
    output += " ";
    _writeTextProperties(pODT, output);
    _writeListLevelProperties(pODT, output);


    UT_UTF8String_sprintf(output, "%s</text:list-level-style-bullet>\n",
        rSpacesOffset.utf8_str());
            
    ODe_writeUTF8String(pODT, output);
    
    return true;
}


/*******************************************************************************
 * ODe_Numbered_ListLevelStyle
 ******************************************************************************/
 
 
/**
 * 
 */
void ODe_Numbered_ListLevelStyle::fetchAttributesFromAbiBlock(
                                                    const PP_AttrProp& rAP) {

    // We first load the common attributes.
    ODe_ListLevelStyle::fetchAttributesFromAbiBlock(rAP);
    
    const gchar* pValue = NULL;
    bool ok = false;

    ok = rAP.getProperty("list-style", pValue);
    
    if (!pValue || !strcmp(pValue, "Numbered List")) {
        m_numFormat = "1";
    } else if (!strcmp(pValue, "Lower Case List")) {
        m_numFormat = "a";
    } else if (!strcmp(pValue, "Upper Case List")) {
        m_numFormat = "A";
    } else if (!strcmp(pValue, "Lower Roman List")) {
        m_numFormat = "i";
    } else if (!strcmp(pValue, "Upper Roman List")) {
        m_numFormat = "I";
    } else if (!strcmp(pValue, "Hebrew List")) {
        // OpenDocument doesn't support this kind of list, as far as I know.
        // Collapse to an ordinary numbered list.
        m_numFormat = "1";
    } else if (!strcmp(pValue, "Arabic List")) {
        // OpenDocument doesn't support this kind of list, as far as I know.
        // Collapse to an ordinary numbered list.
        m_numFormat = "1";
    } else {
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }

    ok = rAP.getProperty("start-value", pValue);
    if (ok && pValue != NULL) {
        if (atoi(pValue) > 0) {
            m_startValue = pValue;
        } else {
            m_startValue = "1"; //start-value must be a positive integer
        }
    }
    
    // Abiword aways displays the entire level hierarchy.
    // OBS: This attribute is irrelevant for the first level.
    if (strcmp(m_level.utf8_str(), "1") != 0) {
        m_displayLevels = m_level;
    }
}


/**
 * 
 */
bool ODe_Numbered_ListLevelStyle::write(GsfOutput* pODT,
                                     const UT_UTF8String& rSpacesOffset) const {
    UT_UTF8String output;

    // Build and write the opening tag.
    
    UT_UTF8String_sprintf(output,
        "%s<text:list-level-style-number text:level=\"%s\" style:num-format=\"%s\"",
        rSpacesOffset.utf8_str(), m_level.utf8_str(), m_numFormat.utf8_str());
    
    ODe_writeAttribute(output, "text:start-value", m_startValue);
    ODe_writeAttribute(output, "text:display-levels", m_displayLevels);
    
    output += ">\n";
    
    ODe_writeUTF8String(pODT, output);


    output = rSpacesOffset;
    output += " ";
    _writeTextProperties(pODT, output);
    _writeListLevelProperties(pODT, output);


    // Write the closing tag.
    
    UT_UTF8String_sprintf(output, "%s</text:list-level-style-number>\n",
        rSpacesOffset.utf8_str());
            
    ODe_writeUTF8String(pODT, output);
    
    return true;
}
