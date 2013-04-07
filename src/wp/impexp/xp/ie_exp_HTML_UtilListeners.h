/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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
#ifndef IE_EXP_HTML_UTILLISTENERS_H
#define	IE_EXP_HTML_UTILLISTENERS_H

// HTML exporter includes
#include "ie_exp_HTML.h"
#include "ie_exp_HTML_Listener.h"
#include "ie_exp_HTML_DocumentWriter.h"


// Abiword includes
#include <pd_Document.h>
#include <pl_Listener.h>

//#include "ie_exp_HTML_MainListener.h"
//#include "ie_exp_HTML_Writer.h"
//
//class IE_Exp_HTML_DocumentWriter;
//
//class ABI_EXPORT IE_Exp_HTML_TemplateHandler : public UT_XML::ExpertListener
//{
//public:
//    IE_Exp_HTML_TemplateHandler(PD_Document * pDocument, IE_Exp_HTML * pie);
//
//    ~IE_Exp_HTML_TemplateHandler();
//
//    /* Implementation of ExpertListener
//     */
//    void StartElement(const gchar * name, const gchar ** atts);
//    void EndElement(const gchar * name);
//    void CharData(const gchar * buffer, int length);
//    void ProcessingInstruction(const gchar * target, const gchar * data);
//    void Comment(const gchar * data);
//    void StartCdataSection();
//    void EndCdataSection();
//    void Default(const gchar * buffer, int length);
//
//private:
//    void _handleMetaTag(const gchar * key, UT_UTF8String & value);
//    void _handleMeta();
//
//    bool echo() const;
//    bool condition(const gchar * data) const;
//
//    PD_Document * m_pDocument;
//    IE_Exp_HTML * m_pie;
//
//    bool m_cdata;
//    bool m_empty;
//
//    UT_UTF8String m_utf8;
//    UT_UTF8String m_root;
//    typedef std::map<std::string, std::string> hash_type;
//    hash_type m_hash;
//    UT_NumberStack m_mode;
//};
//

class IE_Exp_HTML_DocumentWriter;
class IE_Exp_HTML_Listener;
class ABI_EXPORT IE_Exp_HTML_HeaderFooterListener : public PL_Listener {
public:
    IE_Exp_HTML_HeaderFooterListener(PD_Document * pDocument,
            IE_Exp_HTML_DocumentWriter *pDocumentWriter,
            IE_Exp_HTML_Listener *pListener);

    ~IE_Exp_HTML_HeaderFooterListener();

    bool populate(fl_ContainerLayout* sfh,
            const PX_ChangeRecord * pcr);

    bool populateStrux(pf_Frag_Strux* sdh,
            const PX_ChangeRecord * pcr,
            fl_ContainerLayout* * psfh);

    //See note in _writeDocument
    //bool 	startOfDocument ();
    bool endOfDocument();

    bool change(fl_ContainerLayout* sfh,
            const PX_ChangeRecord * pcr);

    bool insertStrux(fl_ContainerLayout* sfh,
            const PX_ChangeRecord * pcr,
            pf_Frag_Strux* sdh,
            PL_ListenerId lid,
            void (*pfnBindHandles) (pf_Frag_Strux* sdhNew,
            PL_ListenerId lid,
            fl_ContainerLayout* sfhNew));

    bool signal(UT_uint32 iSignal);
    void doHdrFtr(bool bHeader);
private:
    PD_DocumentRange * m_pHdrDocRange;
    PD_DocumentRange * m_pFtrDocRange;
    PD_Document * m_pDocument;
    IE_Exp_HTML_DocumentWriter *m_pDocumentWriter;
    IE_Exp_HTML_Listener *m_pListener;

    bool m_bHaveHeader;
    bool m_bHaveFooter;
};

#endif	/* IE_EXP_HTML_UTILLISTENERS_H */

