#ifndef IE_EXP_HTML_LISTENER_H
#define	IE_EXP_HTML_LISTENER_H

#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML_NavigationHelper.h"
#include "ie_exp_HTML_StyleTree.h"

// External includes
#include <vector>

// ABiword includes
#include <pd_Document.h>
#include <pl_Listener.h>
#include <px_ChangeRecord.h>
#include <px_CR_Span.h>
#include <px_CR_Strux.h>
#include <px_CR_Object.h>
#include <fd_Field.h>
#include <fl_TOCLayout.h>
#include <ie_Table.h>
#include <ie_TOC.h>

/**
 * Listener for the {X,P}HTML and MHT document generators. Contains all methods
 * that can be handled to generate complete document.
 */
class IE_Exp_HTML_ListenerImpl {
public:

    virtual void openSpan(const gchar */*szStyleName*/) {}
    virtual void closeSpan() {}
    
    virtual void openHeading(size_t /*level*/, const gchar * /*id*/, 
        const gchar */*szStyleName*/) {}
    virtual void closeHeading() {}
    
    virtual void openBlock(const gchar */*szStyleName*/) {}
    virtual void closeBlock() {}
    
    virtual void openSection(const gchar */*szStyleName*/) {}
    virtual void closeSection() {}
    
    virtual void openField(const UT_UTF8String& /*fieldType*/, 
        const UT_UTF8String& /*fieldValue*/) {}
    virtual void closeField(const UT_UTF8String& /*fieldType*/) {}
    
    virtual void openTable(const PP_AttrProp* /*pAP*/) {}
    virtual void closeTable() {}

    virtual void openRow() {}
    virtual void closeRow() {}

    virtual void openCell(const PP_AttrProp* /*pAP*/) {}
    virtual void closeCell() {}

    virtual void openAnnotation() {}
    virtual void closeAnnotation() {}

    virtual void openFrame(const PP_AttrProp* /*pAP*/) {}
    virtual void closeFrame() {}

    virtual void openBookmark(const gchar */*szBookmarkName*/) {}
    virtual void closeBookmark() {}

    virtual void openHyperlink(const gchar */*szUri*/, 
        const gchar */*szStyleName*/, const gchar */*szId*/) {}
    virtual void closeHyperlink() {}

    virtual void openList(bool /*ordered*/, const gchar */*szStyleName*/) {}
    virtual void closeList() {}

    virtual void openListItem() {}
    virtual void closeListItem() {}
    
    virtual void openDocument() {}
    virtual void closeDocument() {}
    
    virtual void openHead() {}
    virtual void closeHead() {}
    
    virtual void openBody() {}
    virtual void closeBody() {}
    
    virtual void insertDTD() {}
    virtual void insertLink(const UT_UTF8String &/*rel*/, 
        const UT_UTF8String &/*type*/, const UT_UTF8String &/*uri*/) {}
    virtual void insertTitle(const UT_UTF8String &/*title*/) {}
    virtual void insertMeta(const UT_UTF8String &/*name*/, 
        const UT_UTF8String &/*content*/) {}

    virtual void insertImage(const UT_UTF8String &/*url*/, 
        const UT_UTF8String &/*width*/, const UT_UTF8String &/*height*/,
        const UT_UTF8String &/*top*/, const UT_UTF8String &/*left*/,
        const UT_UTF8String &/*title*/, const UT_UTF8String &/*alt*/) {}
        
    virtual void insertText(const UT_UTF8String &/*text*/) {}
    
    virtual void insertTOC(const gchar */*title*/, 
        const std::vector<UT_UTF8String> &/*items*/,
        const std::vector<UT_UTF8String> &/*itemUri*/){}
     
    virtual void insertEndnotes(const std::vector<UT_UTF8String> &/*endnotes*/) {}
    virtual void insertFootnotes(const std::vector<UT_UTF8String> &/*footnotes*/) {}
    virtual void insertAnnotations(const std::vector<UT_UTF8String> &/*titles*/,
        const std::vector<UT_UTF8String> &/*authors*/,
        const std::vector<UT_UTF8String> &/*annotations*/) {}
    
    virtual void insertStyle(const UT_UTF8String &/*style*/) {}
    virtual void insertJavaScript(const gchar */*src*/, const gchar* /*script*/) {}

};


struct ListInfo
{
    const gchar *szId;
    UT_uint32 iLevel;
    UT_uint32 iItemCount;
};

/**
 * Adapter that converts PL_Listener calls into more suitable for HTML
 * exporter form. Inspired by the OpenDocument export plugin.
 */
class IE_Exp_HTML_Listener : public PL_Listener {
public:
    IE_Exp_HTML_Listener(PD_Document *pDocument, 
            IE_Exp_HTML_DataExporter *pDataExporter,
            IE_Exp_HTML_StyleTree    *pStyleTree,
            IE_Exp_HTML_NavigationHelper *pNavigationHelper,
            IE_Exp_HTML_ListenerImpl *pListenerImpl);

    virtual bool populate(PL_StruxFmtHandle sfh,
            const PX_ChangeRecord * pcr);

