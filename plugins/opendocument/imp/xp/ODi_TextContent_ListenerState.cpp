/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* AbiSource
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
 * Copyright (C) 2011-2012 Ben Martin
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
#include "ODi_Abi_Data.h"
#include "ut_growbuf.h"
#include "pf_Frag.h"
#include "ie_exp_RTF.h"
#include "ut_units.h"
#include "ut_std_string.h"

// AbiWord includes
#include <ut_misc.h>
#include <pd_Document.h>
#include <pf_Frag_Strux.h>

#include <list>
#include <sstream>



#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "pd_RDFQuery.h"




/************************************************************/
/************************************************************/
/************************************************************/

/**
 * Constructor
 */
ODi_TextContent_ListenerState::ODi_TextContent_ListenerState (
                PD_Document* pDocument,
                ODi_Office_Styles* pStyles,
                ODi_ElementStack& rElementStack,
		ODi_Abi_Data& rAbiData)
                : ODi_ListenerState("TextContent", rElementStack),
                  m_pAbiDocument ( pDocument ),
                  m_pStyles(pStyles),
                  m_bAcceptingText(false),
                  m_bOpenedBlock(false),
                  m_inAbiSection(false),
                  m_openedFirstAbiSection(false),
                  m_bPendingSection(false),
                  m_currentODSection(ODI_SECTION_NONE),
                  m_elementParsingLevel(0),
                  m_pCurrentTOCParser(NULL),
                  m_bOnContentStream(false),
                  m_pCurrentListStyle(NULL),
                  m_listLevel(0),
                  m_alreadyDefinedAbiParagraphForList(false),
                  m_pendingNoteAnchorInsertion(false),
                  m_bPendingAnnotation(false),
                  m_bPendingAnnotationAuthor(false),
                  m_bPendingAnnotationDate(false),
                  m_iAnnotation(0),
		  m_bPageReferencePending(false),
		  m_iPageNum(0),
		  m_dXpos(0.0),
		  m_dYpos(0.0),
		  m_sProps(""),
		  m_rAbiData(rAbiData),
		  m_bPendingTextbox(false),
		  m_bHeadingList(false),
		  m_prevLevel(0),
		  m_bContentWritten(false),
                  m_columnsCount(1),
                  m_columnIndex(1)
{
    UT_ASSERT_HARMLESS(m_pAbiDocument);
    UT_ASSERT_HARMLESS(m_pStyles);
}


/**
 * Destructor
 */
