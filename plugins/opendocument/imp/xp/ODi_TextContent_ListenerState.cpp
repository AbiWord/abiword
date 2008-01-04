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

// Class definition include
#include "ODi_TextContent_ListenerState.h"

// Internal includes
#include "ODi_Office_Styles.h"
#include "ODi_Style_List.h"
#include "ODi_Style_Style.h"
#include "ODi_Style_MasterPage.h"
#include "ODi_ListenerStateAction.h"
#include "ODi_ListLevelStyle.h"
#include "ODi_NotesConfiguration.h"
#include "ODi_StartTag.h"
#include "ODi_ElementStack.h"
#include "ODi_TableOfContent_ListenerState.h"

// AbiWord includes
#include <ut_misc.h>
#include <pd_Document.h>
#include <pf_Frag_Strux.h>


/**
 * Constructor
 */
ODi_TextContent_ListenerState::ODi_TextContent_ListenerState (
                PD_Document* pDocument,
                ODi_Office_Styles* pStyles,
                ODi_ElementStack& rElementStack)
                : ODi_ListenerState("TextContent", rElementStack),
                  m_pAbiDocument ( pDocument ),
                  m_pStyles(pStyles),
                  m_bAcceptingText(false),
                  m_bOpenedBlock(false),
                  m_inAbiSection(false),
                  m_openedFirstAbiSection(false),
                  m_currentODSection(ODI_SECTION_NONE),
                  m_elementParsingLevel(0),
                  m_pCurrentTOCParser(NULL),
                  m_bOnContentStream(false),
                  m_pCurrentListStyle(NULL),
                  m_listLevel(0),
                  m_alreadyDefinedAbiParagraphForList(false),
                  m_pendingNoteAnchorInsertion(false)
{
    UT_ASSERT_HARMLESS(m_pAbiDocument);
    UT_ASSERT_HARMLESS(m_pStyles);
}


/**
 * Destructor
 */
ODi_TextContent_ListenerState::~ODi_TextContent_ListenerState() {

    UT_ASSERT_HARMLESS(m_tablesOfContentProps.getItemCount()==0);
   
    if (m_tablesOfContentProps.getItemCount() > 0) {
        UT_VECTOR_PURGEALL(UT_UTF8String*, m_tablesOfContentProps);
        m_tablesOfContentProps.clear();
    }
}


/**
 * Called when the XML parser finds a start element tag.
 * 
 * @param pName The name of the element.
 * @param ppAtts The attributes of the parsed start tag.
 */
