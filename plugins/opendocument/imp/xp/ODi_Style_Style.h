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


#ifndef _ODI_STYLE_STYLE_H_
#define _ODI_STYLE_STYLE_H_

// Internal includes
#include "ODi_ListenerState.h"

// Internal classes
class ODi_FontFaceDecls;
class ODi_Abi_Data;

// AbiWord classes
class PD_Document;


/**
 * An OpenDocument regular style (<style:style>).
 */
class ODi_Style_Style : public ODi_ListenerState {

public:

    // Used to specify whether a given cell has a border (top, left, etc).
    enum HAVE_BORDER {
        HAVE_BORDER_YES,
        HAVE_BORDER_NO,
        HAVE_BORDER_UNSPECIFIED
    };


    ODi_Style_Style(ODi_ElementStack& rElementStack,
		    ODi_Abi_Data & rAbiData);

    virtual ~ODi_Style_Style() {}

    void startElement(const gchar* pName, const gchar** ppAtts,
                      ODi_ListenerStateAction& rAction);

    void endElement(const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* /*pBuffer*/, int /*length*/) {}


    const std::string& getDisplayName() const {return m_displayName;}
    void setDisplayName(std::string& rDisplayName) {
        m_displayName = rDisplayName;
    }

    /**
     * Defines an AbiWord style that is equivalent to this
     * OpenDocument style.
     *
     * Called by text and paragraph styles.
     *
     * @param pDocument The AbiWord document on which the style will be defined.
     */
    void defineAbiStyle(PD_Document* pDocument);
    ODi_Abi_Data & getAbiData(void)
      { return m_rAbiData;}

    /**
     * Builds the AbiWord "props" attribute value that describes this
     * Style.
     */
    void buildAbiPropsAttrString(ODi_FontFaceDecls& rFontFaceDecls);

    /**
     * @param rProps The string that will have appended to it the properties of this
     *               style.
     * @param appendParentProps If TRUE, it will append all parent props before appending its own props.
     *                          If FALSE, it will append only its own props.
     */
    void getAbiPropsAttrString(std::string& rProps, bool appendParentProps=TRUE) const;

    void setParentStyleName(const gchar* pParentStyleName) {
        m_parentStyleName = pParentStyleName ? pParentStyleName : "";
    }

    const ODi_Style_Style* getParent() const {
        return m_pParentStyle;
    }

    void setParentStylePointer(const ODi_Style_Style* pParentStyle) {
        m_pParentStyle = pParentStyle;
    }

    void setNextStylePointer(const ODi_Style_Style* pNextStyle) {
        m_pNextStyle = pNextStyle;
    }


    const std::string& getBreakBefore() const {return m_breakBefore;}
    const std::string& getBreakAfter() const {return m_breakAfter;}

    const std::string& getName() const {return m_name;}
    void setName(std::string& rName) {
        m_name = rName;
    }

    const std::string& getParentName() const {return m_parentStyleName;}
    void setParentName(const char* pName) {if (pName) m_parentStyleName.assign(pName);}
    void setParentName(const std::string& rName) {m_parentStyleName = rName;}

    inline const std::string& getNextStyleName() const {return m_nextStyleName;}
    inline void setNextStyleName(const char* pName) {if (pName) m_nextStyleName.assign(pName);}
    void setNextStyleName(const std::string& rName) {m_nextStyleName = rName;}

	const std::string& getListStyleName() const {return m_listStyleName;}

    bool hasProperties() const {
        return !m_listStyleName.empty() ||
               !m_masterPageName.empty() ||

               !m_lineHeight.empty() ||
               !m_align.empty() ||
               !m_breakBefore.empty() ||
               !m_breakAfter.empty() ||
               !m_widows.empty() ||
               !m_orphans.empty() ||
               !m_marginLeft.empty() ||
               !m_marginRight.empty() ||
               !m_marginTop.empty() ||
               !m_marginBottom.empty() ||
               !m_bgcolor.empty() ||
               !m_keepWithNext.empty() ||
               !m_textIndent.empty() ||
               !m_direction.empty() ||

               !m_color.empty() ||
               !m_textDecoration.empty() ||
               !m_textPos.empty() ||
               !m_fontName.empty() ||
               !m_fontSize.empty() ||
               !m_lang.empty() ||
               !m_fontStyle.empty() ||
               !m_fontWeight.empty() ||
               !m_display.empty() ||
               !m_transform.empty() ||

               !m_columns.empty() ||
               !m_columnGap.empty() ||

               !m_wrap.empty() ||
	       !m_HorizRel.empty() ||
	       !m_HorizPos.empty() ||
	       !m_VerticalPos.empty() ||
	       !m_VerticalRel.empty() ||

               !m_backgroundColor.empty() ||
               !m_backgroundImageID.empty() ||

               !m_columnWidth.empty() ||
               !m_columnRelWidth.empty() ||

               !m_minRowHeight.empty() ||
               !m_TableMarginLeft.empty() ||
               !m_TableMarginRight.empty() ||
               !m_TableWidth.empty() ||
               !m_TableRelWidth.empty() ||
               !m_rowHeight.empty() ||

	       !m_paddingLeft.empty() ||
	       !m_paddingRight.empty() ||
	       !m_paddingTop.empty()||
	       !m_paddingBot.empty()||
	       !m_mergeBorders.empty()||

	  (m_haveTopBorder == HAVE_BORDER_YES) ||
	  (m_haveBottomBorder == HAVE_BORDER_YES) ||
	  (m_haveLeftBorder == HAVE_BORDER_YES) ||
	  (m_haveRightBorder == HAVE_BORDER_YES) ||

               !m_tabStops.empty();
    }

