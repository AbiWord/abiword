/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* AbiSource Program Utilities
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 
// Class definition include
#include "ODi_Style_PageLayout.h"

// Internal includes
#include "ODi_ListenerStateAction.h"
#include "ODi_ElementStack.h"
#include "ODi_Abi_Data.h"

// AbiWord includes
#include <ut_math.h>
#include <fp_PageSize.h>
#include <pd_Document.h>
#include <ut_locale.h>
#include <ut_std_string.h>

/**
 * 
 */
void ODi_Style_PageLayout::startElement(const gchar* pName,
                                       const gchar** ppAtts,
										ODi_ListenerStateAction& /*rAction*/) 
{

    const gchar* pVal;
                                        
    if (!strcmp("style:page-layout", pName)) {
        
        pVal = UT_getAttribute ("style:name", ppAtts);
        UT_ASSERT(pVal);
        m_name = pVal;
        
    } else if (!strcmp("style:page-layout-properties", pName)) {
        
        _parsePageLayoutProperties(ppAtts);
        
    } else if (!strcmp("style:columns", pName)) {
        
        pVal = UT_getAttribute ("fo:column-count", ppAtts);
        if (pVal) {
            if (!strcmp(pVal,"0")) {  // see Bug 10076
                pVal = "1";
            }
            m_columnCount = pVal;
        }
        
        pVal = UT_getAttribute ("fo:column-gap", ppAtts);
        if (pVal) {
            m_columnGap = pVal;
        }
        
    } else if (!strcmp("style:column-sep", pName)) {

        pVal = UT_getAttribute ("style:width", ppAtts);
        if (pVal) {
            if (UT_convertDimensionless(pVal) > 0) {
                m_columnLine = "on";
            }
        }

        if(m_columnLine.empty()) {
            pVal = UT_getAttribute ("style:style", ppAtts);
        } else {
            pVal = NULL; //m_columnLine is already set to "on" - don't bother looking up more props
        }

        if (pVal) {
            if (strcmp(pVal,"none")) {
                m_columnLine = "on";
            }
        }

    } else if (!strcmp("style:header-footer-properties", pName)) {
        _parseHeaderFooterProperties(ppAtts);

    } else if (!strcmp("style:background-image", pName)) {
        _parseBackgroundImage(ppAtts);
    }
}


/**
 * 
 */
void ODi_Style_PageLayout::endElement(const gchar* pName,
                                     ODi_ListenerStateAction& rAction) {
                                        
    if (!strcmp("style:page-layout", pName)) {
        _buildSectionDataIDString();
        rAction.popState();
    }
}


/**
 * Sets the <pagesize> tag.
 */
void ODi_Style_PageLayout::definePageSizeTag(PD_Document* pDocument) const {

    double pageWidthMmNumeric = 0.0f;
    double pageHeightMmNumeric = 0.0f;
    UT_LocaleTransactor lt(LC_NUMERIC, "C");

    PP_PropertyVector pageAtts = {
        "page-scale", "1.0",
        "pagetype", "",
        "units", "mm"
    };

	// width and height are rounded to full mm because that's how they are
    // predefined in Abi and there seem to be rounding errors in oow's exporter

    if (!m_pageWidth.empty()) {
        pageAtts.push_back("width");
        pageWidthMmNumeric = rint(UT_convertToDimension(m_pageWidth.c_str(),
                                                        DIM_MM));
        pageAtts.push_back(UT_std_string_sprintf("%f", pageWidthMmNumeric));
    }

    if (!m_pageHeight.empty()) {
        pageAtts.push_back("height");
        pageHeightMmNumeric = rint(UT_convertToDimension(m_pageHeight.c_str(),
                                                         DIM_MM));
        pageAtts.push_back(UT_std_string_sprintf("%f", pageHeightMmNumeric));
    }

    fp_PageSize ps(pageWidthMmNumeric, pageHeightMmNumeric, DIM_MM);
    pageAtts[3] = ps.getPredefinedName();

    if (!m_printOrientation.empty()) {
        pageAtts.push_back("orientation");
        pageAtts.push_back(m_printOrientation);
    }
    pDocument->setPageSizeFromFile(pageAtts);
}

