/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include "ie_exp_HTML.h"

IE_Exp_HTML_Sniffer::IE_Exp_HTML_Sniffer()
#ifdef HTML_NAMED_CONSTRUCTORS
: IE_ExpSniffer(IE_IMPEXPNAME_XHTML, true)
#endif
{
    // 
}

bool IE_Exp_HTML_Sniffer::recognizeSuffix(const char * szSuffix)
{
    return (!g_ascii_strcasecmp(szSuffix, ".xhtml") ||
            !(g_ascii_strcasecmp(szSuffix, ".html")) ||
            !(g_ascii_strcasecmp(szSuffix, ".htm")));
}

UT_Error IE_Exp_HTML_Sniffer::constructExporter(PD_Document * pDocument,
                                                IE_Exp ** ppie)
{
    IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
    *ppie = p;
    return UT_OK;
}

bool IE_Exp_HTML_Sniffer::getDlgLabels(const char ** pszDesc,
                                       const char ** pszSuffixList,
                                       IEFileType * ft)
{
    *pszDesc = "HTML/XHTML (.html)";
    *pszSuffixList = "*.html";
    *ft = getFileType();
    return true;
}

UT_Confidence_t IE_Exp_HTML_Sniffer::supportsMIME(const char * szMimeType)
{
    if (!strcmp(szMimeType, IE_MIMETYPE_XHTML) ||
        !strcmp(szMimeType, "application/xhtml") ||
        !strcmp(szMimeType, "text/html"))
        return UT_CONFIDENCE_PERFECT;
    return UT_CONFIDENCE_ZILCH;
}

#ifdef HTML_ENABLE_HTML4

// HTML 4

IE_Exp_HTML4_Sniffer::IE_Exp_HTML4_Sniffer()
#ifdef HTML_NAMED_CONSTRUCTORS
: IE_ExpSniffer(IE_IMPEXPNAME_HTML, true)
#endif
{
    // 
}

bool IE_Exp_HTML4_Sniffer::recognizeSuffix(const char * szSuffix)
{
    return (!(g_ascii_strcasecmp(szSuffix, ".html")) || !(g_ascii_strcasecmp(szSuffix, ".htm")));
}

UT_Error IE_Exp_HTML4_Sniffer::constructExporter(PD_Document * pDocument,
                                                 IE_Exp ** ppie)
{
    IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
    if (p) p->set_HTML4();
    *ppie = p;
    return UT_OK;
}

bool IE_Exp_HTML4_Sniffer::getDlgLabels(const char ** pszDesc,
                                        const char ** pszSuffixList,
                                        IEFileType * ft)
{
    *pszDesc = "HTML 4.0 (.html, .htm)";
    *pszSuffixList = "*.html; *.htm";
    *ft = getFileType();
    return true;
}

UT_Confidence_t IE_Exp_HTML4_Sniffer::supportsMIME(const char * szMimeType)
{
    if (!strcmp(szMimeType, "text/html"))
        return UT_CONFIDENCE_PERFECT;
    return UT_CONFIDENCE_ZILCH;
}

#endif /* HTML_ENABLE_HTML4 */

#ifdef HTML_ENABLE_PHTML

// XHTML w/ PHP instructions for AbiWord Web Docs

IE_Exp_PHTML_Sniffer::IE_Exp_PHTML_Sniffer()
#ifdef HTML_NAMED_CONSTRUCTORS
: IE_ExpSniffer(IE_IMPEXPNAME_PHTML, false)
#endif
{
    // 
}

bool IE_Exp_PHTML_Sniffer::recognizeSuffix(const char * szSuffix)
{
    return (!(g_ascii_strcasecmp(szSuffix, ".phtml")));
}

UT_Error IE_Exp_PHTML_Sniffer::constructExporter(PD_Document * pDocument,
                                                 IE_Exp ** ppie)
{
    IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
    if (p) p->set_PHTML();
    *ppie = p;
    return UT_OK;
}

bool IE_Exp_PHTML_Sniffer::getDlgLabels(const char ** pszDesc,
                                        const char ** pszSuffixList,
                                        IEFileType * ft)
{
    *pszDesc = "XHTML+PHP (.phtml)";
    *pszSuffixList = "*.phtml";
    *ft = getFileType();
    return true;
}

#endif /* HTML_ENABLE_PHTML */

#ifdef HTML_ENABLE_MHTML

// Multipart HTML: http://www.rfc-editor.org/rfc/rfc2557.txt