    bool isAutomatic() const {return m_bAutomatic;}
    const std::string* getMarginLeft() const {return &m_marginLeft;}
    const std::string* getTextIndent() const {return &m_textIndent;}
    const std::string* getFamily() const {return &m_family;}
    const std::string* getFontName() const {return &m_fontName;}

    /**
     * @param local If "true", It returns the plain value of the corresponding
     *              variable. Otherwise, it considers the final value of this
     *              property, taking into account its value on the parent styles.
     */
    const std::string* getWrap(bool local) const;
    const std::string* getHorizPos(bool local) const;
    const std::string* getVerticalPos(bool local) const;

    const std::string* getBackgroundColor() const;
    const std::string* getBackgroundImageID() const;

    const std::string* getColumnWidth() const {return &m_columnWidth;}
    const std::string* getColumnRelWidth() const {return &m_columnRelWidth;}

    const std::string* getMinRowHeight() const {return &m_minRowHeight;}
    const std::string* getRowHeight() const {return &m_rowHeight;}


    const std::string* getBorderTop_thickness() const {return &m_borderTop_thickness;}
    const std::string* getBorderTop_color() const {return &m_borderTop_color;}
    HAVE_BORDER hasTopBorder() const {return m_haveTopBorder;}

    const std::string* getBorderBottom_thickness() const {return &m_borderBottom_thickness;}
    const std::string* getBorderBottom_color() const {return &m_borderBottom_color;}
    HAVE_BORDER hasBottomBorder() const {return m_haveBottomBorder;}

    const std::string* getBorderLeft_thickness() const {return &m_borderLeft_thickness;}
    const std::string* getBorderLeft_color() const {return &m_borderLeft_color;}
    HAVE_BORDER hasLeftBorder() const {return m_haveLeftBorder;}

    const std::string* getBorderRight_thickness() const {return &m_borderRight_thickness;}
    const std::string* getBorderRight_color() const {return &m_borderRight_color;}
    HAVE_BORDER hasRightBorder() const {return m_haveRightBorder;}

    const std::string* getMasterPageName() const {return &m_masterPageName;}

    const std::string* getTableMarginLeft() const {return &m_TableMarginLeft;}
    const std::string* getTableMarginRight() const {return &m_TableMarginRight;}
    const std::string* getTableWidth() const {return &m_TableWidth;}
    const std::string* getTableRelWidth() const {return &m_TableRelWidth;}

    const std::string* getVerticalAlign() const {return &m_VerticalAlign;}
private:

    // <style:style />
    void _parse_style_style(const gchar** ppAtts);

    // <style:paragraph-properties />
    void _parse_style_paragraphProperties(const gchar** ppProps);

    // <style:tab-stop />
    void _parse_style_tabStopProperties(const gchar** ppProps);

    // <style:text-properties />
    void _parse_style_textProperties(const gchar** ppProps);

    // <style:section-properties />
    void _parse_style_sectionProperties(const gchar** ppProps);

    // <style:graphic-properties />
    void _parse_style_graphicProperties(const gchar** ppProps);

    // <style:table-properties />
    void _parse_style_tableProperties(const gchar** ppProps);

    // <style:table-column-properties />
    void _parse_style_tableColumnProperties(const gchar** ppProps);

    // <style:table-row-properties />
    void _parse_style_tableRowProperties(const gchar** ppProps);

    // <style:table-cell-properties />
    void _parse_style_tableCellProperties(const gchar** ppProps);

    // <style:background-image />
    void _parse_style_background_image(const gchar** ppProps);

    /**
     * If pString is "0.0556in solid #0000ff", rColor will receive "#0000ff",
     * rLength "0.0556in" and rHaveBorder "yes".
     *
     * If pString is "none", both rColor and rLenght will be empty and
     * rHaveBorder will be "no"
     */
    void _stripColorLength(std::string& rColor, std::string& rLength,
                           HAVE_BORDER& rHaveBorder,
                           const gchar* pString) const;

    /**
     * This function shouldn't exist. The code should use
     * UT_isValidDimensionString instead. The problem with the UT function is
     * that it doesn't check the dimension specifier and only accepts NULL
     * terminated strings.
     *
     * @param length 0 for NULL terminated strings.
     */
    bool _isValidDimensionString(const gchar* pString, UT_uint32 length=0) const;

    // true if it is an OpenDocument automatic style.
    // ie., it's defined inside a <office:automatic-styles> element.
    bool m_bAutomatic;

    const ODi_Style_Style* m_pParentStyle;
    const ODi_Style_Style* m_pNextStyle;

    std::string m_abiPropsAttr;


