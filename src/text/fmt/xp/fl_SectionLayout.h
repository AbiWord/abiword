/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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

#ifndef SECTIONLAYOUT_H
#define SECTIONLAYOUT_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

typedef enum _SectionType
{
	FL_SECTION_DOC,
    FL_SECTION_HDRFTR,
    FL_SECTION_SHADOW,
	FL_SECTION_ENDNOTE,
    FL_SECTION_TABLE,
	FL_SECTION_CELL,
	FL_SECTION_FOOTNOTE,
	FL_SECTION_MARGINNOTE,
	FL_SECTION_FRAME,
	FL_SECTION_TOC,
	FL_SECTION_ANNOTATION
} SectionType;


typedef enum _HdrFtrType
{
	FL_HDRFTR_HEADER,
	FL_HDRFTR_HEADER_EVEN,
	FL_HDRFTR_HEADER_FIRST,
	FL_HDRFTR_HEADER_LAST,
	FL_HDRFTR_FOOTER,
	FL_HDRFTR_FOOTER_EVEN,
	FL_HDRFTR_FOOTER_FIRST,
	FL_HDRFTR_FOOTER_LAST,
	FL_HDRFTR_NONE
} HdrFtrType;


#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "fl_ContainerLayout.h"
#include "pl_Listener.h"
#include "ut_debugmsg.h"
#include "ut_misc.h" // for UT_RGBColor
#include "fg_Graphic.h"
#include "fb_ColumnBreaker.h"

class fp_Page;
class FL_DocLayout;
class fl_Layout;
class fl_ContainerLayout;
class fl_BlockLayout;
class fl_HdrFtrSectionLayout;
class fl_HdrFtrShadow;
class fl_FootnoteLayout;
class fl_AnnotationLayout;
class fb_LineBreaker;
class fp_ShadowContainer;
class fp_Column;
class fp_Run;
class fp_Line;
class fp_Container;
class fp_HdrFtrContainer;
class PD_Document;
class PP_AttrProp;
class pf_Frag_Strux;
class PX_ChangeRecord_FmtMark;
class PX_ChangeRecord_FmtMarkChange;
class PX_ChangeRecord_Object;
class PX_ChangeRecord_ObjectChange;
class PX_ChangeRecord_Span;
class PX_ChangeRecord_SpanChange;
class PX_ChangeRecord_Strux;
class PX_ChangeRecord_StruxChange;
class fb_ColumnBreaker;
class fp_EndnoteContainer;
class fl_TableLayout;
class fl_CellLayout;
class UT_Worker;
class ABI_EXPORT fl_SectionLayout : public fl_ContainerLayout
{
	friend class fl_DocListener;

public:
	fl_SectionLayout(FL_DocLayout* pLayout, pf_Frag_Strux* sdh, PT_AttrPropIndex ap, SectionType iType, fl_ContainerType iCType, PTStruxType ptType, fl_ContainerLayout * pMyContainerLayout);
	virtual ~fl_SectionLayout();

	SectionType     	getType(void) const { return m_iType; }

