#ifndef IE_EXP_HTML_WRITER_H
#define	IE_EXP_HTML_WRITER_H


// HTML exporter includes
#include "ie_impexp_HTML.h"
#include "ie_exp_HTML.h"
#include "xap_Dlg_HTMLOptions.h"

// External includes
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <gsf/gsf-output.h>
#include <map>

// Abiword includes
#include <ie_types.h>
#include <ie_TOC.h>
#include <ie_Table.h>
#include <ut_locale.h>
#include <ut_debugmsg.h>
#include <ut_assert.h>
#include <ut_string.h>
#include <ut_bytebuf.h>
#include <ut_base64.h>
#include <ut_hash.h>
#include <ut_units.h>
#include <ut_wctomb.h>
#include <ut_path.h>
#include <ut_math.h>
#include <ut_misc.h>
#include <ut_string_class.h>
#include <ut_png.h>
#include <xap_App.h>
#include <xap_EncodingManager.h>
#include <xap_Dialog_Id.h>
#include <xap_DialogFactory.h>
#include <xap_Frame.h>
#include <xav_View.h>
#include <pt_Types.h>
#include <pl_Listener.h>
#include <pd_Document.h>
#include <pd_Style.h>
#include <pp_AttrProp.h>
#include <pp_Property.h>
#include <pp_PropertyMap.h>
#include <px_ChangeRecord.h>
#include <px_CR_Object.h>
#include <px_CR_Span.h>
#include <px_CR_Strux.h>
#include <ut_mbtowc.h>
#include <gr_Graphics.h>
#include <fd_Field.h>
#include <fl_AutoNum.h>
#include <ap_Strings.h>

enum WhiteSpace {
    ws_None = 0,
    ws_Pre = 1,
    ws_Post = 2,
    ws_Both = 3
};

class ABI_EXPORT IE_Exp_HTML_Writer {
    friend class IE_Exp_HTML_HeaderFooterListener;
public:
    IE_Exp_HTML_Writer(PD_Document * pDocument, IE_Exp_HTML * pie, bool bClipBoard,
            bool bTemplateBody, const XAP_Exp_HTMLOptions * exp_opt,
            IE_Exp_HTML_StyleTree * style_tree,
            UT_UTF8String & linkCSS,
            UT_UTF8String & title,
            bool bIndexFile = true);

    ~IE_Exp_HTML_Writer();


public:
    void startEmbeddedStrux(void);
    void writeImageBase64(const UT_ByteBuf * pByteBuf);
    void handleImage(PT_AttrPropIndex api);
    void handleImage(const PP_AttrProp * pAP, const char * szDataID, bool isPositioned, bool bIgnoreSize);
    void handlePendingImages();
    void handleField(const PX_ChangeRecord_Object * pcro, PT_AttrPropIndex api);
    void handleHyperlink(PT_AttrPropIndex api);
    void handleAnnotationMark(PT_AttrPropIndex api);
    void handleBookmark(PT_AttrPropIndex api);
    void handleMath(PT_AttrPropIndex api);
    void handleEmbedded(PT_AttrPropIndex api);
    void handleEmbedded(const PP_AttrProp * pAP, const gchar * szDataID, const UT_ByteBuf* pByteBuf, const std::string mimeType);
    void handleMetaTag(const char * key, UT_UTF8String & value);
    void handleMeta();

    bool getPropertySize(const PP_AttrProp * pAP, const gchar* szWidthProp, const gchar* szHeightProp,
            const gchar** szWidth, double& widthPercentage, const gchar** szHeight);
    UT_UTF8String getStyleSizeString(const gchar * szWidth, double widthPercentage, UT_Dimension widthDim,
            const gchar * szHeight, UT_Dimension heightDim);

    void addFootnote(PD_DocumentRange * pDocRange);
    void addEndnote(PD_DocumentRange * pDocRange);
    void addAnnotation(PD_DocumentRange * pDocRange);
    UT_uint32 getNumFootnotes(void);
    UT_uint32 getNumEndnotes(void);
    UT_uint32 getNumAnnotations(void);
    void doEndnotes();
    void doFootnotes();
    void doAnnotations();
    void emitTOC(PT_AttrPropIndex api);
    void popUnendedStructures(void);

    void openTag(PT_AttrPropIndex api, PL_StruxDocHandle sdh);
    void closeTag(void);
    void closeSpan(void);
    void openSpan(PT_AttrPropIndex api);

    void openSection(PT_AttrPropIndex api, UT_uint16 iSectionSpecialType);
    void closeSection(void);