IE_Exp_MHTML_Sniffer::IE_Exp_MHTML_Sniffer()
#ifdef HTML_NAMED_CONSTRUCTORS
: IE_ExpSniffer(IE_IMPEXPNAME_MHTML, true)
#endif
{
    // 
}

bool IE_Exp_MHTML_Sniffer::recognizeSuffix(const char * szSuffix)
{
    return (!(g_ascii_strcasecmp(szSuffix, ".mht")));
}

UT_Error IE_Exp_MHTML_Sniffer::constructExporter(PD_Document * pDocument,
                                                 IE_Exp ** ppie)
{
    IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
    if (p) p->set_MHTML();
    *ppie = p;
    return UT_OK;
}

bool IE_Exp_MHTML_Sniffer::getDlgLabels(const char ** pszDesc,
                                        const char ** pszSuffixList,
                                        IEFileType * ft)
{
    *pszDesc = "Multipart HTML (.mht)";
    *pszSuffixList = "*.mht";
    *ft = getFileType();
    return true;
}

#endif /* HTML_ENABLE_MHTML */


/*****************************************************************/

/*****************************************************************/

IE_Exp_HTML::IE_Exp_HTML(PD_Document * pDocument)
: IE_Exp(pDocument),
m_style_tree(new IE_Exp_HTML_StyleTree(pDocument)),
m_styleListener(new IE_Exp_HTML_StyleListener(m_style_tree)),
m_bSuppressDialog(false),
m_toc(new IE_TOCHelper(pDocument)),
m_minTOCLevel(0),
m_minTOCIndex(0),
m_suffix("")
{
    m_exp_opt.bIs4 = false;
    m_exp_opt.bIsAbiWebDoc = false;
    m_exp_opt.bDeclareXML = true;
    m_exp_opt.bAllowAWML = true;
    m_exp_opt.bEmbedCSS = true;
    m_exp_opt.bLinkCSS = false;
    m_exp_opt.bEmbedImages = false;
    m_exp_opt.bMultipart = false;
    m_exp_opt.bClassOnly = false;
    m_exp_opt.bAbsUnits = false;
    m_exp_opt.bAddIdentifiers = false;
    m_exp_opt.iCompact = 0;

    m_error = UT_OK;

#ifdef HTML_DIALOG_OPTIONS
    XAP_Dialog_HTMLOptions::getHTMLDefaults(&m_exp_opt, XAP_App::getApp());
#endif
}

IE_Exp_HTML::~IE_Exp_HTML()
{
    DELETEP(m_styleListener);
    DELETEP(m_style_tree);
    DELETEP(m_toc);
}

void IE_Exp_HTML::_buildStyleTree()
{
    const PD_Style * p_pds = 0;
    const gchar * szStyleName = 0;

    UT_GenericVector<PD_Style*> * pStyles = NULL;
    getDoc()->enumStyles(pStyles);
    UT_return_if_fail(pStyles);
    UT_uint32 iStyleCount = getDoc()->getStyleCount();

    for (size_t n = 0; n < iStyleCount; n++)
    {
        p_pds = pStyles->getNthItem(n);
        UT_continue_if_fail(p_pds);

        szStyleName = p_pds->getName();

        if (p_pds == 0) continue;

        PT_AttrPropIndex api = p_pds->getIndexAP();

        const PP_AttrProp * pAP_style = 0;
        bool bHaveProp = getDoc()->getAttrProp(api, &pAP_style);

        if (bHaveProp && pAP_style /* && p_pds->isUsed () */) // can't trust ->isUsed() :-(
        {
            m_style_tree->add(szStyleName, getDoc());
        }
    }

    delete pStyles;

    if (isCopying()) // clipboard
        getDoc()->tellListenerSubset(m_styleListener, getDocRange());
    else
        getDoc()->tellListener(m_styleListener);
}

UT_Error IE_Exp_HTML::_doOptions()
{
#ifdef HTML_DIALOG_OPTIONS
    XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();

    if (m_bSuppressDialog || !pFrame || isCopying()) return UT_OK;
    if (pFrame)
    {
        AV_View * pView = pFrame->getCurrentView();
        if (pView)
        {
            GR_Graphics * pG = pView->getGraphics();
            if (pG && pG->queryProperties(GR_Graphics::DGP_PAPER))
            {
                return UT_OK;
            }
        }
    }
    /* run the dialog
     */

    XAP_Dialog_Id id = XAP_DIALOG_ID_HTMLOPTIONS;

    XAP_DialogFactory * pDialogFactory
            = static_cast<XAP_DialogFactory *> (XAP_App::getApp()->getDialogFactory());

    XAP_Dialog_HTMLOptions * pDialog
            = static_cast<XAP_Dialog_HTMLOptions *> (pDialogFactory->requestDialog(id));

    UT_return_val_if_fail(pDialog, false);

    pDialog->setHTMLOptions(&m_exp_opt, XAP_App::getApp());

    pDialog->runModal(pFrame);

    /* extract what they did
     */
    bool bSave = pDialog->shouldSave();

    pDialogFactory->releaseDialog(pDialog);

    if (!bSave)
    {
        return UT_SAVE_CANCELLED;
    }
#endif
    return UT_OK;
}