	virtual bool		recalculateFields(UT_uint32 iUpdateCount);
	fl_BlockLayout *        getFirstBlock(void) const;
	virtual fp_Container*		getNewContainer(fp_Container * pFirstContainer = NULL) = 0;
	virtual FL_DocLayout*		getDocLayout(void) const;
	virtual void                markAllRunsDirty(void) =0;
	virtual bool                isCollapsed(void) const
		{return m_bIsCollapsed;}
	virtual void                setNeedsReformat(fl_ContainerLayout * pCL, UT_uint32 offset = 0);
	        void                clearNeedsReformat(fl_ContainerLayout * pCL);
	virtual void                setNeedsRedraw(void);
	virtual void                removeFromUpdate(fl_ContainerLayout * pL);
	virtual bool                needsReformat(void) const
		{return m_bNeedsReformat;}
	virtual bool                needsRedraw(void) const
		{return m_bNeedsRedraw;}
	virtual void                clearNeedsRedraw(void)
		{m_bNeedsRedraw = false;}

	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc) = 0;
	void                        checkAndAdjustCellSize(void);
	virtual bool bl_doclistener_populateSpan(fl_ContainerLayout*, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	virtual bool bl_doclistener_populateObject(fl_ContainerLayout*, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_insertSpan(fl_ContainerLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_deleteSpan(fl_ContainerLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_changeSpan(fl_ContainerLayout*, const PX_ChangeRecord_SpanChange * pcrsc);
	virtual bool bl_doclistener_deleteStrux(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx);
	virtual bool bl_doclistener_changeStrux(fl_ContainerLayout*, const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool bl_doclistener_insertBlock(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx,
											pf_Frag_Strux* sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	PL_ListenerId lid,
																	fl_ContainerLayout* sfhNew));
	virtual bool bl_doclistener_insertSection(fl_ContainerLayout*,
											  SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));
	virtual fl_SectionLayout * bl_doclistener_insertTable(fl_ContainerLayout*,
											  SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));
	virtual fl_SectionLayout * bl_doclistener_insertTable(SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));
	virtual fl_SectionLayout * bl_doclistener_insertFrame(fl_ContainerLayout*,
											  SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));

	virtual bool bl_doclistener_insertObject(fl_ContainerLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_deleteObject(fl_ContainerLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_changeObject(fl_ContainerLayout*, const PX_ChangeRecord_ObjectChange * pcroc);

	virtual bool bl_doclistener_insertFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_deleteFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_changeFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMarkChange * pcrfmc);

	virtual void         checkGraphicTick(GR_Graphics * pG);
	virtual void         setImageWidth(UT_sint32 iWidth);
	virtual void         setImageHeight(UT_sint32 iHeight);
	GR_Image *           getBackgroundImage(void)
	  {	return m_pImageImage;}

#ifdef FMT_TEST
	virtual void		__dump(FILE * fp) const;
#endif

protected:

	void				_purgeLayout();

	SectionType			m_iType;

	FL_DocLayout*		m_pLayout;
	bool                m_bIsCollapsed;
	bool                m_bNeedsReformat;
	bool                m_bNeedsRedraw;
	FG_SharedGraphicPtr m_pGraphicImage;
	GR_Image *          m_pImageImage;
	UT_uint32           m_iGraphicTick;
	UT_sint32           m_iDocImageWidth;
	UT_sint32           m_iDocImageHeight;
	UT_GenericVector<fl_ContainerLayout *> m_vecFormatLayout;
};