const std::string ODi_Style_PageLayout::getSectionProps(bool hasHeader, bool hasFooter) const
{
	return _buildSectionPropsString(hasHeader, hasFooter);
}

/**
 * Parses <style:header-footer-properties> start tags.
 */
void ODi_Style_PageLayout::_parseHeaderFooterProperties(const gchar** ppAtts) {
    
    const gchar* pVal = NULL;
    pVal = UT_getAttribute ("svg:height", ppAtts);
    
    if (m_rElementStack.hasElement("style:header-style")) {
        m_headerHeight = pVal ? pVal : "";
        
        pVal = UT_getAttribute ("fo:margin-bottom", ppAtts);
        if (pVal) {
            m_headerMarginBottom = pVal ? pVal : "";
        }
    } else {
        UT_ASSERT(m_rElementStack.hasElement("style:footer-style"));
        m_footerHeight = pVal ? pVal : "";
        
        pVal = UT_getAttribute ("fo:margin-top", ppAtts);
        if (pVal) {
            m_footerMarginTop = pVal ? pVal : "";
        }
    }
}


/**
 * Parses the <style:page-layout-properties> start tag.
 */
void ODi_Style_PageLayout::_parsePageLayoutProperties(const gchar** ppAtts) {
    
    const gchar* pVal = NULL;
    
    pVal = UT_getAttribute ("fo:page-width", ppAtts);
    if (pVal) {
        m_pageWidth = pVal ? pVal : "";
    }

    pVal = UT_getAttribute ("fo:page-height", ppAtts);
    if (pVal) {
        m_pageHeight = pVal ? pVal : "";
    }

    pVal = UT_getAttribute ("style:print-orientation", ppAtts);
    if (pVal) {
        m_printOrientation = pVal ? pVal : "";
    }

    pVal = UT_getAttribute ("fo:margin-left", ppAtts);
    if (pVal) {
        m_marginLeft = pVal ? pVal : "";
    }

    pVal = UT_getAttribute ("fo:margin-top", ppAtts);
    if (pVal) {
        m_marginTop = pVal ? pVal : "";
    }

    pVal = UT_getAttribute ("fo:margin-right", ppAtts);
    if (pVal) {
        m_marginRight = pVal ? pVal : "";
    }

    pVal = UT_getAttribute ("fo:margin-bottom", ppAtts);
    if (pVal) {
        m_marginBottom = pVal ? pVal : "";
    }

    pVal = UT_getAttribute ("fo:background-color", ppAtts);
    if (pVal) {
        m_backgroundColor = pVal ? pVal : "";
    }
}


/**
 * Parses the <style:background-image> start tag.
 */
void ODi_Style_PageLayout::_parseBackgroundImage(const gchar** ppAtts) {
    
    const gchar* pVal = NULL;
    
    pVal = UT_getAttribute ("xlink:href", ppAtts);
    if(!pVal) {
        // this is a perfectly valid case.
        UT_DEBUGMSG(("ODT import: no background image found.\n"));
        return;
    }

    UT_String dataId; // id of the data item that contains the image.

    if(!m_rAbiData.addImageDataItem(dataId, ppAtts)) {
        UT_DEBUGMSG(("ODT import: no suitable image importer found\n"));
        return;
    }

    m_backgroundImage = dataId.c_str();
}


/**
 * 
 */
