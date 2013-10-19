/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
* Copyright (C) 2011 Volodymyr Rudyj <vladimir.rudoy@gmail.com>
* Copyright (C) 2007, 2009 Hubert Figuiere
* Copyright (C) 2003-2005 Mark Gilbert <mg_abimail@yahoo.com>
* Copyright (C) 2002, 2004 Francis James Franklin <fjf@alinameridon.com>
* Copyright (C) 2001-2002 AbiSource, Inc.
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

#include "ie_exp_HTML_Listener.h"
#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML.h"

IE_Exp_HTML_Listener::IE_Exp_HTML_Listener(PD_Document *pDocument, 
                                           IE_Exp_HTML_DataExporter* pDataExporter,
                                           IE_Exp_HTML_StyleTree    *pStyleTree,
                                           IE_Exp_HTML_NavigationHelper *pNavigationHelper,
                                           IE_Exp_HTML_ListenerImpl* pListenerImpl,
                                           const UT_UTF8String &filename) :
m_bFirstWrite(true),										   
m_bInSpan(false),
m_bInBlock(false),
m_bInBookmark(false),
m_bInHyperlink(false),
m_bInSection(false),
m_bInAnnotation(false),
m_bInAnnotationSection(false),
m_bInEndnote(false),
m_bInFootnote(false),
m_bInHeading(false),
 m_bInTextbox(false),
m_bSkipSection(false),
m_pCurrentField(0),
m_currentFieldType(""),
m_bookmarkName(""),
m_apiLastSpan(0),
m_iInTable(0),
m_iInRow(0),
m_iInCell(0),
m_bFirstRow(true),
m_pDocument(pDocument),
m_pCurrentImpl(pListenerImpl),
m_tableHelper(pDocument),
m_pDataExporter(pDataExporter),
m_bEmbedCss(false),
m_bEmbedImages(false),
m_bRenderMathToPng(false),
m_bScaleUnits(false),
m_bAbsUnits(false),
m_filename(filename),
m_pStyleTree(pStyleTree),
m_pNavigationHelper(pNavigationHelper),
m_iHeadingCount(0),
m_dPageWidthInches(0.0),
m_dSecLeftMarginInches(0.0),
m_dSecRightMarginInches(0.0),
m_dSecTopMarginInches(0.0),
m_dSecBottomMarginInches(0.0),
m_dCellWidthInches(0.0),
m_bHasMathMl(false)
{

}

bool IE_Exp_HTML_Listener::populate(fl_ContainerLayout* /*sfh*/, const PX_ChangeRecord* pcr)
{
    if (m_bSkipSection)
        return true;
    
    switch (pcr->getType())
    {

    case PX_ChangeRecord::PXT_InsertSpan:
    {
        const PX_ChangeRecord_Span * pcrs =
                static_cast<const PX_ChangeRecord_Span *> (pcr);

        if (pcrs->getField() != m_pCurrentField)
        {
            _closeField();
        }

        PT_AttrPropIndex api = pcr->getIndexAP();
        

        PT_BufIndex bi = pcrs->getBufIndex();

        UT_UTF8String utf8String(m_pDocument->getPointer(bi),
                                 pcrs->getLength());
        
        if (m_bInEndnote)
        {
            m_endnotes.push_back(utf8String);
            m_bInEndnote = false;
        } else
        if (m_bInFootnote)
        {
            m_footnotes.push_back(utf8String);
            m_bInFootnote = false;
        } else
        if (m_bInAnnotationSection)
        { 
            m_annotationContents.push_back(utf8String);
            m_bInAnnotationSection = false;
        }else
        {
            _openSpan(api);
            _outputData(m_pDocument->getPointer(bi), pcrs->getLength());
        }
    }
        break;

    case PX_ChangeRecord::PXT_InsertObject:
    {
        const PX_ChangeRecord_Object * pcro =
                static_cast<const PX_ChangeRecord_Object *> (pcr);

        PT_AttrPropIndex api = pcr->getIndexAP();
        switch (pcro->getObjectType())
        {
        case PTO_Image:
        {
            _closeSpan();
            _closeField();
            _insertImage(api);
            return true;
        }

        case PTO_Field:
        {
            _closeSpan();
            _closeField();
            _openField(pcro, api);
            return true;
        }

        case PTO_Math:
        {
            _closeSpan();
            _closeField();
            if (m_bRenderMathToPng)
            {
                _insertEmbeddedImage(api);
            } else
            {
                _insertMath(api);
            }
            return true;
        }

        case PTO_Embed:
        {
            //TODO: we may want to save the actual chart xml one day,
            // but saving the image will do for now
            _closeSpan();
            _closeField();
            _insertEmbeddedImage(api);
            return true;
        }

        case PTO_Bookmark:
        {
            _closeSpan();
            _closeField();

            const PP_AttrProp* pAP = NULL;
            m_pDocument->getAttrProp(api, &pAP);
            const gchar* pValue = NULL;

            if (pAP && pAP->getAttribute("type", pValue) && pValue 
				&& (strcmp(pValue, "start") == 0))
            {
                _openBookmark(api);
            }
            else
            {
                _closeBookmark();
            }

            return true;
        }

        case PTO_Hyperlink:
        {
            _closeSpan();
            _closeField();
            const PP_AttrProp* pAP = NULL;
            m_pDocument->getAttrProp(api, &pAP);
            const gchar* pValue = NULL;

            if (pAP && pAP->getAttribute("xlink:href", pValue) && pValue)
            {
                _openHyperlink(api);
            }
            else
            {
                _closeHyperlink();
            }

            return true;
        }

        case PTO_Annotation:
        {
            _closeSpan();
            _closeField();
            
            if (m_bInAnnotation)
            {
                _closeAnnotation();
            } else
            {
                _openAnnotation(api);
            }
            
            return true;
        }

        case PTO_RDFAnchor:
        {
            UT_DEBUGMSG(("populate() PTO_RDFAnchor\n"));
            _closeSpan();
            _closeField();
            return true;
        }

        default:
            UT_ASSERT_HARMLESS(UT_TODO);
            return true;
        }
    }

    case PX_ChangeRecord::PXT_InsertFmtMark:
        // fmt marks are temporary placeholders for props and
        // attributes and should not be saved
        return true;

    default:
        UT_ASSERT_HARMLESS(UT_TODO);
        return true;
    }


    return true;
}

bool IE_Exp_HTML_Listener::populateStrux(pf_Frag_Strux* sdh, 
	const PX_ChangeRecord* pcr, fl_ContainerLayout** psfh)
{
    
    UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, 
						  false);
    bool returnVal = true;

    const PX_ChangeRecord_Strux * pcrx =
            static_cast<const PX_ChangeRecord_Strux *> (pcr);

    // // TESTING
    // {
    //     UT_DEBUGMSG(("TESTING AAA ... testing if we have a psfh\n"));
    //     if( *psfh )
    //     {
    //         UT_DEBUGMSG(("TESTING AAA ... have psfh\n"));
    //         PT_AttrPropIndex api = m_pDocument->getAPIFromSDH( psfh );
    //         const PP_AttrProp * AP = NULL;
    //         m_pDocument->getAttrProp(api,&AP);
    //         if( AP )
    //         {
    //             const gchar * v = NULL;
    //             if(AP->getAttribute("xml:id", v))
    //                 UT_DEBUGMSG(("TESTING AAA ... xmlid:%s\n",v));
    //             if(AP->getAttribute("props", v))
    //                 UT_DEBUGMSG(("TESTING AAA ... props:%s\n",v));
    //         }
    //         api = pcr->getIndexAP();
    //         m_pDocument->getAttrProp(api,&AP);
    //         if( AP )
    //         {
    //             const gchar * v = NULL;
    //             if(AP->getAttribute("xml:id", v))
    //                 UT_DEBUGMSG(("TESTING AAA2 ... xmlid:%s\n",v));
    //             if(AP->getAttribute("props", v))
    //                 UT_DEBUGMSG(("TESTING AAA2 ... props:%s\n",v));
    //         }
    //     }
    // }


    *psfh = 0; // we don't need it.

    PT_AttrPropIndex api = pcr->getIndexAP();
    //const gchar* image_name =
    //    _getObjectKey(api, static_cast<const gchar*>(PT_STRUX_IMAGE_DATAID));


    if (m_bFirstWrite)
        _beginOfDocument(api);
    switch (pcrx->getStruxType())
    {
    case PTX_SectionHdrFtr:
        m_bSkipSection = true;
        break;
    case PTX_Section:
    {
        m_bSkipSection = false;
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _closeSection();
        _openSection(api);
    }
        break;

    case PTX_SectionTable:
    {
        m_bSkipSection = false;
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
		m_tableHelper.OpenTable(sdh, api);
        _openTable(api);
        m_bFirstRow = true;
    }
        break;

    case PTX_SectionCell:
    {
        m_bSkipSection = false;
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();

        m_tableHelper.OpenCell(api);
        if (m_bFirstRow || m_tableHelper.isNewRow())
        {
            if (m_bFirstRow)
            {
                m_bFirstRow = false;
            }
            else
            {
                _closeRow();
            }
            _openRow(api);
        }

        _openCell(api);
    }
        break;

    case PTX_SectionFootnote:
    {
        m_bSkipSection = false;
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _openFootnote(api);
    }
        break;

    case PTX_SectionEndnote:
    {
        m_bSkipSection = false;
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _openEndnote(api);
    }
        break;

    case PTX_SectionAnnotation:
    {
        m_bSkipSection = false;        
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        // _openAnnotation(api);
        m_bInAnnotationSection = true;
        _handleAnnotationData(api);
    }
        break;

    case PTX_SectionTOC:
    {
        m_bSkipSection = false;
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _insertTOC(api);
    }
        break;

    case PTX_SectionMarginnote:
    {
        m_bSkipSection = false;
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        //_openTag("margin","",true,pcr->getIndexAP(),pcr->getXID());
    }
        break;

    case PTX_SectionFrame:
    {
        m_bSkipSection = false;
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _openFrame(api, pcr);
    }
        break;

    case PTX_EndTable:
    {
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _closeRow();
        _closeTable();
        m_tableHelper.CloseTable();
    }
        break;

    case PTX_EndCell:
    {
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _closeCell();

        m_tableHelper.CloseCell();
    }
        break;

    case PTX_EndFootnote:
    {
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _closeFootnote();
    }
        break;

    case PTX_EndEndnote:
    {
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _closeEndnote();
    }
        break;

    case PTX_EndAnnotation:
    {
        _closeSpan();
        _closeField();
       /* _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();*/
        // _closeAnnotation();
        m_bInAnnotationSection = false;
    }
        break;

    case PTX_EndTOC:
    {
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
    }
        break;

    case PTX_EndMarginnote:
    {
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();

    }
        break;

    case PTX_EndFrame:
    {
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _closeFrame();
    }
        break;

    case PTX_Block:
    {
        if (m_bInEndnote || m_bInFootnote || m_bInAnnotationSection)
        {
            break;
        }
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();


        const gchar *szBlockStyle = _getObjectKey(api, PT_STYLE_ATTRIBUTE_NAME);
        const gchar *szListId = _getObjectKey(api, PT_LISTID_ATTRIBUTE_NAME);
        bool zeroListId = true;

        if (szListId != NULL)
        {
            UT_DEBUGMSG(("List found!!!!\n"));
            zeroListId = !g_ascii_strcasecmp(szListId, "0");
        }
        
        if (zeroListId)
        {
            _closeLists();
        } else
        {
            _openList(api);
            break;
        }

        if (szBlockStyle != NULL)
        {                
            if ((g_ascii_strcasecmp(static_cast<const gchar *> 
									(szBlockStyle), "Heading 1") == 0) ||
                (g_ascii_strcasecmp(static_cast<const gchar *> 
									(szBlockStyle), "Numbered Heading 1") == 0))
            {
                _openHeading(api, 1);
            }
            else
                if ((g_ascii_strcasecmp(static_cast<const gchar *> 
										(szBlockStyle), "Heading 2") == 0) ||
                    (g_ascii_strcasecmp(static_cast<const gchar *> 
										(szBlockStyle), 
										"Numbered Heading 2") == 0))
            {
                _openHeading(api, 2);
            }
            else
                if ((g_ascii_strcasecmp(static_cast<const gchar *> 
										(szBlockStyle), "Heading 3") == 0) ||
                    (g_ascii_strcasecmp(static_cast<const gchar *> 
										(szBlockStyle), 
										"Numbered Heading 3") == 0))
            {
                _openHeading(api, 3);
            }
            else
                if ((g_ascii_strcasecmp(static_cast<const gchar *> 
										(szBlockStyle), "Heading 4") == 0) ||
                    (g_ascii_strcasecmp(static_cast<const gchar *> 
										(szBlockStyle), 
										"Numbered Heading 4") == 0))
            {
                _openHeading(api, 4);
            }
             else _openBlock(api);
        } else _openBlock(api);
        
            
    }
        break;

    default:
        UT_ASSERT_HARMLESS(UT_TODO);
        returnVal = true;
    }

    return returnVal;
}

