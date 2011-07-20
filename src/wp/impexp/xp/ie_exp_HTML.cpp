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

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <gsf/gsf-output.h>
#include <map>

#include "ut_locale.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_hash.h"
#include "ut_units.h"
#include "ut_wctomb.h"
#include "ut_path.h"
#include "ut_math.h"
#include "ut_misc.h"
#include "ut_string_class.h"
#include "ut_png.h"

#include "xap_App.h"
#include "xap_EncodingManager.h"

#include "pt_Types.h"
#include "pl_Listener.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "pp_PropertyMap.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "ut_mbtowc.h"
#include "xap_Frame.h"
#include "xav_View.h"
#include "gr_Graphics.h"

#include "fd_Field.h"

#include "fl_AutoNum.h"

#include "ie_types.h"
#include "ie_TOC.h"
#include "ie_impexp_HTML.h"
#include "ie_exp_HTML.h"
#include "ap_Strings.h"

#ifdef HTML_DIALOG_OPTIONS
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#endif

#ifdef HTML_TABLES_SUPPORTED
#include "ie_Table.h"
#endif

#define MYEOL "\n"
#define MAX_LINE_LEN 200


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

/* TODO: is there a better way to do this?
*/
#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML_StyleTree.h"
#include "ie_exp_HTML_MainListener.h"

class IE_Exp_HTML_MainListener;
class s_HTML_HdrFtr_Listener;
class s_HTML_Bookmark_Listener;



class ABI_EXPORT s_HTML_HdrFtr_Listener : public PL_Listener
{
friend class IE_Exp_HTML_MainListener;
public:
s_HTML_HdrFtr_Listener(PD_Document * pDocument, IE_Exp_HTML * pie, PL_Listener * pHTML_Listener);

~s_HTML_HdrFtr_Listener();

bool populate(PL_StruxFmtHandle sfh,
              const PX_ChangeRecord * pcr);

bool populateStrux(PL_StruxDocHandle sdh,
                   const PX_ChangeRecord * pcr,
                   PL_StruxFmtHandle * psfh);

//See note in _writeDocument
//bool 	startOfDocument ();
bool endOfDocument();

bool change(PL_StruxFmtHandle sfh,
            const PX_ChangeRecord * pcr);

bool insertStrux(PL_StruxFmtHandle sfh,
                 const PX_ChangeRecord * pcr,
                 PL_StruxDocHandle sdh,
                 PL_ListenerId lid,
                 void (*pfnBindHandles) (PL_StruxDocHandle sdhNew,
                 PL_ListenerId lid,
                 PL_StruxFmtHandle sfhNew));

bool signal(UT_uint32 iSignal);
void doHdrFtr(bool bHeader);
private:
PD_DocumentRange * m_pHdrDocRange;
PD_DocumentRange * m_pFtrDocRange;
PD_Document * m_pDocument;
PL_Listener * m_pHTML_Listener;
};

class ABI_EXPORT s_HTML_Bookmark_Listener : public PL_Listener
{
public:
s_HTML_Bookmark_Listener(PD_Document* pDoc, IE_Exp_HTML * pie);
bool populate(PL_StruxFmtHandle sfh,
              const PX_ChangeRecord * pcr);
// Not used

bool populateStrux(PL_StruxDocHandle /*sdh*/,
                   const PX_ChangeRecord * /*pcr*/,
                   PL_StruxFmtHandle * /*psfh*/)
{
    return true;
}
// Not used

bool change(PL_StruxFmtHandle /*sfh*/,
            const PX_ChangeRecord * /*pcr*/)
{
    return true;
}
// Not used

bool insertStrux(PL_StruxFmtHandle /*sfh*/,
                 const PX_ChangeRecord * /*pcr*/,
                 PL_StruxDocHandle /*sdh*/,
                 PL_ListenerId /*lid*/,
                 void (*/*pfnBindHandles*/) (PL_StruxDocHandle sdhNew,
                 PL_ListenerId lid,
                 PL_StruxFmtHandle sfhNew))
{
    return true;
}
// Not used

bool signal(UT_uint32 /*iSignal*/)
{
    return true;
}

inline std::map<UT_UTF8String, UT_UTF8String> getBookmarks() const
{
    return m_bookmarks;
}
private:
std::map<UT_UTF8String, UT_UTF8String> m_bookmarks;
PD_Document * m_pDoc;
IE_Exp_HTML * m_pie;

};

