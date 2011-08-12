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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
* 02111-1307, USA.
*/

#include "ie_exp_HTML.h"

#define MYEOL "\n"
#define MAX_LINE_LEN 200


IE_Exp_HTML_Sniffer::IE_Exp_HTML_Sniffer()
        : IE_ExpSniffer(IE_IMPEXPNAME_HTML, true)
{

}

bool IE_Exp_HTML_Sniffer::recognizeSuffix(const gchar * szSuffix)
{
return (!(g_ascii_strcasecmp(szSuffix, ".xhtml")) ||
        !(g_ascii_strcasecmp(szSuffix, ".html")) ||
        !(g_ascii_strcasecmp(szSuffix, ".htm")) ||
        !(g_ascii_strcasecmp(szSuffix, ".mht")) ||
        !(g_ascii_strcasecmp(szSuffix, ".phtml")));
}

UT_Error IE_Exp_HTML_Sniffer::constructExporter(PD_Document * pDocument,
                                            IE_Exp ** ppie)
{
    IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
    *ppie = p;
    return UT_OK;
}

bool IE_Exp_HTML_Sniffer::getDlgLabels(const gchar ** pszDesc,
                                   const gchar ** pszSuffixList,
                                   IEFileType * ft)
{
    *pszDesc = "HTML/XHTML/PHTML/MHT (.html, .xhtml, .phtml, .mht)";
    *pszSuffixList = "*.html;*.xhtml;*.phtml;*.mht";
    *ft = getFileType();
    return true;
}

UT_Confidence_t IE_Exp_HTML_Sniffer::supportsMIME(const gchar * szMimeType)
{
if (!strcmp(szMimeType, IE_MIMETYPE_XHTML) ||
    !strcmp(szMimeType, "application/xhtml") ||
    !strcmp(szMimeType, "text/html"))
    return UT_CONFIDENCE_PERFECT;
return UT_CONFIDENCE_ZILCH;
}


/*****************************************************************/
/*****************************************************************/


class IE_Exp_HTML_DocumentWriter;
class IE_Exp_HTML_HeaderFooterListener;
class IE_Exp_HTML_BookmarkListener;



IE_Exp_HTML::IE_Exp_HTML(PD_Document * pDocument)
        : IE_Exp(pDocument),
        m_style_tree(new IE_Exp_HTML_StyleTree(pDocument)),
        m_styleListener(new IE_Exp_HTML_StyleListener(m_style_tree)),
        m_bSuppressDialog(false),
        m_suffix(""),
    m_pNavigationHelper(new IE_Exp_HTML_NavigationHelper(getDoc(), getFileName())),
	m_pWriterFactory( 
                     new IE_Exp_HTML_DefaultWriterFactory(pDocument,this->m_exp_opt))
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

    XAP_Dialog_HTMLOptions::getHTMLDefaults(&m_exp_opt, XAP_App::getApp());
}

IE_Exp_HTML::~IE_Exp_HTML()
{
    DELETEP(m_style_tree);
	DELETEP(m_pWriterFactory);
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
    return UT_OK;
}