    // <attribute name="style:name">
    std::string m_name;

    // <attribute name="style:display-name"> (optional)
    // If this attribute is not present, the display name equals the style name.
    // In AbiWord, maps to the "name" attribute.
    std::string m_displayName;

    // Maps to the "type" attribute.
    // OpenDocument | AbiWord
    // "character"  - "C"
    // "paragraph"  - "P"
    // "section"    - none (AbiWord don't have section styles)
    //
    // An exception is "graphic" styles. AbiWord don't have then.
    std::string m_family;

    // <attribute name="style:parent-style-name"> (optional)
    // If a parent style is not specified, a default parent style defined by
    // the application is used.
    //
    // In AbiWord, maps to the "basedon" attribute.
    std::string m_parentStyleName;

    // <attribute name="style:next-style-name">
    // By default, the current style is used as the next style.
    // In AbiWord, maps to the "followedby" attribute.
    std::string m_nextStyleName;

    // <attribute name="style:list-style-name"> (optional)
    //
    // Is only applied to headings and to paragraphs that are contained in a
    // list, where the list does not specify a list style itself, and the list
    // has no list style specification for any of its parents.
    //
    // Maps to AbiWord, but not directly.
    std::string m_listStyleName;

    // <attribute name="style:master-page-name"> (optional)
    //
    // If this attribute is associated with a style, a page break is inserted
    // when the style is applied and the specified master page is applied to the
    // preceding page.
    // This attribute is ignored if it is associated with a paragraph style that
    // is applied to a paragraph within a table.
    //
    // Maps to AbiWord, but not directly.
    std::string m_masterPageName;


    ////
    // <style:paragraph-properties> attributes
    // These goes inside the Abi "props" attribute
    std::string m_lineHeight;
    std::string m_align;
    std::string m_breakBefore; // fo:break-before
    std::string m_breakAfter; // fo:break-after
    std::string m_widows;
    std::string m_orphans;
    std::string m_marginLeft;
    std::string m_marginRight;
    std::string m_marginTop;
    std::string m_marginBottom;
    std::string m_bgcolor;
    std::string m_keepWithNext;
    std::string m_textIndent; // fo:text-indent
    std::string m_direction; // style:writing-mode
    std::string m_defaultTabInterval; // style:tab-stop-distance
    std::string m_tabStops; // style:tab-stops

    ////
    // <style:text-properties />
    // These goes inside the Abi "props" attribute
    std::string m_color;
    std::string m_textDecoration;
    std::string m_textPos;
    std::string m_fontName;
    std::string m_fontSize;
    std::string m_lang;
    std::string m_fontStyle;
    std::string m_fontWeight;
    std::string m_display; //text:display
    std::string m_transform; //fo:text-transform


    // fo:background-color
    // For <style:table-properties> and <style:table-cell-properties>
    std::string m_backgroundColor;

    // <style:bakground-image>
    std::string m_backgroundImageID; // xlink:href

    // For <style:table-properties
    // fo:margin-left
    // fo:margin-right
    // style:width
    // style:rel-width

    std::string   m_TableMarginLeft;
    std::string   m_TableMarginRight;
    std::string   m_TableWidth;
    std::string   m_TableRelWidth;


    ////
    // <style:section-properties>
    // These goes inside the Abi "props" attribute
    std::string m_columns;
    std::string m_columnGap;

    ////
    // <style:graphic-properties>
    std::string m_wrap; // style:wrap
    std::string m_HorizRel; // style:horizontal-rel
    std::string m_HorizPos; // style:horizontal-pos
    std::string m_VerticalPos; // style:vertical-pos
    std::string m_VerticalRel; //  style:vertical-rel

    ////
    // <style:table-column-properties>
    // style:column-width
    // rel-column-width
    std::string m_columnWidth; // style:column-width
    std::string m_columnRelWidth; // style:rel-column-width

    ////
    // <style:table-row-properties>
    std::string m_minRowHeight; // style:min-row-height
    std::string m_rowHeight; // style:row-height


    ////
    // <style:table-cell-properties>

    // style:vertical-align
    std::string m_VerticalAlign;

    // fo:border-top
    std::string m_borderTop_thickness;
    std::string m_borderTop_color;
    HAVE_BORDER m_haveTopBorder;

    // fo:border-bottom
    std::string m_borderBottom_thickness;
    std::string m_borderBottom_color;
    HAVE_BORDER m_haveBottomBorder;

    // fo:border-left
    std::string m_borderLeft_thickness;
    std::string m_borderLeft_color;
    HAVE_BORDER m_haveLeftBorder;

    // fo:border-right
    std::string m_borderRight_thickness;
    std::string m_borderRight_color;
    HAVE_BORDER m_haveRightBorder;

    // fo:padding
    std::string m_paddingLeft;
    std::string m_paddingRight;
    std::string m_paddingTop;
    std::string m_paddingBot;

    // style:merge-borders
    std::string m_mergeBorders;

    ODi_Abi_Data& m_rAbiData;

    // OBS: If "fo:border" is defined, its value will fill all "fo:border-*"
};

#endif //_ODI_STYLE_STYLE_H_