/*****************************************************************/

/*****************************************************************/

s_HTML_HdrFtr_Listener::s_HTML_HdrFtr_Listener(PD_Document * pDocument, IE_Exp_HTML * /*pie*/, PL_Listener * pHTML_Listener) :
        m_pHdrDocRange(NULL),
        m_pFtrDocRange(NULL),
        m_pDocument(pDocument),
        m_pHTML_Listener(pHTML_Listener)
{
}

s_HTML_HdrFtr_Listener::~s_HTML_HdrFtr_Listener()
{
}

void s_HTML_HdrFtr_Listener::doHdrFtr(bool bHeader)
{
    IE_Exp_HTML_MainListener * pHL = (IE_Exp_HTML_MainListener *) m_pHTML_Listener;
    if (bHeader && pHL->m_bHaveHeader)
    {
        pHL->_openSection(0, 1);
        m_pDocument->tellListenerSubset(m_pHTML_Listener, m_pHdrDocRange);
        pHL->_closeSection();
    }
    if (!bHeader && pHL->m_bHaveFooter)
    {
        pHL->_openSection(0, 2);
        m_pDocument->tellListenerSubset(m_pHTML_Listener, m_pFtrDocRange);
        pHL->_closeSection();
    }
    if (bHeader && pHL->m_bHaveHeader)
    {
        pHL->_openSection(0, 3);
    }
    if (bHeader)
        DELETEP(m_pHdrDocRange);
    else
        DELETEP(m_pFtrDocRange);
}

bool s_HTML_HdrFtr_Listener::populateStrux(PL_StruxDocHandle sdh,
                                           const PX_ChangeRecord * pcr,
                                           PL_StruxFmtHandle * psfh)
{
    /* Housekeeping and prep */
    UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
    *psfh = 0; // we don't need it.
    const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
    PT_AttrPropIndex api = pcr->getIndexAP();
    switch (pcrx->getStruxType())
    {
    case PTX_SectionHdrFtr:
    {
        const PP_AttrProp * pAP = 0;
        bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);

        if (!bHaveProp || (pAP == 0)) return true;

        const gchar * szType = 0;
        pAP->getAttribute("type", szType);
        /* // */

        PT_DocPosition m_iHdrFtrStartPos = m_pDocument->getStruxPosition(sdh) + 1;
        PT_DocPosition m_iHdrFtrStopPos = 0;
        PL_StruxDocHandle * nextSDH = NULL;
        bool bHaveNextSection = m_pDocument->getNextStruxOfType(sdh, PTX_Section, nextSDH);
        if (bHaveNextSection)
        {
            m_iHdrFtrStopPos = m_pDocument->getStruxPosition(nextSDH);
        }
        else
        {
            m_pDocument->getBounds(true, m_iHdrFtrStopPos);
        }
        PD_DocumentRange * pDocRange = new PD_DocumentRange(m_pDocument, m_iHdrFtrStartPos, m_iHdrFtrStopPos);
        if (!strcmp(szType, "header"))
        {
            m_pHdrDocRange = pDocRange;
            IE_Exp_HTML_MainListener * pHL = (IE_Exp_HTML_MainListener *) m_pHTML_Listener;
            pHL->setHaveHeader();
        }
        else
        {
            m_pFtrDocRange = pDocRange;
            IE_Exp_HTML_MainListener * pHL = (IE_Exp_HTML_MainListener *) m_pHTML_Listener;
            pHL->setHaveFooter();
        }
        return true;
    }
    default:
        return true;
    }
}

bool s_HTML_HdrFtr_Listener::populate(PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * /*pcr*/)
{
    return true;
}

bool s_HTML_HdrFtr_Listener::change(PL_StruxFmtHandle /*sfh*/,
                                    const PX_ChangeRecord * /*pcr*/)
{
    UT_ASSERT_HARMLESS(0); // this function is not used.
    return false;
}

