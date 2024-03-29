/* AbiSource Program Utilities
 *
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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

#ifndef _ODI_STYLE_PAGELAYOUT_H_
#define _ODI_STYLE_PAGELAYOUT_H_

// Internal includes
#include "ODi_ListenerState.h"

// AbiWord includes
#include "ut_string_class.h"

// Internal classes
class ODi_Abi_Data;

// AbiWord classes
class PD_Document;


/**
 * An OpenDocument page layout style (<style:page-layout>).
 */
class ODi_Style_PageLayout : public ODi_ListenerState {
public:
    ODi_Style_PageLayout(ODi_ElementStack& rElementStack, ODi_Abi_Data& rAbiData) :
        ODi_ListenerState("StylePageLayout", rElementStack),
        m_rAbiData(rAbiData) {}

    virtual ~ODi_Style_PageLayout() {}

    virtual void startElement(const gchar* pName, const gchar** ppAtts,
                      ODi_ListenerStateAction& rAction) override;

    virtual void endElement(const gchar* pName, ODi_ListenerStateAction& rAction) override;

    virtual void charData (const gchar* /*pBuffer*/, int /*length*/) override {}

    void definePageSizeTag(PD_Document* pDocument) const;

    // Returns the value to be used on every <section> tag of the AbiWord
    // document.
    const std::string getSectionProps(bool hasHeader, bool hasFooter) const;

    const std::string getMarginLeft() const {
        return m_marginLeft;
    }

    const std::string getMarginRight() const {
        return m_marginRight;
    }

    inline const std::string& getSectionDataID() const {return m_sectionDataID;}

    inline const std::string& getName() const {return m_name;}

private:

    void _parseHeaderFooterProperties(const gchar** ppAtts);
    void _parsePageLayoutProperties(const gchar** ppAtts);
    void _parseBackgroundImage(const gchar** ppAtts);
    std::string _buildSectionPropsString(bool hasHeader, bool hasFooter) const;
    void _buildSectionDataIDString();

    ODi_Abi_Data& m_rAbiData;

    std::string m_name;

    ////
    // <style:page-layout-properties>
    std::string m_pageWidth;        // fo:page-width
    std::string m_pageHeight;       // fo:page-height
    std::string m_printOrientation; // style:print-orientation
    std::string m_marginLeft;       // fo:margin-left
    std::string m_marginRight;      // fo:margin-right
    std::string m_marginTop;        // fo:margin-top
    std::string m_marginBottom;     // fo:margin-bottom
    std::string m_backgroundColor;  // fo:background-color

    ////
    // <style:columns>
    std::string m_columnCount; // fo:column-count
    std::string m_columnGap;   // fo:column-gap

    ////
    // <style:column-sep>
    std::string m_columnLine; // style:style or style:width

    ////
    // <style:header-style>
    std::string m_headerHeight;       // svg:height
    std::string m_headerMarginBottom; // fo:margin-bottom

    ////
    // <style:footer-style>
    std::string m_footerHeight;    // svg:height
    std::string m_footerMarginTop; // fo:margin-top

    ////
    // <style:background-image>
    std::string m_backgroundImage; // xlink:href

    // The strux-image-dataid attribute for the section
    std::string m_sectionDataID;
};

#endif //_ODI_STYLE_PAGELAYOUT_H_