void ODi_TextContent_ListenerState::startElement (const gchar* pName,
                                          const gchar** ppAtts,
                                          ODi_ListenerStateAction& rAction)
{
    if (strcmp(pName, "text:section" ) != 0 ) {
        _flushPendingParagraphBreak();
    }
    
    if (!strcmp(pName, "text:section" )) {

        // OBS: AbiWord also can't have nested sections (but OpenDocument can).

        const gchar* pStyleName = UT_getAttribute ("text:style-name", ppAtts);
        UT_ASSERT(pStyleName != NULL);
        
        const ODi_Style_Style* pStyle = m_pStyles->getSectionStyle(pStyleName,
                                                        m_bOnContentStream);
                                                        
        UT_UTF8String props = "";

        if (pStyle) {        
            pStyle->getAbiPropsAttrString(props);
        }
        
        
        // If it don't have any properties it's useless.
        //
        // OpenDocument sections can be used just to structure the document (like
        // naming sections and subsections). AbiWord will consider only sections
        // that contains meaningful formating properties, like number of text
        // columns, etc.
        if (props.empty()) {
            m_currentODSection = ODI_SECTION_IGNORED;
        } else {
            m_currentODSection = ODI_SECTION_MAPPED;
            _openAbiSection(props);
        }

    } else if (!strcmp(pName, "text:p" )) {
        
        // It's so big that it deserves its own function.
        _startParagraphElement(pName, ppAtts, rAction);
        
    } else if (!strcmp(pName, "text:h" )) {

        const gchar* pStyleName;
        const gchar* pOutlineLevel;
        const ODi_Style_Style* pStyle;
        
        pStyleName = UT_getAttribute("text:style-name", ppAtts);
        UT_ASSERT(pStyleName);
        
        pOutlineLevel = UT_getAttribute("text:outline-level", ppAtts);
        if (pOutlineLevel == NULL) {
            // Headings without a level attribute are assumed to
            // be at level 1.
            pOutlineLevel = "1";
        }
        
        pStyle = m_pStyles->getParagraphStyle(pStyleName, m_bOnContentStream);
        UT_ASSERT_HARMLESS(pStyle);
        
        if (pStyle && (pStyle->isAutomatic())) {

            if (pStyle->getParent() != NULL) {

                m_headingStyles[pOutlineLevel] = 
					pStyle->getParent()->getDisplayName().utf8_str();
            } else {
                UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
                // This is not expected from a well formed file.
                // So, it's corrputed. But we can ignore this error without
                // compromising the doc load as a whole
                // (the TOC will not be displayed correctly, though).
            }
            
        } else if (pStyle) {
            m_headingStyles[pOutlineLevel] =
				pStyle->getDisplayName().utf8_str();
        }
        
        // It's so big that it deserves its own function.
        _startParagraphElement(pName, ppAtts, rAction);
        
    } else if (!strcmp(pName, "text:s")) {
        // A number of consecutive white-space characters.
        
        const gchar* pSpaceCount;
        UT_uint32 spaceCount, i;
        UT_UCS4String string;
        
        pSpaceCount = UT_getAttribute("text:c", ppAtts);
        
        if (pSpaceCount) {
            i = sscanf(pSpaceCount, "%d", &spaceCount);
            UT_ASSERT(i==1);
        } else {
            // From the OpenDocument specification:
            // "A missing text:c attribute is interpreted as meaning a
            // single SPACE character."
            spaceCount = 1;
        }
        
        
        // TODO: A faster (wiser) implementation can be done, I think. (Daniel d'Andrada)
        string.clear();
        for (i=0; i<spaceCount; i++) {
            string += " ";
        }
        
        // Write the text that has not been written yet.
        // Otherwise the spaces will appear in the wrong position.
        _flush();
        
        m_pAbiDocument->appendSpan(string.ucs4_str(), string.size());
        
    } else if (!strcmp(pName, "text:tab")) {
        // A tab character.

        UT_UCS4String string = "\t";
       
        // Write the text that has not been written yet.
        // Otherwise the spaces will appear in the wrong position.
        _flush();
        
        m_pAbiDocument->appendSpan(string.ucs4_str(), string.size());
        
    } else if (!strcmp(pName, "text:table-of-content")) {
        
        _flush ();
        _insureInBlock(NULL);
        
        UT_ASSERT(m_pCurrentTOCParser == NULL);
        
        m_pCurrentTOCParser = new ODi_TableOfContent_ListenerState(
            m_pAbiDocument, m_pStyles, m_rElementStack);
            
        rAction.pushState(m_pCurrentTOCParser, false);

    } else if (!strcmp(pName, "text:span")) {
        // Write all text that is between the last element tag and this
        // <text:span>
        _flush ();

        const gchar* pStyleName = UT_getAttribute("text:style-name", ppAtts);
        const ODi_Style_Style* pStyle;
        
        if (!pStyleName) {
            // I haven't seen default styles for "text" family.
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            
        } else {
            pStyle = m_pStyles->getTextStyle(pStyleName, m_bOnContentStream);
            
            if (pStyle) {
            
                const gchar* ppStyAttr[3];
                bool ok;
                UT_UTF8String props;
                
                if (pStyle->isAutomatic()) {
                    pStyle->getAbiPropsAttrString(props);
                    
                    // It goes "hardcoded"
                    ppStyAttr[0] = "props";
                    ppStyAttr[1] = props.utf8_str();
                    ppStyAttr[2] = 0;
                } else {
                    ppStyAttr[0] = "style";
                    ppStyAttr[1] = pStyle->getDisplayName().utf8_str();
                    ppStyAttr[2] = 0;                
                }
                
                _pushInlineFmt(ppStyAttr);
                ok = m_pAbiDocument->appendFmt(&m_vecInlineFmt);
                UT_ASSERT(ok);
                
            } else {
                // We just ignore this <text:span>.
            }
        }
        
    } else if (!strcmp(pName, "text:line-break")) {
        
        m_charData += UCS_LF;
        _flush ();
        
    } else if (!strcmp(pName, "text:a")) {
        
        _flush();
        const gchar * xlink_atts[3];
        xlink_atts[0] = "xlink:href";
        xlink_atts[1] = UT_getAttribute("xlink:href", ppAtts);
        xlink_atts[2] = 0;
        m_pAbiDocument->appendObject(PTO_Hyperlink, xlink_atts);
        
    } else if (!strcmp(pName, "text:bookmark")) {
        
        _flush ();
        const gchar * pName = UT_getAttribute ("text:name", ppAtts);

        if(pName) {
            _insertBookmark (pName, "start");
            _insertBookmark (pName, "end");
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        }
        
    } else if (!strcmp(pName, "text:bookmark-start")) {

        _flush ();
        const gchar * pName = UT_getAttribute ("text:name", ppAtts);

        if(pName) {
            _insertBookmark (pName, "start");
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        }

    } else if (!strcmp(pName, "text:bookmark-end")) {

        _flush ();
        const gchar * pName = UT_getAttribute ("text:name", ppAtts);

        if(pName) {
            _insertBookmark (pName, "end");
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        }

    } else if (!strcmp(pName, "text:date") ||
            !strcmp(pName, "text:time") ||
            !strcmp(pName, "text:page-number") ||
            !strcmp(pName, "text:page-count") ||
            !strcmp(pName, "text:file-name") ||
            !strcmp(pName, "text:paragraph-count") ||
            !strcmp(pName, "text:word-count") ||
            !strcmp(pName, "text:character-count") ||
            !strcmp(pName, "text:initial-creator") ||
            !strcmp(pName, "text:author-name") ||
            !strcmp(pName, "text:description") ||
            !strcmp(pName, "text:keywords") ||
            !strcmp(pName, "text:subject") ||
            !strcmp(pName, "text:title")) {
                
        _flush ();

        const gchar * type = "";
        if(!strcmp(pName, "text:date"))
            type = "date_ddmmyy";
        else if(!strcmp(pName, "text:time"))
            type = "time";
        else if(!strcmp(pName, "text:page-number"))
            type = "page_number";
        else if(!strcmp(pName, "text:page-count"))
            type = "page_count";
        else if(!strcmp(pName, "text:file-name"))
            type = "file_name";
        else if(!strcmp(pName, "text:paragraph-count"))
            type = "para_count";
        else if(!strcmp(pName, "text:word-count"))
            type = "word_count";
        else if(!strcmp(pName, "text:character-count"))
            type = "char_count";
        else if(!strcmp(pName, "text:initial-creator") || !strcmp(pName, "text:author-name"))
            type = "meta_creator";
        else if(!strcmp(pName, "text:description"))
            type = "meta_description";
        else if(!strcmp(pName, "text:keywords"))
            type = "meta_keywords";
        else if(!strcmp(pName, "text:subject"))
            type = "meta_subject";
        else if(!strcmp(pName, "text:title"))
            type = "meta_title";

        const gchar *field_fmt[3];
        field_fmt[0] = "type";
        field_fmt[1] = type;
        field_fmt[2] = 0;
        m_pAbiDocument->appendObject(PTO_Field, (const gchar**)field_fmt);
        m_bAcceptingText = false;
        
    } else if (!strcmp(pName, "style:header") ||
               !strcmp(pName, "style:footer") ||
               !strcmp(pName, "style:header-left") ||
               !strcmp(pName, "style:footer-left")) {
                
        UT_ASSERT(m_elementParsingLevel == 0);
                
        // We are inside a header/footer so, there is already a section defined
        // on the AbiWord document.
        m_inAbiSection = true;
        m_bOnContentStream = false;
        
    } else if (!strcmp(pName, "office:text")) {
        UT_ASSERT(m_elementParsingLevel == 0);
        m_bOnContentStream = true;
        
    } else if (!strcmp(pName, "text:list")) {
        
        if (m_pCurrentListStyle != NULL) {
            m_listLevel++;
        } else {
            const gchar* pVal;
            
            pVal = UT_getAttribute("text:style-name", ppAtts);
            
            
            
            m_pCurrentListStyle = m_pStyles->getList(pVal);
            UT_ASSERT(m_pCurrentListStyle != NULL);
            
            m_listLevel = 1;
        }
        
    } else if (!strcmp(pName, "text:list-item")) {
        m_alreadyDefinedAbiParagraphForList = false;
        
    } else if (!strcmp(pName, "draw:frame")) {
        
        if (!strcmp(m_rElementStack.getStartTag(0)->getName(), "text:p") ||
            !strcmp(m_rElementStack.getStartTag(0)->getName(), "text:h")) {
            
            const gchar* pVal = NULL;
            
            pVal = UT_getAttribute("text:anchor-type", ppAtts);
            UT_ASSERT(pVal);
            
            if (pVal && (!strcmp(pVal, "paragraph") ||
                !strcmp(pVal, "page"))) {
                // It's postponed because AbiWord uses frames *after* the
                // paragraph but OpenDocument uses then inside the paragraphs,
                // right *before* its content.
                rAction.postponeElementParsing("Frame");
            } else {
                // It's an inlined frame.
                _flush();
                rAction.pushState("Frame");
            }
            
        } else if (!strcmp(m_rElementStack.getStartTag(0)->getName(), "office:text")) {
            
            // A page anchored frame.
            //
            // Page anchored frames defined outside a paragraph specifies the
            // number of the page that they are anchored to. As I can't translate
            // this info easily (if not at all) into AbiWord I will ignore all
            // OpenDocument page anchored frames defined that way.
            
            // Ignore this element
            rAction.ignoreElement();
           
        } else if (!strcmp(m_rElementStack.getStartTag(0)->getName(),
                              "text:span")) {
            // Must be an inlined image, otherwise we can't handle it.
            
            const gchar* pVal;
            
            pVal = UT_getAttribute("text:anchor-type", ppAtts);
            UT_ASSERT(pVal);
            
            if (pVal && (!strcmp(pVal, "as-char") || !strcmp(pVal, "char"))) {
                _flush();
                rAction.pushState("Frame");
            } else {
                // Ignore this element
                rAction.ignoreElement();
            }

        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            // TODO: Figure out what to do then.
        }
        
    } else if (!strcmp(pName, "draw:text-box")) {
        UT_ASSERT(m_elementParsingLevel == 0);
        
        // We're inside a text-box, parsing its text contents.
        m_inAbiSection = true;
        m_bOnContentStream = true;
        
    } else if (!strcmp(pName, "draw:g")) {
       rAction.ignoreElement();  // ignore drawing shapes since AbiWord can't handle them

    } else if (!strcmp(pName, "table:table")) {
        _insureInSection();
        rAction.pushState("Table");
        
    } else if (!strcmp(pName, "table:table-cell")) {
        UT_ASSERT(m_elementParsingLevel == 0);
        
        // We're inside a table cell, parsing its text contents.
        m_inAbiSection = true;
        m_bOnContentStream = true;
        m_openedFirstAbiSection = true;
        
    }  else if (!strcmp(pName, "text:note")) {

        _flush();
        m_bAcceptingText = false;
        
    } else if (!strcmp(pName, "text:note-body")) {

        const gchar* ppAtts[10];
        bool ok;
        UT_uint32 id;
        const ODi_NotesConfiguration* pNotesConfig;
        const ODi_Style_Style* pStyle = NULL;
        const UT_UTF8String* pCitationStyleName = NULL;
        UT_uint8 i;
        bool isFootnote;
        const gchar* pNoteClass;
        
        pNoteClass = m_rElementStack.getStartTag(0)->getAttributeValue("text:note-class");
        UT_ASSERT_HARMLESS(pNoteClass != NULL);
        
        if (pNoteClass && !strcmp(pNoteClass, "footnote")) {
            isFootnote = true;
        } else if (pNoteClass && !strcmp(pNoteClass, "endnote")) {
            isFootnote = false;
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            // Unrecognized note class.
        }
        
        if (isFootnote) {
            id = m_pAbiDocument->getUID(UT_UniqueId::Footnote);
        } else {
            id = m_pAbiDocument->getUID(UT_UniqueId::Endnote);
        }
        UT_UTF8String_sprintf(m_currentNoteId, "%d", id);
        
        pNotesConfig = m_pStyles->getNotesConfiguration(pNoteClass);
        
        if (pNotesConfig) {
            pCitationStyleName = pNotesConfig->getCitationStyleName();
            
            if (!pCitationStyleName->empty()) {
                pStyle = m_pStyles->getTextStyle(pCitationStyleName->utf8_str(),
                            m_bOnContentStream);
            }
        }

        i = 0;
        ppAtts[i++] = "type";
        if (isFootnote) {
            ppAtts[i++] = "footnote_ref";
            ppAtts[i++] = "footnote-id";
        } else {
            ppAtts[i++] = "endnote_ref";
            ppAtts[i++] = "endnote-id";
        }
        ppAtts[i++] = m_currentNoteId.utf8_str();
        if (pCitationStyleName && (!pCitationStyleName->empty()) && (pStyle != NULL)) {
            ppAtts[i++] = "style";
            ppAtts[i++] = pStyle->getDisplayName().utf8_str();
        }
        ppAtts[i++] = "props";
        ppAtts[i++] = "text-position:superscript";
        ppAtts[i] = 0;
        
        ok = m_pAbiDocument->appendObject(PTO_Field, ppAtts);
        UT_ASSERT(ok);
        
        if (isFootnote) {
            ppAtts[0] = "footnote-id";
        } else {
            ppAtts[0] = "endnote-id";
        }
        ppAtts[1] = m_currentNoteId.utf8_str();
        ppAtts[2] = 0;
        
        if (isFootnote) {
            ok = m_pAbiDocument->appendStrux(PTX_SectionFootnote, ppAtts);
        } else {
            ok = m_pAbiDocument->appendStrux(PTX_SectionEndnote, ppAtts);
        }
        UT_ASSERT(ok);
        
        m_pendingNoteAnchorInsertion = true;
    }
    
    m_elementParsingLevel++;
}


