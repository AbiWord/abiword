#include "ie_exp_HTML_Listener.h"
#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML.h"

IE_Exp_HTML_Listener::IE_Exp_HTML_Listener(PD_Document *pDocument, 
                                           IE_Exp_HTML_DataExporter* pDataExporter,
                                           IE_Exp_HTML_StyleTree    *pStyleTree,
                                           IE_Exp_HTML_ListenerImpl* pListenerImpl) :
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
m_bRenderMathToPng(true),   
m_pStyleTree(pStyleTree)
{

}

bool IE_Exp_HTML_Listener::populate(PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord* pcr)
{
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

            if (pAP && pAP->getAttribute("type", pValue) && pValue && (strcmp(pValue, "start") == 0))
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
                UT_DEBUGMSG(("RUDYJ: Left anotation\n"));
                _closeAnnotation();
            } else
            {
                UT_DEBUGMSG(("RUDYJ: Entered annotation\n"));
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

bool IE_Exp_HTML_Listener::populateStrux(PL_StruxDocHandle sdh, const PX_ChangeRecord* pcr, PL_StruxFmtHandle* psfh)
{
    UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
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


    switch (pcrx->getStruxType())
    {
    case PTX_Section:
    case PTX_SectionHdrFtr:
    {
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
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _openTable(api);

        m_tableHelper.OpenTable(sdh, api);
        m_bFirstRow = true;
    }
        break;

    case PTX_SectionCell:
    {
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
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _openFootnote(api);
    }
        break;

    case PTX_SectionEndnote:
    {
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _openEndnote(api);
    }
        break;

    case PTX_SectionAnnotation:
    {
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
        _closeSpan();
        _closeField();
        _closeBookmark();
        _closeHyperlink();
        _closeBlock();
        _closeHeading();
        _closeLists();
        _openFrame(api);
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
            if ((g_ascii_strcasecmp(static_cast<const gchar *> (szBlockStyle), "Heading 1") == 0) ||
                (g_ascii_strcasecmp(static_cast<const gchar *> (szBlockStyle), "Numbered Heading 1") == 0))
            {
                _openHeading(1);
            }
            else
                if ((g_ascii_strcasecmp(static_cast<const gchar *> (szBlockStyle), "Heading 2") == 0) ||
                    (g_ascii_strcasecmp(static_cast<const gchar *> (szBlockStyle), "Numbered Heading 2") == 0))
            {
                _openHeading(2);
            }
            else
                if ((g_ascii_strcasecmp(static_cast<const gchar *> (szBlockStyle), "Heading 3") == 0) ||
                    (g_ascii_strcasecmp(static_cast<const gchar *> (szBlockStyle), "Numbered Heading 3") == 0))
            {
                _openHeading(3);
            }
            else
                if ((g_ascii_strcasecmp(static_cast<const gchar *> (szBlockStyle), "Heading 4") == 0) ||
                    (g_ascii_strcasecmp(static_cast<const gchar *> (szBlockStyle), "Numbered Heading 4") == 0))
            {
                _openHeading(4);
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

bool IE_Exp_HTML_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
                                       const PX_ChangeRecord * /*pcr*/,
                                       PL_StruxDocHandle /*sdh*/,
                                       PL_ListenerId /*lid*/,
                                       void (*/*pfnBindHandles*/) (PL_StruxDocHandle sdhNew,
                                       PL_ListenerId lid,
                                       PL_StruxFmtHandle sfhNew))
{
    return true;
}

bool IE_Exp_HTML_Listener::change(PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord* /*pcr*/)
{
    return true;
}

bool IE_Exp_HTML_Listener::signal(UT_uint32 /*iSignal*/)
{
    return true;
}

bool IE_Exp_HTML_Listener::beginOfDocument()
{
    _insertDTD();
    _openDocument();
    _openHead();
    _insertTitle();
    _insertMeta();
    _makeStylesheet();
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

void IE_Exp_HTML_Listener::_outputData(const UT_UCSChar* pData, UT_uint32 length)
{
    UT_UTF8String sBuf;
    const UT_UCSChar* p;

    UT_ASSERT(sizeof (UT_Byte) == sizeof (char));
    sBuf.reserve(length);

    for (p = pData; (p < pData + length); /**/)
    {
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
            sBuf += "&nbsp;";
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
    m_pCurrentImpl->openSpan(styleName);
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
    UT_DEBUGMSG(("CLOSED SPAN\n"));
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
    m_pCurrentImpl->openBlock(styleName);
    
    
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

void IE_Exp_HTML_Listener::_openHeading(size_t level, const gchar* szStyleName)
{
    m_bInHeading = true;
    m_pCurrentImpl->openHeading(level, szStyleName, NULL);
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

    m_pCurrentImpl->openTable(pAP);

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


    m_pCurrentImpl->openCell(pAP);
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
        if ((m_listInfoStack.size() > 0) && (g_ascii_strcasecmp(szListId, m_listInfoStack.back().szId) == 0))
        {

            _openListItem();
        }
        else
        {
            UT_DEBUGMSG(("LEVEL: Current %d\n", iCurrentLevel));
            if ((m_listInfoStack.size() == 0) || (iCurrentLevel > m_listInfoStack.back().iLevel))
            {
                _openList(api, true);
            } else

            {
                while((m_listInfoStack.size() > 0) && (iCurrentLevel < m_listInfoStack.back().iLevel))
                {
                    _closeList();
                }
                _openList(api, true);
            }
        }
        
    }else
    {
        const gchar* szListStyle;
        pAP->getProperty("list-style", szListStyle);
        bool isOrdered = g_ascii_strcasecmp(szListStyle, "Bullet List") != 0;
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
        m_pCurrentImpl->openList(isOrdered, styleName);
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
        if ((m_listInfoStack.size() == 0)||(m_listInfoStack.back().iItemCount == 0))
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
void IE_Exp_HTML_Listener::_openFrame(PT_AttrPropIndex api)
{
    const PP_AttrProp* pAP;
    bool ok;

    ok = m_pDocument->getAttrProp(api, &pAP);
    if (!ok)
    {
        pAP = NULL;
    }

    m_pCurrentImpl->openFrame(pAP);

}

/**
 * 
 */
void IE_Exp_HTML_Listener::_closeFrame()
{
    m_pCurrentImpl->closeFrame();
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
}

void IE_Exp_HTML_Listener::_insertMeta()
{
    UT_UTF8String metaProp;

    if (m_pDocument->getMetaDataProp(PD_META_KEY_TITLE, metaProp) && metaProp.size())
        m_pCurrentImpl->insertMeta("title", metaProp);

    if (m_pDocument->getMetaDataProp(PD_META_KEY_CREATOR, metaProp) && metaProp.size())
        m_pCurrentImpl->insertMeta("author", metaProp);

    if (m_pDocument->getMetaDataProp(PD_META_KEY_KEYWORDS, metaProp) && metaProp.size())
        m_pCurrentImpl->insertMeta("keywords", metaProp);

    if (m_pDocument->getMetaDataProp(PD_META_KEY_SUBJECT, metaProp) && metaProp.size())
        m_pCurrentImpl->insertMeta("subject", metaProp);
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
       szTOCHeading = fl_TOCLayout::getDefaultHeading().utf8_str();
    }
    
    std::vector<UT_UTF8String> tocItems;
    std::vector<UT_UTF8String> tocItemsUri;
    IE_TOCHelper *pTOCHelper  = new IE_TOCHelper(m_pDocument);
    for (int i = 0; i < pTOCHelper->getNumTOCEntries(); i++)
    {
        UT_UTF8String tocItem = pTOCHelper->getNthTOCEntry(i, NULL);
        UT_UTF8String tocItemUri = tocItem;
        
        tocItems.push_back(tocItem);
        tocItemsUri.push_back(tocItemUri);
    }
    DELETEP(pTOCHelper);
    
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
            url.escapeURL();
            szEscapedUrl = url.utf8_str();
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
    
    if ( szDataId == NULL)
    {
        return;
    }
    
    std::string mimeType;
    
    if (!m_pDocument->getDataItemDataByName(szDataId, NULL, 
        &mimeType, NULL))
		return;
    
    if (mimeType == "image/svg+xml")
	{
		_insertEmbeddedImage(api);
		return;
	}
	
	if ((mimeType != "image/png") && (mimeType != "image/jpeg"))
	{
		UT_DEBUGMSG(("Image not of a suppored MIME type - ignoring...\n"));
		return;
	}
    
      
    std::string extension;
    if(!m_pDocument->getDataItemFileExtension(szDataId, extension, true))
	{
         extension = ".png";
    }
    
    const gchar * szTitle  = 0;
	UT_UTF8String title;
	pAP->getAttribute ("title",  szTitle);
	if (szTitle) {
        title = szTitle;
        title.escapeXML();
	}

	const gchar * szAlt  = 0;
    UT_UTF8String alt;
	pAP->getAttribute ("alt",  szAlt);
	if (szAlt) {
        alt = szAlt;
        alt.escapeXML();
	}
    
    UT_UTF8String imageName;
    
    if (m_bEmbedImages)
    {
        m_pDataExporter->encodeDataBase64(szDataId, imageName);
    } else
    {
        imageName = m_pDataExporter->saveData(szDataId, extension.c_str());
    }
        
    m_pCurrentImpl->insertImage(imageName, "", "", "", "", title, alt);
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
        // m_pCurrentImpl->insertImage(snapshot.utf8_str(), pAP);
    }
    
    
    const gchar * szTitle  = 0;
	UT_UTF8String title;
	pAP->getAttribute ("title",  szTitle);
	if (szTitle) {
        title = szTitle;
        title.escapeXML();
	}

	const gchar * szAlt  = 0;
    UT_UTF8String alt;
	pAP->getAttribute ("alt",  szAlt);
	if (szAlt) {
        alt = szAlt;
        alt.escapeXML();
	}
    
    UT_UTF8String imageName;
    
    if (m_bEmbedImages)
    {
        m_pDataExporter->encodeDataBase64(snapshot.utf8_str(), imageName);
    } else
    {
        imageName = m_pDataExporter->saveData(snapshot.utf8_str(),".png");
    }
        
    m_pCurrentImpl->insertImage(imageName, "", "", "", "", title, alt);

}

/**
 * 
 */
void IE_Exp_HTML_Listener::_insertMath(PT_AttrPropIndex api)
{
    /*
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

        UT_UCS4String buf = sMathML.utf8_str();
        UT_UTF8String output = "";

        const PP_AttrProp * pAP = NULL;
        bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
        UT_LocaleTransactor t(LC_NUMERIC, "C");
        UT_UTF8String dimension;
        double dInch;

        UT_return_if_fail(bHaveProp && pAP);

        _openSpan(api);

        if(pAP->getProperty("width", szMath)) {
            dInch = static_cast<double>(atoi(szMath))/UT_LAYOUT_RESOLUTION;
            UT_UTF8String_sprintf(dimension,"%fin",dInch);
            output += "<draw:frame svg:width=\"";
            output += dimension;
            output += "\" svg:height=\"";
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            _closeSpan();
            return;
        }

        if(pAP->getProperty("height", szMath)) {
            dInch = static_cast<double>(atoi(szMath))/UT_LAYOUT_RESOLUTION;
            dimension.clear();
            UT_UTF8String_sprintf(dimension,"%fin",dInch);
            output += dimension;
            output += "\"><draw:object>";
        } else {
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
            _closeSpan();
            return;
        }

        for (UT_uint32 i = 0; i < buf.length(); i++) {
            if (buf[i] == '<') {
                if (((i + 1) < buf.length()) && (buf[i+1] == '/')) {
                    output += "</math:";
                    i++; // skip the '/'
                } else if ((i + 1) < buf.length()) {
                    output += "<math:";
                } else {
                    UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
                }
            } else {
                output += buf[i];
            }
        }
        output += "</draw:object></draw:frame>";
        m_pCurrentImpl->insertText(output);
        _closeSpan();*/
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
    UT_UTF8String filename;
    GsfOutput *css = m_pDataExporter->createFile("style.css", filename);
    IE_Exp_HTML_OutputWriter* pWriter = new IE_Exp_HTML_OutputWriter(css);
    pWriter->write(m_stylesheet.utf8_str(), m_stylesheet.length());
    m_pCurrentImpl->insertLink("stylesheet", "text/css", filename);
    
    DELETEP(pWriter);
    gsf_output_close(css);
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

void IE_Exp_HTML_Listener::_makeStylesheet()
{
    UT_ByteBuf buffer;
    StyleListener styleListener(buffer);
    m_pStyleTree->print(&styleListener);
    
    m_stylesheet = sStyleSheet;
    m_stylesheet += reinterpret_cast<const char*>(buffer.getPointer(0));
}