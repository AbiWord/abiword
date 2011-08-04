#ifndef IE_EXP_HTML_UTILLISTENERS_H
#define	IE_EXP_HTML_UTILLISTENERS_H

// HTML exporter includes
#include "ie_exp_HTML.h"

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
//class ABI_EXPORT IE_Exp_HTML_HeaderFooterListener : public PL_Listener
//{
//public:
//IE_Exp_HTML_HeaderFooterListener(PD_Document * pDocument, IE_Exp_HTML * pie, IE_Exp_HTML_Writer *pWriter, IE_Exp_HTML_DocumentWriter *pMainListener);
//
//~IE_Exp_HTML_HeaderFooterListener();
//
//bool populate(PL_StruxFmtHandle sfh,
//              const PX_ChangeRecord * pcr);
//
//bool populateStrux(PL_StruxDocHandle sdh,
//                   const PX_ChangeRecord * pcr,
//                   PL_StruxFmtHandle * psfh);
//
////See note in _writeDocument
////bool 	startOfDocument ();
//bool endOfDocument();
//
//bool change(PL_StruxFmtHandle sfh,
//            const PX_ChangeRecord * pcr);
//
//bool insertStrux(PL_StruxFmtHandle sfh,
//                 const PX_ChangeRecord * pcr,
//                 PL_StruxDocHandle sdh,
//                 PL_ListenerId lid,
//                 void (*pfnBindHandles) (PL_StruxDocHandle sdhNew,
//                 PL_ListenerId lid,
//                 PL_StruxFmtHandle sfhNew));
//
//bool signal(UT_uint32 iSignal);
//void doHdrFtr(bool bHeader);
//private:
//PD_DocumentRange * m_pHdrDocRange;
//PD_DocumentRange * m_pFtrDocRange;
//PD_Document * m_pDocument;
//IE_Exp_HTML_Writer *m_pWriter;
//IE_Exp_HTML_DocumentWriter *m_pMainListener;
//};
//
class ABI_EXPORT IE_Exp_HTML_BookmarkListener : public PL_Listener
{
public:
IE_Exp_HTML_BookmarkListener(PD_Document* pDoc, IE_Exp_HTML * pie);
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
#endif	/* IE_EXP_HTML_UTILLISTENERS_H */