/**
 * Called when an "end of element" tag is parsed (like <myElementName/>)
 * 
 * @param pName The name of the element
 */
void ODi_TextContent_ListenerState::endElement (const gchar* pName,
                                               ODi_ListenerStateAction& rAction)
{
    UT_ASSERT(m_elementParsingLevel >= 0);
    
    if (!strcmp(pName, "text:table-of-content")) {
        
        m_tablesOfContent.addItem( m_pCurrentTOCParser->getTOCStrux() );
        m_tablesOfContentProps.addItem( new UT_UTF8String(m_pCurrentTOCParser->getProps()) );
        DELETEP(m_pCurrentTOCParser);
        
    } else if (!strcmp(pName, "text:section" )) {

        if (m_currentODSection == ODI_SECTION_MAPPED) {
            // Just close the current section
            m_currentODSection = ODI_SECTION_UNDEFINED;
            m_inAbiSection = false;
        }

    } else if (!strcmp(pName, "text:p" ) || !strcmp(pName, "text:h" )) {

        _endParagraphElement(pName, rAction);
        
    } else if (!strcmp(pName, "text:span")) {
        
        _flush ();
        _popInlineFmt();
        m_pAbiDocument->appendFmt(&m_vecInlineFmt);
        
    } else if (!strcmp(pName, "text:a")) {
        
        _flush ();
        m_pAbiDocument->appendObject(PTO_Hyperlink, NULL);
        
    } else if (!strcmp(pName, "text:date") ||
        !strcmp(pName, "text:time") ||
        !strcmp(pName, "text:page-number") ||
        !strcmp(pName, "text:page-count") ||
        !strcmp(pName, "text:file-name") ||
        !strcmp(pName, "text:paragraph-count") ||
        !strcmp(pName, "text:word-count") ||
        !strcmp(pName, "text:character-count") ||
        !strcmp(pName, "text:initial-creator") ||
        !strcmp(pName, "text:author-name") ||
        !strcmp(pName, "text:description") ||
        !strcmp(pName, "text:keywords") ||
        !strcmp(pName, "text:subject") ||
        !strcmp(pName, "text:title")) {
            
        m_bAcceptingText = true;
        
    } else if (!strcmp(pName, "office:text")) {

        UT_ASSERT(m_elementParsingLevel == 1);        
        UT_ASSERT(m_bOnContentStream);
        
        // We were inside a <office:text> element.
        
        // That's it, we can't have anymore <text:h> elements.
        // So, let's define the heading styles on all Abi TOCs (<toc> struxs).
        _defineAbiTOCHeadingStyles();
        
        UT_VECTOR_PURGEALL(UT_UTF8String*, m_tablesOfContentProps);
        m_tablesOfContentProps.clear();        
        
        // We can now bring up the postponed parsing (headers/footers and
        // page-anchored frames)
        rAction.bringUpPostponedElements(false);
        
    } else if (!strcmp(pName, "style:header") ||
               !strcmp(pName, "style:footer") ||
               !strcmp(pName, "style:header-left") ||
               !strcmp(pName, "style:footer-left")) {

        UT_ASSERT(m_elementParsingLevel == 1);
        UT_ASSERT(!m_bOnContentStream);
        
        // We were inside a <style:header/footer> element.
        rAction.popState();
        
    } else if (!strcmp(pName, "text:list")) {
        m_listLevel--;
        if (m_listLevel == 0) {
            m_pCurrentListStyle = NULL;
        }
        
    } else if (!strcmp(pName, "draw:text-box")) {
        UT_ASSERT(m_elementParsingLevel == 1);
        
        // We were inside a <draw:text-box> element.
        rAction.popState();
        
    } else if (!strcmp(pName, "table:table-cell")) {
        UT_ASSERT(m_elementParsingLevel == 1);
        
        // We were inside a <table:table-cell> element.
        rAction.popState();
        
    } else if (!strcmp(pName, "text:note-body")) {
        bool ok;
        const gchar* pNoteClass;
        
        pNoteClass = m_rElementStack.getStartTag(1)->getAttributeValue("text:note-class");
        UT_ASSERT_HARMLESS(pNoteClass != NULL);
        
        if (pNoteClass && !strcmp(pNoteClass, "footnote")) {
            ok = m_pAbiDocument->appendStrux(PTX_EndFootnote, NULL);
        } else if (pNoteClass && !strcmp(pNoteClass, "endnote")) {
            ok = m_pAbiDocument->appendStrux(PTX_EndEndnote, NULL);
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            // Unrecognized note class.
        }
        
        UT_ASSERT(ok);

    } else if (!strcmp(pName, "text:note")) {
        UT_ASSERT(!m_pendingNoteAnchorInsertion);
        
        m_pendingNoteAnchorInsertion = false;
        m_currentNoteId.clear();

        // Back to paragraph text.
        m_bAcceptingText = true;        
    }
    
    m_elementParsingLevel--;
}