class ABI_EXPORT fl_DocSectionLayout : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_DocSectionLayout(FL_DocLayout* pLayout, pf_Frag_Strux* sdh, PT_AttrPropIndex ap, SectionType iType);
	virtual ~fl_DocSectionLayout();

	fl_DocSectionLayout* getNextDocSection(void) const;
	fl_DocSectionLayout* getPrevDocSection(void) const;

	virtual void		format(void);
	virtual void		updateLayout(bool bDoFull);
	void                updateDocSection(void);
	virtual void        collapse(void);
	virtual fp_Container * getFirstContainer(void) const;
	virtual fp_Container * getLastContainer(void) const;
	virtual void        setFirstContainer(fp_Container * pCon);
	virtual void        setLastContainer(fp_Container * pCon);

	fl_FootnoteLayout  *       getFootnoteLayout(UT_uint32 footnotePID);
	fl_AnnotationLayout  *       getAnnotationLayout(UT_uint32 footnotePID);


	virtual void        markAllRunsDirty(void);
	virtual fl_SectionLayout *  getSectionLayout(void) const
		{ return NULL; }

	virtual void		redrawUpdate(void);
	virtual fp_Container*		getNewContainer(fp_Container * pFirstContainer = NULL);

	inline UT_sint32			getLeftMargin(void) const { return m_iLeftMargin; }
	inline UT_sint32			getRightMargin(void) const { return m_iRightMargin; }
	UT_sint32                   getTopMargin(void) const;
    UT_sint32                   getBottomMargin(void) const;
	inline UT_sint32			getFooterMargin(void) const { return m_iFooterMargin; }
	inline UT_sint32			getHeaderMargin(void) const { return m_iHeaderMargin; }
	inline UT_sint32			getSpaceAfter(void) const { return m_iSpaceAfter; }
	inline UT_sint32            getMaxSectionColumnHeight(void) const { return m_iMaxSectionColumnHeight;}
	UT_sint32                   getColumnGap(void) const;
	UT_uint32			        getFootnoteLineThickness(void) const
		{ return m_iFootnoteLineThickness;}
	UT_uint32			        getFootnoteYoff(void) const
		{ return m_iFootnoteYoff;}

	void                completeBreakSection(void);
	bool                arePageNumbersRestarted (void) const { return m_bRestart;}
	UT_sint32           getRestartedPageNumber(void) const { return m_iRestartValue;}
	UT_uint32			getNumColumns(void) const;
	bool				getColumnLineBetween(void) const {return m_bColumnLineBetween;}
	UT_uint32			getColumnOrder(void) const;
	void                setPaperColor();
	void				deleteEmptyColumns(void);
	virtual bool 	    doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	bool				doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);

	virtual bool        bl_doclistener_insertFootnote(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));


	virtual bool        bl_doclistener_insertAnnotation(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));

	void				setHdrFtr(HdrFtrType iType, fl_HdrFtrSectionLayout* pHFSL);

	fl_HdrFtrSectionLayout*         getHeader(void) const;
	fl_HdrFtrSectionLayout*         getFooter(void) const;
	fl_HdrFtrSectionLayout*         getHeaderEven(void) const;
	fl_HdrFtrSectionLayout*         getFooterEven(void) const;
	fl_HdrFtrSectionLayout*         getHeaderFirst(void) const;
	fl_HdrFtrSectionLayout*         getFooterFirst(void) const;
	fl_HdrFtrSectionLayout*         getHeaderLast(void) const;
	fl_HdrFtrSectionLayout*         getFooterLast(void) const;

	bool                            setHdrFtrHeightChange(bool bDoHdr, UT_sint32 newHeight);
	static void         _HdrFtrChangeCallback(UT_Worker * pWorker);

	void				addOwnedPage(fp_Page*);
	void                            prependOwnedHeaderPage(fp_Page * p_Page);
	void                            prependOwnedFooterPage(fp_Page * p_Page);
	void				deleteOwnedPage(fp_Page*, bool bReallyDeleteIT=true);
	fp_Page *                       getFirstOwnedPage(void) const {return m_pFirstOwnedPage;}
	void                markForRebuild(void) { m_bNeedsRebuild = true;}
	void                clearRebuild(void) { m_bNeedsRebuild = false;}
	bool                needsRebuild(void) const { return m_bNeedsRebuild;}
	void				checkAndAdjustColumnGap(UT_sint32 iLayoutWidth);
    void                markForReformat(void) { m_bNeedsFormat = true;}
    bool                needsReFormat(void) const { return m_bNeedsFormat;}
	bool                isThisPageValid(HdrFtrType hfType, fp_Page * pThisPage);
	bool                isFirstPageValid(void) const; 
    void                getVecOfHdrFtrs(UT_GenericVector<fl_HdrFtrSectionLayout *> * vecHdrFtr);
	void                formatAllHdrFtr(void);
	void                doMarginChangeOnly(void);
	void                checkAndRemovePages(void);
	void                addValidPages(void);
	UT_sint32           getPageCount(void) {return m_iPageCount;}
	void                setNeedsSectionBreak(bool bSet, fp_Page * pPage );
	bool                needsSectionBreak(void) const { return m_bNeedsSectionBreak;}
	void                setFirstEndnoteContainer(fp_EndnoteContainer * pECon);
	void                setLastEndnoteContainer(fp_EndnoteContainer * pECon);
	fp_Container *      getFirstEndnoteContainer(void);
	fp_Container *      getLastEndnoteContainer(void);
	void                deleteBrokenTablesFromHere(fl_ContainerLayout * pTL);
	UT_sint32           getWidth(void);
	UT_sint32           getActualColumnHeight(void);
	UT_sint32           getActualColumnWidth(void);
	bool                isCollapsing(void) const
		{ return m_bDoingCollapse;}
private:
	virtual void		_lookupProperties(const PP_AttrProp* pAP);
	virtual void		_lookupMarginProperties(const PP_AttrProp* pAP);
	fb_ColumnBreaker    m_ColumnBreaker;
	/*
	  TODO support special case header/footer for first page of section
	*/
	fl_HdrFtrSectionLayout*		m_pHeaderSL;
	fl_HdrFtrSectionLayout*		m_pFooterSL;
	fl_HdrFtrSectionLayout*		m_pHeaderEvenSL;
	fl_HdrFtrSectionLayout*		m_pFooterEvenSL;
	fl_HdrFtrSectionLayout*		m_pHeaderFirstSL;
	fl_HdrFtrSectionLayout*		m_pFooterFirstSL;
	fl_HdrFtrSectionLayout*		m_pHeaderLastSL;
	fl_HdrFtrSectionLayout*		m_pFooterLastSL;

	UT_uint32			m_iNumColumns;
	UT_sint32			m_iColumnGap;
	bool				m_bColumnLineBetween;
	UT_uint32			m_iColumnOrder;

	UT_sint32			m_iSpaceAfter;
	bool                m_bRestart;
	UT_sint32           m_iRestartValue;
	UT_sint32			m_iLeftMargin;
	double				m_dLeftMarginUserUnits;
	UT_sint32			m_iRightMargin;
	double				m_dRightMarginUserUnits;
	UT_sint32			m_iTopMargin;
	double				m_dTopMarginUserUnits;
	UT_sint32			m_iBottomMargin;
	double				m_dBottomMarginUserUnits;
	UT_sint32			m_iFooterMargin;
	double				m_dFooterMarginUserUnits;
	UT_sint32			m_iHeaderMargin;
	double				m_dHeaderMarginUserUnits;
	UT_sint32           m_iMaxSectionColumnHeight;
	double              m_dMaxSectionColumnHeight;
	UT_sint32           m_iFootnoteLineThickness;
	UT_sint32           m_iFootnoteYoff;

	//! First column in the section
	fp_Column*			m_pFirstColumn;
	//! Last column in the section
	fp_Column*			m_pLastColumn;
	fp_Page *           m_pFirstOwnedPage;
	UT_sint32           m_iPageCount;
	bool                m_bNeedsFormat;
	bool                m_bNeedsRebuild;
	bool                m_bNeedsSectionBreak;
	fp_EndnoteContainer * m_pFirstEndnoteContainer;
	fp_EndnoteContainer * m_pLastEndnoteContainer;
	bool                m_bDeleteingBrokenContainers;
	UT_String           m_sPaperColor;
	UT_String           m_sScreenColor;
	UT_sint32           m_iNewHdrHeight;
	UT_sint32           m_iNewFtrHeight;
	UT_Worker *         m_pHdrFtrChangeTimer;
	UT_String           m_sHdrFtrChangeProps;
	bool                m_bDoingCollapse;
};