bool IE_Exp_HTML_Listener::insertStrux(fl_ContainerLayout* /*sfh*/,
                                       const PX_ChangeRecord * /*pcr*/,
                                       pf_Frag_Strux* /*sdh*/,
                                       PL_ListenerId /*lid*/,
                                       void (* /*pfnBindHandles*/) 
									   (pf_Frag_Strux* sdhNew,
                                       PL_ListenerId lid,
                                       fl_ContainerLayout* sfhNew))
{
    return true;
}

bool IE_Exp_HTML_Listener::change(fl_ContainerLayout* /*sfh*/, 
								  const PX_ChangeRecord* /*pcr*/)
{
    return true;
}

bool IE_Exp_HTML_Listener::signal(UT_uint32 /*iSignal*/)
{
    return true;
}

bool IE_Exp_HTML_Listener::_beginOfDocument(const PT_AttrPropIndex& api)
{
	m_bFirstWrite = false;
    _insertDTD();
    _openDocument();
    _openHead();
    _insertTitle();
    _insertMeta();
    _makeStylesheet(api);
    if (m_bEmbedCss)
    {
        _insertStyle();
    } else
    {
        _insertLinkToStyle();
    }
    _insertLinks();
    _closeHead();
    _openBody();
    
    return true;
}

bool IE_Exp_HTML_Listener::endOfDocument()
{
    _closeSpan();
    _closeField();
    _closeBookmark();
    _closeHyperlink();
    _closeBlock();
    _closeHeading();
    _closeLists();
    _closeCell();
    _closeTable();
    _closeSection();
    
    _insertFootnotes();
    _insertAnnotations();
    _closeBody();
    _closeDocument();
    
    return true;
}

void IE_Exp_HTML_Listener::_outputData(const UT_UCSChar* pData, 
									   UT_uint32 length)
{
    UT_UTF8String sBuf;
    const UT_UCSChar* p;

    UT_ASSERT(sizeof (UT_Byte) == sizeof (char));
    sBuf.reserve(length);
	UT_uint32 spaceCount = 0;
	
    for (p = pData; (p < pData + length); /**/)
    {
		if ((*p != ' ') && (spaceCount > 0))
		{
			sBuf += ' ';
			spaceCount--;
			while (spaceCount > 0)
			{
				sBuf += "&nbsp;";
				spaceCount--;
			}
		}
        switch (*p)
        {
        case '<':
            sBuf += "&lt;";
            p++;
            break;

        case '>':
            sBuf += "&gt;";
            p++;
            break;

        case '&':
            sBuf += "&amp;";
            p++;
            break;

        case ' ':
			spaceCount++;
            p++;
            break;

        case UCS_LF:
            sBuf.clear();
            p++;
            break;

        case UCS_VTAB:
            m_pCurrentImpl->insertText(sBuf);
            // m_pCurrentImpl->insertColumnBreak();
            sBuf.clear();
            p++;
            break;

        case UCS_TAB:
            m_pCurrentImpl->insertText(sBuf);
            //m_pCurrentImpl->insertTabChar();
            sBuf.clear();
            p++;
            break;

        case UCS_FF: // FF -- representing a Forced-Page-Break
            m_pCurrentImpl->insertText(sBuf);
            //m_pCurrentImpl->insertPageBreak();
            sBuf.clear();
            p++;
            break;

        default:
            if (*p < 0x20) // Silently eat these characters.
            {
                p++;
            }
            else
            {
                sBuf.appendUCS4(p, 1);
                p++;
            }
        }
    }

    if (!sBuf.empty())
    {
        m_pCurrentImpl->insertText(sBuf);
    }
}