bool s_HTML_HdrFtr_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
                                         const PX_ChangeRecord * /*pcr*/,
                                         PL_StruxDocHandle /*sdh*/,
                                         PL_ListenerId /* lid */,
                                         void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
                                         PL_ListenerId /* lid */,
                                         PL_StruxFmtHandle /* sfhNew */))
{
    UT_ASSERT_HARMLESS(0); // this function is not used.
    return false;
}

bool s_HTML_HdrFtr_Listener::signal(UT_uint32 /* iSignal */)
{
    UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    return false;
}

/*****************************************************************/
/*****************************************************************/

#ifdef TH_SKIP_REST
#undef TH_SKIP_REST
#endif
#define TH_SKIP_REST  1

#ifdef TH_SKIP_THIS
#undef TH_SKIP_THIS
#endif
#define TH_SKIP_THIS  2

class ABI_EXPORT s_TemplateHandler : public UT_XML::ExpertListener
{
public:
    s_TemplateHandler(PD_Document * pDocument, IE_Exp_HTML * pie);

    ~s_TemplateHandler();

    /* Implementation of ExpertListener
     */
    void StartElement(const gchar * name, const gchar ** atts);
    void EndElement(const gchar * name);
    void CharData(const gchar * buffer, int length);
    void ProcessingInstruction(const gchar * target, const gchar * data);
    void Comment(const gchar * data);
    void StartCdataSection();
    void EndCdataSection();
    void Default(const gchar * buffer, int length);

private:
    void _handleMetaTag(const char * key, UT_UTF8String & value);
    void _handleMeta();

    bool echo() const;
    bool condition(const gchar * data) const;

    PD_Document * m_pDocument;
    IE_Exp_HTML * m_pie;

    bool m_cdata;
    bool m_empty;

    UT_UTF8String m_utf8;
    UT_UTF8String m_root;
    typedef std::map<std::string, std::string> hash_type;
    hash_type m_hash;
    UT_NumberStack m_mode;
};

s_TemplateHandler::s_TemplateHandler(PD_Document * pDocument, IE_Exp_HTML * pie) :
        m_pDocument(pDocument),
        m_pie(pie),
        m_cdata(false),
        m_empty(false)
{
    const std::string & prop = m_pie->getProperty("href-prefix");
    if (!prop.empty())
        m_root = prop;
}

s_TemplateHandler::~s_TemplateHandler()
{
    // 
}

void s_TemplateHandler::StartElement(const gchar * name, const gchar ** atts)
{
    if (!echo()) return;

    if (m_empty)
    {
        m_pie->write(">", 1);
        m_empty = false;
    }

    m_utf8 = "<";
    m_utf8 += name;

    if (atts)
    {
        const gchar ** attr = atts;

        UT_UTF8String tmp;

        while (*attr)
        {
            bool href = ((strcmp(*attr, "href") == 0) ||
                    ((strcmp(*attr, "src") == 0) && (strcmp(name, "img") == 0)));

            m_utf8 += " ";
            m_utf8 += *attr++;
            m_utf8 += "=\"";

            if (href && (**attr == '$'))
            {
                tmp = m_root;
                tmp += (*attr++ +1);
            }
            else
            {
                tmp = *attr++;
            }
            tmp.escapeXML();

            m_utf8 += tmp;
            m_utf8 += "\"";
        }
    }
    m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());

    m_empty = true;
}

void s_TemplateHandler::EndElement(const gchar * name)
{
    if (!echo()) return;

    if (m_empty)
    {
        m_pie->write(" />", 3);
        m_empty = false;
    }
    else
    {
        m_utf8 = "</";
        m_utf8 += name;
        m_utf8 += ">";
        m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
    }
}

void s_TemplateHandler::CharData(const gchar * buffer, int length)
{
    if (!echo()) return;

    if (m_empty)
    {
        m_pie->write(">", 1);
        m_empty = false;
    }
    if (m_cdata)
    {
        m_pie->write(buffer, length);
        return;
    }
    m_utf8 = buffer;
    m_utf8.escapeXML();
    m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
}