UT_Error IE_Exp_HTML::_writeDocument()
{
    UT_Error errOptions = _doOptions();

    if (errOptions == UT_SAVE_CANCELLED) //see Bug 10840
    {
        return UT_SAVE_CANCELLED;
    }
    else if (errOptions != UT_OK)
    {
        return UT_ERROR;
    }

    _buildStyleTree();

    if (isCopying()) // ClipBoard
    {
        m_exp_opt.bEmbedImages = true;
        return _writeDocument(true, false);
    }

    /* Export options:

    html4			yes | no	whether to write HTML4 or XHTML
    php-includes	yes | no	whether to add <?php instructions (for Abi's website)
    declare-xml		yes | no	whether to declare as <?xml (sometimes problematic with <?php )
    use-awml		yes | no	whether to add extra attributes in AWML namespace
    embed-css		yes | no	whether to embed the stylesheet
    link-css        <file>      styles in external sheet, insert
    appropriate <link> statement, do not
    export any style definition in the document
    class-only      yes | no    if text is formated with style, export only
    style name (class="style name") ignoring
    any explicit fmt properties
    embed-images	yes | no	whether to embed images in URLs
    html-template	<file>		use <file> as template for output
    href-prefix		<path>		use <path> as prefix for template href attributes marked with initial '$'
    title           <utf8 string> can contain the following special tokens
                                  %n - file name without extension
                                  %f - file name with extension
                                  %F - file name including full path
    abs-units       yes | no    use absolute rather than relative units in tables, etc. (defaults to no units)
    scale-units     yes | no    use scale (relative) rather than absolute units in tables, etc. (defaults to no units)
    compact         yes | no | number -- if set we avoid ouputing unnecessary whitespace; numerical value
                                         indicates max line length (default MAX_LINE_LEN)
     */

    std::string prop;

    prop = getProperty("html4");
    if (!prop.empty())
        m_exp_opt.bIs4 = UT_parseBool(prop.c_str(), m_exp_opt.bIs4);

    prop = getProperty("php-includes");
    if (!prop.empty())
        m_exp_opt.bIsAbiWebDoc = UT_parseBool(prop.c_str(), m_exp_opt.bIsAbiWebDoc);

    prop = getProperty("declare-xml");
    if (!prop.empty())
        m_exp_opt.bDeclareXML = UT_parseBool(prop.c_str(), m_exp_opt.bDeclareXML);

    prop = getProperty("use-awml");
    if (!prop.empty())
        m_exp_opt.bAllowAWML = UT_parseBool(prop.c_str(), m_exp_opt.bAllowAWML);

    prop = getProperty("embed-css");
    if (!prop.empty())
        m_exp_opt.bEmbedCSS = UT_parseBool(prop.c_str(), m_exp_opt.bEmbedCSS);

    prop = getProperty("mathml-render-png");
    if (!prop.empty())
        m_exp_opt.bMathMLRenderPNG = UT_parseBool(prop.c_str(), m_exp_opt.bMathMLRenderPNG);

    prop = getProperty("split-document");
    if (!prop.empty())
        m_exp_opt.bSplitDocument = UT_parseBool(prop.c_str(), m_exp_opt.bSplitDocument);

    prop = getProperty("abs-units");
    if (!prop.empty())
        m_exp_opt.bAbsUnits = UT_parseBool(prop.c_str(), m_exp_opt.bAbsUnits);

    prop = getProperty("add-identifiers");
    if (!prop.empty())
        m_exp_opt.bAddIdentifiers = UT_parseBool(prop.c_str(), m_exp_opt.bAddIdentifiers);

    prop = getProperty("compact");
    if (!prop.empty())
    {
        UT_sint32 iLen = atoi(prop.c_str());
        if (iLen != 0)
            m_exp_opt.iCompact = (UT_uint32) iLen;
        else
        {
            m_exp_opt.iCompact = (UT_uint32) UT_parseBool(prop.c_str(), (bool)m_exp_opt.iCompact);
            if (m_exp_opt.iCompact)
                m_exp_opt.iCompact = MAX_LINE_LEN;
        }
    }


    prop = getProperty("link-css");
    if (!prop.empty())
    {
        m_exp_opt.bEmbedCSS = false;
        m_exp_opt.bLinkCSS = true;
        m_sLinkCSS = prop;
    }

    prop = getProperty("class-only");
    if (!prop.empty() && !g_ascii_strcasecmp("yes", prop.c_str()))
    {
        m_exp_opt.bClassOnly = true;
    }

    prop = getProperty("title");
    if (!prop.empty())
    {
        m_sTitle.clear();
        // FIXME: less optimal -- hub
        UT_UTF8String utf8prop(prop.c_str());

        UT_UTF8Stringbuf::UTF8Iterator propIt = utf8prop.getIterator();

        UT_UCS4Char c = UT_UTF8Stringbuf::charCode(propIt.current());
        bool bToken = false;

        while (c)
        {
            if (bToken)
            {
                const char * fname = getDoc()->getFilename();
                if (fname)
                {
                    const char * base = UT_basename(fname);
                    UT_uint32 iNameLen = strlen(base);

                    const char * dot = strrchr(base, '.');
                    if (dot)
                    {
                        iNameLen = dot - base;
                    }

                    switch (c)
                    {
                    case 'n':
                        m_sTitle.append(base, iNameLen);
                        break;

                    case 'f':
                        m_sTitle += base;
                        break;

                    case 'F':
                        m_sTitle += fname;
                        break;

                    default:
                        m_sTitle.appendUCS4(&c, 1);
                    }
                }

                bToken = false;
            }
            else if (c == '%')
            {
                bToken = true;
                //m_sTitle.appendUCS4(&c,1);
            }
            else
            {
                m_sTitle.appendUCS4(&c, 1);
            }

            c = UT_UTF8Stringbuf::charCode(propIt.advance());
        }
    }



    prop = getProperty("embed-images");
    if (!prop.empty())
        m_exp_opt.bEmbedImages = UT_parseBool(prop.c_str(), m_exp_opt.bEmbedImages);

    prop = getProperty("html-template");
    if (prop.empty())
        return _writeDocument(false, false);

    /* template mode...
     */
    m_exp_opt.bIs4 = false;

    UT_UTF8String declaration;

    if (m_exp_opt.bDeclareXML)
        declaration += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" MYEOL;

    declaration += "<";
    declaration += DTD_XHTML;
    declaration += ">" MYEOL;

    write(declaration.utf8_str(), declaration.byteLength());

    s_TemplateHandler TH(getDoc(), this);

    UT_XML parser;
    parser.setExpertListener(&TH);

    UT_Error err = parser.parse(prop.c_str());

    return err;
}

