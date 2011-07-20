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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef IE_EXP_HTML_MAINLISTENER_H
#define IE_EXP_HTML_MAINLISTENER_H

#include "ie_exp_HTML.h"
#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML_StyleTree.h"

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

class ABI_EXPORT IE_Exp_HTML_MainListener : public PL_Listener {
    friend class IE_Exp_HTML_HeaderFooterListener;
public:
    IE_Exp_HTML_MainListener(PD_Document * pDocument, IE_Exp_HTML * pie, bool bClipBoard,
            bool bTemplateBody, const XAP_Exp_HTMLOptions * exp_opt,
            IE_Exp_HTML_StyleTree * style_tree,
            UT_UTF8String & linkCSS,
            UT_UTF8String & title,
            bool bIndexFile = true);

    ~IE_Exp_HTML_MainListener();

    bool populate(PL_StruxFmtHandle sfh,
            const PX_ChangeRecord * pcr);

    bool populateStrux(PL_StruxDocHandle sdh,
            const PX_ChangeRecord * pcr,
            PL_StruxFmtHandle * psfh);

    //bool 	startOfDocument ();  // Unused
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
    void startEmbeddedStrux(void);

private:
    void _outputBegin(PT_AttrPropIndex api);
    void _outputEnd();
    bool _openStyleSheet(UT_UTF8String & css_path);
    void _closeStyleSheet();
    void _outputStyles(const PP_AttrProp * pAP);
    void _openSection(PT_AttrPropIndex api, UT_uint16 iSectionSpecialType);
    void _closeSection(void);

    void _openTag(PT_AttrPropIndex api, PL_StruxDocHandle sdh);
    void _closeTag(void);
    void _closeSpan(void);
    void _openSpan(PT_AttrPropIndex api);

    void _popUnendedStructures(void);

#ifdef HTML_TABLES_SUPPORTED
    void _openTable(PT_AttrPropIndex api);
    void _closeTable();
    void _openRow(PT_AttrPropIndex api);
    void _openCell(PT_AttrPropIndex api);
    void _closeCell();
#endif

    void _openTextBox(PT_AttrPropIndex api);
    void _openPosImage(PT_AttrPropIndex api);
    void _closeTextBox();

    void _outputData(const UT_UCSChar * p, UT_uint32 length);
    bool _inherits(const char * style, const char * from);
    void _storeStyles(void);
    void _populateHeaderStyle();
    void _populateFooterStyle();

    void _writeImageBase64(const UT_ByteBuf * pByteBuf);
    void _handleImage(PT_AttrPropIndex api);
    void _handleImage(const PP_AttrProp * pAP, const char * szDataID, bool isPositioned, bool bIgnoreSize);
    void _handlePendingImages();
    void _handleField(const PX_ChangeRecord_Object * pcro, PT_AttrPropIndex api);
    void _handleHyperlink(PT_AttrPropIndex api);
    void _handleAnnotationMark(PT_AttrPropIndex api);
    void _handleBookmark(PT_AttrPropIndex api);
    void _handleMath(PT_AttrPropIndex api);
    void _handleEmbedded(PT_AttrPropIndex api);
    void _handleEmbedded(const PP_AttrProp * pAP, const gchar * szDataID, const UT_ByteBuf* pByteBuf, const std::string mimeType);

    bool _getPropertySize(const PP_AttrProp * pAP, const gchar* szWidthProp, const gchar* szHeightProp,
            const gchar** szWidth, double& widthPercentage, const gchar** szHeight);
    UT_UTF8String _getStyleSizeString(const gchar * szWidth, double widthPercentage, UT_Dimension widthDim,
            const gchar * szHeight, UT_Dimension heightDim);

#ifdef HTML_META_SUPPORTED
    void _handleMetaTag(const char * key, UT_UTF8String & value);
    void _handleMeta();
#endif

    void _doEndnotes();
    void _doFootnotes();
    void _doAnnotations();
    void _emitTOC(PT_AttrPropIndex api);

    PD_Document * m_pDocument;
    PT_AttrPropIndex m_apiLastSpan;
    IE_Exp_HTML * m_pie;
    bool m_bClipBoard;
    bool m_bTemplateBody;
    const XAP_Exp_HTMLOptions * m_exp_opt;
    IE_Exp_HTML_StyleTree * m_style_tree;

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

#ifdef HTML_TABLES_SUPPORTED
    ie_Table m_TableHelper;
#endif

    // Need to look up proper type, and place to stick #defines...

    UT_uint32 m_iBlockType; // BT_*
    UT_uint32 m_iListDepth; // 0 corresponds to not in a list
    UT_NumberStack m_utsListType;
    UT_uint32 m_iImgCnt;
    UT_Wctomb m_wmctomb;

    enum WhiteSpace {
        ws_None = 0,
        ws_Pre = 1,
        ws_Post = 2,
        ws_Both = 3
    };

    /* low-level; these may use m_utf8_0 but not m_utf8_1
     * WARNING: Use of purely tag-operative methods (tagOpen,tagClose,etc..) may circumvent
     * important checks and preparations done in strux-operative methods (_open*,_close*,etc...)
     * and thus these should only be used by likewise low-level code.
     */
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

public:
    void styleOpen(const UT_UTF8String & rule);
    void styleClose();
    void styleNameValue(const char * name, const UT_UTF8String & value);
    void setHaveHeader();
    void setHaveFooter();
private:
    void styleText(const UT_UTF8String & content);
    void textTrusted(const UT_UTF8String & text);
    void textUntrusted(const char * text);

    void multiHeader(const UT_UTF8String & title);
    void multiBoundary(bool end = false);
    void multiField(const char * name, const UT_UTF8String & value);
    void multiBreak();

    UT_uint32 listDepth();
    UT_uint32 listType();
    void listPush(UT_uint32 type, const char * ClassName);
    void listPop();
    void listPopToDepth(UT_uint32 depth);

    bool compareStyle(const char * key, const char * value);
    void _fillColWidthsVector();
    void _setCellWidthInches(void);

    void addFootnote(PD_DocumentRange * pDocRange);
    void addEndnote(PD_DocumentRange * pDocRange);
    void addAnnotation(PD_DocumentRange * pDocRange);
    UT_uint32 getNumFootnotes(void);
    UT_uint32 getNumEndnotes(void);
    UT_uint32 getNumAnnotations(void);
    void _write(const char* data, UT_uint32 size);

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

    bool m_bIgnoreTillEnd;
    bool m_bIgnoreTillNextSection;
    PT_DocPosition m_iEmbedStartPos;

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

#endif