void s_TemplateHandler::ProcessingInstruction(const gchar * target, const gchar * data)
{
    bool bAbiXHTML = (strncmp(target, "abi-xhtml-", 10) == 0);

    if (!bAbiXHTML && !echo()) return;

    if (m_empty)
    {
        m_pie->write(">", 1);
        m_empty = false;
    }
    if (!bAbiXHTML)
    {
        /* processing instruction not relevant to this - could be PHP, etc.
         */
        m_utf8 = "<?";
        m_utf8 += target;
        m_utf8 = " ";
        m_utf8 += data;
        m_utf8 = "?>";
        m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
        return;
    }
    m_utf8 = target + 10;

    if ((m_utf8 == "insert") && echo())
    {
        m_utf8 = data;

        if (m_utf8 == "title")
        {
#ifdef HTML_META_SUPPORTED
            m_utf8 = "";

            m_pDocument->getMetaDataProp(PD_META_KEY_TITLE, m_utf8);

            if (m_utf8.byteLength() == 0)
                m_utf8 = m_pie->getFileName();

            m_utf8.escapeXML();
            m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
#endif /* HTML_META_SUPPORTED */
        }
        else if (m_utf8 == "creator")
        {
#ifdef HTML_META_SUPPORTED
            m_utf8 = "";

            m_pDocument->getMetaDataProp(PD_META_KEY_CREATOR, m_utf8);

            if (m_utf8.byteLength())
            {
                m_utf8.escapeXML();
                m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
            }
#endif /* HTML_META_SUPPORTED */
        }
        else if (m_utf8 == "meta")
        {
#ifdef HTML_META_SUPPORTED
            _handleMeta();
#endif /* HTML_META_SUPPORTED */
        }
        else if (m_utf8 == "body")
        {
            m_pie->_writeDocument(false, true);
        }
    }
    else if ((m_utf8 == "comment-replace") && echo())
    {
        m_hash.clear();
        UT_parse_attributes(data, m_hash);

        const std::string & sz_property(m_hash["property"]);
        const std::string & sz_comment(m_hash["comment"]);

        if (sz_property.size() && sz_comment.size())
        {
#ifdef HTML_META_SUPPORTED
            UT_UTF8String creator = "";
#endif /* HTML_META_SUPPORTED */
            std::string prop;

            if (sz_property == "meta::creator")
            {
#ifdef HTML_META_SUPPORTED
                m_pDocument->getMetaDataProp(PD_META_KEY_CREATOR, creator);

                if (creator.byteLength())
                    prop = creator.utf8_str();
#endif /* HTML_META_SUPPORTED */
            }
            else
            {
                prop = m_pie->getProperty(sz_property.c_str());
            }
            if (!prop.empty())
            {
                const UT_UTF8String DD("$$");

                m_utf8 = sz_comment.c_str();
                m_utf8.escape(DD, prop.c_str());

                m_pie->write("<!--", 4);
                m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
                m_pie->write("-->", 3);
            }
        }
    }
    else if ((m_utf8 == "menuitem") && echo())
    {
        m_hash.clear();
        UT_parse_attributes(data, m_hash);

        const std::string sz_property = m_hash["property"];
        const std::string sz_class = m_hash["class"];
        std::string sz_href = m_hash["href"];
        const std::string sz_label = m_hash["label"];

        if (sz_property.size() && sz_class.size()
            && sz_href.size() && sz_label.size())
        {
            const char * href = sz_href.c_str();

            if (*href == '$')
            {
                m_utf8 = m_root;
                m_utf8 += href + 1;

                m_hash["href"] = m_utf8.utf8_str();

                sz_href = m_utf8.utf8_str();
            }

            const std::string & prop = m_pie->getProperty(sz_property.c_str());

            bool ne = (prop != sz_class);

            m_utf8 = "<td class=\"";
            m_utf8 += sz_class;
            if (ne)
            {
                m_utf8 += "\"><a href=\"";
                m_utf8 += sz_href;
            }
            m_utf8 += "\"><div>";
            m_utf8 += sz_label;
            m_utf8 += "</div>";
            if (ne)
            {
                m_utf8 += "</a>";
            }
            m_utf8 += "</td>";

            m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
        }
    }
    else if (m_utf8 == "if")
    {
        if (echo())
        {
            if (condition(data))
                m_mode.push(0);
            else
                m_mode.push(TH_SKIP_THIS);
        }
        else
        {
            m_mode.push(TH_SKIP_REST);
        }
    }
    else if (m_mode.getDepth())
    {
        UT_sint32 mode;
        m_mode.viewTop(mode);

        if (m_utf8 == "elif")
        {
            if (mode == TH_SKIP_THIS)
            {
                if (condition(data))
                {
                    m_mode.pop();
                    m_mode.push(0);
                }
            }
            else if (mode != TH_SKIP_REST)
            {
                m_mode.pop();
                m_mode.push(TH_SKIP_REST);
            }
        }
        else if (m_utf8 == "else")
        {
            if (mode == TH_SKIP_THIS)
            {
                if (condition(data))
                {
                    m_mode.pop();
                    m_mode.push(0);
                }
            }
            else if (mode != TH_SKIP_REST)
            {
                m_mode.pop();
                m_mode.push(TH_SKIP_REST);
            }
        }
        else if (m_utf8 == "fi")
        {
            m_mode.pop();
        }
    }
}

