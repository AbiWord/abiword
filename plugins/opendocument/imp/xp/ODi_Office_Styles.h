/* AbiSource
 *
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef _ODI_OFFICE_STYLES_H_
#define _ODI_OFFICE_STYLES_H_

#include <string>
#include <map>

// Internal includes
#include "ODi_Style_Style_Family.h"

// Internal classes
class ODi_FontFaceDecls;
class ODi_Style_Style;
class ODi_Style_PageLayout;
class ODi_Style_MasterPage;
class ODi_Style_List;
class ODi_NotesConfiguration;
class ODi_ElementStack;
class ODi_Abi_Data;

// AbiWord classes
class PD_Document;



/**
 * This class holds all the styles defined by the OpenDocument stream, both the
 * common (defined inside <office:styles>), automatic (definded
 * inside <office:automatic-styles>) and master page ones (defined inside
 * <office:master-styles>).
 *
 * It includes the page layouts (<style:page-layout>).
 *
 * Usage:
 * 1 - Add (and parse) all styles
 * 2 - Call addedAllStyles
 */
class ODi_Office_Styles {
public:

    ODi_Office_Styles() {}
    ~ODi_Office_Styles();

    ODi_Style_Style* addStyle(const gchar** ppAtts,
			      ODi_ElementStack& rElementStack,
			      ODi_Abi_Data& rAbiData);

    ODi_Style_PageLayout* addPageLayout(const gchar** ppAtts,
                                       ODi_ElementStack& rElementStack,
                                       ODi_Abi_Data& rAbiData);

    ODi_Style_MasterPage* addMasterPage(const gchar** ppAtts,
                                       PD_Document* pDocument,
                                       ODi_ElementStack& rElementStack);

    ODi_Style_Style* addDefaultStyle(const gchar** ppAtts,
				     ODi_ElementStack& rElementStack,
				     ODi_Abi_Data &rAbiData );

    ODi_Style_List* addList(const gchar** ppAtts,
    					   ODi_ElementStack& rElementStack);

    ODi_NotesConfiguration* addNotesConfiguration(const gchar** ppAtts,
                                               ODi_ElementStack& rElementStack);

    /**
     * Will do some post-processing and then define all AbiWord styles.
     */
    void addedAllStyles(PD_Document* pDocument,
                        ODi_FontFaceDecls& rFontFaceDecls) {
        _fixStyles();
        _linkStyles();
        _buildAbiPropsAttrString(rFontFaceDecls);
        _defineAbiStyles(pDocument);
    }

    const ODi_Style_Style* getTextStyle(const gchar* pStyleName,
                                       bool bOnContentStream) const;

    const ODi_Style_Style* getParagraphStyle(const gchar* pStyleName,
                                            bool bOnContentStream) const;

    const ODi_Style_Style* getSectionStyle(const gchar* pStyleName,
                                          bool bOnContentStream) const;

    const ODi_Style_Style* getGraphicStyle(const gchar* pStyleName,
                                          bool bOnContentStream) const;

    const ODi_Style_Style* getTableStyle(const gchar* pStyleName,
                                        bool bOnContentStream) const;

    const ODi_Style_Style* getTableColumnStyle(const gchar* pStyleName,
                                              bool bOnContentStream) const;

    const ODi_Style_Style* getTableRowStyle(const gchar* pStyleName,
                                           bool bOnContentStream) const;

    const ODi_Style_Style* getTableCellStyle(const gchar* pStyleName,
                                            bool bOnContentStream) const;

    const ODi_Style_Style* getDefaultParagraphStyle() const
        {
            return m_paragraphStyleStyles.getDefaultStyle();
        }

    const ODi_Style_PageLayout* getPageLayoutStyle(
                                             const gchar* pStyleName) const
        {
            std::map<std::string, ODi_Style_PageLayout*>::const_iterator
                iter = m_pageLayoutStyles.find(pStyleName);
            if(iter != m_pageLayoutStyles.end()) {
                return iter->second;
            }
            return NULL;
        }

    const ODi_Style_MasterPage* getMasterPageStyle(
                                             const gchar* pStyleName) const
        {
            std::map<std::string, ODi_Style_MasterPage*>::const_iterator
                iter = m_masterPageStyles.find(pStyleName);
            if(iter != m_masterPageStyles.end()) {
                return iter->second;
            }
            return NULL;
        }

    ODi_Style_List* getList(const gchar* pStyleName) const
        {
            std::map<std::string, ODi_Style_List*>::const_iterator
                iter = m_listStyles.find(pStyleName);
            if(iter != m_listStyles.end()) {
                return iter->second;
            }
            return NULL;
        }

    const ODi_NotesConfiguration* getNotesConfiguration(
                                               const gchar* pNoteClass) const
        {
            std::map<std::string, ODi_NotesConfiguration*>::const_iterator
                iter = m_notesConfigurations.find(pNoteClass);
            if(iter != m_notesConfigurations.end()) {
                return iter->second;
            }
            return NULL;
        }

private:

    void _fixStyles();
    void _linkStyles();
    void _buildAbiPropsAttrString(ODi_FontFaceDecls& rFontFaceDecls);
    void _defineAbiStyles(PD_Document* pDocument) const;

    void _linkMasterStyles();
    void _linkListStyles() const;


    ////
    // Styles (<style:style>) are separated by family.

    // <style:style style:family="text">
    ODi_Style_Style_Family m_textStyleStyles;

    // <style:style style:family="paragraph">
    ODi_Style_Style_Family m_paragraphStyleStyles;

    // <style:style style:family="section">
    ODi_Style_Style_Family m_sectionStyleStyles;

    // <style:style style:family="graphic">
    ODi_Style_Style_Family m_graphicStyleStyles;

    // <style:style style:family="table">
    ODi_Style_Style_Family m_tableStyleStyles;

    // <style:style style:family="table-column">
    ODi_Style_Style_Family m_tableColumnStyleStyles;

    // <style:style style:family="table-row">
    ODi_Style_Style_Family m_tableRowStyleStyles;

    // <style:style style:family="table-cell">
    ODi_Style_Style_Family m_tableCellStyleStyles;



    // <text:list-style>
    typedef std::map<std::string, ODi_Style_List*> ListMap;
    ListMap m_listStyles;

    // <style:page-layout>
    typedef std::map<std::string, ODi_Style_PageLayout*> PageLayoutMap;
    PageLayoutMap m_pageLayoutStyles;

    // <style:master-page>
    typedef std::map<std::string, ODi_Style_MasterPage*> MasterPageMap;
    MasterPageMap m_masterPageStyles;

    // <text:notes-configuration>
    typedef std::map<std::string, ODi_NotesConfiguration*> NotesConfigMap;
    NotesConfigMap m_notesConfigurations;
};

#endif //_ODI_OFFICE_STYLES_H_