    void openTable(PT_AttrPropIndex api);
    void closeTable();
    void openRow(PT_AttrPropIndex api);
    void openCell(PT_AttrPropIndex api);
    void closeCell();

    void openTextBox(PT_AttrPropIndex api);
    void openPosImage(PT_AttrPropIndex api);
    void closeTextBox();

    void outputData(const UT_UCSChar * p, UT_uint32 length);
    void write(const char* data, UT_uint32 size);
    
    UT_uint32 listDepth();
    UT_uint32 listType();
    void listPush(UT_uint32 type, const char * ClassName);
    void listPop();
    void listPopToDepth(UT_uint32 depth);

    inline bool get_HTML4() const {
        return m_exp_opt->bIs4;
    }

    inline bool get_PHTML() const {
        return m_exp_opt->bIsAbiWebDoc;
    }

    inline bool get_Declare_XML() const {
        return m_exp_opt->bDeclareXML && !m_exp_opt->bIs4;
    }

    inline bool get_Allow_AWML() const {
        return m_exp_opt->bAllowAWML && !m_exp_opt->bIs4;
    }

    inline bool get_Embed_CSS() const {
        return m_exp_opt->bEmbedCSS;
    }

    inline bool get_Link_CSS() const {
        return m_exp_opt->bLinkCSS;
    }

    inline bool get_Abs_Units() const {
        return m_exp_opt->bAbsUnits;
    }

    inline bool get_Scale_Units() const {
        return m_exp_opt->bScaleUnits;
    }

    inline UT_uint32 get_Compact() const {
        return m_exp_opt->iCompact;
    }

    inline bool get_Embed_Images() const {
        return m_exp_opt->bEmbedImages;
    }

    inline bool get_Multipart() const {
        return m_exp_opt->bMultipart;
    }

    inline bool get_Class_Only() const {
        return m_exp_opt->bClassOnly;
    }

    inline bool get_AddIdentifiers() const {
        return m_exp_opt->bAddIdentifiers;
    }

    inline bool get_MathML_Render_PNG() const {
        return m_exp_opt->bMathMLRenderPNG;
    }

    inline bool get_Split_Document() const {
        return m_exp_opt->bSplitDocument;
    }

    inline PD_Document* getDocument() const {
        return m_pDocument;
    }

    inline ie_Table & getTableHelper() {
        return m_TableHelper;
    }
    
    inline UT_uint32 get_ListDepth() const{
        return m_iListDepth;
    }

    inline bool is_FirstWrite() const {
        return m_bFirstWrite;
    }

    inline bool is_InFrame() const {
        return m_bInFrame;
    }

    inline bool is_InTextBox() const {
        return m_bInTextBox;
    }

    inline bool is_InTOC() const {
        return m_bInTOC;
    }

    inline bool is_InSpan() const {
        return m_bInSpan;
    }

    inline bool is_InBlock() const {
        return m_bInBlock;
    }

    inline bool is_NextIsSpace() const {
        return m_bNextIsSpace;
    }

    inline bool is_QuotedPrintable() const {
        return m_bQuotedPrintable;
    }

    inline bool is_HaveHeader() const {
        return m_bHaveHeader;
    }

    inline bool is_HaveFooter() const {
        return m_bHaveFooter;
    }

    inline bool is_WroteText() const {
        return m_bWroteText;
    }

    inline bool is_ClipBoard() const {
        return m_bClipBoard;
    }
    
    inline void set_WroteText(){
        m_bWroteText = true;
    }
    
    // Ugly hack to separate IE_Exp_HTML_MainListener. Will be deleted
    // soon
    inline UT_UTF8String& get_Buffer() { return m_utf8_1; }
private:


    void _outputBegin(PT_AttrPropIndex api);
    void _outputEnd();
    bool _openStyleSheet(UT_UTF8String & css_path);
    void _closeStyleSheet();
    void _outputStyles(const PP_AttrProp * pAP);


    bool _inherits(const char * style, const char * from);
    void _storeStyles(void);
    void _populateHeaderStyle();
    void _populateFooterStyle();