struct StyleListener
{
    UT_ByteBuf& m_sink;
    UT_UTF8String m_utf8_0;
    UT_uint32 m_styleIndent;

    StyleListener(UT_ByteBuf & sink)
    : m_sink(sink), m_styleIndent(0)
    {
    }

    bool get_Compact()
    {
        return false;
    }

    void tagRaw(UT_UTF8String & content)
    {
        m_sink.append((const UT_Byte*) content.utf8_str(), content.byteLength());
    }

    void styleIndent()
    {
        m_utf8_0 = "";

        for (UT_uint32 i = 0; i < m_styleIndent; i++) m_utf8_0 += "\t";
    }

    void styleOpen(const UT_UTF8String & rule)
    {
        styleIndent();

        m_utf8_0 += rule;
        m_utf8_0 += " {";
        if (!get_Compact())
            m_utf8_0 += MYEOL;

        tagRaw(m_utf8_0);

        m_styleIndent++;
    }

    void styleClose()
    {
        if (m_styleIndent == 0)
        {
            UT_DEBUGMSG(("WARNING: CSS style group over-closing!\n"));
            return;
        }
        m_styleIndent--;

        styleIndent();

        m_utf8_0 += "}";
        if (!get_Compact())
            m_utf8_0 += MYEOL;

        tagRaw(m_utf8_0);
    }

    void styleNameValue(const char * name, const UT_UTF8String & value)
    {
        styleIndent();

        m_utf8_0 += name;
        m_utf8_0 += ":";
        m_utf8_0 += value;
        m_utf8_0 += ";";
        if (!get_Compact())
            m_utf8_0 += MYEOL;

        tagRaw(m_utf8_0);
    }