class _PageHdrFtrShadowPair;


class ABI_EXPORT fl_HdrFtrSectionLayout : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_HdrFtrSectionLayout(HdrFtrType iHFType, FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, pf_Frag_Strux* sdh, PT_AttrPropIndex ap);
	virtual ~fl_HdrFtrSectionLayout();

	inline fl_DocSectionLayout*	getDocSectionLayout(void) const { return m_pDocSL; }
	HdrFtrType      			getHFType(void) const { return m_iHFType; }
	void                        setDocSectionLayout(fl_DocSectionLayout * pDSL) { m_pDocSL = pDSL;}
	void                        setHdrFtr(HdrFtrType iHFType) { 	m_iHFType = iHFType;}
	virtual bool				recalculateFields(UT_uint32 iUpdateCount);
	bool                        doclistener_deleteStrux(const PX_ChangeRecord * pcr);
	void                        checkAndAdjustCellSize(fl_ContainerLayout * pCL);
	void                        localFormat(void);
	virtual void                markAllRunsDirty(void);
	void                        checkAndRemovePages(void);
	void                        addValidPages(void);
	bool                        isPageHere( fp_Page *pPage);
	bool                        isPointInHere(PT_DocPosition pos);
	void                        collapseBlock(fl_ContainerLayout * pBlock);
	virtual void				format(void);
	virtual fl_SectionLayout *  getSectionLayout(void) const
		{ return static_cast<fl_SectionLayout *>(m_pDocSL);}
	virtual void				updateLayout(bool bDoFull);
	void                        layout(void);
	fl_ContainerLayout *        findMatchingContainer( fl_ContainerLayout * pBL);
	virtual void				redrawUpdate(void);
	virtual fp_Container*		getNewContainer(fp_Container * pFirstContainer = NULL);
	virtual fp_Container*		getFirstContainer() const;
	virtual fp_Container*		getLastContainer() const;
	fl_HdrFtrShadow *               getFirstShadow(void);
	fl_HdrFtrShadow *               findShadow( fp_Page * pPage);
	virtual bool 			doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	void                                    changeIntoHdrFtrSection( fl_DocSectionLayout * pSL);
	void				    addPage(fp_Page*);
	void					deletePage(fp_Page*);
	void                    clearScreen(void);
	virtual void            collapse(void);
	bool                    bl_doclistener_insertCell(fl_ContainerLayout* pCell,
													  const PX_ChangeRecord_Strux * pcrx,
													  pf_Frag_Strux* sdh,
													  PL_ListenerId lid,
													  fl_TableLayout * pTL);
	bool                    bl_doclistener_insertEndTable(fl_ContainerLayout* pTab,
													  const PX_ChangeRecord_Strux * pcrx,
													  pf_Frag_Strux* sdh,
													  PL_ListenerId lid);
	virtual bool bl_doclistener_populateSpan(fl_ContainerLayout*, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	virtual bool bl_doclistener_populateObject(fl_ContainerLayout*, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_insertSpan(fl_ContainerLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_deleteSpan(fl_ContainerLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_changeSpan(fl_ContainerLayout*, const PX_ChangeRecord_SpanChange * pcrsc);
	virtual bool bl_doclistener_deleteStrux(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx);
	bool bl_doclistener_deleteCellStrux(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx);
	bool bl_doclistener_deleteTableStrux(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx);
	virtual bool bl_doclistener_changeStrux(fl_ContainerLayout*, const PX_ChangeRecord_StruxChange * pcrxc);
	virtual fl_SectionLayout * bl_doclistener_insertTable(fl_ContainerLayout*,
											  SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));
	virtual fl_SectionLayout * bl_doclistener_insertTable(SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));
	bool         bl_doclistener_insertFirstBlock(fl_ContainerLayout* pCL, const PX_ChangeRecord_Strux * pcrx,pf_Frag_Strux* sdh,PL_ListenerId lid);
	virtual bool bl_doclistener_insertBlock(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx,
											pf_Frag_Strux* sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	PL_ListenerId lid,
																	fl_ContainerLayout* sfhNew));
	virtual bool bl_doclistener_insertSection(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));
	virtual bool bl_doclistener_insertObject(fl_ContainerLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_deleteObject(fl_ContainerLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_changeObject(fl_ContainerLayout*, const PX_ChangeRecord_ObjectChange * pcroc);

	virtual bool bl_doclistener_insertFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_deleteFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_changeFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMarkChange * pcrfmc);

private:
	UT_sint32					_findShadow(fp_Page * pPage);
	virtual void				_lookupProperties(const PP_AttrProp* pAP);
	virtual void		        _lookupMarginProperties(const PP_AttrProp* pAP);
	void                        _localCollapse(void);

	fl_DocSectionLayout*		m_pDocSL;
	HdrFtrType					m_iHFType;
	UT_GenericVector<_PageHdrFtrShadowPair*> m_vecPages;
	fp_Container *              m_pHdrFtrContainer;
};