/**
 * 
 */
void ODi_TextContent_ListenerState::charData (
                            const gchar* pBuffer, int length)
{
    if (pBuffer && length && m_bAcceptingText) {
        m_charData += UT_UCS4String (pBuffer, length, true);
    }
}


/**
 * 
 */
void ODi_TextContent_ListenerState::_insertBookmark (const gchar* pName,
                                             const gchar* pType)
{
    UT_return_if_fail(pName && pType);

    const gchar* pPropsArray[5];
    pPropsArray[0] = (gchar *)"name";
    pPropsArray[1] = pName;
    pPropsArray[2] = (gchar *)"type";
    pPropsArray[3] = pType;
    pPropsArray[4] = 0;
    m_pAbiDocument->appendObject (PTO_Bookmark, pPropsArray);
}


/**
 * 
 */
void ODi_TextContent_ListenerState::_flush ()
{
    if (m_charData.size () > 0 && m_bAcceptingText) {
        m_pAbiDocument->appendSpan (m_charData.ucs4_str(), m_charData.size ());
        m_charData.clear ();
    } 
}


/**
 * 
 */
bool ODi_TextContent_ListenerState::_pushInlineFmt(const gchar ** atts)
{
    UT_uint32 start = m_vecInlineFmt.getItemCount()+1;
    UT_uint32 k;
    gchar* p;
    
    for (k=0; (atts[k]); k++)
    {
        if (!(p = g_strdup(atts[k]))) {
            return false;
        }
        
        if (m_vecInlineFmt.addItem(p)!=0) {
            return false;
        }
        
    }
    
    if (!m_stackFmtStartIndex.push(start)) {
        return false;
    }
        
    return true;
}