ODi_TextContent_ListenerState::~ODi_TextContent_ListenerState() 
{
    if (m_tablesOfContentProps.getItemCount() > 0) {
        UT_DEBUGMSG(("ERROR ODti: table of content props not empty\n"));
        UT_VECTOR_PURGEALL(std::string*, m_tablesOfContentProps);
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
    m_bHeadingList = false;
    if (!strcmp(pName, "text:section" )) {
		
        if (m_bPendingSection)
        {
            // this can only occur when we have a section pending with 
            // no content in it, which AbiWord does not support. Since I'm 
            // not sure if OpenDocument even allows it, I'll assert on it
            // for now - MARCM
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN); // or should it?
        }


        const gchar* pStyleName = UT_getAttribute ("text:style-name", ppAtts);
        UT_ASSERT(pStyleName != NULL);
        
        const ODi_Style_Style* pStyle = m_pStyles->getSectionStyle(pStyleName,
                                                        m_bOnContentStream);
                                                        
        std::string props = "";

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
			// Styles placed on elements such as <text:p> can actually
			// contain section related properties such as headers/footers 
			// via the master page style. Bug 10399 shows an example of this,
			// See http://bugzilla.abisource.com/show_bug.cgi?id=10399 for details.

			// Therfore, we delay writing out the section until we encounter
			// the first element in the section. This allows us to merge the 'section' 
			// parts of both element's styles.
			m_bPendingSection = true;
        }

    } else if (!strcmp(pName, "text:p" )) {

        if (m_bPendingAnnotation) {
            _insertAnnotation();
        }

        // It's so big that it deserves its own function.
        _startParagraphElement(pName, ppAtts, rAction);
        
    } else if (!strcmp(pName, "text:h" )) {

        const gchar* pStyleName = NULL;
        const gchar* pOutlineLevel = NULL;
        const ODi_Style_Style* pStyle = NULL;
        
        pOutlineLevel = UT_getAttribute("text:outline-level", ppAtts);
        if (pOutlineLevel == NULL) {
            // Headings without a level attribute are assumed to
            // be at level 1.
            pOutlineLevel = "1";
        }
        std::string sHeadingListName = "BaseHeading";
        m_listLevel = atoi(pOutlineLevel);
        m_pCurrentListStyle =  m_pStyles->getList( sHeadingListName.c_str());
        if(m_pCurrentListStyle)
        {
            ODi_ListLevelStyle *pLevelStyle = m_pCurrentListStyle->getLevelStyle(m_listLevel);
            if (pLevelStyle && pLevelStyle->isVisible())
            {
                xxx_UT_DEBUGMSG(("Found %s ! outline level %s \n",sHeadingListName.utf8_str(),pOutlineLevel));
                m_bHeadingList = true;
            }
        }

        pStyleName = UT_getAttribute("text:style-name", ppAtts);
        if (pStyleName) 
        {
            pStyle = m_pStyles->getParagraphStyle(pStyleName, m_bOnContentStream);
        }
        
        if (pStyle && (pStyle->isAutomatic())) {

            if (pStyle->getParent() != NULL) {

                m_headingStyles[pOutlineLevel] = 
					pStyle->getParent()->getDisplayName().c_str();
            } else {
                UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
                // This is not expected from a well formed file.
                // So, it's corrputed. But we can ignore this error without
                // compromising the doc load as a whole
                // (the TOC will not be displayed correctly, though).
            }
            
        } else if (pStyle) {
            m_headingStyles[pOutlineLevel] =
				pStyle->getDisplayName().c_str();
        }

        // It's so big that it deserves its own function.
        m_alreadyDefinedAbiParagraphForList = false;
        _startParagraphElement(pName, ppAtts, rAction);
        m_bHeadingList = false;
        m_pCurrentListStyle = NULL;
    } else if (!strcmp(pName, "text:s")) {
        // A number of consecutive white-space characters.
        
        const gchar* pSpaceCount;
        UT_uint32 spaceCount, i;
        UT_sint32 tmpSpaceCount = 0;
        UT_UCS4String string;
        
        pSpaceCount = UT_getAttribute("text:c", ppAtts);
        
        if (pSpaceCount && *pSpaceCount) {
            i = sscanf(pSpaceCount, "%d", &tmpSpaceCount);
            if ((i != 1) || (tmpSpaceCount <= 1)) {
                spaceCount = 1;
            } else {
                spaceCount = tmpSpaceCount;
            }
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
	m_bContentWritten = true;
       
    } else if (!strcmp(pName, "text:tab")) {
        // A tab character.

        UT_UCS4String string = "\t";
       
        // Write the text that has not been written yet.
        // Otherwise the spaces will appear in the wrong position.
        _flush();
        
        m_pAbiDocument->appendSpan(string.ucs4_str(), string.size());
	m_bContentWritten = true; 

    } else if (!strcmp(pName, "text:table-of-content")) {

        _flush ();
        _insureInBlock(PP_NOPROPS);

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

                PP_PropertyVector ppStyAttr(2);
                std::string props;

                if (pStyle->isAutomatic()) {
                    pStyle->getAbiPropsAttrString(props);

                    // It goes "hardcoded"
                    ppStyAttr[0] = "props";
                    ppStyAttr[1] = props;
                } else {
                    ppStyAttr[0] = "style";
                    ppStyAttr[1] = pStyle->getDisplayName();
                }
                
                _pushInlineFmt(ppStyAttr);
                UT_DebugOnly<bool> ok;
                ok = m_pAbiDocument->appendFmt(m_vecInlineFmt);
                UT_ASSERT(ok);
                
            } else {
                // We just ignore this <text:span>.
            }
        }

    } else if (!strcmp(pName, "text:meta")) {

        _flush ();

        std::string generatedID;
        const gchar* xmlid = UT_getAttribute("xml:id", ppAtts);
        if( !xmlid )
        {
            generatedID = UT_std_string_sprintf("%d", m_pAbiDocument->getUID( UT_UniqueId::Annotation ));
            xmlid = generatedID.c_str();
        }

        const PP_PropertyVector pa = {
          PT_XMLID, xmlid,
          // sanity check
          "this-is-an-rdf-anchor", "yes"
        };

        m_pAbiDocument->appendObject( PTO_RDFAnchor, pa );
        xmlidStackForTextMeta.push_back( xmlid );

    } else if (!strcmp(pName, "text:line-break")) {
        
        m_charData += UCS_LF;
        _flush ();
        
    } else if (!strcmp(pName, "text:a")) {
        
        _flush();
        const PP_PropertyVector xlink_atts = {
            "xlink:href", UT_getAttribute("xlink:href", ppAtts)
        };
        m_pAbiDocument->appendObject(PTO_Hyperlink, xlink_atts);
        
    } else if (!strcmp(pName, "text:bookmark")) {
        
        _flush ();
        const gchar * pAttr = UT_getAttribute ("text:name", ppAtts);
        const gchar* xmlid = UT_getAttribute("xml:id", ppAtts);

        if(pAttr) {
            _insertBookmark (pAttr, "start", xmlid );
            _insertBookmark (pAttr, "end");
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        }
        
    } else if (!strcmp(pName, "text:bookmark-start")) {

        _flush ();
        const gchar * pAttr = UT_getAttribute ("text:name", ppAtts);
        const gchar* xmlid = UT_getAttribute("xml:id", ppAtts);
        xmlidMapForBookmarks[pAttr] = ( xmlid ? xmlid : "" );
        
        if(pAttr) {
            _insertBookmark (pAttr, "start", xmlid );
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        }

    } else if (!strcmp(pName, "text:bookmark-end")) {

        _flush ();
        const gchar * pAttr = UT_getAttribute ("text:name", ppAtts);
        std::string xmlid = "";
        

        if( const gchar* t = UT_getAttribute("xml:id", ppAtts))
        {
            xmlid = t;
        }
        else
        {
            xmlid = xmlidMapForBookmarks[pAttr];
        }
        xmlidMapForBookmarks.erase(pAttr);
        
        if(pAttr) {
            _insertBookmark (pAttr, "end", xmlid.c_str() );
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
            !strcmp(pName, "text:title") ||
            !strcmp(pName, "text:modification-date")) {

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
        else if(!strcmp(pName, "text:file-name")){
        	const gchar * pDisplay = UT_getAttribute ("text:display", ppAtts);
        	if (!strcmp(pDisplay, "name-and-extension"))
        		type = "short_file_name";
        	else type = "file_name";
        }
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
        else if(!strcmp(pName, "text:modification-date"))
            type = "meta_date_last_changed";


        PP_PropertyVector field_fmt = {
            "type", type
        };
        m_pAbiDocument->appendObject(PTO_Field, field_fmt);
        m_bAcceptingText = false;

    } else if (!strcmp(pName, "text:tracked-changes")){
		 UT_DEBUGMSG(("Ignoring text:tracked-changes \n"));
		 rAction.ignoreElement(-1);
       
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

        if (m_bPendingAnnotation) {
            _insertAnnotation();
        }
        m_listLevel++;
        if (m_pCurrentListStyle == NULL) {
			const gchar* pVal;

			pVal = UT_getAttribute("text:style-name", ppAtts);


			if (pVal && *pVal)
				m_pCurrentListStyle = m_pStyles->getList(pVal);
			UT_ASSERT(m_pCurrentListStyle != NULL);
        }
        
    } else if (!strcmp(pName, "text:list-item")) {
        m_alreadyDefinedAbiParagraphForList = false;
        
    } else if (!strcmp(pName, "draw:frame")) {
        
        if (!strcmp(m_rElementStack.getStartTag(0)->getName(), "text:p") ||
            !strcmp(m_rElementStack.getStartTag(0)->getName(), "text:h") ||
            !strcmp(m_rElementStack.getStartTag(0)->getName(), "office:text")) 
	{
            
            const gchar* pVal = NULL;
            
            pVal = UT_getAttribute("text:anchor-type", ppAtts);
            UT_ASSERT(pVal);
            
            if (pVal && (!strcmp(pVal, "paragraph") ||
                !strcmp(pVal, "page"))) {
                // It's postponed because AbiWord uses frames *after* the
                // paragraph but OpenDocument uses them inside the paragraphs,
                // right *before* its content.
                rAction.postponeElementParsing("Frame");
            } else {
                // It's an inlined frame.
                _flush();
                rAction.pushState("Frame");
            }
            
        } 
	else if (!strcmp(m_rElementStack.getStartTag(0)->getName(), "office:text")) 
	{
            
 	  // A page anchored frame.
 	  // Store this in the PD_Document until after the rest of the
	  // is layed out.
	  // First acquired the info we need
	  const gchar* pVal = NULL;
	  bool bCont = true;
	  m_iPageNum = 0;
	  m_dXpos = 0.0;
	  m_dYpos = 0.0;
	  m_sProps.clear();
	  m_bPageReferencePending = false;
	  pVal = UT_getAttribute("text:anchor-page-number", ppAtts);
	  if(!pVal || !*pVal)
	  {
	      rAction.ignoreElement();
	      bCont = false;
	  }
	  if(bCont)
	  {
	      m_iPageNum = atoi(pVal);
	  }
	  pVal = UT_getAttribute("svg:x", ppAtts);
	  if(!pVal || !*pVal)
	  {
	      rAction.ignoreElement();
	      bCont = false;
	  }
	  if(bCont)
	  {
	      m_dXpos = UT_convertToInches(pVal);
	  }
	  pVal = UT_getAttribute("svg:y", ppAtts);
	  if(!pVal || !*pVal)
	  {
	      rAction.ignoreElement();
	      bCont = false;
	  }
	  if(bCont)
	  {
	      m_dYpos = UT_convertToInches(pVal);
	      m_bPageReferencePending=true;
	  }
	  pVal = UT_getAttribute("svg:width", ppAtts);
	  if(pVal && *pVal)
	  {
	      UT_std_string_setProperty(m_sProps,"frame-width",pVal);
	  }
	  pVal = UT_getAttribute("svg:height", ppAtts);
	  if(pVal && *pVal)
	  {
	      UT_std_string_setProperty(m_sProps,"frame-height",pVal);
	  }
	  //
	  // Get wrapping style
	  //
	  const gchar* pStyleName = NULL;
	  pStyleName = UT_getAttribute("draw:style-name", ppAtts);
	  if(pStyleName)
	  {
	      const ODi_Style_Style* pGraphicStyle = m_pStyles->getGraphicStyle(pStyleName, m_bOnContentStream);
	      if(pGraphicStyle)
	      {
		  const std::string* pWrap=NULL;
		  pWrap = pGraphicStyle->getWrap(false);
		  if(pWrap)
		  {
		      if ( !strcmp(pWrap->c_str(), "run-through")) 
		      {
			  // Floating wrapping.
			  m_sProps += "; wrap-mode:above-text";
		      } 
		      else if ( !strcmp(pWrap->c_str(), "left")) 
		      {
			  m_sProps += "; wrap-mode:wrapped-to-left";
		      } 
		      else if ( !strcmp(pWrap->c_str(), "right")) 
		      {
			  m_sProps += "; wrap-mode:wrapped-to-right";
		      } 
		      else if ( !strcmp(pWrap->c_str(), "parallel")) 
		      {
			  m_sProps += "; wrap-mode:wrapped-both";
		      } 
		      else 
		      {
			  // Unsupported.        
			  // Let's put an arbitrary wrap mode to avoid an error.
			  m_sProps += "; wrap-mode:wrapped-both";
		      }
		  }
	      }
	  }
        } 
	else if (!strcmp(m_rElementStack.getStartTag(0)->getName(),
                              "text:span")) 
        {
            // Must be an inlined image, otherwise we can't handle it.
            
            const gchar* pVal;
            
            pVal = UT_getAttribute("text:anchor-type", ppAtts);
            
            if (!pVal || !strcmp(pVal, "as-char") || !strcmp(pVal, "char")) {
                _flush();
                rAction.pushState("Frame");
            }
	    else {
                // Ignore this element
                rAction.ignoreElement();
            }

        }
	else 
        {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            // TODO: Figure out what to do then.
        }
        
    } 
    else if (!strcmp(pName, "draw:image"))
    {
	if(m_bPageReferencePending)
	{
	  //
	  // We have page referenced image to handle.
	  //  
	  UT_String dataId; // id of the data item that contains the image.
	  m_rAbiData.addImageDataItem(dataId, ppAtts);
	  const gchar* pStyleName;
	  const ODi_Style_Style* pGraphicStyle;
	  const std::string* pWrap;
    
	  pStyleName = m_rElementStack.getStartTag(0)->getAttributeValue("draw:style-name");
	  UT_ASSERT(pStyleName);
    
	  pGraphicStyle = m_pStyles->getGraphicStyle(pStyleName, true);
	  if(pGraphicStyle)
	  {    
	    pWrap = pGraphicStyle->getWrap(false);
                                                    
	    if ( !strcmp(pWrap->c_str(), "run-through")) 
	    {
		// Floating wrapping.
		m_sProps += "; wrap-mode:above-text";
		
	    } 
	    else if ( !strcmp(pWrap->c_str(), "left")) 
	    {
		m_sProps += "; wrap-mode:wrapped-to-left";
	    } 
	    else if ( !strcmp(pWrap->c_str(), "right")) 
	    {
		m_sProps += "; wrap-mode:wrapped-to-right";
	    } 
	    else if ( !strcmp(pWrap->c_str(), "parallel")) 
	    {
		m_sProps += "; wrap-mode:wrapped-both";
	    } 
	    else 
	    {
		// Unsupported.        
		// Let's put an arbitrary wrap mode to avoid an error.
		m_sProps += "; wrap-mode:wrapped-both";
	    }
	  }
	    //
	    // OK lets write this into the document for later use
	    //
	  UT_UTF8String sImageId = dataId.c_str();
	  m_pAbiDocument->addPageReferencedImage(sImageId, m_iPageNum, m_dXpos, m_dYpos, m_sProps.c_str());
	  
	  m_bPageReferencePending = false;
	}
	else
	{
	  // Ignore this element
	  rAction.ignoreElement();
	}
    }
    else if (!strcmp(pName, "draw:text-box")) 
    {
        
        // We're inside a text-box, parsing its text contents.
        m_inAbiSection = true;
        m_bOnContentStream = true;
	if(m_bPageReferencePending)
	{
	  //
	  // We have page referenced text box to handle.
	  // Start collecting text after first gathering the infor we
	  // for the frame
	  //  
	  m_bPendingTextbox = true;
	  m_sProps += ";bot-style:1; left-style:1; right-style:1; top-style:1";
	  
	}        
    } else if (!strcmp(pName, "draw:g")) {
      UT_DEBUGMSG(("Unallowed drawing element %s \n",pName));
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
        
    } else if (!strcmp(pName, "text:note-citation")) {  
        m_bPendingNoteCitation = true;
        m_bAcceptingText = false;
    } 
    else if (!strcmp(pName, "text:note-body")) {

        // Make sure the ppAtts2 size is correct.
        UT_uint32 id;
        const ODi_NotesConfiguration* pNotesConfig;
        const ODi_Style_Style* pStyle = NULL;
        const std::string* pCitationStyleName = NULL;
        bool isFootnote = false;
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
        m_currentNoteId = UT_std_string_sprintf("%d", id);
        
        pNotesConfig = m_pStyles->getNotesConfiguration(pNoteClass);
        
        if (pNotesConfig) {
            pCitationStyleName = pNotesConfig->getCitationStyleName();
            
            if (!pCitationStyleName->empty()) {
                pStyle = m_pStyles->getTextStyle(pCitationStyleName->c_str(),
                            m_bOnContentStream);
            }
        }

        PP_PropertyVector ppAtts2;
        ppAtts2.push_back("type");
        if (isFootnote) {
            ppAtts2.push_back("footnote_ref");
            ppAtts2.push_back("footnote-id");
        } else {
            ppAtts2.push_back("endnote_ref");
            ppAtts2.push_back("endnote-id");
        }
        ppAtts2.push_back(m_currentNoteId);
        if (pCitationStyleName && (!pCitationStyleName->empty()) && (pStyle != NULL)) {
            ppAtts2.push_back("style");
            ppAtts2.push_back(pStyle->getDisplayName());
        }
        ppAtts2.push_back("props");
        ppAtts2.push_back("text-position:superscript");
        if (m_bPendingNoteCitation) {
            ppAtts2.push_back("text:note-citation");
            ppAtts2.push_back(m_noteCitation);
            m_bPendingNoteCitation = false;
        }

        UT_DebugOnly<bool> ok;
        ok = m_pAbiDocument->appendObject(PTO_Field, ppAtts2);
        UT_ASSERT(ok);

        // new props
        ppAtts2.resize(2);
        if (isFootnote) {
            ppAtts2[0] = "footnote-id";
        } else {
            ppAtts2[0] = "endnote-id";
        }
        ppAtts2[1] = m_currentNoteId.c_str();

        if (isFootnote) {
            ok = m_pAbiDocument->appendStrux(PTX_SectionFootnote, ppAtts2);
        } else {
            ok = m_pAbiDocument->appendStrux(PTX_SectionEndnote, ppAtts2);
        }
        UT_ASSERT(ok);

        m_pendingNoteAnchorInsertion = true;

    }
    else if (!strcmp(pName, "office:annotation-end"))
    {

        UT_DEBUGMSG(("found annotation-end... m_bAcceptingText:%d m_bPendingAnnotation:%d\n",
                     m_bAcceptingText, m_bPendingAnnotation ));
        
        const gchar* name = UT_getAttribute("office:name", ppAtts);
        if( name && m_openAnnotationNames.count(name) )
        {
            m_openAnnotationNames.erase(name);
            
            if (m_bPendingAnnotation) {
                // Don't crash on empty annotations
                _insertAnnotation();
                m_bPendingAnnotation = false;
            }

            _flush ();
            _popInlineFmt();
            m_pAbiDocument->appendFmt(m_vecInlineFmt);

            const PP_PropertyVector pa = {
                "name", name
            };
            m_pAbiDocument->appendObject(PTO_Annotation, pa);
            m_bAcceptingText = true;

            UT_DEBUGMSG(("found annotation-end... name:%s m_bAcceptingText:%d m_bPendingAnnotation:%d\n",
                         name, m_bAcceptingText, m_bPendingAnnotation ));
        }
    }
    else if (!strcmp(pName, "office:annotation"))
    {

        _flush();
        UT_DEBUGMSG(("open annotation... \n"));
        

        if (!m_bPendingAnnotation)
        {
            std::string id;
            m_sAnnotationAuthor.clear();
            m_sAnnotationDate.clear();
            m_sAnnotationName.clear();
            m_sAnnotationXMLID.clear();
            if( const gchar* s = UT_getAttribute("office:name", ppAtts))
            {
                m_sAnnotationName = s;
                m_openAnnotationNames.insert(s);
            }
            m_iAnnotation = m_pAbiDocument->getUID( UT_UniqueId::Annotation );
            id = UT_std_string_sprintf("%d", m_iAnnotation);
            std::string generatedID;
            const gchar* xmlid = UT_getAttribute("xml:id", ppAtts);
            if( !xmlid )
            {
                generatedID = UT_std_string_sprintf("anno%s", id.c_str() );
                xmlid = generatedID.c_str();
            }
            m_sAnnotationXMLID = xmlid;

            PP_PropertyVector ppAtts2 = {
                PT_ANNOTATION_NUMBER, id,
                PT_XMLID, xmlid
            };
            if( !m_sAnnotationName.empty() )
            {
                ppAtts2.push_back(PT_NAME_ATTRIBUTE_NAME);
                ppAtts2.push_back(m_sAnnotationName);
            }

            UT_DEBUGMSG(("open annotation... anno-id:%s xmlid:%s \n", id.c_str(), xmlid ));

            m_pAbiDocument->appendObject(PTO_Annotation, ppAtts2);
            m_bPendingAnnotation = true;
        }

    } else if (!strcmp(pName, "dc:creator")) {

        if (m_bPendingAnnotation) {
            m_bPendingAnnotationAuthor = true;
            m_bAcceptingText = false;
        }

    } else if (!strcmp(pName, "dc:date")) {

        if (m_bPendingAnnotation) {
            m_bPendingAnnotationDate = true;
            m_bAcceptingText = false;
        }

    } else if (!strcmp(pName, "text:soft-page-break")){
        // soft page breaks are NOT manual page breaks, we must ignore them,
        // see http://bugzilla.abisource.com/show_bug.cgi?id=13661
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
        m_tablesOfContentProps.addItem( new std::string(m_pCurrentTOCParser->getProps().utf8_str()) );
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
        m_pAbiDocument->appendFmt(m_vecInlineFmt);

    } else if (!strcmp(pName, "text:meta")) {

        _flush ();

        std::string xmlid = xmlidStackForTextMeta.back();
        xmlidStackForTextMeta.pop_back();

        const PP_PropertyVector ppAtts = {
            PT_XMLID, xmlid,
            // sanity check
            "this-is-an-rdf-anchor", "yes",
            PT_RDF_END, "yes"
        };
        m_pAbiDocument->appendObject( PTO_RDFAnchor, ppAtts );

    } else if (!strcmp(pName, "text:a")) {

        _flush ();
        m_pAbiDocument->appendObject(PTO_Hyperlink, PP_NOPROPS);

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
        
        UT_VECTOR_PURGEALL(std::string*, m_tablesOfContentProps);
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
        
        // We were inside a <draw:text-box> element.
	if(m_bPageReferencePending || m_bPendingTextbox)
	{
	    m_bPageReferencePending = false;
	}
	else
	{
	    rAction.popState();
	}
        
    } else if (!strcmp(pName, "table:table-cell")) {
        UT_ASSERT(m_elementParsingLevel == 1);
        
        // We were inside a <table:table-cell> element.
        rAction.popState();
        
    } else if (!strcmp(pName, "text:note-citation")) {  
        m_bAcceptingText = true;
    } else if (!strcmp(pName, "text:note-body")) {
        UT_DebugOnly<bool> ok = false;
        const gchar* pNoteClass;
        
        pNoteClass = m_rElementStack.getStartTag(1)->getAttributeValue("text:note-class");
        UT_ASSERT_HARMLESS(pNoteClass != NULL);
        
        if (pNoteClass && !strcmp(pNoteClass, "footnote")) {
            ok = m_pAbiDocument->appendStrux(PTX_EndFootnote, PP_NOPROPS);
        } else if (pNoteClass && !strcmp(pNoteClass, "endnote")) {
            ok = m_pAbiDocument->appendStrux(PTX_EndEndnote, PP_NOPROPS);
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
    else if (!strcmp(pName, "office:annotation"))
    {

        UT_DEBUGMSG(("close annotation... name:%s m_bAcceptingText:%d m_bPendingAnnotation:%d\n",
                     m_sAnnotationName.c_str(),
                     m_bAcceptingText,
                     m_bPendingAnnotation ));
        
        if (m_bPendingAnnotation) {
            // Don't crash on empty annotations
            _insertAnnotation();
            m_bPendingAnnotation = false;
        }

#if 0
        const gchar* pPropsArray[5] = { NULL, NULL, NULL, NULL, NULL };
	std::string id = UT_std_string_sprintf("%d", m_iAnnotation);
        UT_DEBUGMSG(("closing tag for id:%s\n", id.c_str() ));
        pPropsArray[0] = "annotation-id";
        pPropsArray[1] = id.c_str();
#endif
        
        m_pAbiDocument->appendStrux(PTX_EndAnnotation, PP_NOPROPS );
        //
        // MIQ: If there is no annotation name or there is no matching annotation-end
        // XML element then we assume it is a single point in document range.
        //
        if( m_sAnnotationName.empty()
            || !m_rAbiData.m_rangedAnnotationNames.count(m_sAnnotationName) )
        {
//            m_pAbiDocument->appendStrux(PTX_EndAnnotation, PP_NOPROPS);
            m_pAbiDocument->appendObject(PTO_Annotation, PP_NOPROPS);
        }
        else
        {
	    UT_DebugOnly<bool> ok = false;
//            _pushInlineFmt(ppStyAttr);
            ok = m_pAbiDocument->appendFmt(m_vecInlineFmt);
            UT_ASSERT(ok);
        }
        m_bAcceptingText = true;


        // m_pAbiDocument->appendStrux(PTX_EndAnnotation, PP_NOPROPS);
        // m_pAbiDocument->appendObject(PTO_Annotation, PP_NOPROPS);
        // m_bAcceptingText = true;
        
    } else if (!strcmp(pName, "dc:creator")) {

        if (m_bPendingAnnotationAuthor) {
            m_bPendingAnnotationAuthor = false;
            m_bAcceptingText = true;
        }

    } else if (!strcmp(pName, "dc:date")) {

        if (m_bPendingAnnotationDate) {
            m_bPendingAnnotationDate = false;
            m_bAcceptingText = true;
        }
    }
    else if (!strcmp(pName, "draw:frame")) {
      m_bPageReferencePending = false;
      m_bAcceptingText = true;
      if(m_bPendingTextbox)
      {
	  m_bPendingTextbox = false;
	  _flush();
	  PT_DocPosition pos2 = 0;
	  m_pAbiDocument->getBounds(true,pos2);
	  UT_ByteBuf * pBuf = new UT_ByteBuf(1024);
	  pf_Frag * pfLast = m_pAbiDocument->getLastFrag();
	  pf_Frag * pfFirst = pfLast;
	  while(pfFirst->getPrev())
	  {
	      pfFirst = pfFirst->getPrev();
	  }
	  PT_DocPosition pos1 = pfFirst->getPos();
	  IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(m_pAbiDocument);
	  PD_DocumentRange docRange(m_pAbiDocument, pos1,pos2);
	  //
	  // Copy the Textbox content to RTF and store for later insertion
	  //
	  pExpRtf->copyToBuffer(&docRange,pBuf);
	  delete pExpRtf;
	  m_pAbiDocument->addPageReferencedTextbox(*pBuf,m_iPageNum, m_dXpos,m_dYpos,m_sProps.c_str());
	  delete pBuf;
	  pf_Frag * pfNext = pfFirst; 
	  //
	  // Remove this textbox content from the PT
	  //
	  while(pfFirst)
	  {
	      pfNext = pfFirst->getNext();
	      m_pAbiDocument->deleteFragNoUpdate(pfFirst);
	      pfFirst = pfNext;
	  }
	  m_bAcceptingText = false;
      }
    }
    
    m_elementParsingLevel--;
}

#if 0 // not used yet. Suppress the warning.
/**
 * 6.1.2 of the ODF spec "What space Characters" lists these as
 * characters to normalize to a SPACE char in certain cases.
 * 
 * HORIZONTAL TABULATION (U+0009)
 * CARRIAGE RETURN (U+000D)
 * LINE FEED (U+000A)
 * SPACE (U+0020)
 *
 * UT_UCS4_isspace() uses whitespace_table which itself does not consider
 * U+000A as whitespace.
 */
static bool ODi_UCS4_whitespace( UT_UCS4Char c )
{
    return c == 0x000A || UT_UCS4_isspace(c);
}
#endif

/**
 * It seems from "6.1.2 White Space Characters" of the spec [2], in
 * particular page 120 of [3], that internal whitespace is to be
 * folded into space characters.
 *
 * Prior to folding anything in 
 *   Z = { U+0009, U+000D, U+000A )
 * Is first replaced with 
 *   SPACE = U+0020.
 */
static UT_UCS4String ODi_textp_fold_whitespace( const gchar* pBuffer, int length )
{
    bool strip_whitespace = false;
	UT_UCS4String ret( pBuffer, length, strip_whitespace );
    int len = ret.length();

    for( int i = 0; i < len; i++ )
    {
        UT_UCS4Char c = ret[i];
        if( c < 0x000E )
        {
            if( c == 0x000A || c == 0x0009 || c == 0x000D )
            {
                ret[i] = UCS_SPACE;
            }
        }
    }
    
    return ret;
}

/**
 * A stream of 2+ SPACE to be replaced with a single SPACE.
 */
static UT_UCS4String ODi_textp_compact_two_or_more_spaces( const UT_UCS4String& s )
{
    int len = s.length();
    UT_UCS4String ret;
    ret.reserve( len+1 );

    bool lastWasSpace = false;
    for( int i = 0; i < len; i++ )
    {
        UT_UCS4Char c = s[i];
        if( c != UCS_SPACE )
        {
            ret += c;
            lastWasSpace = false;
            continue;
        }
        
        if( lastWasSpace )
            continue;

        ret += c;
        lastWasSpace = true;
    }

    return ret;
}


/**
 * Trim leading whitespace completely
 */
static UT_UCS4String ODi_textp_trim_whitespace_leading( const UT_UCS4String& s )
{
    // UT_UCS4Char ucs = s[0];
    // UT_uint32 i = 0;
    // while(ucs != 0 && UT_UCS4_isspace(ucs) && (i<sUCS.size()))
    // {
    //     i++;
    //     ucs = s[i];
    // }

    const UT_UCS4Char* iter = std::find_if(
        s.begin(), s.end(),
        std::bind2nd( std::not_equal_to<UT_UCS4Char>(),
                      UCS_SPACE ));

    UT_UCS4String ret = s.substr( iter );
    return ret;
}



/**
 * 
 */
void ODi_TextContent_ListenerState::charData (
                            const gchar* pBuffer, int length)
{
    if (!pBuffer || !length)
        return; // nothing to do

    if (m_bAcceptingText) 
    {
#ifdef DEBUG
        {
            UT_DEBUGMSG(("charData() cw:%d length:%d\n", m_bContentWritten, length ));
            UT_UCS4String t;
            t += UT_UCS4String (pBuffer, length, false);
            UT_DEBUGMSG(("charData() pBuffer:%s\n", t.utf8_str() ));
        }
#endif

        UT_UCS4String s = ODi_textp_fold_whitespace( pBuffer, length );
        s = ODi_textp_compact_two_or_more_spaces( s );
        if(!m_bContentWritten)
        {
            s = ODi_textp_trim_whitespace_leading( s );
        }
        m_charData += s;

        UT_DEBUGMSG(("charData() content written s:%s\n", s.utf8_str() ));
    } 
    else if (m_bPendingAnnotationAuthor) 
    {
        m_sAnnotationAuthor = std::string(pBuffer, length);
    } 
    else if (m_bPendingAnnotationDate) 
    {
        m_sAnnotationDate = std::string(pBuffer, length);
    }
    else if (m_bPendingNoteCitation) {  
        m_noteCitation = std::string(pBuffer, length);
    } 
}


/**
 * 
 */
void ODi_TextContent_ListenerState::_insertBookmark (const gchar* pName,
                                                     const gchar* pType,
                                                     const gchar* xmlid )
{
    UT_return_if_fail(pName && pType);

    PP_PropertyVector pPropsArray = {
        "name", pName,
        "type", pType
    };
    if( xmlid && strlen(xmlid) )
    {
        pPropsArray.push_back(PT_XMLID);
        pPropsArray.push_back(xmlid);
    }
    m_pAbiDocument->appendObject (PTO_Bookmark, pPropsArray);
}


/**
 * 
 */
void ODi_TextContent_ListenerState::_flush ()
{
    if (m_charData.size () > 0 && m_bAcceptingText)
    {
        m_pAbiDocument->appendSpan (m_charData.ucs4_str(), m_charData.size ());
        m_charData.clear ();
        m_bContentWritten = true;
    } 
}


/**
 *
 */
bool ODi_TextContent_ListenerState::_pushInlineFmt(const PP_PropertyVector & atts)
{
    UT_uint32 start = m_vecInlineFmt.size() + 1;

    m_vecInlineFmt.insert(m_vecInlineFmt.end(), atts.begin(), atts.end());

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

    m_vecInlineFmt.erase(m_vecInlineFmt.begin() + (start - 1),
                           m_vecInlineFmt.end());
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
                                         const std::string* pMasterPageName) {
    
    if (m_inAbiSection && !m_bPendingSection)
        return;
    
    const ODi_StartTag* pStartTag;
    std::string props = "";
    
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
    
    if (!props.empty()){
        gchar* propsCopy = g_strdup(props.c_str());
        const gchar** propsArray = UT_splitPropsToArray(propsCopy);
        const gchar* pColumns = UT_getAttribute("columns", propsArray);
        
        if (pColumns != NULL){
            m_columnsCount = atoi(pColumns);
            m_columnIndex = 1;
        } else{
            m_columnsCount = 1;
            m_columnIndex = 1;
        }
        
        g_free(propsArray);
    }
    _openAbiSection(props, pMasterPageName);
}


/**
 * Tries to open an abi <section> given its properties.
 * 
 * @param pProps The properties of the abi <section> to be opened.
 */
void ODi_TextContent_ListenerState::_openAbiSection(
                                         const std::string& rProps,
                                         const std::string* pMasterPageName) {

    std::string masterPageProps;
    std::string dataID;
    bool hasLeftPageMargin = false;
    bool hasRightPageMargin = false;

    const ODi_Style_MasterPage* pMasterPageStyle = NULL;

    if (pMasterPageName != NULL && !pMasterPageName->empty()) {
        
        pMasterPageStyle = m_pStyles->getMasterPageStyle(pMasterPageName->c_str());
        
        if (pMasterPageStyle && pMasterPageStyle->getPageLayout()) {
            masterPageProps = pMasterPageStyle->getSectionProps();
            dataID = pMasterPageStyle->getSectionDataID();
            if (pMasterPageStyle->getPageLayout()->getMarginLeft().size()) {
                m_currentPageMarginLeft = pMasterPageStyle->getPageLayout()->getMarginLeft();
                hasLeftPageMargin = true;
            }
            if (pMasterPageStyle->getPageLayout()->getMarginRight().size()) {
                m_currentPageMarginRight = pMasterPageStyle->getPageLayout()->getMarginRight();
                hasRightPageMargin = true;
            }
            UT_ASSERT(!masterPageProps.empty());
        }
        //
        // Page size is defined from the first section properties
        //
        if(!m_openedFirstAbiSection)
        {
            std::string sProp, sWidth, sHeight, sOri;
            bool bValid = true;
	    
            sProp="page-width";
            sWidth = UT_std_string_getPropVal(masterPageProps,sProp);
            if(sWidth.size()==0)
                bValid = false;

            sProp="page-height";
            sHeight = UT_std_string_getPropVal(masterPageProps,sProp);
            if(sHeight.size()==0)
                bValid = false;

            sProp="page-orientation";
            sOri = UT_std_string_getPropVal(masterPageProps,sProp);
            if(sOri.size()==0)
	            bValid = false;

            if (bValid) {
                const PP_PropertyVector atts = {
                    "pagetype", "Custom",
                    "orientation", sOri,
                    "width", sWidth,
                    "height", sHeight,
                    "units", UT_dimensionName(UT_determineDimension(sWidth.c_str())),
                    "page-scale","1.0"
                };
                m_pAbiDocument->setPageSizeFromFile(atts);
            }
        }
        m_openedFirstAbiSection = true;
    }

	if (!m_openedFirstAbiSection) {
        // We haven't defined any page properties yet. It's done on the
        // first abi section.
        
        // For now we just use the Standard page master. AbiWord doesn't support
        // multiple page formats anyway.
        
        pMasterPageStyle = m_pStyles->getMasterPageStyle("Standard");

        if (pMasterPageStyle) {
            masterPageProps = pMasterPageStyle->getSectionProps();
            dataID = pMasterPageStyle->getSectionDataID();
            if (pMasterPageStyle->getPageLayout() && pMasterPageStyle->getPageLayout()->getMarginLeft().size()){
                m_currentPageMarginLeft = pMasterPageStyle->getPageLayout()->getMarginLeft();
                hasLeftPageMargin = true;
            }
            if (pMasterPageStyle->getPageLayout() && pMasterPageStyle->getPageLayout()->getMarginRight().size()) {
                m_currentPageMarginRight = pMasterPageStyle->getPageLayout()->getMarginRight();
                hasRightPageMargin = true;
            }
        }

        m_openedFirstAbiSection = true;
    }
    
    // AbiWord always needs to have the page-margin-left and page-margin-right properties
    // set on a section, otherwise AbiWord will reset those properties to their default
    // values. This is because AbiWord can have multiple left and right margins on 1 page,
    // something OpenOffice.org/OpenDocument can't do. Left and right page margins in 
    // OpenDocument are only set once per page layout.
    // This means that when we encounter a new OpenDocument section without an accompanying
    // page layout style (a section that thus causes no left or right page margin changes), 
    // we will manually need to add the 'current' left and right page margin to AbiWord's
    // section properties to achieve the same effect.
    // Bug 10884 has an example of this situation.
    if (!hasLeftPageMargin && m_currentPageMarginLeft.size()) {
       if (!masterPageProps.empty())
            masterPageProps += "; ";
        masterPageProps += "page-margin-left:" + m_currentPageMarginLeft;
    }
    if (!hasRightPageMargin && m_currentPageMarginRight.size()) {
        if (!masterPageProps.empty())
            masterPageProps += "; ";
        masterPageProps += "page-margin-right:" + m_currentPageMarginRight;
    }

    // The AbiWord section properties are taken part from the OpenDocument 
    // page layout (from the master page style) and part from the OpenDocument
    // section properties.
    
    // TODO: What happens if there are duplicated properties on the page layout
    // and on the section?

    std::string allProps = masterPageProps;
    if (!allProps.empty() && !rProps.empty()) {
        allProps += "; ";
    }
    allProps += rProps;

    PP_PropertyVector atts = {
        "props", allProps
    };
    if (pMasterPageStyle != NULL) {
        // The standard master page may have headers/footers as well.

        if (!pMasterPageStyle->getAWEvenHeaderSectionID().empty()) {
            atts.push_back("header-even");
            atts.push_back(pMasterPageStyle->getAWEvenHeaderSectionID());
        }

        if (!pMasterPageStyle->getAWHeaderSectionID().empty()) {
            atts.push_back("header");
            atts.push_back(pMasterPageStyle->getAWHeaderSectionID());
        }

        if (!pMasterPageStyle->getAWEvenFooterSectionID().empty()) {
            atts.push_back("footer-even");
            atts.push_back(pMasterPageStyle->getAWEvenFooterSectionID());
        }

        if (!pMasterPageStyle->getAWFooterSectionID().empty()) {
            atts.push_back("footer");
            atts.push_back(pMasterPageStyle->getAWFooterSectionID());
        }

        if (dataID.length()) {
            atts.push_back("strux-image-dataid");
            atts.push_back(dataID.c_str());
        }
    }

// Bug 12716 - this cause an stack overflow
// Reverting it seems to no cause bug 10627 to fail anymore
//    if(m_inAbiSection && !m_bOpenedBlock) {
//        _insureInBlock(NULL); //see Bug 10627 - hang on empty <section>
//    }

    m_pAbiDocument->appendStrux(PTX_Section, atts);
	m_bPendingSection = false;
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
void ODi_TextContent_ListenerState::_insureInBlock(const PP_PropertyVector & atts)
{
    if (m_bAcceptingText)
        return;

    _insureInSection();

    if (!m_bAcceptingText) {
        m_pAbiDocument->appendStrux(PTX_Block, atts);
        m_bOpenedBlock = true;
        m_bAcceptingText = true;
    }
}


/**
 * Process <text:p> and <text:h> startElement calls
 */
void ODi_TextContent_ListenerState::_startParagraphElement (const gchar* /*pName*/,
                                          const gchar** ppParagraphAtts,
															ODi_ListenerStateAction& /*rAction*/) 
{
        bool bIsListParagraph = m_bHeadingList ;
        const gchar* pStyleName;
        gchar listLevel[10];
        std::string props;
        const ODi_Style_Style* pStyle;
        m_bContentWritten = false;
        const gchar* xmlid = 0;

        xmlid = UT_getAttribute ("xml:id", ppParagraphAtts);
        
        if (!strcmp(m_rElementStack.getStartTag(0)->getName(), "text:list-item")) {
            // That's a list paragraph.
            bIsListParagraph = true;
        }

        pStyleName = UT_getAttribute ("text:style-name", ppParagraphAtts);
        if (pStyleName) {
            pStyle = m_pStyles->getParagraphStyle(pStyleName, m_bOnContentStream);

            if (!pStyle) {
                pStyle = m_pStyles->getTextStyle(pStyleName, m_bOnContentStream);
            }
            
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
                    m_pAbiDocument->appendStrux(PTX_Block, PP_NOPROPS);
                    m_pAbiDocument->appendSpan (&ucs, 1);
                    m_bOpenedBlock = true;
                    m_bContentWritten = false;
                }
            } else {
                _insureInSection();
                UT_UCSChar ucs;
                if ((m_columnIndex <= m_columnsCount)){
                    
                    if((pStyle == NULL) || (pStyle->getBreakBefore().empty()))
                    {
                        if ((m_columnIndex > 1)){
                            ucs = UCS_VTAB;
                            // Append an empty paragraph with this one char
                            m_pAbiDocument->appendStrux(PTX_Block, PP_NOPROPS);
                            m_pAbiDocument->appendSpan (&ucs, 1);
                        }
                    }
                    m_columnIndex++;
                }
                
                // Should we insert a break before this paragraph?
                if (pStyle != NULL && !pStyle->getBreakBefore().empty()) {
                    if (pStyle->getBreakBefore() == "page") {
                        ucs = UCS_FF;
                        // Append an empty paragraph with this one char
                        m_pAbiDocument->appendStrux(PTX_Block, PP_NOPROPS);
                        m_pAbiDocument->appendSpan (&ucs, 1);
                        m_bOpenedBlock = true;
                        m_bContentWritten = false;
                    } else if (pStyle->getBreakBefore() == "column") {
                        ucs = UCS_VTAB;
                        // Append an empty paragraph with this one char
                        m_pAbiDocument->appendStrux(PTX_Block, PP_NOPROPS);
                        m_pAbiDocument->appendSpan (&ucs, 1);
                        m_bOpenedBlock = true;
                        m_bContentWritten = false;
                    }
                }
            }

        }
        
	UT_DebugOnly<bool> ok;
        if (bIsListParagraph && !m_alreadyDefinedAbiParagraphForList) {
            ODi_ListLevelStyle* pListLevelStyle = NULL;
            
            m_alreadyDefinedAbiParagraphForList = true;
            if (m_pCurrentListStyle) {
                pListLevelStyle = m_pCurrentListStyle->getLevelStyle(m_listLevel);
            }

            sprintf(listLevel, "%u", m_listLevel);

            PP_PropertyVector ppAtts;
            ppAtts.push_back("level");
            ppAtts.push_back(listLevel);
            if (pListLevelStyle && pListLevelStyle->getAbiListID() && pListLevelStyle->getAbiListParentID())
	        {
	            if(m_listLevel < m_prevLevel)
		        {
		            m_pCurrentListStyle->redefine( m_pAbiDocument,m_prevLevel);
		        }
                m_prevLevel = m_listLevel;
		
                ppAtts.push_back("listid");
                ppAtts.push_back(*pListLevelStyle->getAbiListID());
            
                // Is this really necessary? Because we already have this info on
                // the <l> tag.
                ppAtts.push_back("parentid");
                ppAtts.push_back(*pListLevelStyle->getAbiListParentID());
                xxx_UT_DEBUGMSG(("Level |%s| Listid |%s| Parentid |%s| \n",ppAtts[i-5],ppAtts[i-3],ppAtts[i-1]));
            }

            if (pStyle!=NULL) {
                if (pStyle->isAutomatic()) {
                    // Automatic styles are not defined on the document, so, we
                    // just paste its properties.
                    pStyle->getAbiPropsAttrString(props);

                    // but we need to add the style forr the outline level
                    // see #13706
                    ppAtts.push_back("style");
                    ppAtts.push_back(m_headingStyles[listLevel]);
                } else {
                    // We refer to the style
                    ppAtts.push_back("style");
                    ppAtts.push_back(pStyle->getDisplayName());
                }
            }

            if (pListLevelStyle) {
                pListLevelStyle->getAbiProperties(props, pStyle);
                
                ppAtts.push_back("props");
                ppAtts.push_back(props);
            }
                
            ok = m_pAbiDocument->appendStrux(PTX_Block, ppAtts);
            UT_ASSERT(ok);
            m_bOpenedBlock = true;

            const PP_PropertyVector ppAtts2 = {
                "type", "list_label"
            };
            ok = m_pAbiDocument->appendObject(PTO_Field, ppAtts2);
            UT_ASSERT(ok);
            m_bContentWritten = true;
            
            // Inserts a tab character. AbiWord seems to need it in order to
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
               m_bContentWritten = true;

            PP_PropertyVector ppAtts;
            if (pStyle!=NULL) {
                if (pStyle->isAutomatic()) {
                    // Automatic styles are not defined on the document, so, we
                    // just paste its properties.
                    pStyle->getAbiPropsAttrString(props);
                    ppAtts.push_back("props");
                    ppAtts.push_back(props);
                } else {
                    // We refer to the style
                    ppAtts.push_back("style");
                    ppAtts.push_back(pStyle->getDisplayName());
                }
            }

            ok = m_pAbiDocument->appendFmt(ppAtts);
            UT_ASSERT(ok);

        } else {

            PP_PropertyVector ppAtts;
            if (pStyle != NULL) {
                if (pStyle->isAutomatic()) {
                    // Automatic styles are not defined on the document, so, we
                    // just paste its properties.
                    pStyle->getAbiPropsAttrString(props, FALSE);
                    ppAtts.push_back("props");
                    ppAtts.push_back(props);

                    if (pStyle->getParent() != NULL) {
                        ppAtts.push_back("style");
                        ppAtts.push_back(pStyle->getParent()->getDisplayName());
                    }
                } else {
                    // We refer to the style
                    ppAtts.push_back("style");
                    ppAtts.push_back(pStyle->getDisplayName());
                }
            }

            if( xmlid )
            {
                ppAtts.push_back(PT_XMLID);
                ppAtts.push_back(xmlid);
            }

            m_pAbiDocument->appendStrux(PTX_Block, ppAtts);
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

            PP_PropertyVector ppAtts = { "type", "", "", "" };
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
            ppAtts[3] = m_currentNoteId;

            ok = m_pAbiDocument->appendObject(PTO_Field, ppAtts);
	        //
	        // Now insert the tab after the anchor
	        //
	        UT_UCSChar ucs = UCS_TAB;
	        m_pAbiDocument->appendSpan (&ucs, 1);
	        m_bContentWritten = true;
            UT_ASSERT(ok);
        }
}


/**
 * 
 */
void ODi_TextContent_ListenerState::_endParagraphElement (
	const gchar* /*pName*/,
	ODi_ListenerStateAction& rAction) 
{
                                            
    const gchar* pStyleName;
    const ODi_Style_Style* pStyle;

    _flush ();
    m_bAcceptingText = false;
    
   
    pStyleName = m_rElementStack.getStartTag(0)->
                    getAttributeValue("text:style-name");
                    
    if (pStyleName) {
        pStyle = m_pStyles->getParagraphStyle(pStyleName, m_bOnContentStream);

        if (!pStyle) {
            pStyle = m_pStyles->getTextStyle(pStyleName, m_bOnContentStream);
        }
        
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
    std::string str;
    std::string props;
    std::string styleName;
    
    count = m_tablesOfContent.getItemCount();
    for (i=0; i<count; i++) {
        pTOCStrux = m_tablesOfContent[i];
        props = *(m_tablesOfContentProps[i]);
        
        for (j=1; j<5; j++) {
            str = UT_std_string_sprintf("%d", j);
            styleName = m_headingStyles[str];

            if (!styleName.empty()) {
                str = UT_std_string_sprintf("toc-source-style%d:%s", j,
                                      styleName.c_str());
                
                if (!props.empty()) {
                    props += "; ";
                }
                props += str;
            }
        }
        
	UT_DebugOnly<bool> ok;
        ok = m_pAbiDocument->changeStruxAttsNoUpdate(
                            pTOCStrux, "props",
                            props.c_str());
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
            m_pAbiDocument->appendStrux(PTX_Block, PP_NOPROPS);
            UT_UCSChar ucs = UCS_FF;
            m_pAbiDocument->appendSpan (&ucs, 1);
            m_bOpenedBlock = true;
            m_bContentWritten = false;
        } else if (m_pendingParagraphBreak == "column") {
            m_pAbiDocument->appendStrux(PTX_Block, PP_NOPROPS);
            UT_UCSChar ucs = UCS_VTAB;
            m_pAbiDocument->appendSpan (&ucs, 1);
            m_bOpenedBlock = true;
            m_bContentWritten = false;
        }
        
        m_pendingParagraphBreak.clear();
    }
}


/**
 * Inserts an <annotate> element into the document before inserting
 * a paragraph or list item.
 */
void ODi_TextContent_ListenerState::_insertAnnotation() {

    UT_return_if_fail(m_bPendingAnnotation);

    std::string id = UT_std_string_sprintf("%d", m_iAnnotation);
    std::string props;

    UT_DEBUGMSG(("_insertAnnotation() id:%s\n", id.c_str() ));

    if (!m_sAnnotationAuthor.empty()) {
        props = "annotation-author: ";
        props += m_sAnnotationAuthor;
        m_sAnnotationAuthor.clear();
    }

    if (!m_sAnnotationDate.empty()) {
        if (!props.empty()) {
            props += "; ";
        }
        props += "annotation-date: ";
        props += ODc_reorderDate(m_sAnnotationDate, true);
        m_sAnnotationDate.clear();
    }

    //
    // MIQ: The annotation-title might have been saved into RDF by
    // abiword or others. We thus try to find it again using SPARQL
    // and the annotation's xml:id attribute.
    //
    if( !m_sAnnotationXMLID.empty() )
    {
        std::string xmlid = m_sAnnotationXMLID;
        std::stringstream sparql;
        sparql << "prefix rdf:   <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
               << "prefix foaf:  <http://xmlns.com/foaf/0.1/>  \n"
               << "prefix pkg:   <http://docs.oasis-open.org/opendocument/meta/package/common#>  \n"
               << "prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#> \n"
               << "prefix dc:    <http://purl.org/dc/elements/1.1/> \n"
               << " \n"
               << "select ?s ?title ?rdflink  \n"
               << "where {  \n"
               << " ?s dc:title  ?title .  \n"
               << " ?s pkg:idref ?rdflink .  \n"
               << "   filter( str(?rdflink) = \"" << xmlid << "\" ) \n"
               << "} \n";
        PD_Document* doc = m_pAbiDocument;
        PD_DocumentRDFHandle rdf = doc->getDocumentRDF();
        PD_RDFQuery q( rdf, rdf );
        PD_ResultBindings_t bindings = q.executeQuery(sparql.str());
        UT_DEBUGMSG(("bindings.sz:%lu\n", (long unsigned)bindings.size() ));
        for( PD_ResultBindings_t::iterator iter = bindings.begin(); iter != bindings.end(); ++iter )
        {
            std::map< std::string, std::string > d = *iter;
            std::string title = d["title"];
            if (!props.empty()) {
                props += "; ";
            }
            props += "annotation-title: ";
            props += title;

            // DEBUG
            UT_DEBUGMSG(("title:%s\n", title.c_str() ));
#if DEBUG
            for( std::map< std::string, std::string >::iterator x = d.begin(); x != d.end(); ++x )
            {
                UT_DEBUGMSG(("first:%s sec:%s\n", x->first.c_str(), x->second.c_str() ));
            }
#endif
        }

    }

    const PP_PropertyVector pPropsArray = {
        "annotation-id", id,
        PT_PROPS_ATTRIBUTE_NAME, props
    };

    m_pAbiDocument->appendStrux(PTX_SectionAnnotation, pPropsArray);
    m_bPendingAnnotation = false;
}