#ifdef HTML_META_SUPPORTED

void s_TemplateHandler::_handleMetaTag(const char * key, UT_UTF8String & value)
{
    m_utf8 = "<meta name=\"";
    m_utf8 += key;
    m_utf8 += "\" content=\"";
    m_utf8 += value.escapeXML();
    m_utf8 += "\" />";
    m_utf8 += MYEOL;
    m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
}

void s_TemplateHandler::_handleMeta()
{
    UT_UTF8String metaProp = "<meta http-equiv=\"content-type\" content=\"text/html;charset=UTF-8\" />" MYEOL;

    m_pie->write(metaProp.utf8_str(), metaProp.byteLength());

    if (!m_pie->isCopying())
    {

        if (m_pDocument->getMetaDataProp(PD_META_KEY_CREATOR, metaProp) && metaProp.size())
            _handleMetaTag("Author", metaProp);

        if (m_pDocument->getMetaDataProp(PD_META_KEY_KEYWORDS, metaProp) && metaProp.size())
            _handleMetaTag("Keywords", metaProp);

        if (m_pDocument->getMetaDataProp(PD_META_KEY_SUBJECT, metaProp) && metaProp.size())
            _handleMetaTag("Subject", metaProp);
    }
}

#endif /* HTML_META_SUPPORTED */

bool s_TemplateHandler::echo() const
{
    if (!m_mode.getDepth())
        return true;

    UT_sint32 mode;
    m_mode.viewTop(mode);

    return (mode == 0);
}

bool s_TemplateHandler::condition(const gchar * data) const
{
    const char * eq = strstr(data, "==");
    const char * ne = strstr(data, "!=");

    if (!eq && !ne)
        return false;
    if (eq && ne)
    {
        if (eq < ne)
            ne = 0;
        else
            eq = 0;
    }

    UT_UTF8String var;
    const char * value = NULL;

    if (eq)
    {
        var.assign(data, eq - data);
        value = eq + 2;
    }
    else
    {
        var.assign(data, ne - data);
        value = ne + 2;
    }
    const std::string & prop = m_pie->getProperty(var.utf8_str());

    bool match;

    match = (prop == value);

    return (eq ? match : !match);
}

void s_TemplateHandler::Comment(const gchar * data)
{
    if (!echo()) return;

    if (m_empty)
    {
        m_pie->write(">", 1);
        m_empty = false;
    }
    m_pie->write("<!--", 4);
    m_pie->write(data, strlen(data));
    m_pie->write("-->", 3);
}

void s_TemplateHandler::StartCdataSection()
{
    if (!echo()) return;

    if (m_empty)
    {
        m_pie->write(">", 1);
        m_empty = false;
    }
    m_pie->write("<![CDATA[", 9);
    m_cdata = true;
}

void s_TemplateHandler::EndCdataSection()
{
    if (!echo()) return;

    if (m_empty)
    {
        m_pie->write(">", 1);
        m_empty = false;
    }
    m_pie->write("]]>", 3);
    m_cdata = false;
}