/**
 * 
 */
void ODi_TextContent_ListenerState::_popInlineFmt(void)
{
    UT_sint32 start;
    
    if (!m_stackFmtStartIndex.pop(&start))
        return;
        
    UT_uint32 k;
    UT_uint32 end = m_vecInlineFmt.getItemCount();
    const gchar* p;
    
    for (k=end; k>=start; k--) {
        
        p = (const gchar *)m_vecInlineFmt.getNthItem(k-1);
        m_vecInlineFmt.deleteNthItem(k-1);
        
        if (p)
            free((void *)p);
    }
}


/**
 * Makes sure that an AbiWord section have already been created. Unlike
 * OpenDocument, AbiWord can't have paragraphs without a section to hold them.
 * 
 * @param pMasterPageName The name of the master page to be used. i.e.: The name
 *                        of the master page which will have its properties used
 *                        in this section.
 */
void ODi_TextContent_ListenerState::_insureInSection(
                                         const UT_UTF8String* pMasterPageName) {
    
    if (m_inAbiSection) {
        if (pMasterPageName == NULL ||
            (pMasterPageName != NULL && pMasterPageName->empty()) ) {
            // There's nothing to be done.
            return;
        }
    }
    
    
    const ODi_StartTag* pStartTag;
    UT_UTF8String props = "";
    
    // Now we open an abi <section> according to the OpenDocument parent
    // section, if there is one.

    pStartTag = m_rElementStack.getClosestElement("text:section");
    
    if (pStartTag!=NULL) {
        const gchar* pStyleName;
        const ODi_Style_Style* pStyle;
        
        pStyleName = pStartTag->getAttributeValue("text:style-name");
        UT_ASSERT(pStyleName != NULL);
        
        pStyle = m_pStyles->getSectionStyle(pStyleName, m_bOnContentStream);
        if (pStyle) {
            pStyle->getAbiPropsAttrString(props);
        }
        
        
        // If it don't have any properties it's useless.
        //
        // OpenDocument sections can be used just to structure the
        // document (like naming sections and subsections). AbiWord will
        // consider only sections that contains meaningful formating
        // properties, like number of text columns, etc.
        if (props.empty()) {
            m_currentODSection = ODI_SECTION_IGNORED;
        } else {
            m_currentODSection = ODI_SECTION_MAPPED;
        }
    } else {
        // We will open an empty section
        m_currentODSection = ODI_SECTION_NONE;
    }
    
    _openAbiSection(props, pMasterPageName);
}