    void styleText(const UT_UTF8String & content)
    {
        m_utf8_0 = content;
        tagRaw(m_utf8_0);
    }
};

void IE_Exp_HTML::printStyleTree(PD_Document *pDocument, UT_ByteBuf& sink)
{
    IE_Exp_HTML html(pDocument);
    html._buildStyleTree();

    StyleListener listener(sink);
    html.m_style_tree->print(&listener);
}

UT_UTF8String IE_Exp_HTML::ConvertToClean(const UT_UTF8String &str)
{
    UT_UTF8String result = "";

    UT_UTF8Stringbuf::UTF8Iterator i = str.getIterator();
    i = i.start();


    if (i.current())
    {
        while (true)
        {
            const char *pCurrent = i.current();

            if (*pCurrent == 0)
            {
                break;
            }

            if (isalnum(*pCurrent) || (*pCurrent == '-') || (*pCurrent == '_'))
            {
                result += *pCurrent;
            }

            i.advance();
        }
    }
    return result;
}

UT_Error IE_Exp_HTML::_writeDocument(bool bClipBoard, bool bTemplateBody)
{
    UT_UTF8String basename = UT_go_basename(getFileName());
    const char* szSuffix = strchr(basename.utf8_str(), '.');

    if (szSuffix == NULL)
    {
        // If user wants, but this is a strange idea
        m_suffix = "";
    }
    else
    {
        m_suffix = szSuffix;
    }
    /**
     * We must check if user wants to split the document into several parts.
     * File will be splitted using level 1 toc elements, e.g. 'Heading 1' style
     */
    if (m_exp_opt.bSplitDocument && m_toc->hasTOC() && !m_exp_opt.bMultipart)
    {
        m_minTOCLevel = 10; // Hope this will be enough, see impl. of TOC_Helper

        for (int i = 0; i < m_toc->getNumTOCEntries(); i++)
        {
            int currentLevel = 10;
            m_toc->getNthTOCEntry(i, &currentLevel);
            if (currentLevel < m_minTOCLevel)
            {
                m_minTOCLevel = currentLevel;
                m_minTOCIndex = i;
            }
        }

        UT_DEBUGMSG(("Minimal TOC level is %d\n", m_minTOCLevel));

        s_HTML_Bookmark_Listener * bookmarkListener = new s_HTML_Bookmark_Listener(getDoc(), this);
        getDoc()->tellListener(bookmarkListener);
        m_bookmarks = bookmarkListener->getBookmarks();
        DELETEP(bookmarkListener);




        PT_DocPosition posBegin;
        PT_DocPosition posEnd; // End of the chapter
        PT_DocPosition posCurrent;
        PT_DocPosition docBegin;
        UT_UTF8String chapterTitle;
        UT_UTF8String currentTitle;
        int currentLevel = 0;
        bool firstChapter = true;

        getDoc()->getBounds(false, posEnd);
        docBegin = posEnd;
        posEnd = 0;
        currentTitle = m_toc->getNthTOCEntry(0, NULL);
        bool isIndex = true;
        for (int i = m_minTOCIndex; i < m_toc->getNumTOCEntries(); i++)
        {

            m_toc->getNthTOCEntry(i, &currentLevel);

            if (currentLevel == m_minTOCLevel)
            {
                chapterTitle = m_toc->getNthTOCEntry(i, NULL);
                m_toc->getNthTOCEntryPos(i, posCurrent);
                posBegin = posEnd;

                if (firstChapter)
                {

                    UT_DEBUGMSG(("POS: %d %d\n", posBegin, posCurrent));
                    if (posCurrent <= docBegin)
                    {
                        UT_DEBUGMSG(("Document is starting from a heading\n"));
                        isIndex = true;
                        continue;

                    }
                    firstChapter = false;

                }

                posEnd = posCurrent;
                PD_DocumentRange *range = new PD_DocumentRange(getDoc(), posBegin, posEnd);
                UT_DEBUGMSG(("POS: BEGIN %d END %d\n", posBegin, posEnd));
                UT_DEBUGMSG(("Now will create chapter of the document with title %s\n", currentTitle.utf8_str()));

                _createChapter(range, currentTitle, isIndex);
                if (isIndex)
                {
                    isIndex = false;
                }
                currentTitle = chapterTitle;
            }
        }

        posBegin = posEnd;
        getDoc()->getBounds(true, posEnd);

        if (posBegin != posEnd)
        {
            PD_DocumentRange *range = new PD_DocumentRange(getDoc(), posBegin, posEnd);
            _createChapter(range, chapterTitle, isIndex);
        }
        return UT_OK;
    }
    else
    {
        IE_Exp_HTML_MainListener * pListener = new IE_Exp_HTML_MainListener(getDoc(), this, bClipBoard, bTemplateBody,
                                                                            &m_exp_opt, m_style_tree,
                                                                            m_sLinkCSS, m_sTitle);
        if (pListener == 0) return UT_IE_NOMEMORY;

        PL_Listener * pL = static_cast<PL_Listener *> (pListener);

        bool okay = true;

        s_HTML_HdrFtr_Listener * pHdrFtrListener = new s_HTML_HdrFtr_Listener(getDoc(), this, pL);
        if (pHdrFtrListener == 0) return UT_IE_NOMEMORY;
        PL_Listener * pHFL = static_cast<PL_Listener *> (pHdrFtrListener);

        if (!bClipBoard)
        {
            okay = getDoc()->tellListener(pHFL);
            pHdrFtrListener->doHdrFtr(1);
        }

        if (bClipBoard)
        {
            okay = getDoc()->tellListenerSubset(pL, getDocRange());
        }
        else if (okay)
        {
            okay = getDoc()->tellListener(pL);
            if (okay) okay = pListener->endOfDocument();
        }

        if (!bClipBoard)
            pHdrFtrListener->doHdrFtr(0); // Emit footer

        DELETEP(pListener);
        DELETEP(pHdrFtrListener);

        if ((m_error == UT_OK) && (okay == true)) return UT_OK;
        return UT_IE_COULDNOTWRITE;
    }
}