void IE_Exp_HTML_Listener::_openSpan(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP;
    bool ok;

    if (m_bInSpan)
    {
        if (m_apiLastSpan == api)
            return;
        _closeSpan();
    }

    if (!api) // don't write tag for empty A/P
        return;

    m_bInSpan = true;
    m_apiLastSpan = api;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    const gchar *szStyleName = _getObjectKey(api, PT_STYLE_ATTRIBUTE_NAME);
    const IE_Exp_HTML_StyleTree *tree = m_pStyleTree->find(szStyleName);
    const gchar *styleName = NULL;
    if (tree != NULL)
    {
        styleName = tree->class_name().utf8_str();
    }

	const gchar * szP_FontWeight = 0;
	const gchar * szP_FontStyle = 0;
	const gchar * szP_FontSize = 0;
	const gchar * szP_FontFamily = 0;
	const gchar * szP_TextDecoration = 0;
	const gchar * szP_TextPosition = 0;
	const gchar * szP_TextTransform = 0;
	const gchar * szP_Color = 0;
	const gchar * szP_BgColor = 0;
	const gchar * szP_Display = 0;

	pAP->getProperty("font-weight", szP_FontWeight);
	pAP->getProperty("font-style", szP_FontStyle);
	pAP->getProperty("font-size", szP_FontSize);
	pAP->getProperty("font-family", szP_FontFamily);
	pAP->getProperty("text-decoration", szP_TextDecoration);
	pAP->getProperty("text-position", szP_TextPosition);
	pAP->getProperty("text-transform", szP_TextTransform);
	pAP->getProperty("color", szP_Color);
	pAP->getProperty("bgcolor", szP_BgColor);
	pAP->getProperty("display", szP_Display);

	UT_UTF8String style;
	UT_UTF8String tmp;
	bool first = true;
	/* TODO: this bold/italic check needs re-thought
	 */
	if (szP_FontWeight)
	{
		if (strcmp(szP_FontWeight, "bold") == 0)
			//if (!compareStyle("font-weight", "bold")) {
				if (!first) style += ";";
				style += "font-weight:bold";
				first = false;
			// }
	}
	if (szP_FontStyle)
	{
		if (strcmp(szP_FontStyle, "italic") == 0)
		{
			//if (!compareStyle("font-style", "italic")) {
				if (!first) style += ";";
				style += "font-style:italic";
				first = false;
			//}
		}
	}

	if (szP_FontSize) {
		char buf[16];

		{
			sprintf(buf, "%g", UT_convertToPoints(szP_FontSize));
		}

		tmp = buf;
		tmp += "pt";

		//if (!compareStyle("font-size", style.utf8_str())) {
			if (!first) style += ";";
			style += "font-size:";
			style += tmp;
			first = false;
		//}
	}

	if (szP_FontFamily) {
		if ((strcmp(szP_FontFamily, "serif") == 0) ||
			(strcmp(szP_FontFamily, "sans-serif") == 0) ||
			(strcmp(szP_FontFamily, "cursive") == 0) ||
			(strcmp(szP_FontFamily, "fantasy") == 0) ||
			(strcmp(szP_FontFamily, "monospace") == 0)) {
			tmp = static_cast<const char *> (szP_FontFamily);
		}
		else {
			tmp = "'";
			tmp += static_cast<const char *> (szP_FontFamily);
			tmp += "'";
		}
		//if (!compareStyle("font-family", style.utf8_str())) {
			if (!first) style += ";";
			style += "font-family:";
			style += tmp;
			first = false;
		//}
	}
	if (szP_TextDecoration) {
		bool bUnderline = (strstr(szP_TextDecoration, "underline") != NULL);
		bool bLineThrough = (strstr(szP_TextDecoration, "line-through") != NULL);
		bool bOverline = (strstr(szP_TextDecoration, "overline") != NULL);

		if (bUnderline || bLineThrough || bOverline) {
			tmp  = "";
			if (bUnderline) tmp += "underline";
			if (bLineThrough) {
				if (bUnderline) tmp += ", ";
				tmp += "line-through";
			}
			if (bOverline) {
				if (bUnderline || bLineThrough) style += ", ";
				tmp += "overline";
			}
			//if (!compareStyle("text-decoration", style.utf8_str())) {
				if (!first) style += ";";
				style += "text-decoration:";
				style += tmp;
				first = false;
			//}
		}
	}
	if (szP_TextTransform) {
		//if (!compareStyle("text-transform", szP_TextTransform)) {
			if (!first) style += ";";
			style += "text-transform:";
			style += szP_TextTransform;
			first = false;
		//}
	}

	if (szP_TextPosition) {
		if (strcmp(szP_TextPosition, "superscript") == 0) {
			//if (!compareStyle("vertical-align", "super")) {
				if (!first) style += ";";
				style += "vertical-align:super";
				first = false;
			//}
		}
		else if (strcmp(szP_TextPosition, "subscript") == 0) {
			//if (!compareStyle("vertical-align", "sub")) {
				if (!first) style += ";";
				style += "vertical-align:sub";
				first = false;
			//}
		}
	}
	if (szP_Color && *szP_Color)
		if (!IS_TRANSPARENT_COLOR(szP_Color)) {
			//if (!compareStyle("color", style.utf8_str())) {
				if (!first) style += ";";
				style += "color:";
				style += UT_colorToHex(szP_Color, true);
				first = false;
			//}
		}
	if (szP_BgColor && *szP_BgColor)
		if (!IS_TRANSPARENT_COLOR(szP_BgColor)) {
			//if (!compareStyle("background", style.utf8_str())) {
				if (!first) style += ";";
				style += "background:";
				style += UT_colorToHex(szP_BgColor, true);;
				first = false;
			//}
		}

	if (szP_Display) {
		if (strcmp(szP_Display, "none") == 0) {
			if (!first) style += ";";
			style += "display:none";
			first = false;
		}
	}
    m_pCurrentImpl->openSpan(styleName, style);
    return;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeSpan(void)
{
    if (!m_bInSpan)
        return;

    m_bInSpan = false;
    m_pCurrentImpl->closeSpan();
    return;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openBlock(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP;
    bool ok;

    m_bInBlock = true;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    const gchar *szStyleName = _getObjectKey(api, PT_STYLE_ATTRIBUTE_NAME);
    const IE_Exp_HTML_StyleTree *tree = m_pStyleTree->find(szStyleName);
    const gchar *styleName = NULL;
    if (tree != NULL)
    {
        styleName = tree->class_name().utf8_str();
    }

	const gchar * szP_TextAlign = 0;
	const gchar * szP_MarginBottom = 0;
	const gchar * szP_MarginTop = 0;
	const gchar * szP_MarginLeft = 0;
	const gchar * szP_MarginRight = 0;
	const gchar * szP_TextIndent = 0;

	pAP->getProperty("text-align", szP_TextAlign);
	pAP->getProperty("margin-bottom", szP_MarginBottom);
	pAP->getProperty("margin-top", szP_MarginTop);
	pAP->getProperty("margin-right", szP_MarginRight);

	if (pAP->getProperty("margin-left", szP_MarginLeft))
		if (strstr(szP_MarginLeft, "0.0000"))
			szP_MarginLeft = 0;

	if (pAP->getProperty("text-indent", szP_TextIndent))
		if (strstr(szP_TextIndent, "0.0000"))
			szP_TextIndent = 0;

	UT_UTF8String style;
	bool first = true;

	if (szP_TextAlign) {
		if (!first) style += ";";
		style += "text-align:";
		style += szP_TextAlign;
		first = false;
	}
	if (szP_MarginBottom) {
		if (!first) style += ";";
		style += "margin-bottom:";
		style += szP_MarginBottom;
		first = false;
	}
	if (szP_MarginTop) {
		if (!first) style += ";";
		style += "margin-top:";
		style += szP_MarginTop;
		first = false;
	}
	if (szP_MarginRight) {
		if (!first) style += ";";
		style += "margin-right:";
		style += szP_MarginRight;
		first = false;
	}
	if (szP_MarginLeft) {
		if (!first) style += ";";
		style += "margin-left:";
		style += szP_MarginLeft;
		first = false;
	}
	if (szP_TextIndent) {
		if (!first) style += ";";
		style += "text-indent:";
		style += szP_TextIndent;
		first = false;
	}
	
    m_pCurrentImpl->openBlock(styleName, style, pAP);
    
    
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeBlock()
{
    if (m_bInBlock)
    {
        m_bInBlock = false;
        m_pCurrentImpl->closeBlock();
    }
    
    
}

void IE_Exp_HTML_Listener::_openHeading(PT_AttrPropIndex api, size_t level, 
                                        const gchar* szStyleName)
{
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok) 
    {
        pAP = NULL;
    }
    
    m_bInHeading = true;
    UT_UTF8String id = UT_UTF8String_sprintf("AbiTOC%d", m_iHeadingCount);
    m_pCurrentImpl->openHeading(level, szStyleName, id.utf8_str(), pAP);
    m_iHeadingCount++;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeHeading()
{
    if (m_bInHeading)
    {
        m_pCurrentImpl->closeHeading();
        m_bInHeading = false;
    }
}

/**
 *
 */
void IE_Exp_HTML_Listener::_openSection(PT_AttrPropIndex api, bool recursiveCall)
{
    const PP_AttrProp* pAP;
    bool ok;

    if (!recursiveCall)
    {
        UT_ASSERT(!m_bInSection);

        m_bInSection = true;
    }

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }
    
    const gchar *szStyleName = _getObjectKey(api, PT_STYLE_ATTRIBUTE_NAME);
	m_dPageWidthInches = m_pDocument->m_docPageSize.Width(DIM_IN);

	const char* pszLeftMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszBottomMargin = NULL;
	pAP->getProperty("page-margin-left", (const gchar *&)pszLeftMargin);
	pAP->getProperty("page-margin-right", (const gchar *&)pszRightMargin);
	pAP->getProperty("page-margin-top", (const gchar *&)pszTopMargin);
	pAP->getProperty("page-margin-bottom", (const gchar *&)pszBottomMargin);
	
	if(pszLeftMargin && pszLeftMargin[0])
	{
		m_dSecLeftMarginInches = UT_convertToInches(pszLeftMargin);
	}
	else
	{
		m_dSecLeftMarginInches = 1.0;
	}

	if(pszRightMargin && pszRightMargin[0])
	{
		m_dSecRightMarginInches = UT_convertToInches(pszRightMargin);
	}
	else
	{
		m_dSecRightMarginInches = 1.0;
	}
	
	if(pszTopMargin && pszTopMargin[0])
	{
		m_dSecTopMarginInches = UT_convertToInches(pszTopMargin);
	}
	else
	{
		m_dSecTopMarginInches = 1.0;
	}

	if(pszBottomMargin && pszBottomMargin[0])
	{
		m_dSecBottomMarginInches = UT_convertToInches(pszBottomMargin);
	}
	else
	{
		m_dSecBottomMarginInches = 1.0;
	}
    m_pCurrentImpl->openSection(szStyleName);
    m_endnotes.clear();
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeSection(bool recursiveCall)
{

    if (!recursiveCall)
    {
        if (!m_bInSection)
            return;

        m_bInSection = false;
    }

    m_pCurrentImpl->insertEndnotes(m_endnotes);
    m_pCurrentImpl->closeSection();
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openField(const PX_ChangeRecord_Object* pcro,
                                      PT_AttrPropIndex api)
{
    UT_return_if_fail(pcro);

    const PP_AttrProp* pAP = NULL;
    fd_Field* pField = pcro->getField();

    UT_return_if_fail(pField);
    UT_return_if_fail(m_pDocument->getAttrProp(api, &pAP) && pAP);

    UT_UTF8String fieldValue = pField->getValue();
    UT_UTF8String fieldType;
    const gchar * szValue = NULL;

    UT_return_if_fail(pAP->getAttribute("type", szValue) && szValue);
    fieldType = szValue;
    
    if (fieldType != "list_label")
    {
        if (fieldType == "endnote_anchor")
        {
            m_bInEndnote = true;
        } else
        if (fieldType == "footnote_anchor")
        {
            m_bInFootnote = true;
        } else
        {
            m_pCurrentField = pField;
            m_currentFieldType = fieldType;
            m_pCurrentImpl->openField(m_currentFieldType, fieldValue);
        }
    }
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeField(void)
{
    if (!m_pCurrentField || !m_currentFieldType.length())
        return;
    _closeSpan();

    m_pCurrentImpl->closeField(m_currentFieldType);

    m_pCurrentField = NULL;
    m_currentFieldType.clear();
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openFootnote(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    m_bInFootnote = true;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeFootnote()
{
    m_bInBlock = true;

    m_bInFootnote = false;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openEndnote(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }


    m_bInEndnote = true;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeEndnote()
{
    m_bInEndnote = false;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openAnnotation(PT_AttrPropIndex api)
{

    if (m_bInAnnotation)
    {
        return;
    }

    const PP_AttrProp* pAP = NULL;
    bool ok = false;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    m_pCurrentImpl->openAnnotation();
    m_bInAnnotation = true;
    m_bInBlock = false;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeAnnotation()
{

    if (!m_bInAnnotation)
    {
        return;
    }

    m_pCurrentImpl->closeAnnotation();
    m_bInAnnotation = false;
    m_bInBlock = true;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openTable(PT_AttrPropIndex api, bool recursiveCall)
{
    const PP_AttrProp* pAP;
    bool ok;


    if (!recursiveCall)
    {
        m_iInTable++;
    }

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }
	_fillColWidthsVector();


	//UT_sint32 cellPadding = 0;
	UT_UTF8String styles;

	const char * prop = m_tableHelper.getTableProp ("table-line-thickness");

	UT_sint32 border = 0;

	if(prop && atof(prop) != 0.0)
		border = 1;

	UT_UTF8String border_default = "1pt";
	if (prop)
	{
		double dPT = UT_convertToDimension(prop, DIM_PT);
		border_default = UT_UTF8String_sprintf("%.2fpt", dPT);
	}

#if 0
	const gchar * pszLeftOffset = 0;
	const gchar * pszTopOffset = 0;
	const gchar * pszRightOffset = 0;
	const gchar * pszBottomOffset = 0;

	pSectionAP->getProperty ("cell-margin-left",   pszLeftOffset);
	pSectionAP->getProperty ("cell-margin-top",    pszTopOffset);
	pSectionAP->getProperty ("cell-margin-right",  pszRightOffset);
	pSectionAP->getProperty ("cell-margin-bottom", pszBottomOffset);
#endif
	const char * pszWidth = m_tableHelper.getTableProp ("width");
	if (m_bAbsUnits) {
		if (pszWidth) {
			if (styles.byteLength ()) styles += ";";
			styles += "width:";
			// use mm (inches are too big, since we want to use an int).
			double dMM = UT_convertToDimension(pszWidth, DIM_MM);
			UT_UTF8String t2;
			UT_UTF8String_sprintf(t2, "%.1fmm", dMM);
			styles += t2;
		}
	} else if (m_bScaleUnits) {
		// TEST ME!
		if (pszWidth) {
			if (styles.byteLength ()) styles += ";";
			styles += "width:";
			double tMM = UT_convertToDimension(pszWidth, DIM_MM);
			double totWidth = m_dPageWidthInches - m_dSecLeftMarginInches - m_dSecRightMarginInches;
			UT_UTF8String tws = UT_UTF8String_sprintf("%d", totWidth);
			double pMM = UT_convertToDimension(tws.utf8_str(), DIM_MM);
			double dPCT = tMM / pMM;
			UT_UTF8String t2;
			UT_UTF8String_sprintf(t2, "%d%%", dPCT);
			styles += t2;
		}
	} 
	else {
		// this should match abi because tables always cover width by default
		if (styles.byteLength ()) styles += ";";
		styles += "width:100%";
	}

	const char * pszBgColor = m_tableHelper.getTableProp ("bgcolor");
	if (pszBgColor == NULL)
		pszBgColor = m_tableHelper.getTableProp ("background-color");
	if (pszBgColor)
	{
		if (styles.byteLength ()) styles += ";";
		styles += "background-color:";

		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBgColor);
		if (hash)
			styles += hash;
		else
			styles += pszBgColor;
	}

	const char * pszBorderColor = NULL;

	pszBorderColor = m_tableHelper.getTableProp ("color");
	if (pszBorderColor)
	{
		if (styles.byteLength ()) styles += ";";
		styles += "color:";

		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			styles += hash;
		else
			styles += pszBorderColor;
	}

	// more often than not border attributes are same all around, so
	// we want to use the border shortcut
	// 0-L, 1-R, 2-T, 3-B
	double dB[4] = {0.0,0.0,0.0,0.0};
	UT_UTF8String sB[4];
	UT_UTF8String sC[4];
	UT_UTF8String sS[4];
	
	pszBorderColor = m_tableHelper.getTableProp ("bot-color");
	if (pszBorderColor)
	{
		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			sC[3]= hash;
		else
			sC[3]= pszBorderColor;
	}
	pszBorderColor = m_tableHelper.getTableProp ("left-color");
	if (pszBorderColor)
	{
		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			sC[0] = hash;
		else
			sC[0] = pszBorderColor;
	}
	pszBorderColor = m_tableHelper.getTableProp ("right-color");
	if (pszBorderColor)
	{
		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			sC[1] = hash;
		else
			sC[1] = pszBorderColor;
	}
	pszBorderColor = m_tableHelper.getTableProp ("top-color");
	if (pszBorderColor)
	{
		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			sC[2] = hash;
		else
			sC[2] = pszBorderColor;
	}

	const char * pszBorderStyle = NULL;

	pszBorderStyle = m_tableHelper.getTableProp ("bot-style");
	if (pszBorderStyle)
	{
		sS[3]= PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
	}
	pszBorderStyle = m_tableHelper.getTableProp ("left-style");
	if (pszBorderStyle)
	{
		sS[0] = PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
	}
	pszBorderStyle = m_tableHelper.getTableProp ("right-style");
	if (pszBorderStyle)
	{
		sS[1] = PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
	}
	pszBorderStyle = m_tableHelper.getTableProp ("top-style");
	if (pszBorderStyle)
	{
		sS[2] = PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
	}

	const char * pszBorderWidth = NULL;

	pszBorderWidth = m_tableHelper.getTableProp ("bot-thickness");
	if (pszBorderWidth)
	{
		dB[3] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[3] = UT_UTF8String_sprintf("%.2fpt", dB[3]);
	}
	else
		sB[3] += border_default;
	pszBorderWidth = m_tableHelper.getTableProp ("left-thickness");
	if (pszBorderWidth)
	{
		dB[0] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[0] = UT_UTF8String_sprintf("%.2fpt", dB[0]);
	}
	else
		sB[0] = border_default;
	pszBorderWidth = m_tableHelper.getTableProp ("right-thickness");
	if (pszBorderWidth)
	{
		dB[1] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[1] = UT_UTF8String_sprintf("%.2fpt", dB[1]);
	}
	else
		sB[1] = border_default;
	pszBorderWidth = m_tableHelper.getTableProp ("top-thickness");
	if (pszBorderWidth)
	{
		dB[2] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[2] = UT_UTF8String_sprintf("%.2fpt", dB[2]);
	}
	else
		sB[2] += border_default;

	// now we need to decide which attributes are to be used in the
	// shortcut
	UT_uint32 iBCount[4] = {0,0,0,0}; // 0 - L, 1 - R, 2 - T, 3 - B
	UT_uint32 iCCount[4] = {0,0,0,0}; // 0 - L, 1 - R, 2 - T, 3 - B
	UT_uint32 iSCount[4] = {0,0,0,0}; // 0 - L, 1 - R, 2 - T, 3 - B
	UT_uint32 iBMaxIndx = 0, iCMaxIndx = 0, iSMaxIndx = 0;
	UT_uint32 i = 0;
	
	for(i = 0; i < 4; ++i)
	{
		for(UT_sint32 j = i+1; j < 4; j++)
		{
			if(dB[i] == dB[j])
			{
				iBCount[i]++;
				iBCount[j]++;
			}
		}
	}

	for(i = 1; i < 4; i++)
	{
		if(iBMaxIndx < iBCount[i])
			iBMaxIndx = i;
	}

	for(i = 0; i < 4; ++i)
	{
		for(UT_sint32 j = i+1; j < 4; j++)
		{
			if(sC[i] == sC[j])
			{
				iCCount[i]++;
				iCCount[j]++;
			}
		}
	}

	for(i = 1; i < 4; i++)
	{
		if(iCMaxIndx < iCCount[i])
			iCMaxIndx = i;
	}

	for(i = 0; i < 4; ++i)
	{
		for(UT_sint32 j = i+1; j < 4; j++)
		{
			if(sS[i] == sS[j])
			{
				iSCount[i]++;
				iSCount[j]++;
			}
		}
	}

	for(i = 1; i < 4; i++)
	{
		if(iSMaxIndx < iSCount[i])
			iSMaxIndx = i;
	}
	
	if(styles.size() != 0) styles += ";";
	
	styles += "border:";
	styles += sB[iBMaxIndx];

	if(sS[iSMaxIndx].size())
	{
		styles += " ";
		styles += sS[iSMaxIndx];
	}
	

	if(sC[iCMaxIndx].size())
	{
		styles += " ";
		styles += sC[iCMaxIndx];
	}

	if(styles.size() != 0) styles += ";";
	styles += "border-collapse:collapse;empty-cells:show;table-layout:fixed";
	// only add the border style if we didn't already add it in the "border shortcut"
	if (!sS[iSMaxIndx].size()) styles += ";border-style:solid";
	
	if(iBCount[iBMaxIndx] != 3)
	{
		for(i = 0; i < 4; ++i)
		{
			if(i == iBMaxIndx || dB[i] == dB[iBMaxIndx] || sB[i].size() == 0)
				continue;

			switch(i)
			{
				case 0: styles += "border-left-width:"; break;
				case 1: styles += "border-right-width:";  break;
				case 2: styles += "border-top-width:";  break;
				case 3: styles += "border-bottom-width:";  break;
			}

			styles += sB[i];
			styles += ";";
		}
	}
	
	if(iSCount[iSMaxIndx] != 3)
	{
		for(i = 0; i < 4; ++i)
		{
			if(i == iSMaxIndx || sS[i] == sS[iSMaxIndx] || sS[i].size() == 0)
				continue;

			switch(i)
			{
				case 0: styles += "border-left-style:"; break;
				case 1: styles += "border-right-style:"; break;
				case 2: styles += "border-top-style:"; break;
				case 3: styles += "border-bottom-style:"; break;
			}

			styles += sS[i];
			styles += ";";
		}
	}

	if(iCCount[iCMaxIndx] != 3)
	{
		for(i = 0; i < 4; ++i)
		{
			if(i == iCMaxIndx  || sC[i] == sC[iCMaxIndx] || sC[i].size() == 0)
				continue;

			switch(i)
			{
				case 0: styles += "border-left-color:"; break;
				case 1: styles += "border-right-color:"; break;
				case 2: styles += "border-top-color:"; break;
				case 3: styles += "border-bottom-color:"; break;
			}

			styles += sC[i];
			styles += ";";
		}
	}

	const char * p = styles.utf8_str();
	UT_UTF8String s;
	if(p[styles.byteLength()-1] == ';')
	{
		s.append(p, styles.byteLength()-1);
	}
	else
	{
		s = p;
	}
	
	//m_utf8_1  = "table cellpadding=\"";
	//m_utf8_1 += UT_UTF8String_sprintf ("%d\" border=\"%d", cellPadding, border);
	//m_utf8_1 = UT_UTF8String_sprintf ("table cellpadding=\"0\" border=\"%d\" style=\"", border);
	//m_utf8_1 += s;
	// m_utf8_1 += "\"";
	
    m_pCurrentImpl->openTable(s, "0",
							  UT_UTF8String_sprintf("%d", border));
	
	

}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeTable(bool recursiveCall)
{

    if (!recursiveCall)
    {
        if (m_iInTable == 0)
            return;

        m_iInTable--;
    }

    m_pCurrentImpl->closeTable();
}

void IE_Exp_HTML_Listener::_openRow(PT_AttrPropIndex api, bool recursiveCall)
{
    const PP_AttrProp* pAP;
    bool ok;


    if (!recursiveCall)
    {
        m_iInRow++;
    }

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    m_pCurrentImpl->openRow();
}

void IE_Exp_HTML_Listener::_closeRow(bool recursiveCall)
{
    if (!recursiveCall)
    {
        if (m_iInRow == 0)
            return;

        m_iInRow--;
    }

    m_pCurrentImpl->closeRow();
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openCell(PT_AttrPropIndex api, bool recursiveCall)
{
    const PP_AttrProp* pAP;
    bool ok;

    if (!recursiveCall)
    {
        m_iInCell++;
    }

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

	_setCellWidthInches();
	double dColSpacePT = 0;
	double dRowSpacePT = 0;
	const gchar * pszTableColSpacing = m_tableHelper.getTableProp("table-col-spacing");
	const gchar * pszTableRowSpacing = m_tableHelper.getTableProp("table-row-spacing");

	if (pszTableColSpacing)
		dColSpacePT = UT_convertToDimension(pszTableColSpacing, DIM_PT);

	if (pszTableRowSpacing)
		dRowSpacePT = UT_convertToDimension(pszTableRowSpacing, DIM_PT);

	UT_UTF8String styles;

	if (dColSpacePT == dRowSpacePT) {
		styles += UT_UTF8String_sprintf("padding: %.2fpt", dColSpacePT);
	}
	else {
		styles += UT_UTF8String_sprintf("padding: %.2fpt %.2fpt", dRowSpacePT, dColSpacePT);
	}

	UT_sint32 rowspan = m_tableHelper.getBot() - m_tableHelper.getTop();
	UT_sint32 colspan = m_tableHelper.getRight() - m_tableHelper.getLeft();

	const char * pszBgColor = m_tableHelper.getCellProp("bgcolor");
	if (pszBgColor == NULL)
		pszBgColor = m_tableHelper.getCellProp("background-color");
	if (pszBgColor) {
		if (styles.byteLength()) styles += ";";
		styles += "background-color:";

		UT_HashColor color;
		const char * hash = color.setHashIfValid(pszBgColor);
		if (hash)
			styles += hash;
		else
			styles += pszBgColor;
	}

	const char * pszBorderColor = NULL;

	pszBorderColor = m_tableHelper.getCellProp("color");
	if (pszBorderColor) {
		if (styles.byteLength()) styles += ";";
		styles += "color:";

		UT_HashColor color;
		const char * hash = color.setHashIfValid(pszBorderColor);
		if (hash)
			styles += hash;
		else
			styles += pszBorderColor;
	}

	// more often than not border attributes are same all around, so
	// we want to use the border shortcut
	// 0-L, 1-R, 2-T, 3-B
	double dB[4] = {0.0, 0.0, 0.0, 0.0};
	UT_UTF8String sB[4];
	UT_UTF8String sC[4];
	UT_UTF8String sS[4];

	pszBorderColor = m_tableHelper.getCellProp("bot-color");
	if (pszBorderColor) {
		UT_HashColor color;
		const char * hash = color.setHashIfValid(pszBorderColor);
		if (hash)
			sC[3] = hash;
		else
			sC[3] = pszBorderColor;
	}
	pszBorderColor = m_tableHelper.getCellProp("left-color");
	if (pszBorderColor) {
		UT_HashColor color;
		const char * hash = color.setHashIfValid(pszBorderColor);
		if (hash)
			sC[0] = hash;
		else
			sC[0] = pszBorderColor;
	}
	pszBorderColor = m_tableHelper.getCellProp("right-color");
	if (pszBorderColor) {
		UT_HashColor color;
		const char * hash = color.setHashIfValid(pszBorderColor);
		if (hash)
			sC[1] = hash;
		else
			sC[1] = pszBorderColor;
	}
	pszBorderColor = m_tableHelper.getCellProp("top-color");
	if (pszBorderColor) {
		UT_HashColor color;
		const char * hash = color.setHashIfValid(pszBorderColor);
		if (hash)
			sC[2] = hash;
		else
			sC[2] = pszBorderColor;
	}

	const char * pszBorderStyle = NULL;

	pszBorderStyle = m_tableHelper.getCellProp("bot-style");
	if (pszBorderStyle) {
		sS[3] = PP_PropertyMap::linestyle_for_CSS(pszBorderStyle);
	}
	pszBorderStyle = m_tableHelper.getCellProp("left-style");
	if (pszBorderStyle) {
		sS[0] = PP_PropertyMap::linestyle_for_CSS(pszBorderStyle);
	}
	pszBorderStyle = m_tableHelper.getCellProp("right-style");
	if (pszBorderStyle) {
		sS[1] = PP_PropertyMap::linestyle_for_CSS(pszBorderStyle);
	}
	pszBorderStyle = m_tableHelper.getCellProp("top-style");
	if (pszBorderStyle) {
		sS[2] = PP_PropertyMap::linestyle_for_CSS(pszBorderStyle);
	}

	const char * pszBorderWidth = NULL;

	pszBorderWidth = m_tableHelper.getCellProp("bot-thickness");
	if (pszBorderWidth) {
		dB[3] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[3] = UT_UTF8String_sprintf("%.2fpt", dB[3]);
	}
	pszBorderWidth = m_tableHelper.getCellProp("left-thickness");
	if (pszBorderWidth) {
		dB[0] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[0] = UT_UTF8String_sprintf("%.2fpt", dB[0]);
	}
	pszBorderWidth = m_tableHelper.getCellProp("right-thickness");
	if (pszBorderWidth) {
		dB[1] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[1] = UT_UTF8String_sprintf("%.2fpt", dB[1]);
	}
	pszBorderWidth = m_tableHelper.getCellProp("top-thickness");
	if (pszBorderWidth) {
		dB[2] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[2] = UT_UTF8String_sprintf("%.2fpt", dB[2]);
	}

	// now we need to decide which attributes are to be used in the
	// shortcut
	UT_uint32 iBCount[4] = {0, 0, 0, 0}; // 0 - L, 1 - R, 2 - T, 3 - B
	UT_uint32 iCCount[4] = {0, 0, 0, 0}; // 0 - L, 1 - R, 2 - T, 3 - B
	UT_uint32 iSCount[4] = {0, 0, 0, 0}; // 0 - L, 1 - R, 2 - T, 3 - B
	UT_uint32 iBMaxIndx = 0, iCMaxIndx = 0, iSMaxIndx = 0;
	UT_sint32 i = 0;

	for (i = 0; i < 4; ++i) {
		for (UT_sint32 j = i + 1; j < 4; j++) {
			if (dB[i] == dB[j]) {
				iBCount[i]++;
				iBCount[j]++;
			}
		}
	}

	for (i = 1; i < 4; i++) {
		if (iBMaxIndx < iBCount[i])
			iBMaxIndx = i;
	}

	for (i = 0; i < 4; ++i) {
		for (UT_sint32 j = i + 1; j < 4; j++) {
			if (sC[i] == sC[j]) {
				iCCount[i]++;
				iCCount[j]++;
			}
		}
	}

	for (i = 1; i < 4; i++) {
		if (iCMaxIndx < iCCount[i])
			iCMaxIndx = i;
	}

	for (i = 0; i < 4; ++i) {
		for (UT_sint32 j = i + 1; j < 4; j++) {
			if (sS[i] == sS[j]) {
				iSCount[i]++;
				iSCount[j]++;
			}
		}
	}

	for (i = 1; i < 4; i++) {
		if (iSMaxIndx < iSCount[i])
			iSMaxIndx = i;
	}

	if (styles.size() != 0) styles += ";";

	styles += "border:";

	if (sB[iBMaxIndx].size()) {
		styles += sB[iBMaxIndx];
	}
	else {
		styles += "inherit";
	}

	styles += " ";

	if (sS[iSMaxIndx].size()) {
		styles += sS[iSMaxIndx];
	}
	else {
		styles += "inherit";
	}

	styles += " ";

	if (sC[iCMaxIndx].size()) {
		styles += sC[iCMaxIndx];
	}
	else {
		styles += "inherit";
	}

	if (styles.size() != 0) styles += ";";
	if (iBCount[iBMaxIndx] != 3) {
		for (i = 0; i < 4; ++i) {
			if ((UT_uint32) i == iBMaxIndx || dB[i] == dB[iBMaxIndx])
				continue;

			switch (i) {
			case 0: styles += "border-left-width:";
				break;
			case 1: styles += "border-right-width:";
				break;
			case 2: styles += "border-top-width:";
				break;
			case 3: styles += "border-bottom-width:";
				break;
			}

			if (sB[i].size())
				styles += sB[i];
			else
				styles += "inherit";

			styles += ";";
		}
	}

	if (iSCount[iSMaxIndx] != 3) {
		for (i = 0; i < 4; ++i) {
			if ((UT_uint32) i == iSMaxIndx || sS[i] == sS[iSMaxIndx])
				continue;

			switch (i) {
			case 0: styles += "border-left-style:";
				break;
			case 1: styles += "border-right-style:";
				break;
			case 2: styles += "border-top-style:";
				break;
			case 3: styles += "border-bottom-style:";
				break;
			}

			if (sS[i].size())
				styles += sS[i];
			else
				styles += "inherit";

			styles += ";";
		}
	}

	if (iCCount[iCMaxIndx] != 3) {
		for (i = 0; i < 4; ++i) {
			if ((UT_uint32) i == iCMaxIndx || sC[i] == sC[iCMaxIndx])
				continue;

			switch (i) {
			case 0: styles += "border-left-color:";
				break;
			case 1: styles += "border-right-color:";
				break;
			case 2: styles += "border-top-color:";
				break;
			case 3: styles += "border-bottom-color:";
				break;
			}

			if (sC[i].size())
				styles += sC[i];
			else
				styles += "inherit";

			styles += ";";
		}
	}

	const char * p = styles.utf8_str();
	UT_UTF8String s;
	if (p[styles.byteLength() - 1] == ';') {
		s.append(p, styles.byteLength() - 1);
	}
	else {
		s = p;
	}


	UT_UTF8String rowspanStr;
	UT_UTF8String colspanStr;

	if (rowspan > 1) {
		rowspanStr += UT_UTF8String_sprintf("%d", rowspan);
	}
	if (colspan > 1) {
		colspanStr = UT_UTF8String_sprintf("%d", colspan);
	}
	m_pCurrentImpl->openCell(s, rowspanStr, colspanStr);
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeCell(bool recursiveCall)
{

    if (!recursiveCall)
    {
        if (m_iInCell == 0)
            return;

        m_iInCell--;
    }


    m_pCurrentImpl->closeCell();
}

void IE_Exp_HTML_Listener::_openList(PT_AttrPropIndex api, bool recursiveCall)
{
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    if (pAP == NULL)
    {
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }

    UT_uint32 iCurrentLevel = 0;
    const gchar *szListId = _getObjectKey(api, PT_LISTID_ATTRIBUTE_NAME);
    const gchar *szListLevel = _getObjectKey(api, PT_LEVEL_ATTRIBUTE_NAME);
    if (szListLevel)
    {
        iCurrentLevel = atoi (static_cast<const char *>(szListLevel));
    }
    
    if (iCurrentLevel == 0)
    {
        return;
    }
    
    if (!recursiveCall)
    {
        if ((m_listInfoStack.size() > 0) && (g_ascii_strcasecmp(szListId, 
				m_listInfoStack.back().szId) == 0))
        {

            _openListItem();
        }
        else
        {
            UT_DEBUGMSG(("LEVEL: Current %d\n", iCurrentLevel));
            if ((m_listInfoStack.size() == 0) 
				 || (iCurrentLevel > m_listInfoStack.back().iLevel))
            {
                _openList(api, true);
            } else

            {
                while((m_listInfoStack.size() > 0) 
					   && (iCurrentLevel < m_listInfoStack.back().iLevel))
                {
                    _closeList();
                }
                _openList(api, true);
            }
        }
    }
    else
    {
        const gchar* szListStyle = NULL;
        pAP->getProperty("list-style", szListStyle);
        bool isOrdered = szListStyle
			&& (g_ascii_strcasecmp(szListStyle, "Bullet List") != 0);
#ifdef DEBUG
        if(!szListStyle) {
            UT_DEBUGMSG(("***BUG*** szListStyle is NULL - http://bugzilla.abisource.com/show_bug.cgi?id=13564\n"));
        }
#endif
        ListInfo info;
        if (iCurrentLevel == 0)
        {
            iCurrentLevel = 1;
        }
        info.szId = szListId;
        info.iLevel = iCurrentLevel;
        info.iItemCount = 0;
        m_listInfoStack.push_back(info);
        UT_DEBUGMSG(("OPENED LIST\n"));
        const IE_Exp_HTML_StyleTree *tree = m_pStyleTree->find(szListStyle);
        const gchar *styleName = NULL;
        if (tree != NULL)
        {
            styleName = tree->class_name().utf8_str();
        }
        m_pCurrentImpl->openList(isOrdered, styleName, pAP);
        _openListItem();
    }
      
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeList(bool recursiveCall)
{
    _closeListItem(recursiveCall);

    if (!recursiveCall)
    {
        if (m_listInfoStack.size() == 0)
            return;

        UT_DEBUGMSG(("CLOSED LIST\n"));
        m_pCurrentImpl->closeList();
        m_listInfoStack.pop_back();
    }



}

void IE_Exp_HTML_Listener::_closeLists()
{
    while ((m_listInfoStack.size() > 0))
    {
        _closeList();
    }
}

void IE_Exp_HTML_Listener::_openListItem(bool recursiveCall)
{
    _closeListItem();
    if (!recursiveCall)
    {
        ListInfo info = m_listInfoStack.back();
        m_listInfoStack.pop_back();
        info.iItemCount++;
        m_listInfoStack.push_back(info);
    }
    UT_DEBUGMSG(("OPENED LIST ITEM: %d\n", m_listInfoStack.size()));
    m_pCurrentImpl->openListItem();
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeListItem(bool recursiveCall)
{

    if (!recursiveCall)
    {
        if ((m_listInfoStack.size() == 0)
			 ||(m_listInfoStack.back().iItemCount == 0))
            return;

        ListInfo info = m_listInfoStack.back();
        m_listInfoStack.pop_back();
        info.iItemCount--;
        m_listInfoStack.push_back(info);
    }

    UT_DEBUGMSG(("CLOSED LIST ITEM\n"));
    m_pCurrentImpl->closeListItem();
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openFrame(PT_AttrPropIndex api, const PX_ChangeRecord* pcr)
{
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }
    
    const gchar *szType;
    if (pAP->getProperty("frame-type", szType))
    {
        if (!strcmp(szType, "textbox"))
        {
            _openTextbox(pcr->getIndexAP());
        } else if (!strcmp(szType, "image"))
        {
          _insertPosImage(pcr->getIndexAP());
        }
    }


}

void IE_Exp_HTML_Listener::_insertPosImage(PT_AttrPropIndex api)
{
    const PP_AttrProp * pAP = NULL;
    bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);
    if (!bHaveProp || (pAP == 0)) return;

    const gchar * pszDataID = NULL;
    if (pAP->getAttribute(PT_STRUX_IMAGE_DATAID, (const gchar *&) pszDataID) && pszDataID)
        _handleImage(api, pszDataID, true);
}
/**
 * 
 */
void IE_Exp_HTML_Listener::_closeFrame()
{
   _closeTextbox();
}


void IE_Exp_HTML_Listener::_openDocument()
{
    m_pCurrentImpl->openDocument();
}

void IE_Exp_HTML_Listener::_closeDocument()
{
    m_pCurrentImpl->closeDocument();
}

void IE_Exp_HTML_Listener::_openHead()
{
    m_pCurrentImpl->openHead();
}

void IE_Exp_HTML_Listener::_closeHead()
{
    m_pCurrentImpl->closeHead();
}

void IE_Exp_HTML_Listener::_openBody()
{
    m_pCurrentImpl->openBody();
}

void IE_Exp_HTML_Listener::_closeBody()
{
    m_pCurrentImpl->closeBody();
}

void IE_Exp_HTML_Listener::_insertDTD()
{
    m_pCurrentImpl->insertDTD();
}

void IE_Exp_HTML_Listener::_insertLinks()
{
}

void IE_Exp_HTML_Listener::_insertTitle()
{
	std::string metaProp;

    if (m_pDocument->getMetaDataProp(PD_META_KEY_TITLE, metaProp) 
									 && !metaProp.empty())
	{
        m_pCurrentImpl->insertTitle(metaProp);
	} else
	{
		m_pCurrentImpl->insertTitle("Abiword HTML Document");
	}
}

void IE_Exp_HTML_Listener::_insertMeta()
{
	std::string metaProp;

    if (m_pDocument->getMetaDataProp(PD_META_KEY_TITLE, metaProp) 
									 && metaProp.size())
        m_pCurrentImpl->insertMeta("title", metaProp, std::string());

    if (m_pDocument->getMetaDataProp(PD_META_KEY_CREATOR, metaProp) 
									 && metaProp.size())
        m_pCurrentImpl->insertMeta("author", metaProp, std::string());

    if (m_pDocument->getMetaDataProp(PD_META_KEY_KEYWORDS, metaProp) 
									 && metaProp.size())
        m_pCurrentImpl->insertMeta("keywords", metaProp, std::string());

    if (m_pDocument->getMetaDataProp(PD_META_KEY_SUBJECT, metaProp) 
									 && metaProp.size())
        m_pCurrentImpl->insertMeta("subject", metaProp, std::string());
}
    
void IE_Exp_HTML_Listener::_insertTOC(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    const gchar *pValue = 0;
    bool hasHeading = true; // AbiWord's default
    ok = pAP->getProperty("toc-has-heading", pValue);
    if (ok && pValue)
    {
        hasHeading = (*pValue == '1');
    }

    // determine the style of the TOC heading
    UT_UTF8String headingStyle;
    ok = pAP->getProperty("toc-heading-style", pValue);
    if (ok && pValue)
    {
        headingStyle = pValue;
    }
    else
    {
        const PP_Property* pProp = PP_lookupProperty("toc-heading-style");
        UT_ASSERT_HARMLESS(pProp);
        if (pProp)
            headingStyle = pProp->getInitial();
    }

    const gchar* szTOCHeading;
    ok = pAP->getProperty("toc-heading", szTOCHeading);
    if (!(ok && szTOCHeading))
    {
       szTOCHeading = fl_TOCLayout::getDefaultHeading().c_str();
    }
    
    std::vector<UT_UTF8String> tocItems;
    std::vector<UT_UTF8String> tocItemsUri;
    UT_uint32 tocNum = 0;
    UT_UTF8String prevName;
    PT_DocPosition pos;
    m_pNavigationHelper->getNthTOCEntryPos(0,pos);
    prevName =  m_pNavigationHelper->getFilenameByPosition(pos);
    for (int i = 0; i < m_pNavigationHelper->getNumTOCEntries(); i++)
    {
        
        UT_UTF8String tocItem = m_pNavigationHelper->getNthTOCEntry(i, NULL);
        UT_UTF8String tocItemUri;
        if (m_bSplitDocument)
        {
            PT_DocPosition tocPos;
            m_pNavigationHelper->getNthTOCEntryPos(i, tocPos);
            UT_UTF8String tocItemFile = m_pNavigationHelper->
                getFilenameByPosition(tocPos);
            
            if (tocItemFile != prevName)
            {
                tocNum = 0;
                prevName = tocItemFile;
            }
            tocItemUri = UT_UTF8String_sprintf("%s#AbiTOC%d", 
                                               tocItemFile.utf8_str(), tocNum);
            tocNum++;
        } else
        {
            tocItemUri = UT_UTF8String_sprintf("#AbiTOC%d", i);
        }
        
        
        
        tocItems.push_back(tocItem);
        tocItemsUri.push_back(tocItemUri);
    }
    
    m_pCurrentImpl->insertTOC(szTOCHeading, tocItems, tocItemsUri);
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openBookmark(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP = NULL;
    bool ok = false;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (ok && pAP)
    {
        const gchar* pValue = NULL;
        if (pAP->getAttribute("name", pValue) && pValue)
        {
            m_bookmarkName = pValue;
            m_pCurrentImpl->openBookmark(pValue);
        }
        else
        {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        }
        m_bInBookmark = true;

    }
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeBookmark()
{
    if (!m_bInBookmark)
        return;

    _closeSpan();
    m_pCurrentImpl->closeBookmark();
    m_bInBookmark = false;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_openHyperlink(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP = NULL;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (ok && pAP)
    {
        m_bInHyperlink = true;

        const gchar *szEscapedUrl = NULL;
        const gchar *szUrl = _getObjectKey(api, "xlink:href");
        UT_UTF8String url = szUrl;
        if (szUrl != NULL)
        {
            
        
            if (m_bSplitDocument)
            {
                if (szUrl[0] == '#')
                {
                    UT_DEBUGMSG(("Internal reference found\n"));
                    UT_UTF8String filename = m_pNavigationHelper->
						getBookmarkFilename(szUrl + 1);

                    if (filename != m_filename)
                    {
                        url = filename + url;
                        UT_DEBUGMSG(("Internal referrence is reference accross chapters to file %s\n", filename.utf8_str()));
                    }
                }
            }
            
            szEscapedUrl = url.escapeXML().utf8_str();
            UT_DEBUGMSG(("Escaped URL %s\n", szEscapedUrl));
        }

        UT_DEBUGMSG(("Opened Hyperlink\n"));
        m_pCurrentImpl->openHyperlink(szEscapedUrl, NULL, NULL);
    }


}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeHyperlink()
{
    if (!m_bInHyperlink)
        return;

    _closeSpan();

    m_bInHyperlink = false;
    UT_DEBUGMSG(("Closed Hyperlink\n"));
    m_pCurrentImpl->closeHyperlink();
}

void IE_Exp_HTML_Listener::_openTextbox(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP = NULL;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (ok && pAP) 
{
        // TODO: Enum frame properties (and in any case where props equal their css counterparts) separately
        // TODO: so here (and places like here) you can just iterate through it getting the prop and setting it.
        // TODO: Actually, you wouldn't have to limit it to where the props were identical, just have
        // TODO: { abiprop, cssprop }.  It would still require that the units used for both specs be compatible.
        //
        //	TODO: Take care of padding as well.
        const gchar * propNames[20] = {"bot-thickness", "border-bottom-width",
            "top-thickness", "border-top-width",
            "right-thickness", "border-right-width",
            "left-thickness", "border-left-width",
            "bot-color", "border-bottom-color",
            "top-color", "border-top-color",
            "right-color", "border-right-color",
            "left-color", "border-left-color",
            "background-color", "background-color",
            NULL, NULL}; // [AbiWord property name, CSS21 property name]
        const gchar * tempProp = 0;
        UT_UTF8String style;

        for (unsigned short int propIdx = 0; propIdx < 18; propIdx += 2) 
        {
            if (pAP->getProperty(propNames[propIdx], tempProp)) // If we successfully retrieve a value (IOW, it's defined)
            {
                style += propNames[propIdx + 1]; // Add the property name of the CSS equivalent
                style += ": "; // Don't ask (:
                if (strstr(propNames[propIdx + 1], "color")) style += "#"; // AbiWord tends to store colors as hex, which must be prefixed by # in CSS
                style += tempProp; // Add the value
                style += "; "; // Terminate the property
            }
        }


        //pAP->getProperty("bot-style", tempProp); // Get the bottom border style
        //<...>
        // We don't do this right now because we don't support multiple styles right now.
        // See bug 7935.  Until we support multiple styles, it is sufficient to set all solid.
        style += " border: solid;";

        // This might need to be updated for textbox (and wrapped-image?) changes that
        // occured in 2.3. 

        // Get the wrap mode
        if (!pAP->getProperty("wrap-mode", tempProp) || !tempProp || !*tempProp)
            tempProp = "wrapped-both"; // this seems like a sane default

        if (!strcmp(tempProp, "wrapped-both"))
            style += " clear: none;";
        else if (!strcmp(tempProp, "wrapped-left"))
            style += " clear: right;";
        else if (!strcmp(tempProp, "wrapped-right"))
            style += " clear: left;";
        else if (!strcmp(tempProp, "above-text"))
            style += " clear: none; z-index: 999;";    

        m_pCurrentImpl->openTextbox(style);
        m_bInTextbox = true;
    }


    
}

void IE_Exp_HTML_Listener::_closeTextbox()
{
    if (m_bInTextbox)
    {
        m_pCurrentImpl->closeTextbox();
        m_bInTextbox = false;
    }
}

/**
 * 
 */
const gchar* IE_Exp_HTML_Listener::_getObjectKey(const PT_AttrPropIndex& api,
                                                 const gchar* key)
{
    const PP_AttrProp * pAP = NULL;
    bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);
    if (bHaveProp && pAP)
    {
        const gchar* value;
        if (pAP->getAttribute(key, value))
            return value;
    }

    return 0;
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_insertImage(PT_AttrPropIndex api)
{
    const gchar* szDataId;
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    szDataId =
            _getObjectKey(api, static_cast<const gchar*> ("dataid"));
    _handleImage(api, szDataId, false);
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_insertEmbeddedImage(PT_AttrPropIndex api)
{
    UT_UTF8String snapshot = "snapshot-png-";
    const gchar* szDataId = NULL;
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    szDataId =
            _getObjectKey(api, static_cast<const gchar*> ("dataid"));

    if (szDataId)
    {
        snapshot += szDataId;
        _handleImage(api, snapshot.utf8_str(), false);
    }
}

void IE_Exp_HTML_Listener::_handleImage(PT_AttrPropIndex api, 
    const gchar* szDataId, bool bIsPositioned)
{
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok) {
        pAP = NULL;
    }
    
    if (szDataId == NULL) {
        return;
    }

    std::string mimeType;

    if (!m_pDocument->getDataItemDataByName(szDataId, NULL,
                                            &mimeType, NULL))
        return;

    if (mimeType == "image/svg")
    {
        _insertEmbeddedImage(api);
        return;
    }
    
    if ((mimeType != "image/png") && (mimeType != "image/jpeg")) {
        UT_DEBUGMSG(("Image not of a suppored MIME type - ignoring...\n"));
        return;
    }


    std::string extension;
    if (!m_pDocument->getDataItemFileExtension(szDataId, extension, true)) {
        extension = ".png";
    }

    const gchar * szTitle = 0;
    UT_UTF8String title;
    pAP->getAttribute("title", szTitle);
    if (szTitle) {
        title = szTitle;
        title.escapeXML();
    }

    const gchar * szAlt = 0;
    UT_UTF8String alt;
    pAP->getAttribute("alt", szAlt);
    if (szAlt) {
        alt = szAlt;
        alt.escapeXML();
    }

    UT_UTF8String imageName;

    if (m_bEmbedImages) {
        m_pDataExporter->encodeDataBase64(szDataId, imageName);
    }
    else {
        imageName = m_pDataExporter->saveData(szDataId, extension.c_str());
    }
    UT_UTF8String align = "";
    if (bIsPositioned) {
        const gchar * szXPos = NULL;
        UT_sint32 ixPos = 0;
        if (pAP->getProperty("xpos", szXPos)) {
            ixPos = UT_convertToLogicalUnits(szXPos);
        }
        else if (pAP->getProperty("frame-col-xpos", szXPos)) {
            ixPos = UT_convertToLogicalUnits(szXPos);
        }
        else if (pAP->getProperty("frame-page-xpos", szXPos)) {
            ixPos = UT_convertToLogicalUnits(szXPos);
        }
        if (ixPos > UT_convertToLogicalUnits("1.0in")) {
            align = "right";
        }
        else {
            align = "left";
        }
    }


    const gchar * szWidth = 0;
    const gchar * szHeight = 0;
    double widthPercentage = 0;
    UT_UTF8String style = "";
    if (!getPropertySize(pAP, !bIsPositioned ? "width" : "frame-width",
                         "height", &szWidth, widthPercentage, &szHeight,
                         m_dPageWidthInches, m_dSecLeftMarginInches, m_dSecRightMarginInches,
                         m_dCellWidthInches, m_tableHelper))
        return;
    UT_DEBUGMSG(("Size of Image: %sx%s\n", szWidth ? szWidth : "(null)",
                szHeight ? szHeight : "(null)"));

    style = getStyleSizeString(szWidth, widthPercentage, DIM_MM, szHeight,
                               DIM_MM, false);

    m_pCurrentImpl->insertImage(imageName, align, style, title, alt);
}

/**
 * 
 */
void IE_Exp_HTML_Listener::_insertMath(PT_AttrPropIndex api)
{
    
        const gchar* szMath = NULL;
        szMath = _getObjectKey(api, static_cast<const gchar*>("dataid"));

        UT_return_if_fail(szMath);

        const UT_ByteBuf * pByteBuf = NULL;
        bool bOK = m_pDocument->getDataItemDataByName(szMath, const_cast<const UT_ByteBuf **>(&pByteBuf), NULL, NULL);

        UT_return_if_fail(bOK);

        UT_UCS4_mbtowc myWC;
        UT_UTF8String sMathML;
        sMathML.appendBuf(*pByteBuf, myWC);

        UT_return_if_fail(!sMathML.empty());

        const PP_AttrProp * pAP = NULL;
        bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
        UT_LocaleTransactor t(LC_NUMERIC, "C");
        double dWidth;
        double dHeight;

        UT_return_if_fail(bHaveProp && pAP);

        if(pAP->getProperty("width", szMath)) {
            dWidth = static_cast<double>(atoi(szMath))/UT_LAYOUT_RESOLUTION;
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            return;
        }

        if(pAP->getProperty("height", szMath)) {
            dHeight = static_cast<double>(atoi(szMath))/UT_LAYOUT_RESOLUTION;
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            return;
        }
        
        UT_UTF8String width = UT_UTF8String_sprintf("%fin", dWidth);
        UT_UTF8String height = UT_UTF8String_sprintf("%fin", dHeight);
        
        m_pCurrentImpl->insertMath(sMathML, width, height);
        m_bHasMathMl = true;
        

}

void IE_Exp_HTML_Listener::_insertEndnotes()
{
    if (m_endnotes.size() > 0)
        m_pCurrentImpl->insertEndnotes(m_endnotes);
}

void IE_Exp_HTML_Listener::_insertFootnotes()
{
    if (m_footnotes.size() > 0)
        m_pCurrentImpl->insertFootnotes(m_footnotes);
}

void IE_Exp_HTML_Listener::_insertAnnotations()
{
    if (m_annotationContents.size() > 0)
    {
        m_pCurrentImpl->insertAnnotations(m_annotationTitles, 
                                          m_annotationAuthors, 
                                          m_annotationContents);
    }
}

void IE_Exp_HTML_Listener::_insertStyle()
{
    m_pCurrentImpl->insertStyle(m_stylesheet);
}

void IE_Exp_HTML_Listener::_insertLinkToStyle()
{
    UT_UTF8String filename = m_pDataExporter->saveData(
        "style.css", m_stylesheet);
    m_pCurrentImpl->insertLink("stylesheet", "text/css", filename);
}


void IE_Exp_HTML_Listener::_handleAnnotationData(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }
    
    const gchar *szTitle = 0;
    const gchar *szAuthor = 0;
    if (pAP)
    {
        pAP->getProperty("annotation-title", szTitle);
        pAP->getProperty("annotation-author", szAuthor);
    }
    
    m_annotationTitles.push_back(szTitle);
    m_annotationAuthors.push_back(szAuthor);
}

void IE_Exp_HTML_Listener::_makeStylesheet(PT_AttrPropIndex api)
{
	const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }
	
    UT_ByteBuf buffer;
    StyleListener styleListener(buffer);
    m_pStyleTree->print(&styleListener);
    
    m_stylesheet = sStyleSheet; // Stylesheet for TOC`s and so on
	if( const char* p = reinterpret_cast<const char*>(buffer.getPointer(0)))
		m_stylesheet += p;

	UT_UTF8String bodyStyle = "body{\n";
	const gchar* szName = NULL;
	const gchar* szValue = NULL;
	// Set margins for paged media to match those set in AbiWord
	// TODO: consolidate all places of awml-css21 matching into one UT/PP function
	const gchar * marginProps [10] = {"page-margin-top", "padding-top",
		"page-margin-bottom", "padding-bottom",
		"page-margin-left", "padding-left",
		"page-margin-right", "padding-right",
		NULL, NULL};
	for (unsigned short int propIdx = 0; propIdx < 8; propIdx += 2) {
		szValue = PP_evalProperty(marginProps[propIdx], 0, 0, pAP, 
								 m_pDocument, true);
		bodyStyle += UT_UTF8String_sprintf("%s : %s;\n", 
										 marginProps[propIdx + 1], szValue);
	}

	

	PD_Style * pStyle = 0;
	m_pDocument->getStyle("Normal", &pStyle);
	UT_UTF8String value;
	for (UT_uint32 i = 0; i < pStyle->getPropertyCount(); i++) {
		pStyle->getNthProperty(i, szName, szValue);

		if ((szName == 0) || (szValue == 0)) continue; // paranoid? moi?
		if ((*szName == 0) || (*szValue == 0)) continue;

		if (strstr(szName, "margin")) continue;
		if (!is_CSS(reinterpret_cast<const char *> (szName))) continue;

		if (strcmp(szName, "font-family") == 0) {
			if ((strcmp(szValue, "serif") == 0) ||
				(strcmp(szValue, "sans-serif") == 0) ||
				(strcmp(szValue, "cursive") == 0) ||
				(strcmp(szValue, "fantasy") == 0) ||
				(strcmp(szValue, "monospace") == 0)) {
				value = static_cast<const char *> (szValue);
			}
			else {
				value = "'";
				value += static_cast<const char *> (szValue);
				value += "'";
			}
		}
		else if (strcmp(szName, "color") == 0) {
			if (IS_TRANSPARENT_COLOR(szValue)) continue;

			value = UT_colorToHex(szValue, true);
		}
		else value = static_cast<const char *> (szValue);

		bodyStyle += UT_UTF8String_sprintf("%s:%s;\n", szName, value.utf8_str());
	}
	szValue = PP_evalProperty("background-color", 0, 0, pAP, m_pDocument, true);
	if (szValue && *szValue && !IS_TRANSPARENT_COLOR(szValue)) {
		value= UT_colorToHex(szValue, true);

		bodyStyle += UT_UTF8String_sprintf("background-color:%s;\n", 
										 szName, value.utf8_str());
	}
	
	bodyStyle += "}";
	m_stylesheet += bodyStyle;
}

void IE_Exp_HTML_Listener::_setCellWidthInches()
{

	UT_sint32 left = m_tableHelper.getLeft();
	UT_sint32 right = m_tableHelper.getRight();
	double tot = 0;
	UT_sint32 i = 0;

	UT_ASSERT_HARMLESS((UT_sint32) m_vecDWidths.size() >= (right - 1));

	for (i = left; i < right; i++) {
		// probably covering up some sort of issue
		// but we assert above, so we'll notice it again
		if (i < (UT_sint32) m_vecDWidths.size())
			tot += m_vecDWidths.getNthItem(i);
	}
	m_dCellWidthInches = tot;

}

void IE_Exp_HTML_Listener::_fillColWidthsVector()
{
	// make sure any unit conversions are correct
	UT_LocaleTransactor t(LC_NUMERIC, "C");

	//
	// Positioned columns controls
	//
	const char * pszColumnProps = m_tableHelper.getTableProp("table-column-props");
	UT_DEBUGMSG(("Number columns in table %d \n",m_tableHelper.getNumCols ()));
	if(m_vecDWidths.getItemCount() > 0)
	{
		m_vecDWidths.clear();
	}
	if(pszColumnProps && *pszColumnProps)
	{
		/*
		  These will be properties applied to all columns. To start with, just the 
		  widths of each column are specifed. These are translated to layout units.
 
		  The format of the string of properties is:

		  table-column-props:1.2in/3.0in/1.3in/;

		  So we read back in pszColumnProps
		  1.2in/3.0in/1.3in/

		  The "/" characters will be used to delineate different column entries.
		  As new properties for each column are defined these will be delineated with "_"
		  characters. But we'll cross that bridge later.
		*/
		UT_DEBUGMSG(("table-column-props:%s \n",pszColumnProps));
		UT_String sProps = pszColumnProps;
		UT_sint32 sizes = sProps.size();
		UT_sint32 i =0;
		UT_sint32 j =0;
		while(i < sizes)
		{
			for (j=i; (j<sizes) && (sProps[j] != '/') ; j++) {}
			if(sProps[j] == 0)
			{
				// reached the end of the props string without finding
				// any further sizes
				break;
			}
			
			if((j+1)>i && sProps[j] == '/')
			{
				UT_String sSub = sProps.substr(i,(j-i));
				i = j + 1;
				m_vecDWidths.addItem(UT_convertToInches(sSub.c_str()));
			}
		}
	}
	//
	// automatic column widths set to total width divided by nCols
	//
	else
	{
		// double total = m_dPageWidthInches - m_dSecLeftMarginInches - m_dSecRightMarginInches;
		UT_sint32 nCols = m_tableHelper.getNumCols ();
		double totWidth = m_dPageWidthInches - m_dSecLeftMarginInches - m_dSecRightMarginInches;
		double colWidth = totWidth/nCols;
		UT_sint32 i = 0;
		for(i =0; i< nCols; i++)
		{
			m_vecDWidths.addItem(colWidth);
		}
	}
}