    /* low-level; these may use m_utf8_0 but not m_utf8_1
     * WARNING: Use of purely tag-operative methods (tagOpen,tagClose,etc..) may circumvent
     * important checks and preparations done in strux-operative methods (_open*,_close*,etc...)
     * and thus these should only be used by likewise low-level code.
     */
public:
    void tagRaw(UT_UTF8String & content);
    UT_uint32 tagIndent();
    void tagNewIndent(UT_UTF8String & utf8, UT_uint32 depth);
    void tagNewIndent(UT_uint32 extra = 0);
    void tagOpenClose(const UT_UTF8String & content, bool suppress,
            WhiteSpace ws = ws_Both);
    void tagOpen(UT_uint32 tagID, const UT_UTF8String & content,
            WhiteSpace ws = ws_Both);
    void tagClose(UT_uint32 tagID, const UT_UTF8String & content,
            WhiteSpace ws = ws_Both);
    void tagClose(UT_uint32 tagID);
    void tagOpenBroken(const UT_UTF8String & content,
            WhiteSpace ws = ws_Pre);
    void tagCloseBroken(const UT_UTF8String & content, bool suppress,
            WhiteSpace ws = ws_Post);
    UT_uint32 tagTop();
    void tagPop();
    void tagPI(const char * target, const UT_UTF8String & content);
    void tagComment(const UT_UTF8String & content);
    void tagCommentOpen();
    void tagCommentClose();
    
    void styleIndent();
    void styleOpen(const UT_UTF8String & rule);
    void styleClose();
    void styleNameValue(const char * name, const UT_UTF8String & value);
    void setHaveHeader();
    void setHaveFooter();
    void styleText(const UT_UTF8String & content);

    void textTrusted(const UT_UTF8String & text);
    void textUntrusted(const char * text);
private:



    void multiHeader(const UT_UTF8String & title);
    void multiBoundary(bool end = false);
    void multiField(const char * name, const UT_UTF8String & value);
    void multiBreak();

    bool compareStyle(const char * key, const char * value);
    void _fillColWidthsVector();
    void _setCellWidthInches(void);

private:
    PD_Document * m_pDocument;
    PT_AttrPropIndex m_apiLastSpan;
    IE_Exp_HTML * m_pie;
    bool m_bClipBoard;
    bool m_bTemplateBody;
    const XAP_Exp_HTMLOptions * m_exp_opt;
    IE_Exp_HTML_StyleTree * m_style_tree;

    bool m_bInSection;
    bool m_bInFrame;
    bool m_bInTextBox; // Necessary?  Possibly not.  Convenient and safe?  Yes.
    bool m_bInTOC;
    bool m_bInBlock;
    bool m_bInSpan;
    bool m_bNextIsSpace;
    bool m_bWroteText;
    bool m_bFirstWrite;
    bool m_bQuotedPrintable;
    bool m_bHaveHeader;
    bool m_bHaveFooter;

    ie_Table m_TableHelper;

    // Need to look up proper type, and place to stick #defines...

    UT_uint32 m_iBlockType; // BT_*
    UT_uint32 m_iListDepth; // 0 corresponds to not in a list
    UT_NumberStack m_utsListType;
    UT_uint32 m_iImgCnt;
    UT_Wctomb m_wmctomb;

    /* temporary strings; use with extreme caution
     */
    UT_UTF8String m_utf8_0; // low-level
    UT_UTF8String m_utf8_1; // intermediate

    UT_UTF8String m_utf8_span; // span tag-string cache
    UT_UTF8String m_utf8_style; // current block style

    const IE_Exp_HTML_StyleTree * m_StyleTreeInline; // current  inline  style tree, if any
    const IE_Exp_HTML_StyleTree * m_StyleTreeBlock; // current  block   style tree, if any
    const IE_Exp_HTML_StyleTree * m_StyleTreeBody; // document default style tree, if any

    const PP_AttrProp * m_pAPStyles;

    UT_UTF8String m_utf8_css_path; // Multipart HTML: cache for content location

    UT_NumberStack m_tagStack;

    UT_uint32 m_styleIndent;

    GsfOutput * m_fdCSS;



    UT_GenericStringMap<UT_UTF8String*> m_SavedURLs;

    double m_dPageWidthInches;
    double m_dSecLeftMarginInches;
    double m_dSecRightMarginInches;
    double m_dSecTopMarginInches;
    double m_dSecBottomMarginInches;
    double m_dCellWidthInches;
    UT_GenericVector<double*> m_vecDWidths;
    UT_UTF8String & m_sLinkCSS;
    UT_UTF8String & m_sTitle;

    UT_uint32 m_iOutputLen;
    bool m_bCellHasData;
    UT_GenericVector<PD_DocumentRange *> m_vecFootnotes;
    UT_GenericVector<PD_DocumentRange *> m_vecEndnotes;
    UT_GenericVector<PD_DocumentRange *> m_vecAnnotations;

    IE_TOCHelper * m_toc;
    int m_heading_count;
    GsfOutput * m_outputFile;
    bool m_bIndexFile;
    UT_UTF8String m_filename;
};


#endif	/* IE_EXP_HTML_WRITER_H */