void IE_Exp_HTML::_createChapter(PD_DocumentRange* range, UT_UTF8String &title, bool isIndex)
{
    IE_Exp_HTML_MainListener* pListener = NULL;
    pListener = new IE_Exp_HTML_MainListener(getDoc(), this, false,
                                             false, &m_exp_opt, m_style_tree, m_sLinkCSS, title,
                                             isIndex);

    PL_Listener * pL = static_cast<PL_Listener *> (pListener);
    /*s_HTML_HdrFtr_Listener * pHdrFtrListener = new s_HTML_HdrFtr_Listener(
                    getDoc(), this, pL);
    PL_Listener * pHFL = static_cast<PL_Listener *> (pHdrFtrListener);
     */
    bool ok;
    // getDoc()->tellListener(pHFL);
    // pHdrFtrListener->doHdrFtr(1);

    ok = getDoc()->tellListenerSubset(pListener, range);

    if (ok)
    {
        pListener->endOfDocument();
        // pHdrFtrListener->doHdrFtr(0);
    }
    else
    {
        UT_DEBUGMSG(("RUDYJ: Listener returned error!\n"));
    }
    DELETEP(range);
    // DELETEP(pHdrFtrListener);
    DELETEP(pListener);

}

UT_UTF8String IE_Exp_HTML::getBookmarkFilename(const UT_UTF8String &id)
{
    std::map<UT_UTF8String, UT_UTF8String>::iterator bookmarkIter = m_bookmarks.find(id);
    if (bookmarkIter != m_bookmarks.end())
    {
        UT_DEBUGMSG(("Found bookmark %s at file %s", id.utf8_str(), m_bookmarks[id].utf8_str()));
        return m_bookmarks[id];
    }
    else
    {
        return UT_UTF8String();
    }
}

UT_UTF8String IE_Exp_HTML::getFilenameByPosition(PT_DocPosition position)
{
    PT_DocPosition posCurrent;
    UT_UTF8String chapterFile = UT_go_basename(getFileName());

    if (m_toc->hasTOC())
    {
        for (int i = m_toc->getNumTOCEntries() - 1; i >= m_minTOCIndex; i--)
        {
            int currentLevel;
            m_toc->getNthTOCEntry(i, &currentLevel);
            m_toc->getNthTOCEntryPos(i, posCurrent);

            if (currentLevel == m_minTOCLevel)
            {
                if ((i != m_minTOCIndex) && (posCurrent <= position))
                {
                    chapterFile = IE_Exp_HTML::ConvertToClean(m_toc->getNthTOCEntry(i, NULL)) + m_suffix;
                    break;
                }
                else if ((i == m_minTOCIndex) && (posCurrent >= position))
                {
                    break;
                }
            }
        }
    }

    return (chapterFile);
}

UT_UTF8String IE_Exp_HTML::getSuffix() const
{
    return m_suffix;
}