UT_Error IE_Exp_HTML::_writeDocument()
{
    UT_UTF8String basename = UT_go_basename(getFileName());
    m_suffix = strchr(basename.utf8_str(), '.');
    UT_DEBUGMSG(("Determined suffix: %s", m_suffix.utf8_str()));
    
    if (!UT_go_utf8_collate_casefold(m_suffix.utf8_str(), ".html")||
        !UT_go_utf8_collate_casefold(m_suffix.utf8_str(), ".htm"))
    {
        set_HTML4();
    } else if (!UT_go_utf8_collate_casefold(m_suffix.utf8_str(), ".phtml"))
    {
        set_PHTML();
    } else if (!UT_go_utf8_collate_casefold(m_suffix.utf8_str(), ".mht"))
    {
        set_MHTML();
    }
    
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
                const gchar * fname = getDoc()->getFilename();
                if (fname)
                {
                    const gchar * base = UT_basename(fname);
                    UT_uint32 iNameLen = strlen(base);

                    const gchar * dot = strrchr(base, '.');
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
    declaration += s_DTD_XHTML;
    declaration += ">" MYEOL;

    write(declaration.utf8_str(), declaration.byteLength());

//    IE_Exp_HTML_TemplateHandler TH(getDoc(), this);

    //UT_XML parser;
    //parser.setExpertListener(&TH);

   // UT_Error err = parser.parse(prop.c_str());

    return UT_OK;
}

UT_Error IE_Exp_HTML::_writeDocument(bool bClipBoard, bool bTemplateBody)
{
    if (m_exp_opt.bSplitDocument && m_pNavigationHelper->hasTOC()
        && !m_exp_opt.bMultipart)
    {
        UT_DEBUGMSG(("Creating multiple file HTML document\n"));
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
        currentTitle = m_pNavigationHelper->getNthTOCEntry(0, NULL);
        bool isIndex = true;
        for (int i = m_pNavigationHelper->getMinTOCIndex();
            i < m_pNavigationHelper->getNumTOCEntries(); i++)
        {
            UT_DEBUGMSG(("MIN TOC LEVEL: %d", m_pNavigationHelper->getMinTOCLevel()));

            m_pNavigationHelper->getNthTOCEntry(i, &currentLevel);

            if (currentLevel == m_pNavigationHelper->getMinTOCLevel())
            {
                chapterTitle = m_pNavigationHelper->getNthTOCEntry(i, NULL);
                m_pNavigationHelper->getNthTOCEntryPos(i, posCurrent);
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
        if (m_exp_opt.bMultipart)
        {
            UT_DEBUGMSG(("Creating MHT document\n"));
            _createMultipart();
        } else
        {
            UT_DEBUGMSG(("Creating single-file HTML document\n"));
            _createChapter(NULL, "", true);
        }
    }

    return UT_OK;
}



void IE_Exp_HTML::_createChapter(PD_DocumentRange* range, const UT_UTF8String &title, 
    bool isIndex)
{
    UT_UTF8String filename;
    GsfOutput *output;
    if (isIndex)
    {
        output = getFp();
        filename = UT_go_basename_from_uri(getFileName());
    } else
    {
        filename = ConvertToClean(title) + m_suffix;
        UT_UTF8String outputUri = UT_go_dirname_from_uri(getFileName(), false);
        outputUri += G_DIR_SEPARATOR_S + filename;
        output = UT_go_file_create(outputUri.utf8_str(), NULL);
    }
    IE_Exp_HTML_OutputWriter *pOutputWriter = 
        new IE_Exp_HTML_FileWriter(output);
//	pOutputWriter->enableQuotedPrintable(m_exp_opt.bMultipart);
    
    IE_Exp_HTML_DataExporter* pDataExporter = 
        new IE_Exp_HTML_FileExporter(getDoc(), 
            getFileName());
    
    IE_Exp_HTML_DocumentWriter* pMainListener = 
		m_pWriterFactory->constructDocumentWriter(pOutputWriter);
    
    IE_Exp_HTML_Listener *pListener = new IE_Exp_HTML_Listener(getDoc(), 
        pDataExporter, m_style_tree, m_pNavigationHelper, pMainListener,
        filename);
    // Time to send some settings to listener
    pListener->set_SplitDocument(m_exp_opt.bSplitDocument);
    pListener->set_EmbedCSS(m_exp_opt.bEmbedCSS);
    pListener->set_RenderMathMLToPng(m_exp_opt.bMathMLRenderPNG);
    pListener->set_EmbedImages(m_exp_opt.bEmbedImages);
    
    IE_Exp_HTML_HeaderFooterListener *pHeaderFooterListener = new 
        IE_Exp_HTML_HeaderFooterListener(getDoc(), pMainListener,
        pListener);
    getDoc()->tellListener(pHeaderFooterListener);
    
    pHeaderFooterListener->doHdrFtr(true);
    
    if (range!= NULL)
    {
        getDoc()->tellListenerSubset(pListener, range);
    } else
    {
        getDoc()->tellListener(pListener);
    }
    pHeaderFooterListener->doHdrFtr(false);
    pListener->endOfDocument();
    
    m_mathmlFlags[filename] = pListener->get_HasMathML();
    
    DELETEP(pHeaderFooterListener);
    DELETEP(pListener);
    DELETEP(pMainListener);
    DELETEP(pDataExporter);
    DELETEP(pOutputWriter);
    
    if (!isIndex)
    {
        gsf_output_close(output);
    }
}

void IE_Exp_HTML::_createMultipart()
{
    
    UT_UTF8String buffer;
    UT_UTF8String title;
   
    IE_Exp_HTML_StringWriter *pOutputWriter = new IE_Exp_HTML_StringWriter();
    
      IE_Exp_HTML_MultipartExporter* pDataExporter = 
        new IE_Exp_HTML_MultipartExporter(getDoc(), 
            getFileName(), buffer, title);
    
    IE_Exp_HTML_DocumentWriter* pMainListener = 
		m_pWriterFactory->constructDocumentWriter(pOutputWriter);
    
    IE_Exp_HTML_Listener *pListener = new IE_Exp_HTML_Listener(getDoc(), 
        pDataExporter, m_style_tree, m_pNavigationHelper, pMainListener,
        getFileName());
    
    // Time to send some settings to listener
    pListener->set_EmbedCSS(m_exp_opt.bEmbedCSS);
    pListener->set_RenderMathMLToPng(m_exp_opt.bMathMLRenderPNG);
    
    IE_Exp_HTML_HeaderFooterListener *pHeaderFooterListener = new 
        IE_Exp_HTML_HeaderFooterListener(getDoc(), pMainListener,
        pListener);
    getDoc()->tellListener(pHeaderFooterListener);
    
    pHeaderFooterListener->doHdrFtr(true);
    

    getDoc()->tellListener(pListener);
 
    pHeaderFooterListener->doHdrFtr(false);
    pListener->endOfDocument();
    
    UT_UTF8String mime;
    if (m_exp_opt.bIs4)
    {
        mime = IE_MIMETYPE_HTML;
    } else
    {
        mime = IE_MIMETYPE_XHTML;
    }
     UT_UTF8String index = pOutputWriter->getString();
     UT_UTF8String header = pDataExporter->generateHeader(index, mime);
    
   
    write(header.utf8_str(), header.byteLength());
    buffer +="--";
    write(buffer.utf8_str(), buffer.byteLength());
    
    DELETEP(pHeaderFooterListener);
    DELETEP(pListener);
    DELETEP(pMainListener);
    DELETEP(pDataExporter);
    DELETEP(pOutputWriter);
}

void IE_Exp_HTML::setWriterFactory(IE_Exp_HTML_WriterFactory* pWriterFactory)
{
	if (m_pWriterFactory != NULL)
	{
		DELETEP(m_pWriterFactory);
	}
	
	if (pWriterFactory == NULL)
	{
		m_pWriterFactory = 
            new IE_Exp_HTML_DefaultWriterFactory(getDoc(),
                                                             this->m_exp_opt);
	} else
    {
        m_pWriterFactory = pWriterFactory;
    }
}

bool IE_Exp_HTML::hasMathML(const UT_UTF8String& file)
{
    if (m_mathmlFlags.find(file) != m_mathmlFlags.end())
    {
        return m_mathmlFlags[file];
    } else
    {
        return false;
    }
}