std::string ODi_Style_PageLayout::_buildSectionPropsString(bool hasHeader, bool hasFooter) const {
    std::string sectionProps;
    double val;
    std::string str;
    UT_LocaleTransactor lt(LC_NUMERIC, "C");

        
#define APPEND_STYLE(abiStyName, styValue) \
    if (styValue.size()) { \
        if(sectionProps.size()) { \
            sectionProps += "; "; \
        } \
        sectionProps += abiStyName; \
        sectionProps += ":"; \
        sectionProps += styValue; \
    }

    APPEND_STYLE("page-margin-left", m_marginLeft);
    APPEND_STYLE("page-margin-right", m_marginRight);
    APPEND_STYLE("page-width",m_pageWidth);
    APPEND_STYLE("page-height",m_pageHeight);
    APPEND_STYLE("page-orientation",m_printOrientation);

    if (!hasHeader) {
        // We don't have a header.
        // The property maps directly.
        APPEND_STYLE("page-margin-top", m_marginTop);
    } else {
        // We do have a header.
        // Abi's page-margin-top = OD's page fo:margin-top +
        //                         OD's header svg:height +
        //                         OD's header fo:margim-bottom

		// The problem with this calculation however is that in ODF the
		// header height is often implicit, and depends on the layout.
		// AbiWord on the other hand always needs an explicit header height. 
		// As a quick workaround we just specify the header height to be 
		// 0.5in (AbiWord's default).
		// This is far from perfect, as this might be too small or to large.
		// But until AbiWord supports implicit header heights, this is the best 
		// we can do.
		//
		// See http://bugzilla.abisource.com/show_bug.cgi?id=12371 for more details.

        val = UT_convertToDimension(m_marginTop.c_str(), DIM_CM);
        
		val += UT_convertToDimension(!m_headerHeight.empty() ? m_headerHeight.c_str() : "0.5in", DIM_CM);
        
        if (!m_headerMarginBottom.empty()) {
            val += UT_convertToDimension(m_headerMarginBottom.c_str(), DIM_CM);
        }
        
        str = UT_std_string_sprintf("%fcm", val);
        APPEND_STYLE("page-margin-top", str);
        APPEND_STYLE("page-margin-header", m_marginTop);
    }
    
    if (!hasFooter) {
        // We don't have a footer.
        // The property maps directly.
        APPEND_STYLE("page-margin-bottom", m_marginBottom);
    } else {
        // We do have a footer.
        // Abi's page-margin-bottom = OD's page fo:margin-bottom +
        //                            OD's footer svg:height +
        //                            OD's footer fo:margim-top

		// The problem with this calculation however is that in ODF the
		// footer height is often implicit, and depends on the layout.
		// AbiWord on the other hand always needs an explicit footer height. 
		// As a quick workaround we just specify the footer height to be 
		// 0.5in (AbiWord's default).
		// This is far from perfect, as this might be too small or to large.
		// But until AbiWord supports implicit footer heights, this is the best 
		// we can do.
		//
		// See http://bugzilla.abisource.com/show_bug.cgi?id=12371 for more details.

        val = UT_convertToDimension(m_marginBottom.c_str(), DIM_CM);
        
		val += UT_convertToDimension(!m_footerHeight.empty() ? m_footerHeight.c_str() : "0.5in", DIM_CM);
        
        if (!m_footerMarginTop.empty()) {
            val += UT_convertToDimension(m_footerMarginTop.c_str(), DIM_CM);
        }
        
        str = UT_std_string_sprintf( "%fcm", val);
        APPEND_STYLE("page-margin-bottom", str);
        APPEND_STYLE("page-margin-footer", m_marginBottom);
    }
    
    APPEND_STYLE("columns", m_columnCount);
    APPEND_STYLE("column-gap", m_columnGap);
    APPEND_STYLE("column-line", m_columnLine);
    APPEND_STYLE("background-color", m_backgroundColor);
#undef APPEND_STYLE

    return sectionProps;
}


/**
 * 
 */
void ODi_Style_PageLayout::_buildSectionDataIDString() {
    m_sectionDataID.clear();

    if(m_backgroundImage.length()) {
        m_sectionDataID = m_backgroundImage;
    }
}