/**
 * Tries to open an abi <section> given its properties.
 * 
 * @param pProps The properties of the abi <section> to be opened.
 */
void ODi_TextContent_ListenerState::_openAbiSection(
                                         const UT_UTF8String& rProps,
                                         const UT_UTF8String* pMasterPageName) {

    UT_UTF8String allProps;
    UT_UTF8String dataID;
    

    const ODi_Style_MasterPage* pMasterPageStyle = NULL;

    if (pMasterPageName != NULL && !pMasterPageName->empty()) {
        
        pMasterPageStyle = m_pStyles->getMasterPageStyle(pMasterPageName->utf8_str());
        
        if (pMasterPageStyle && pMasterPageStyle->getPageLayout()) {
            allProps = pMasterPageStyle->getSectionProps();
            dataID = pMasterPageStyle->getSectionDataID();
            UT_ASSERT(!allProps.empty());
        }
        
        m_openedFirstAbiSection = true;
    }

    if (!m_openedFirstAbiSection) {
        // We haven't defined any page properties yet. It's done on the
        // first abi section.
        
        // For now we just use the Standard page master. AbiWord doesn't support
        // multiple page formats anyway.
        
        pMasterPageStyle = m_pStyles->getMasterPageStyle("Standard");
        UT_return_if_fail(pMasterPageStyle);
        
        UT_ASSERT(pMasterPageStyle->getPageLayout());
        
        allProps = pMasterPageStyle->getSectionProps();
        dataID = pMasterPageStyle->getSectionDataID();
        UT_ASSERT(!allProps.empty());

        m_openedFirstAbiSection = true;
    }
    
    // The AbiWord section properties are taken part from the OpenDocument 
    // page layout (from the master page style) and part from the OpenDocument
    // section properties.
    
    // TODO: What happens if there are duplicated properties on the page layout
    // and on the section?

    if (!allProps.empty() && !rProps.empty()) {
        allProps += "; ";
    }
    allProps += rProps;

    
    const gchar* atts[20];
    UT_uint8 i = 0;
    atts[i++] = "props";
    atts[i++] = allProps.utf8_str();
 
    if (pMasterPageStyle != NULL) {
        // The standard master page may have headers/footers as well.
        
        if (!pMasterPageStyle->getAWEvenHeaderSectionID().empty()) {
            atts[i++] = "header-even";
            atts[i++] = pMasterPageStyle->getAWEvenHeaderSectionID().utf8_str();
        }
        
        if (!pMasterPageStyle->getAWHeaderSectionID().empty()) {
            atts[i++] = "header";
            atts[i++] = pMasterPageStyle->getAWHeaderSectionID().utf8_str();
        }
        
        if (!pMasterPageStyle->getAWEvenFooterSectionID().empty()) {
            atts[i++] = "footer-even";
            atts[i++] = pMasterPageStyle->getAWEvenFooterSectionID().utf8_str();
        }
        
        if (!pMasterPageStyle->getAWFooterSectionID().empty()) {
            atts[i++] = "footer";
            atts[i++] = pMasterPageStyle->getAWFooterSectionID().utf8_str();
        }

        if (dataID.length()) {
            atts[i++] = "strux-image-dataid";
            atts[i++] = dataID.utf8_str();
        }
    }
    
    atts[i] = 0; // No more attributes.

    if(m_inAbiSection && !m_bOpenedBlock) {
        _insureInBlock(NULL); //see Bug 10627 - hang on empty <section>
    }
   
    m_pAbiDocument->appendStrux(PTX_Section, (const gchar**)atts);    
    m_bOpenedBlock = false;

    // For some reason AbiWord can't have a page break right before a new section.
    // In AbiWord, if you want to do that you have to first open the new section
    // and then, inside this new section, do the page break.
    //
    // That's the only reason for the existence of *pending* paragraph
    // (column or page) breaks.
    _flushPendingParagraphBreak();

    m_inAbiSection = true;
    m_bAcceptingText = false;
}


/**
 * 
 */
void ODi_TextContent_ListenerState::_insureInBlock(const gchar ** atts)
{
    if (m_bAcceptingText)
        return;

    _insureInSection();

    if (!m_bAcceptingText) {
        m_pAbiDocument->appendStrux(PTX_Block, (const gchar**)atts);    
        m_bOpenedBlock = true;
        m_bAcceptingText = true;
    }
}


/**
 * Process <text:p> and <text:h> startElement calls
 */