    virtual bool populateStrux(PL_StruxDocHandle sdh,
            const PX_ChangeRecord * pcr,
            PL_StruxFmtHandle * psfh);

    virtual bool beginOfDocument();
    virtual bool endOfDocument();

    virtual bool change(PL_StruxFmtHandle sfh,
            const PX_ChangeRecord * pcr);

    virtual bool insertStrux(PL_StruxFmtHandle sfh,
            const PX_ChangeRecord * pcr,
            PL_StruxDocHandle sdh,
            PL_ListenerId lid,
            void (*pfnBindHandles) (PL_StruxDocHandle sdhNew,
            PL_ListenerId lid,
            PL_StruxFmtHandle sfhNew));

    virtual bool signal(UT_uint32 iSignal);
private:
    const gchar* _getObjectKey(const PT_AttrPropIndex& api,
            const gchar* key);

    void _outputData(const UT_UCSChar* pData, UT_uint32 length);
    void _openSpan(PT_AttrPropIndex api);
    void _closeSpan();

    void _openHeading(size_t level, const gchar *szStyleName = NULL);
    void _closeHeading();

    void _openBlock(PT_AttrPropIndex api);
    void _closeBlock();

    void _openSection(PT_AttrPropIndex api, bool recursiveCall = false);
    void _closeSection(bool recursiveCall = false);

    void _openField(const PX_ChangeRecord_Object* pcro, PT_AttrPropIndex api);
    void _closeField();

    void _openTable(PT_AttrPropIndex api, bool recursiveCall = false);
    void _closeTable(bool recursiveCall = false);

    void _openRow(PT_AttrPropIndex api, bool recursiveCall = false);
    void _closeRow(bool recursiveCall = false);

    void _openCell(PT_AttrPropIndex api, bool recursiveCall = false);
    void _closeCell(bool recursiveCall = false);

    void _openList(PT_AttrPropIndex api, bool recursiveCall = false);
    void _closeList(bool recursiveCall = false);
    void _closeLists();

    void _openListItem(bool recursiveCall = false);
    void _closeListItem(bool recursiveCall = false);

    void _openFootnote(PT_AttrPropIndex api);
    void _closeFootnote();

    void _openEndnote(PT_AttrPropIndex api);
    void _closeEndnote();

    void _openAnnotation(PT_AttrPropIndex api);
    void _closeAnnotation();

    void _openFrame(PT_AttrPropIndex api);
    void _closeFrame();

    void _openBookmark(PT_AttrPropIndex api);
    void _closeBookmark();

    void _openHyperlink(PT_AttrPropIndex api);
    void _closeHyperlink();

    void _openDocument();
    void _closeDocument();
    
    void _openHead();
    void _closeHead();
    
    void _openBody();
    void _closeBody();
    
    void _insertDTD();
    void _insertLinks();
    void _insertTitle();
    void _insertMeta();
    void _insertImage(PT_AttrPropIndex api);
    void _insertEmbeddedImage(PT_AttrPropIndex api);
    void _insertMath(PT_AttrPropIndex api);
    void _insertTOC(PT_AttrPropIndex api);
    void _insertEndnotes();
    void _insertFootnotes();
    void _insertAnnotations();
    void _insertStyle();
    void _insertLinkToStyle();
    void _handleAnnotationData(PT_AttrPropIndex api);
    void _makeStylesheet();
    
    bool m_bInSpan;
    bool m_bInBlock;
    bool m_bInBookmark;
    bool m_bInHyperlink;
    bool m_bInSection;
    bool m_bInAnnotation;
    bool m_bInAnnotationSection;
    bool m_bInEndnote;
    bool m_bInFootnote;
    bool m_bInHeading;
    bool m_bSkipSection;
    fd_Field* m_pCurrentField;
    UT_UTF8String m_currentFieldType;
    UT_UTF8String m_bookmarkName;
    PT_AttrPropIndex m_apiLastSpan;

    UT_sint32 m_iInTable;
    UT_sint32 m_iInRow;
    UT_sint32 m_iInCell;

    bool m_bFirstRow;
    PD_Document *m_pDocument;
    IE_Exp_HTML_ListenerImpl *m_pCurrentImpl;
    ie_Table m_tableHelper;

    UT_GenericVector<ListInfo> m_listInfoStack;
    std::vector<UT_UTF8String> m_endnotes;
    std::vector<UT_UTF8String> m_footnotes;
    std::vector<UT_UTF8String> m_annotationTitles;
    std::vector<UT_UTF8String> m_annotationAuthors;
    std::vector<UT_UTF8String> m_annotationContents;
    
    IE_Exp_HTML_DataExporter *m_pDataExporter;
    
    bool m_bEmbedCss;
    bool m_bEmbedImages;
    bool m_bRenderMathToPng;
    bool m_bSplitDocument;
    
    UT_UTF8String m_filename;
    
    IE_Exp_HTML_StyleTree *m_pStyleTree;
    IE_Exp_HTML_NavigationHelper *m_pNavigationHelper;
    UT_UTF8String m_stylesheet;
};



#endif	/* IE_EXP_HTML_LISTENER_H */

