/* AbiSource Program Utilities
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


#ifndef _ODI_STYLE_MASTERPAGE_H_
#define _ODI_STYLE_MASTERPAGE_H_

// Internal includes
#include "ODi_ListenerState.h"
#include "ODi_Style_PageLayout.h"

// AbiWord includes
#include <ut_string_class.h>

// AbiWord classes
class PD_Document;

/**
 * Class representing a <style:master-page>
 */
class ODi_Style_MasterPage : public ODi_ListenerState {

public:

    ODi_Style_MasterPage(PD_Document* pDocument, ODi_ElementStack& rElementStack);
    virtual ~ODi_Style_MasterPage() {}

    void startElement(const gchar* pName, const gchar** ppAtts,
                      ODi_ListenerStateAction& rAction);

    void endElement(const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* /*pBuffer*/, int /*length*/) {}


    inline const std::string& getLayoutName() const {return m_layoutName;}

    inline void setLayoutStylePointer(ODi_Style_PageLayout* pPageLayoutStyle) {
        m_pPageLayoutStyle = pPageLayoutStyle;
    }

    inline void definePageSizeTag(PD_Document* pDocument) const {
        m_pPageLayoutStyle->definePageSizeTag(pDocument);
    }

    inline const std::string getSectionProps() const {
        return m_pPageLayoutStyle->getSectionProps(
			!m_AW_headerSectionID.empty() || !m_AW_evenHeaderSectionID.empty(),
			!m_AW_footerSectionID.empty() || !m_AW_evenFooterSectionID.empty());
    }

    inline const std::string& getSectionDataID() const {
        return m_pPageLayoutStyle->getSectionDataID();
    }

    inline const std::string& getName() const {return m_name;}

    inline const std::string& getPageLayoutName() const {return m_layoutName;}

    inline const ODi_Style_PageLayout* getPageLayout() const {
        return m_pPageLayoutStyle;
    }


    const std::string& getAWHeaderSectionID() const {return m_AW_headerSectionID;}
    const std::string& getAWFooterSectionID() const {return m_AW_footerSectionID;}
    const std::string& getAWEvenHeaderSectionID() const {return m_AW_evenHeaderSectionID;}
    const std::string& getAWEvenFooterSectionID() const {return m_AW_evenFooterSectionID;}

private:
    PD_Document* m_pAbiDocument;

    ODi_Style_PageLayout* m_pPageLayoutStyle;

    std::string m_name;
    std::string m_layoutName;

    // The AbiWord header section id
    std::string m_AW_headerSectionID;

    // The AbiWord even header section id
    std::string m_AW_evenHeaderSectionID;

    // The AbiWord footer section id
    std::string m_AW_footerSectionID;

    // The AbiWord even footer section id
    std::string m_AW_evenFooterSectionID;

    // The parsing uses a two-pass approach:
    //
    // - On the first pass, master page attributes are gathered.
    // - On the second pass, headers and footers contents are parsed, being
    //   translated into abi sections.
    //
    // The second pass is postponed because headers and footers sections, on
    // AbiWord, must appear after the text sections that uses then. So,
    // they must be parsed after <office:text>, which contains the entire
    // document content.
    enum ODI_ParsingState {
        ODI_FIRST_PASS,
        ODI_SECOND_PASS,
        ODI_POSTPONING,
        ODI_POSTPONED_SECOND_PASS
        } m_parsingState;
};

#endif //_ODI_STYLE_MASTERPAGE_H_