void ODi_TextContent_ListenerState::_startParagraphElement (const gchar* pName,
                                          const gchar** ppParagraphAtts,
                                          ODi_ListenerStateAction& rAction) {
        bool bIsListParagraph = false;
        const gchar* pStyleName;
        const gchar *ppAtts[50];
        UT_uint8 i;
        gchar listLevel[10];
        bool ok;
        UT_UTF8String props;
        const ODi_Style_Style* pStyle;
        
        
        if (!strcmp(m_rElementStack.getStartTag(0)->getName(), "text:list-item")) {
            // That's a list paragraph.
            bIsListParagraph = true;
        }

        pStyleName = UT_getAttribute ("text:style-name", ppParagraphAtts);

        if (pStyleName) {
            pStyle = m_pStyles->getParagraphStyle(pStyleName, m_bOnContentStream);
            UT_ASSERT(pStyle);
            
            // Damn, use the default style
            if (!pStyle) {
                pStyle = m_pStyles->getDefaultParagraphStyle();
            }
        } else {
            // Use the default style
            pStyle = m_pStyles->getDefaultParagraphStyle();
        }

        // We can't define new sections from inside a table cell
        if (!m_rElementStack.hasElement("table:table-cell")) {

            if (pStyle!=NULL && !pStyle->getMasterPageName()->empty()) {
                bool isFirstAbiSection = !m_openedFirstAbiSection;
                
                _insureInSection(pStyle->getMasterPageName());
                
                if (!isFirstAbiSection) {
                    // We must be changing the master page style. A page break must
                    // be inserted before doing this change (as the OpenDocument
                    // standard say).
                    
                    // Append an empty paragraph with this one char
                    UT_UCSChar ucs = UCS_FF;
                    m_pAbiDocument->appendStrux(PTX_Block, NULL);
                    m_pAbiDocument->appendSpan (&ucs, 1);
                    m_bOpenedBlock = true;
                }
            } else {
                _insureInSection();
                
                // Should we insert a break before this paragraph?
                if (pStyle != NULL && !pStyle->getBreakBefore().empty()) {
                    UT_UCSChar ucs;
                    if (pStyle->getBreakBefore() == "page") {
                        ucs = UCS_FF;
                        // Append an empty paragraph with this one char
                        m_pAbiDocument->appendStrux(PTX_Block, NULL);
                        m_pAbiDocument->appendSpan (&ucs, 1);
                        m_bOpenedBlock = true;
                    } else if (pStyle->getBreakBefore() == "column") {
                        ucs = UCS_VTAB;
                        // Append an empty paragraph with this one char
                        m_pAbiDocument->appendStrux(PTX_Block, NULL);
                        m_pAbiDocument->appendSpan (&ucs, 1);
                        m_bOpenedBlock = true;
                    }
                }
            }

        }
        


        i = 0;

        if (bIsListParagraph && !m_alreadyDefinedAbiParagraphForList) {
            ODi_ListLevelStyle* pListLevelStyle = NULL;
            
            m_alreadyDefinedAbiParagraphForList = true;
            if (m_pCurrentListStyle) {
                pListLevelStyle = m_pCurrentListStyle->getLevelStyle(m_listLevel);
            }
            
            sprintf(listLevel, "%u", m_listLevel);
            
            ppAtts[i++] = "level";
            ppAtts[i++] = listLevel;
            if (pListLevelStyle && pListLevelStyle->getAbiListID() && pListLevelStyle->getAbiListParentID()) {
                ppAtts[i++] = "listid";
                ppAtts[i++] = pListLevelStyle->getAbiListID()->utf8_str();
            
                // Is this really necessary? Because we already have this info on
                // the <l> tag.
                ppAtts[i++] = "parentid";
                ppAtts[i++] = pListLevelStyle->getAbiListParentID()->utf8_str();
            }
            
            if (pStyle!=NULL) {
                if (pStyle->isAutomatic()) {
                    // Automatic styles are not defined on the document, so, we
                    // just paste its properties.
                    pStyle->getAbiPropsAttrString(props);
                    
                } else {
                    // We refer to the style
                    ppAtts[i++] = "style";
                    ppAtts[i++] = pStyle->getDisplayName().utf8_str();
                }
            }

            if (pListLevelStyle) {
                pListLevelStyle->getAbiProperties(props, pStyle);
                
                ppAtts[i++] = "props";
                ppAtts[i++] = props.utf8_str();
            }
                
            ppAtts[i] = 0; // Marks the end of the attributes list.
            ok = m_pAbiDocument->appendStrux(PTX_Block,
                                             (const gchar**)ppAtts);
            UT_ASSERT(ok);
            m_bOpenedBlock = true;

            

            ppAtts[0] = "type";
            ppAtts[1] = "list_label";
            ppAtts[2] = 0;
            ok = m_pAbiDocument->appendObject(PTO_Field, ppAtts);
            UT_ASSERT(ok);
            
            // Inserts a tab character. AbiWord seens to need it in order to
            // implement the space between the list mark (number/bullet) and
            // the list text.
            UT_UCS4String string = "\t";
            _flush();
            m_pAbiDocument->appendSpan(string.ucs4_str(), string.size());
            
        } else if (bIsListParagraph && m_alreadyDefinedAbiParagraphForList) {
            // OpenDocument supports multiples paragraphs on a single list item,
            // But AbiWord works differently. So, we will put a <br/> instead
            // of adding a new paragraph.
            
            UT_UCSChar ucs = UCS_LF;
            m_pAbiDocument->appendSpan(&ucs,1);
            
            if (pStyle!=NULL) { 
                if (pStyle->isAutomatic()) {
                    // Automatic styles are not defined on the document, so, we
                    // just paste its properties.
                    pStyle->getAbiPropsAttrString(props);
                    ppAtts[i++] = "props";
                    ppAtts[i++] = props.utf8_str();
                } else {
                    // We refer to the style
                    ppAtts[i++] = "style";
                    ppAtts[i++] = pStyle->getDisplayName().utf8_str();
                }
            }
            ppAtts[i] = 0; // Marks the end of the attributes list.
            
            ok = m_pAbiDocument->appendFmt(ppAtts);
            UT_ASSERT(ok);

        } else {
            
            if (pStyle != NULL) {
                if (pStyle->isAutomatic()) {
                    // Automatic styles are not defined on the document, so, we
                    // just paste its properties.
                    pStyle->getAbiPropsAttrString(props, FALSE);
                    ppAtts[i++] = "props";
                    ppAtts[i++] = props.utf8_str();
                    
                    if (pStyle->getParent() != NULL) {
                        ppAtts[i++] = "style";
                        ppAtts[i++] = pStyle->getParent()->getDisplayName().utf8_str();
                    }
                } else {
                    // We refer to the style
                    ppAtts[i++] = "style";
                    ppAtts[i++] = pStyle->getDisplayName().utf8_str();
                }
            }
        
            ppAtts[i] = 0; // Marks the end of the attributes list.
            m_pAbiDocument->appendStrux(PTX_Block, (const gchar**)ppAtts);
            m_bOpenedBlock = true;
        }

        // We now accept text
        m_bAcceptingText = true;

        if (m_pendingNoteAnchorInsertion) {
            m_pendingNoteAnchorInsertion = false;

            UT_return_if_fail(!m_currentNoteId.empty());
            
            const gchar* pNoteClass;
            const ODi_StartTag* pStartTag;
        
            pStartTag = m_rElementStack.getClosestElement("text:note", 1);
            UT_return_if_fail (pStartTag != NULL);
            
            pNoteClass = pStartTag->getAttributeValue("text:note-class");
                                
            UT_return_if_fail(pNoteClass != NULL);
            
            ppAtts[0] = "type";
            if (!strcmp(pNoteClass, "footnote")) {
                ppAtts[1] = "footnote_anchor";
                ppAtts[2] = "footnote-id";
            } else if (!strcmp(pNoteClass, "endnote")) {
                ppAtts[1] = "endnote_anchor";
                ppAtts[2] = "endnote-id";
            } else {
                UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
                // Unrecognized note class.
            }
            ppAtts[3] = m_currentNoteId.utf8_str();
            ppAtts[4] = 0;
            
            ok = m_pAbiDocument->appendObject(PTO_Field, ppAtts);
            UT_ASSERT(ok);           
        }
}


