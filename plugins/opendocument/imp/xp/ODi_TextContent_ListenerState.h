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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef _ODI_TEXTCONTENT_LISTENERSTATE_H_
#define _ODI_TEXTCONTENT_LISTENERSTATE_H_

#include <string>
#include <map>

// Internal includes
#include "ODi_ListenerState.h"

// AbiWord includes
#include <ut_types.h>
#include <ut_stack.h>

// External includes
#include <gsf/gsf.h>

// Internal classes
class ODi_Office_Styles;
class ODi_Style_List;
class ODi_TableOfContent_ListenerState;
class ODi_Abi_Data;

// AbiWord classes
class PD_Document;
class pf_Frag_Strux;

/**
 * It parses the regular content of a text document. It is used to parse the
 * document text body itself (<office:text>) and the contents of headers
 * (<style:header>) and footers (<style:footer>).
 * 
 * Regular text content may have the following main elements (that is, not
 * mentioning their child elements):
 * <text-h>
 * <text-p>
 * <text-list>
 * <table-table>
 * <text-section>
 * <text-table-of-content>
 * <text-illustration-index>
 * <text-table-index>
 * <text-object-index>
 * <text-user-index>
 * <text-alphabetical-index>
 * <text-bibliography>
 * <text-index-title>
 * <change-marks>
 */
class ODi_TextContent_ListenerState : public ODi_ListenerState {

public:

    ODi_TextContent_ListenerState (
        PD_Document* pDocument,
        ODi_Office_Styles* pStyles,
        ODi_ElementStack& rElementStack,
	ODi_Abi_Data & rAbiData);
        
    virtual ~ODi_TextContent_ListenerState();

    void startElement (const gchar* pName, const gchar** ppAtts,
                       ODi_ListenerStateAction& rAction);
                       
    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);
    
    void charData (const gchar* pBuffer, int length);
    
private:

    void _insertBookmark (const gchar * name, const gchar * type);
    void _flush ();
    void _startParagraphElement (const gchar* pName,
                                 const gchar** ppParagraphAtts,
                                 ODi_ListenerStateAction& rAction);
    void _endParagraphElement (const gchar* pName,
                               ODi_ListenerStateAction& rAction);
    bool _pushInlineFmt(const gchar** ppAtts);
    void _popInlineFmt(void);
    void _insureInBlock(const gchar ** atts);
    void _insureInSection(const UT_UTF8String* pMasterPageName = NULL);
    void _openAbiSection(const UT_UTF8String& rProps,
                         const UT_UTF8String* pMasterPageName = NULL);
    void _defineAbiTOCHeadingStyles();
    void _flushPendingParagraphBreak();
    void _insertAnnotation(void);

    PD_Document* m_pAbiDocument;
    ODi_Office_Styles* m_pStyles;

    bool m_bAcceptingText;
    bool m_bOpenedBlock;

    bool m_inAbiSection;
    bool m_openedFirstAbiSection;
    bool m_bPendingSection;
    UT_UTF8String m_currentPageMarginLeft;
    UT_UTF8String m_currentPageMarginRight;
    
    // For some reason AbiWord can't have a page break right before a new section.
    // In AbiWord, if you want to do that you have to first open the new section
    // and then, inside this new section, do the page break.
    //
    // That's the only reason for the existence of *pending* paragraph
    // (column or page) breaks.
    UT_UTF8String m_pendingParagraphBreak;
    
    enum ODi_CurrentODSection {
        // We're not inside any OpenDocument section.
        ODI_SECTION_NONE,
        
        // The current OpenDocument section has been mapped into the current
        // AbiWord section.
        ODI_SECTION_MAPPED,
        
        // The current OpenDocument section *wasn't* mapped into the current
        // AbiWord section.
        ODI_SECTION_IGNORED,
        
        // It's simply undefined. Have to find out the current situation.
        ODI_SECTION_UNDEFINED
    } m_currentODSection;

    UT_GenericVector<const gchar*> m_vecInlineFmt;
    UT_NumberStack m_stackFmtStartIndex;
    
    UT_sint8 m_elementParsingLevel;

	// Buffer that stores character data defined between start and end element
	// tags. e.g.: <bla>some char data</bla>
    UT_UCS4String m_charData;

    /**
     * In OpenDocument, <text:h> elements along the text defines the document
     * chapter's structure. So, we must get the styles used by those <text:h>
     * for each content level in order to set AbiWord's <toc> properties
     * correctly.
     */
    // It's weird, but a document may actually have several TOCs.
    UT_GenericVector<pf_Frag_Strux*> m_tablesOfContent;
    UT_GenericVector<UT_UTF8String*> m_tablesOfContentProps;
    // Maps a heading level with its style name
    // e.g.: "1" -> "Heading_20_1"
    std::map<std::string, std::string> m_headingStyles;
    ODi_TableOfContent_ListenerState* m_pCurrentTOCParser;

    // Valued as "true" if it is parsing XML content inside a
    // <office:document-content> tag.
    bool m_bOnContentStream;
    
    // List info
    ODi_Style_List* m_pCurrentListStyle;
    UT_uint8 m_listLevel;
    bool m_alreadyDefinedAbiParagraphForList;
    
    // Stuff for footnotes and endnotes.
    bool m_pendingNoteAnchorInsertion;
    UT_UTF8String m_currentNoteId;

    // Annotations
    bool m_bPendingAnnotation;
    bool m_bPendingAnnotationAuthor;
    bool m_bPendingAnnotationDate;
    UT_uint32 m_iAnnotation;
    std::string m_sAnnotationAuthor;
    std::string m_sAnnotationDate;

    // Page referenced stuff
    bool m_bPageReferencePending;
    UT_sint32 m_iPageNum;
    double    m_dXpos;
    double    m_dYpos;
    UT_UTF8String m_sProps;
    ODi_Abi_Data& m_rAbiData;
    bool m_bPendingTextbox;
    bool m_bHeadingList;
    UT_sint32 m_prevLevel;
};

#endif //_ODI_TEXTCONTENT_LISTENERSTATE_H_
