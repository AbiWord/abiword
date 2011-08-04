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

#include "ie_exp_HTML_UtilListeners.h"
//
//
//
///*****************************************************************/
//
///*****************************************************************/
//
//IE_Exp_HTML_HeaderFooterListener::IE_Exp_HTML_HeaderFooterListener(PD_Document * pDocument, IE_Exp_HTML * /*pie*/, IE_Exp_HTML_Writer* pWriter, IE_Exp_HTML_DocumentWriter *pMainListener) :
//        m_pHdrDocRange(NULL),
//        m_pFtrDocRange(NULL),
//        m_pDocument(pDocument),
//        m_pWriter(pWriter),
//        m_pMainListener(pMainListener)
//{
//}
//
//IE_Exp_HTML_HeaderFooterListener::~IE_Exp_HTML_HeaderFooterListener()
//{
//}
//
//void IE_Exp_HTML_HeaderFooterListener::doHdrFtr(bool bHeader)
//{
//    if (bHeader && m_pWriter->is_HaveHeader())
//    {
//        m_pWriter->openSection(0, 1);
//        m_pDocument->tellListenerSubset(m_pMainListener, m_pHdrDocRange);
//        m_pWriter->closeSection();
//    }
//    if (!bHeader && m_pWriter->is_HaveFooter())
//    {
//        m_pWriter->openSection(0, 2);
//        m_pDocument->tellListenerSubset(m_pMainListener, m_pFtrDocRange);
//        m_pWriter->closeSection();
//    }
//    if (bHeader && m_pWriter->is_HaveHeader())
//    {
//        m_pWriter->openSection(0, 3);
//    }
//    if (bHeader)
//        DELETEP(m_pHdrDocRange);
//    else
//        DELETEP(m_pFtrDocRange);
//}
//
//bool IE_Exp_HTML_HeaderFooterListener::populateStrux(PL_StruxDocHandle sdh,
//                                           const PX_ChangeRecord * pcr,
//                                           PL_StruxFmtHandle * psfh)
//{
//    /* Housekeeping and prep */
//    UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
//    *psfh = 0; // we don't need it.
//    const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
//    PT_AttrPropIndex api = pcr->getIndexAP();
//    switch (pcrx->getStruxType())
//    {
//    case PTX_SectionHdrFtr:
//    {
//        const PP_AttrProp * pAP = 0;
//        bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);
//
//        if (!bHaveProp || (pAP == 0)) return true;
//
//        const gchar * szType = 0;
//        pAP->getAttribute("type", szType);
//        /* // */
//
//        PT_DocPosition m_iHdrFtrStartPos = m_pDocument->getStruxPosition(sdh) + 1;
//        PT_DocPosition m_iHdrFtrStopPos = 0;
//        PL_StruxDocHandle * nextSDH = NULL;
//        bool bHaveNextSection = m_pDocument->getNextStruxOfType(sdh, PTX_Section, nextSDH);
//        if (bHaveNextSection)
//        {
//            m_iHdrFtrStopPos = m_pDocument->getStruxPosition(nextSDH);
//        }
//        else
//        {
//            m_pDocument->getBounds(true, m_iHdrFtrStopPos);
//        }
//        PD_DocumentRange * pDocRange = new PD_DocumentRange(m_pDocument, m_iHdrFtrStartPos, m_iHdrFtrStopPos);
//        if (!strcmp(szType, "header"))
//        {
//            m_pHdrDocRange = pDocRange;
//            m_pWriter->setHaveHeader();
//        }
//        else
//        {
//            m_pFtrDocRange = pDocRange;
//            m_pWriter->setHaveFooter();
//        }
//        return true;
//    }
//    default:
//        return true;
//    }
//}
//
//bool IE_Exp_HTML_HeaderFooterListener::populate(PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * /*pcr*/)
//{
//    return true;
//}
//
//bool IE_Exp_HTML_HeaderFooterListener::change(PL_StruxFmtHandle /*sfh*/,
//                                    const PX_ChangeRecord * /*pcr*/)
//{
//    UT_ASSERT_HARMLESS(0); // this function is not used.
//    return false;
//}
//
//bool IE_Exp_HTML_HeaderFooterListener::insertStrux(PL_StruxFmtHandle /*sfh*/,
//                                         const PX_ChangeRecord * /*pcr*/,
//                                         PL_StruxDocHandle /*sdh*/,
//                                         PL_ListenerId /* lid */,
//                                         void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
//                                         PL_ListenerId /* lid */,
//                                         PL_StruxFmtHandle /* sfhNew */))
//{
//    UT_ASSERT_HARMLESS(0); // this function is not used.
//    return false;
//}
//
//bool IE_Exp_HTML_HeaderFooterListener::signal(UT_uint32 /* iSignal */)
//{
//    UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
//    return false;
//}
//
///*****************************************************************/
///*****************************************************************/
//
//
//
//IE_Exp_HTML_TemplateHandler::IE_Exp_HTML_TemplateHandler(PD_Document * pDocument, IE_Exp_HTML * pie) :
//        m_pDocument(pDocument),
//        m_pie(pie),
//        m_cdata(false),
//        m_empty(false)
//{
//    const std::string & prop = m_pie->getProperty("href-prefix");
//    if (!prop.empty())
//        m_root = prop;
//}
//
//IE_Exp_HTML_TemplateHandler::~IE_Exp_HTML_TemplateHandler()
//{
//    // 
//}
//
//void IE_Exp_HTML_TemplateHandler::StartElement(const gchar * name, const gchar ** atts)
//{
//    if (!echo()) return;
//
//    if (m_empty)
//    {
//        m_pie->write(">", 1);
//        m_empty = false;
//    }
//
//    m_utf8 = "<";
//    m_utf8 += name;
//
//    if (atts)
//    {
//        const gchar ** attr = atts;
//
//        UT_UTF8String tmp;
//
//        while (*attr)
//        {
//            bool href = ((strcmp(*attr, "href") == 0) ||
//                    ((strcmp(*attr, "src") == 0) && (strcmp(name, "img") == 0)));
//
//            m_utf8 += " ";
//            m_utf8 += *attr++;
//            m_utf8 += "=\"";
//
//            if (href && (**attr == '$'))
//            {
//                tmp = m_root;
//                tmp += (*attr++ +1);
//            }
//            else
//            {
//                tmp = *attr++;
//            }
//            tmp.escapeXML();
//
//            m_utf8 += tmp;
//            m_utf8 += "\"";
//        }
//    }
//    m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
//
//    m_empty = true;
//}
//
//void IE_Exp_HTML_TemplateHandler::EndElement(const gchar * name)
//{
//    if (!echo()) return;
//
//    if (m_empty)
//    {
//        m_pie->write(" />", 3);
//        m_empty = false;
//    }
//    else
//    {
//        m_utf8 = "</";
//        m_utf8 += name;
//        m_utf8 += ">";
//        m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
//    }
//}
//
//void IE_Exp_HTML_TemplateHandler::CharData(const gchar * buffer, int length)
//{
//    if (!echo()) return;
//
//    if (m_empty)
//    {
//        m_pie->write(">", 1);
//        m_empty = false;
//    }
//    if (m_cdata)
//    {
//        m_pie->write(buffer, length);
//        return;
//    }
//    m_utf8 = buffer;
//    m_utf8.escapeXML();
//    m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
//}
//
//void IE_Exp_HTML_TemplateHandler::ProcessingInstruction(const gchar * target, const gchar * data)
//{
//    bool bAbiXHTML = (strncmp(target, "abi-xhtml-", 10) == 0);
//
//    if (!bAbiXHTML && !echo()) return;
//
//    if (m_empty)
//    {
//        m_pie->write(">", 1);
//        m_empty = false;
//    }
//    if (!bAbiXHTML)
//    {
//        /* processing instruction not relevant to this - could be PHP, etc.
//         */
//        m_utf8 = "<?";
//        m_utf8 += target;
//        m_utf8 = " ";
//        m_utf8 += data;
//        m_utf8 = "?>";
//        m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
//        return;
//    }
//    m_utf8 = target + 10;
//
//    if ((m_utf8 == "insert") && echo())
//    {
//        m_utf8 = data;
//
//        if (m_utf8 == "title")
//        {
//#ifdef HTML_META_SUPPORTED
//            m_utf8 = "";
//
//            m_pDocument->getMetaDataProp(PD_META_KEY_TITLE, m_utf8);
//
//            if (m_utf8.byteLength() == 0)
//                m_utf8 = m_pie->getFileName();
//
//            m_utf8.escapeXML();
//            m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
//#endif /* HTML_META_SUPPORTED */
//        }
//        else if (m_utf8 == "creator")
//        {
//#ifdef HTML_META_SUPPORTED
//            m_utf8 = "";
//
//            m_pDocument->getMetaDataProp(PD_META_KEY_CREATOR, m_utf8);
//
//            if (m_utf8.byteLength())
//            {
//                m_utf8.escapeXML();
//                m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
//            }
//#endif /* HTML_META_SUPPORTED */
//        }
//        else if (m_utf8 == "meta")
//        {
//#ifdef HTML_META_SUPPORTED
//            _handleMeta();
//#endif /* HTML_META_SUPPORTED */
//        }
//        else if (m_utf8 == "body")
//        {
//            m_pie->_writeDocument(false, true);
//        }
//    }
//    else if ((m_utf8 == "comment-replace") && echo())
//    {
//        m_hash.clear();
//        UT_parse_attributes(data, m_hash);
//
//        const std::string & sz_property(m_hash["property"]);
//        const std::string & sz_comment(m_hash["comment"]);
//
//        if (sz_property.size() && sz_comment.size())
//        {
//#ifdef HTML_META_SUPPORTED
//            UT_UTF8String creator = "";
//#endif /* HTML_META_SUPPORTED */
//            std::string prop;
//
//            if (sz_property == "meta::creator")
//            {
//#ifdef HTML_META_SUPPORTED
//                m_pDocument->getMetaDataProp(PD_META_KEY_CREATOR, creator);
//
//                if (creator.byteLength())
//                    prop = creator.utf8_str();
//#endif /* HTML_META_SUPPORTED */
//            }
//            else
//            {
//                prop = m_pie->getProperty(sz_property.c_str());
//            }
//            if (!prop.empty())
//            {
//                const UT_UTF8String DD("$$");
//
//                m_utf8 = sz_comment.c_str();
//                m_utf8.escape(DD, prop.c_str());
//
//                m_pie->write("<!--", 4);
//                m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
//                m_pie->write("-->", 3);
//            }
//        }
//    }
//    else if ((m_utf8 == "menuitem") && echo())
//    {
//        m_hash.clear();
//        UT_parse_attributes(data, m_hash);
//
//        const std::string sz_property = m_hash["property"];
//        const std::string sz_class = m_hash["class"];
//        std::string sz_href = m_hash["href"];
//        const std::string sz_label = m_hash["label"];
//
//        if (sz_property.size() && sz_class.size()
//            && sz_href.size() && sz_label.size())
//        {
//            const gchar * href = sz_href.c_str();
//
//            if (*href == '$')
//            {
//                m_utf8 = m_root;
//                m_utf8 += href + 1;
//
//                m_hash["href"] = m_utf8.utf8_str();
//
//                sz_href = m_utf8.utf8_str();
//            }
//
//            const std::string & prop = m_pie->getProperty(sz_property.c_str());
//
//            bool ne = (prop != sz_class);
//
//            m_utf8 = "<td class=\"";
//            m_utf8 += sz_class;
//            if (ne)
//            {
//                m_utf8 += "\"><a href=\"";
//                m_utf8 += sz_href;
//            }
//            m_utf8 += "\"><div>";
//            m_utf8 += sz_label;
//            m_utf8 += "</div>";
//            if (ne)
//            {
//                m_utf8 += "</a>";
//            }
//            m_utf8 += "</td>";
//
//            m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
//        }
//    }
//    else if (m_utf8 == "if")
//    {
//        if (echo())
//        {
//            if (condition(data))
//                m_mode.push(0);
//            else
//                m_mode.push(TH_SKIP_THIS);
//        }
//        else
//        {
//            m_mode.push(TH_SKIP_REST);
//        }
//    }
//    else if (m_mode.getDepth())
//    {
//        UT_sint32 mode;
//        m_mode.viewTop(mode);
//
//        if (m_utf8 == "elif")
//        {
//            if (mode == TH_SKIP_THIS)
//            {
//                if (condition(data))
//                {
//                    m_mode.pop();
//                    m_mode.push(0);
//                }
//            }
//            else if (mode != TH_SKIP_REST)
//            {
//                m_mode.pop();
//                m_mode.push(TH_SKIP_REST);
//            }
//        }
//        else if (m_utf8 == "else")
//        {
//            if (mode == TH_SKIP_THIS)
//            {
//                if (condition(data))
//                {
//                    m_mode.pop();
//                    m_mode.push(0);
//                }
//            }
//            else if (mode != TH_SKIP_REST)
//            {
//                m_mode.pop();
//                m_mode.push(TH_SKIP_REST);
//            }
//        }
//        else if (m_utf8 == "fi")
//        {
//            m_mode.pop();
//        }
//    }
//}
//
//#ifdef HTML_META_SUPPORTED
//
//void IE_Exp_HTML_TemplateHandler::_handleMetaTag(const gchar * key, UT_UTF8String & value)
//{
//    m_utf8 = "<meta name=\"";
//    m_utf8 += key;
//    m_utf8 += "\" content=\"";
//    m_utf8 += value.escapeXML();
//    m_utf8 += "\" />";
//    m_utf8 += MYEOL;
//    m_pie->write(m_utf8.utf8_str(), m_utf8.byteLength());
//}
//
//void IE_Exp_HTML_TemplateHandler::_handleMeta()
//{
//    UT_UTF8String metaProp = "<meta http-equiv=\"content-type\" content=\"text/html;charset=UTF-8\" />" MYEOL;
//
//    m_pie->write(metaProp.utf8_str(), metaProp.byteLength());
//
//    if (!m_pie->isCopying())
//    {
//
//        if (m_pDocument->getMetaDataProp(PD_META_KEY_CREATOR, metaProp) && metaProp.size())
//            _handleMetaTag("Author", metaProp);
//
//        if (m_pDocument->getMetaDataProp(PD_META_KEY_KEYWORDS, metaProp) && metaProp.size())
//            _handleMetaTag("Keywords", metaProp);
//
//        if (m_pDocument->getMetaDataProp(PD_META_KEY_SUBJECT, metaProp) && metaProp.size())
//            _handleMetaTag("Subject", metaProp);
//    }
//}
//
//#endif /* HTML_META_SUPPORTED */
//
//bool IE_Exp_HTML_TemplateHandler::echo() const
//{
//    if (!m_mode.getDepth())
//        return true;
//
//    UT_sint32 mode;
//    m_mode.viewTop(mode);
//
//    return (mode == 0);
//}
//
//bool IE_Exp_HTML_TemplateHandler::condition(const gchar * data) const
//{
//    const gchar * eq = strstr(data, "==");
//    const gchar * ne = strstr(data, "!=");
//
//    if (!eq && !ne)
//        return false;
//    if (eq && ne)
//    {
//        if (eq < ne)
//            ne = 0;
//        else
//            eq = 0;
//    }
//
//    UT_UTF8String var;
//    const gchar * value = NULL;
//
//    if (eq)
//    {
//        var.assign(data, eq - data);
//        value = eq + 2;
//    }
//    else
//    {
//        var.assign(data, ne - data);
//        value = ne + 2;
//    }
//    const std::string & prop = m_pie->getProperty(var.utf8_str());
//
//    bool match;
//
//    match = (prop == value);
//
//    return (eq ? match : !match);
//}
//
//void IE_Exp_HTML_TemplateHandler::Comment(const gchar * data)
//{
//    if (!echo()) return;
//
//    if (m_empty)
//    {
//        m_pie->write(">", 1);
//        m_empty = false;
//    }
//    m_pie->write("<!--", 4);
//    m_pie->write(data, strlen(data));
//    m_pie->write("-->", 3);
//}
//
//void IE_Exp_HTML_TemplateHandler::StartCdataSection()
//{
//    if (!echo()) return;
//
//    if (m_empty)
//    {
//        m_pie->write(">", 1);
//        m_empty = false;
//    }
//    m_pie->write("<![CDATA[", 9);
//    m_cdata = true;
//}
//
//void IE_Exp_HTML_TemplateHandler::EndCdataSection()
//{
//    if (!echo()) return;
//
//    if (m_empty)
//    {
//        m_pie->write(">", 1);
//        m_empty = false;
//    }
//    m_pie->write("]]>", 3);
//    m_cdata = false;
//}
//
//void IE_Exp_HTML_TemplateHandler::Default(const gchar * /*buffer*/, int /*length*/)
//{
//    // do nothing
//}


IE_Exp_HTML_BookmarkListener::IE_Exp_HTML_BookmarkListener(PD_Document *pDoc, IE_Exp_HTML * pie) :
        m_pDoc(pDoc),
        m_pie(pie)

{

}

bool IE_Exp_HTML_BookmarkListener::populate(PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * pcr)
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