class ABI_EXPORT fl_HdrFtrShadow : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_HdrFtrShadow(FL_DocLayout* pLayout, fp_Page* pPage, fl_HdrFtrSectionLayout* pDocSL, pf_Frag_Strux* sdh, PT_AttrPropIndex ap);
	virtual ~fl_HdrFtrShadow();

virtual	fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const { return m_pHdrFtrSL; }
	fl_ContainerLayout *		findMatchingContainer(fl_ContainerLayout * pBL);
	fl_ContainerLayout *		findBlockAtPosition(PT_DocPosition pos);
	virtual void				format(void);
	virtual void				updateLayout(bool bDoFull);
	virtual void				redrawUpdate(void);
	fp_Page *                       getPage(void) const { return m_pPage;}
	virtual fp_Container*		getNewContainer(fp_Container *pFirstContainer = NULL);
	virtual fp_Container*		getFirstContainer() const;
	virtual fp_Container*		getLastContainer() const;
	void                        layout(void);
	void						clearScreen(void);
	virtual bool				doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	virtual void                collapse(void) {}
    virtual void                markAllRunsDirty(void) {}
	virtual fl_SectionLayout *  getSectionLayout(void) const
		{ return getHdrFtrSectionLayout()->getSectionLayout(); }


private:
	virtual void				_lookupProperties(const PP_AttrProp* pAP);
	virtual void				_lookupMarginProperties(const PP_AttrProp* pAP);
	void						_createContainer(void);

	fp_ShadowContainer*			m_pContainer;
	fp_Page*					m_pPage;
	fl_HdrFtrSectionLayout * m_pHdrFtrSL;
};

class ABI_EXPORT fl_ShadowListener : public PL_Listener
{
public:
	fl_ShadowListener(fl_HdrFtrSectionLayout* pHFSL, fl_HdrFtrShadow* pShadow);
	virtual ~fl_ShadowListener();

	virtual bool				populate(fl_ContainerLayout* sfh,
										 const PX_ChangeRecord * pcr);

	virtual bool				populateStrux(pf_Frag_Strux* sdh,
											  const PX_ChangeRecord * pcr,
											  fl_ContainerLayout* * psfh);

	virtual bool				change(fl_ContainerLayout* sfh,
									   const PX_ChangeRecord * pcr);

	virtual bool				insertStrux(fl_ContainerLayout* sfh,
											const PX_ChangeRecord * pcr,
											pf_Frag_Strux* sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	PL_ListenerId lid,
																	fl_ContainerLayout* sfhNew));

	virtual bool				signal(UT_uint32 iSignal);

private:
	PD_Document*				m_pDoc;
	fl_HdrFtrShadow* 			m_pShadow;
	bool						m_bListening;
	fl_ContainerLayout*			m_pCurrentBL;
	fl_HdrFtrSectionLayout *    m_pHFSL;
	fl_TableLayout *            m_pCurrentTL;
	fl_CellLayout *             m_pCurrentCell;
};

#endif /* SECTIONLAYOUT_H */
