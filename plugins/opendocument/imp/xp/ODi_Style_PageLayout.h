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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef _ODI_STYLE_PAGELAYOUT_H_
#define _ODI_STYLE_PAGELAYOUT_H_

// Internal includes
#include "ODi_ListenerState.h"

// AbiWord includes
#include <ut_string_class.h>

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
    
    void startElement(const gchar* pName, const gchar** ppAtts,
                      ODi_ListenerStateAction& rAction);
                      
    void endElement(const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* /*pBuffer*/, int /*length*/) {}
    
    void definePageSizeTag(PD_Document* pDocument) const;
    
    // Returns the value to be used on every <section> tag of the AbiWord
    // document. 
    const UT_UTF8String getSectionProps(bool hasHeader, bool hasFooter) const;

    const UT_UTF8String getMarginLeft() const {
        return m_marginLeft;
    }

    const UT_UTF8String getMarginRight() const {
        return m_marginRight;
    }

    inline const UT_UTF8String& getSectionDataID() const {return m_sectionDataID;}
    
    inline const UT_UTF8String& getName() const {return m_name;}
    
private:

    void _parseHeaderFooterProperties(const gchar** ppAtts);
    void _parsePageLayoutProperties(const gchar** ppAtts);
    void _parseBackgroundImage(const gchar** ppAtts);
    UT_UTF8String _buildSectionPropsString(bool hasHeader, bool hasFooter) const;
    void _buildSectionDataIDString();

    ODi_Abi_Data& m_rAbiData;

    UT_UTF8String m_name;
    
    ////
    // <style:page-layout-properties>
    UT_UTF8String m_pageWidth;        // fo:page-width
    UT_UTF8String m_pageHeight;       // fo:page-height
    UT_UTF8String m_printOrientation; // style:print-orientation
    UT_UTF8String m_marginLeft;       // fo:margin-left
    UT_UTF8String m_marginRight;      // fo:margin-right
    UT_UTF8String m_marginTop;        // fo:margin-top
    UT_UTF8String m_marginBottom;     // fo:margin-bottom
    UT_UTF8String m_backgroundColor;  // fo:background-color
    
    ////
    // <style:columns>
    UT_UTF8String m_columnCount; // fo:column-count
    UT_UTF8String m_columnGap;   // fo:column-gap

    ////
    // <style:column-sep>
    UT_UTF8String m_columnLine; // style:style or style:width

    ////
    // <style:header-style>
    UT_UTF8String m_headerHeight;       // svg:height
    UT_UTF8String m_headerMarginBottom; // fo:margin-bottom
    
    ////
    // <style:footer-style>
    UT_UTF8String m_footerHeight;    // svg:height
    UT_UTF8String m_footerMarginTop; // fo:margin-top

    ////
    // <style:background-image>
    UT_UTF8String m_backgroundImage; // xlink:href

    // The strux-image-dataid attribute for the section
    UT_UTF8String m_sectionDataID;
};

#endif //_ODI_STYLE_PAGELAYOUT_H_