/**
 * 
 */
void ODi_TextContent_ListenerState::_endParagraphElement (
                                           const gchar* pName,
                                           ODi_ListenerStateAction& rAction) {
                                            
    const gchar* pStyleName;
    const ODi_Style_Style* pStyle;
    
    _flush ();
    m_bAcceptingText = false;
    
   
    pStyleName = m_rElementStack.getStartTag(0)->
                    getAttributeValue("text:style-name");
                    
    if (pStyleName) {
        pStyle = m_pStyles->getParagraphStyle(pStyleName, m_bOnContentStream);
        UT_ASSERT(pStyle);
        
        // Damn, use the default style
        if (!pStyle) {
            pStyle = m_pStyles->getDefaultParagraphStyle();
        }
    } else {
        // Use the default style
        pStyle = m_pStyles->getDefaultParagraphStyle();
    }
    
    if (pStyle != NULL) {
        m_pendingParagraphBreak = pStyle->getBreakAfter();
    }
    
    
    if (!m_rElementStack.hasElement("text:note-body")) {
        // Footnotes/endnotes can't have frames.
        
        // Bring back the possible postponed parsing of a <draw:frame>
        rAction.bringUpMostRecentlyPostponedElement("Frame", true);
    }
}


/**
 * Defines all toc-source-style* propeties on Abi TOCs (<toc> struxs)
 */
void ODi_TextContent_ListenerState::_defineAbiTOCHeadingStyles() {
    UT_uint32 i, j, count;
    pf_Frag_Strux* pTOCStrux;
    UT_UTF8String str;
    UT_UTF8String props;
    std::string styleName;
    bool ok;
    
    count = m_tablesOfContent.getItemCount();
    for (i=0; i<count; i++) {
        pTOCStrux = m_tablesOfContent[i];
        props = *(m_tablesOfContentProps[i]);
        
        for (j=1; j<5; j++) {
            UT_UTF8String_sprintf(str, "%d", j);
            styleName = m_headingStyles[str.utf8_str()];

            if (!styleName.empty()) {
                UT_UTF8String_sprintf(str, "toc-source-style%d:%s", j,
                                      styleName.c_str());
                
                if (!props.empty()) {
                    props += "; ";
                }
                props += str;
            }
        }
        
        ok = m_pAbiDocument->changeStruxAttsNoUpdate(
                            (PL_StruxDocHandle) pTOCStrux, "props",
                            props.utf8_str());
        UT_ASSERT(ok);
    }
}


/**
 * For some reason AbiWord can't have a page break right before a new section.
 * In AbiWord, if you want to do that you have to first open the new section
 * and then, inside this new section, do the page break.
 * 
 * That's the only reason for the existence of *pending* paragraph
 * (column or page) breaks.
 */
void ODi_TextContent_ListenerState::_flushPendingParagraphBreak() {
    if (!m_pendingParagraphBreak.empty()) {
        
        if (m_pendingParagraphBreak == "page") {
            m_pAbiDocument->appendStrux(PTX_Block, NULL);
            UT_UCSChar ucs = UCS_FF;
            m_pAbiDocument->appendSpan (&ucs, 1);
            m_bOpenedBlock = true;
        } else if (m_pendingParagraphBreak == "column") {
            m_pAbiDocument->appendStrux(PTX_Block, NULL);
            UT_UCSChar ucs = UCS_VTAB;
            m_pAbiDocument->appendSpan (&ucs, 1);
            m_bOpenedBlock = true;
        }
        
        m_pendingParagraphBreak.clear();
    }
}
