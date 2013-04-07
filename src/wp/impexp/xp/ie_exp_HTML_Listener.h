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
#ifndef IE_EXP_HTML_LISTENER_H
#define	IE_EXP_HTML_LISTENER_H

// External includes
#include <vector>
#include <string>

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
#include <ut_mbtowc.h>

#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML_NavigationHelper.h"
#include "ie_exp_HTML_StyleTree.h"


/**
 * Listener for the {X,P}HTML and MHT document generators. Contains all methods
 * that can be handled to generate complete document.
 */
class ABI_EXPORT IE_Exp_HTML_ListenerImpl {
public:
    virtual ~IE_Exp_HTML_ListenerImpl(){}
    virtual void openSpan(const gchar * szStyleName,
						  const UT_UTF8String& style) = 0;
    virtual void closeSpan() = 0;

    virtual void openHeading(size_t level, const gchar * id,
							 const gchar * szStyleName,
							 const PP_AttrProp* pAP) = 0;
    virtual void closeHeading() = 0;

    virtual void openBlock(const gchar * szStyleName,
						   const UT_UTF8String & style,
						   const PP_AttrProp* pAP) = 0;
    virtual void closeBlock() = 0;

    virtual void openSection(const gchar * szStyleName) = 0;
    virtual void closeSection() = 0;

    virtual void openField(const UT_UTF8String& fieldType,
						   const UT_UTF8String& fieldValue) = 0;
    virtual void closeField(const UT_UTF8String& fieldType) = 0;

    virtual void openTable(const UT_UTF8String &style,
						   const UT_UTF8String &cellPadding,
						   const UT_UTF8String &border) = 0;
    virtual void closeTable() = 0;

    virtual void openRow() = 0;
    virtual void closeRow() = 0;

    virtual void openCell(const UT_UTF8String &/*style*/,
						  const UT_UTF8String &/*rowSpan*/,
						  const UT_UTF8String &/*colSpan*/) = 0;
    virtual void closeCell() = 0;

    virtual void openAnnotation() = 0;
    virtual void closeAnnotation() = 0;

    virtual void openTextbox(const UT_UTF8String &style) = 0;
    virtual void closeTextbox() = 0;

    virtual void openBookmark(const gchar * szBookmarkName) = 0;
    virtual void closeBookmark() = 0;

    virtual void openHyperlink(const gchar * szUri,
							   const gchar * szStyleName,
							   const gchar * szId) = 0;
    virtual void closeHyperlink() = 0;

    virtual void openList(bool ordered, const gchar * szStyleName,
						  const PP_AttrProp* pAP) = 0;
    virtual void closeList() = 0;

    virtual void openListItem() = 0;
    virtual void closeListItem() = 0;

    virtual void openDocument() = 0;
    virtual void closeDocument() = 0;

    virtual void openHead() = 0;
    virtual void closeHead() = 0;

    virtual void openBody() = 0;
    virtual void closeBody() = 0;

    virtual void insertDTD() = 0;
    virtual void insertLink(const UT_UTF8String &rel,
							const UT_UTF8String &type,
							const UT_UTF8String &uri) = 0;
    virtual void insertMeta(const std::string & name,
        const std::string & content, const std::string& httpEquiv) = 0;
    virtual void insertMath(const UT_UTF8String &mathml,
							const UT_UTF8String &width,
							const UT_UTF8String &height) = 0;

    virtual void insertImage(const UT_UTF8String &url,
							 const UT_UTF8String &align,
							 const UT_UTF8String& style,
							 const UT_UTF8String &title,
							 const UT_UTF8String &alt) = 0;

    virtual void insertText(const UT_UTF8String &text) = 0;

    virtual void insertTOC(const gchar * title,
						   const std::vector<UT_UTF8String> &items,
						   const std::vector<UT_UTF8String> &itemUri) = 0;

    virtual void insertEndnotes(const std::vector<UT_UTF8String> &endnotes) = 0;
    virtual void insertFootnotes(const std::vector<UT_UTF8String> &footnotes) = 0;
    virtual void insertAnnotations(const std::vector<UT_UTF8String> &titles,
								   const std::vector<UT_UTF8String> &authors,
								   const std::vector<UT_UTF8String> &annotations) = 0;

    virtual void insertStyle(const UT_UTF8String &style) = 0;
    virtual void insertJavaScript(const gchar* src,
								  const gchar* script) = 0;
    virtual void insertTitle(const std::string& title) = 0;
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
            IE_Exp_HTML_ListenerImpl *pListenerImpl,
            const UT_UTF8String &filename);

    virtual bool populate(fl_ContainerLayout* sfh,
            const PX_ChangeRecord * pcr);

    virtual bool populateStrux(pf_Frag_Strux* sdh,
            const PX_ChangeRecord * pcr,
            fl_ContainerLayout* * psfh);

    virtual bool endOfDocument();

    virtual bool change(fl_ContainerLayout* sfh,
            const PX_ChangeRecord * pcr);

    virtual bool insertStrux(fl_ContainerLayout* sfh,
            const PX_ChangeRecord * pcr,
            pf_Frag_Strux* sdh,
            PL_ListenerId lid,
            void (*pfnBindHandles) (pf_Frag_Strux* sdhNew,
            PL_ListenerId lid,
            fl_ContainerLayout* sfhNew));

    virtual bool signal(UT_uint32 iSignal);

    void set_EmbedCSS(bool bEmbed = true)
		{ m_bEmbedCss = bEmbed; }
    void set_EmbedImages(bool bEmbed = true)
		{ m_bEmbedImages = bEmbed; }
    void set_SplitDocument(bool bSplit = true)
		{ m_bSplitDocument = bSplit; }
    void set_RenderMathMLToPng (bool bRender = true)
		{ m_bRenderMathToPng = bRender; }
    bool get_HasMathML() const
		{ return m_bHasMathMl; }
private:
    const gchar* _getObjectKey(const PT_AttrPropIndex& api,
            const gchar* key);
    virtual bool _beginOfDocument(const PT_AttrPropIndex& api);
    void _outputData(const UT_UCSChar* pData, UT_uint32 length);
    void _openSpan(PT_AttrPropIndex api);
    void _closeSpan();

    void _openHeading(PT_AttrPropIndex api, size_t level,
        const gchar *szStyleName = NULL);
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

    void _openFrame(PT_AttrPropIndex api, const PX_ChangeRecord* pcr);
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

    void _openTextbox(PT_AttrPropIndex api);
    void _closeTextbox();

    void _insertPosImage (PT_AttrPropIndex api);
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
    void _handleImage(PT_AttrPropIndex api, const gchar *szDataId,
					  bool bIsPositioned);
    void _makeStylesheet(PT_AttrPropIndex api);
    void _setCellWidthInches();
    void _fillColWidthsVector();

    bool m_bFirstWrite;
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
    bool m_bInTextbox;
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
    bool m_bScaleUnits;
    bool m_bAbsUnits;
    UT_UTF8String m_filename;

    IE_Exp_HTML_StyleTree *m_pStyleTree;
    IE_Exp_HTML_NavigationHelper *m_pNavigationHelper;
    UT_UTF8String m_stylesheet;
    UT_uint32 m_iHeadingCount;

    double m_dPageWidthInches;
    double m_dSecLeftMarginInches;
    double m_dSecRightMarginInches;
    double m_dSecTopMarginInches;
    double m_dSecBottomMarginInches;
    double m_dCellWidthInches;
    UT_GenericVector<double> m_vecDWidths;
    bool m_bHasMathMl;
};



#endif	/* IE_EXP_HTML_LISTENER_H */