void s_TemplateHandler::Default(const gchar * /*buffer*/, int /*length*/)
{
    // do nothing
}

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
    declaration += s_DTD_XHTML;
    declaration += ">" MYEOL;

    write(declaration.utf8_str(), declaration.byteLength());

    s_TemplateHandler TH(getDoc(), this);

    UT_XML parser;
    parser.setExpertListener(&TH);

    UT_Error err = parser.parse(prop.c_str());

    return err;
}

void IE_Exp_HTML::printStyleTree(PD_Document *pDocument, UT_ByteBuf & sink)
{
    IE_Exp_HTML html(pDocument);
    html._buildStyleTree();

    StyleListener listener(sink);
    html.m_style_tree->print(&listener);
}

UT_UTF8String IE_Exp_HTML::ConvertToClean(const UT_UTF8String & str)
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

UT_UTF8String IE_Exp_HTML::getBookmarkFilename(const UT_UTF8String & id)
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

void IE_Exp_HTML_MainListener::addFootnote(PD_DocumentRange * pDocRange)
{
    m_vecFootnotes.addItem(pDocRange);
}

void IE_Exp_HTML_MainListener::addEndnote(PD_DocumentRange * pDocRange)
{
    m_vecEndnotes.addItem(pDocRange);
}

void IE_Exp_HTML_MainListener::addAnnotation(PD_DocumentRange * pDocRange)
{
    m_vecAnnotations.addItem(pDocRange);
}

UT_uint32 IE_Exp_HTML_MainListener::getNumAnnotations(void)
{
    return m_vecAnnotations.getItemCount();
}

UT_uint32 IE_Exp_HTML_MainListener::getNumFootnotes(void)
{
    return m_vecFootnotes.getItemCount();
}

UT_uint32 IE_Exp_HTML_MainListener::getNumEndnotes(void)
{
    return m_vecEndnotes.getItemCount();
}

void IE_Exp_HTML_MainListener::setHaveHeader()
{
    m_bHaveHeader = true;
}

void IE_Exp_HTML_MainListener::setHaveFooter()
{
    m_bHaveFooter = true;
}

void IE_Exp_HTML_MainListener::_write(const char *data, UT_uint32 size)
{
    if (m_bIndexFile)
    {
        m_pie->write(data, size);
    }
    else
    {
        gsf_output_write(m_outputFile, size, reinterpret_cast<const guint8*> (data));
    }
}

/**
 *
 */

s_HTML_Bookmark_Listener::s_HTML_Bookmark_Listener(PD_Document *pDoc, IE_Exp_HTML * pie) :
        m_pDoc(pDoc),
        m_pie(pie)

{

}

bool s_HTML_Bookmark_Listener::populate(PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * pcr)
{
    switch (pcr->getType())
    {
    case PX_ChangeRecord::PXT_InsertObject:
    {
        const PX_ChangeRecord_Object * pcro = 0;
        pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
        PT_AttrPropIndex api = pcr->getIndexAP();

        switch (pcro->getObjectType())
        {
        case PTO_Bookmark:
        {
            const PP_AttrProp * pAP = 0;
            bool bHaveProp = (api ? (m_pDoc->getAttrProp(api, &pAP)) : false);

            if (!bHaveProp || (pAP == 0))
                return true;

            const gchar * szType = 0;
            pAP->getAttribute("type", szType);

            if (szType == 0)
                return true; // ??

            if (g_ascii_strcasecmp(szType, "start") == 0)
            {
                const gchar * szName = 0;
                pAP->getAttribute("name", szName);

                if (szName)
                {
                    UT_UTF8String escape = szName;
                    escape.escapeURL();
                    m_bookmarks[escape] = m_pie->getFilenameByPosition(pcr->getPosition());
                    UT_DEBUGMSG(("Added bookmark\n File: %s Id: %s\n", m_bookmarks[escape].utf8_str(), escape.utf8_str()));
                }
            }
            return true;
        }

        default:
            return true;
        }
    }
    default:
        return true;
    }
}
