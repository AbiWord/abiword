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
	FL_SECTION_FRAME
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
#include "fb_ColumnBreaker.h"

class fp_Page;
class FL_DocLayout;
class fl_Layout;
class fl_ContainerLayout;
class fl_BlockLayout;
class fl_HdrFtrSectionLayout;
class fl_HdrFtrShadow;
class fl_FootnoteLayout;
class fb_LineBreaker;
class fp_ShadowContainer;
class fp_Column;
class fp_Run;
class fp_Line;
class fp_Container;
class fp_HdrFtrContainer;
class PD_Document;
class PP_AttrProp;
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

class ABI_EXPORT fl_SectionLayout : public fl_ContainerLayout
{
	friend class fl_DocListener;

public:
	fl_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex ap, SectionType iType, fl_ContainerType iCType, PTStruxType ptType, fl_ContainerLayout * pMyContainerLayout);
	virtual ~fl_SectionLayout();

	SectionType     	getType(void) const { return m_iType; }

	virtual bool		recalculateFields(UT_uint32 iUpdateCount);

	virtual fp_Container*		getNewContainer(fp_Container * pFirstContainer = NULL) = 0;
    virtual void                updateLayout(void) = 0;
	virtual FL_DocLayout*		getDocLayout(void) const;
	virtual void                markAllRunsDirty(void) =0;
	virtual bool                isCollapsed(void) const
		{return m_bIsCollapsed;}
	virtual void                setNeedsReformat(UT_uint32 offset = 0);
	virtual void                setNeedsRedraw(void);
	virtual bool                needsReformat(void) const
		{return m_bNeedsReformat;}
	virtual bool                needsRedraw(void) const
		{return m_bNeedsRedraw;}
	virtual void        updateBackgroundColor(void);

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
											PL_StruxDocHandle sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	PL_ListenerId lid,
																	PL_StruxFmtHandle sfhNew));
	virtual bool bl_doclistener_insertSection(fl_ContainerLayout*,
											  SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));
	virtual fl_SectionLayout * bl_doclistener_insertTable(fl_ContainerLayout*,
											  SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));

	virtual bool bl_doclistener_insertObject(fl_ContainerLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_deleteObject(fl_ContainerLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_changeObject(fl_ContainerLayout*, const PX_ChangeRecord_ObjectChange * pcroc);

	virtual bool bl_doclistener_insertFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_deleteFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_changeFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMarkChange * pcrfmc);

#ifdef FMT_TEST
	virtual void		__dump(FILE * fp) const;
#endif

protected:
	virtual void		_lookupProperties(void) = 0;

	void				_purgeLayout();

	SectionType			m_iType;

	FL_DocLayout*		m_pLayout;
	bool                m_bIsCollapsed;
	bool                m_bNeedsReformat;
	bool                m_bNeedsRedraw;
};

class ABI_EXPORT fl_DocSectionLayout : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_DocSectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex ap, SectionType iType);
	virtual ~fl_DocSectionLayout();

	fl_DocSectionLayout* getNextDocSection(void) const;
	fl_DocSectionLayout* getPrevDocSection(void) const;

	virtual void		format(void);
	virtual void		updateLayout(void);
	void                updateDocSection(void);
	virtual void        collapse(void);
	virtual fp_Container * getFirstContainer(void) const;
	virtual fp_Container * getLastContainer(void) const;
	virtual void        setFirstContainer(fp_Container * pCon);
	virtual void        setLastContainer(fp_Container * pCon);
	
	fl_FootnoteLayout  *       getFootnoteLayout(UT_uint32 footnotePID);


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
	UT_uint32                   getColumnGap(void) const;
	UT_uint32			        getFootnoteLineThickness(void) const
		{ return m_iFootnoteLineThickness;}

	void                completeBreakSection(void);
	bool                arePageNumbersRestarted (void) const { return m_bRestart;}
	UT_sint32           getRestartedPageNumber(void) const { return m_iRestartValue;}
	UT_uint32			getNumColumns(void) const;
	bool				getColumnLineBetween(void) const {return m_bColumnLineBetween;}
	UT_uint32			getColumnOrder(void) const;
	void                setPaperColor();
	UT_RGBColor *       getPaperColor(void);
	void				deleteEmptyColumns(void);
	virtual bool 	    doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	bool				doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);

	virtual bool        bl_doclistener_insertFootnote(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx, 
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));

	void				setHdrFtr(HdrFtrType iType, fl_HdrFtrSectionLayout* pHFSL);

	fl_HdrFtrSectionLayout*         getHeader(void);
	fl_HdrFtrSectionLayout*         getFooter(void);
	fl_HdrFtrSectionLayout*         getHeaderEven(void);
	fl_HdrFtrSectionLayout*         getFooterEven(void);
	fl_HdrFtrSectionLayout*         getHeaderFirst(void);
	fl_HdrFtrSectionLayout*         getFooterFirst(void);
	fl_HdrFtrSectionLayout*         getHeaderLast(void);
	fl_HdrFtrSectionLayout*         getFooterLast(void);

	void				addOwnedPage(fp_Page*);
	void                            prependOwnedHeaderPage(fp_Page * p_Page);
	void                            prependOwnedFooterPage(fp_Page * p_Page);
	void				deleteOwnedPage(fp_Page*, bool bReallyDeleteIT=true);
	void                markForRebuild(void) { m_bNeedsRebuild = true;}
	void                clearRebuild(void) { m_bNeedsRebuild = false;}
	bool                needsRebuild(void) const { return m_bNeedsRebuild;}
	void				checkAndAdjustColumnGap(UT_sint32 iLayoutWidth);
    void                markForReformat(void) { m_bNeedsFormat = true;}
    bool                needsReFormat(void) const { return m_bNeedsFormat;}
	bool                isThisPageValid(HdrFtrType hfType, fp_Page * pThisPage);
    void                getVecOfHdrFtrs(UT_Vector * vecHdrFtr);
	void                formatAllHdrFtr(void);
	void                checkAndRemovePages(void);
	void                addValidPages(void);
	void                setNeedsSectionBreak(bool bSet, fp_Page * pPage );
	bool                needsSectionBreak(void) const { return m_bNeedsSectionBreak;}
	void                setFirstEndnoteContainer(fp_EndnoteContainer * pECon);
	void                setLastEndnoteContainer(fp_EndnoteContainer * pECon);
	fp_Container *      getFirstEndnoteContainer(void);
	fp_Container *      getLastEndnoteContainer(void);
	void                deleteBrokenTablesFromHere(fl_TableLayout * pTL);
private:
	virtual void		_lookupProperties(void);
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
	UT_uint32			m_iColumnGap;
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
	
	bool				m_bForceNewPage;

	//! First column in the section
	fp_Column*			m_pFirstColumn;
	//! Last column in the section
	fp_Column*			m_pLastColumn;
	fp_Page *           m_pFirstOwnedPage;
	UT_RGBColor         m_clrPaper;

	bool                m_bNeedsFormat;
	bool                m_bNeedsRebuild;
	bool                m_bNeedsSectionBreak;
	fp_EndnoteContainer * m_pFirstEndnoteContainer;
	fp_EndnoteContainer * m_pLastEndnoteContainer;
	bool                m_bDeleteingBrokenContainers;
};

class ABI_EXPORT fl_HdrFtrSectionLayout : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_HdrFtrSectionLayout(HdrFtrType iHFType, FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex ap);
	virtual ~fl_HdrFtrSectionLayout();

	inline fl_DocSectionLayout*	getDocSectionLayout(void) const { return m_pDocSL; }
	HdrFtrType      			getHFType(void) const { return m_iHFType; }
	void                        setDocSectionLayout(fl_DocSectionLayout * pDSL) { m_pDocSL = pDSL;}
	void                        setHdrFtr(HdrFtrType iHFType) { 	m_iHFType = iHFType;}
	virtual bool				recalculateFields(UT_uint32 iUpdateCount);
	bool                        doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
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
	virtual void				updateLayout(void);
	void                        layout(void);
	fl_ContainerLayout *        findMatchingContainer( fl_ContainerLayout * pBL);
	virtual void				redrawUpdate(void);
	void                        updateBackgroundColor(void);
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

	virtual bool bl_doclistener_populateSpan(fl_ContainerLayout*, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	virtual bool bl_doclistener_populateObject(fl_ContainerLayout*, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_insertSpan(fl_ContainerLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_deleteSpan(fl_ContainerLayout*, const PX_ChangeRecord_Span * pcrs);
	virtual bool bl_doclistener_changeSpan(fl_ContainerLayout*, const PX_ChangeRecord_SpanChange * pcrsc);
	virtual bool bl_doclistener_deleteStrux(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx);
	virtual bool bl_doclistener_changeStrux(fl_ContainerLayout*, const PX_ChangeRecord_StruxChange * pcrxc);
	virtual fl_SectionLayout * bl_doclistener_insertTable(fl_ContainerLayout*,
											  SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));
	virtual bool bl_doclistener_insertBlock(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx,
											PL_StruxDocHandle sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	PL_ListenerId lid,
																	PL_StruxFmtHandle sfhNew));
	virtual bool bl_doclistener_insertSection(fl_ContainerLayout*, const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));
	virtual bool bl_doclistener_insertObject(fl_ContainerLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_deleteObject(fl_ContainerLayout*, const PX_ChangeRecord_Object * pcro);
	virtual bool bl_doclistener_changeObject(fl_ContainerLayout*, const PX_ChangeRecord_ObjectChange * pcroc);

	virtual bool bl_doclistener_insertFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_deleteFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMark * pcrfm);
	virtual bool bl_doclistener_changeFmtMark(fl_ContainerLayout*, const PX_ChangeRecord_FmtMarkChange * pcrfmc);

private:
	UT_sint32					_findShadow(fp_Page * pPage);
	virtual void				_lookupProperties(void);
	void                        _localCollapse(void);

	fl_DocSectionLayout*		m_pDocSL;
	HdrFtrType					m_iHFType;
	UT_Vector					m_vecPages;
	fp_Container *              m_pHdrFtrContainer;
};


class ABI_EXPORT fl_HdrFtrShadow : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_HdrFtrShadow(FL_DocLayout* pLayout, fp_Page* pPage, fl_HdrFtrSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex ap);
	virtual ~fl_HdrFtrShadow();

virtual	fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const { return m_pHdrFtrSL; }
	fl_ContainerLayout *		findMatchingContainer(fl_ContainerLayout * pBL);
	fl_ContainerLayout *		findBlockAtPosition(PT_DocPosition pos);
	virtual void				format(void);
	virtual void				updateLayout(void);
	virtual void				redrawUpdate(void);
	fp_Page *                       getPage(void) { return m_pPage;}
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
	virtual void				_lookupProperties(void);
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

	virtual bool				populate(PL_StruxFmtHandle sfh,
										 const PX_ChangeRecord * pcr);

	virtual bool				populateStrux(PL_StruxDocHandle sdh,
											  const PX_ChangeRecord * pcr,
											  PL_StruxFmtHandle * psfh);

	virtual bool				change(PL_StruxFmtHandle sfh,
									   const PX_ChangeRecord * pcr);

	virtual bool				insertStrux(PL_StruxFmtHandle sfh,
											const PX_ChangeRecord * pcr,
											PL_StruxDocHandle sdh,
											PL_ListenerId lid,
											void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	PL_ListenerId lid,
																	PL_StruxFmtHandle sfhNew));

	virtual bool				signal(UT_uint32 iSignal);

private:
	PD_Document*				m_pDoc;
	fl_HdrFtrShadow* 			m_pShadow;
	bool						m_bListening;
	fl_ContainerLayout*			m_pCurrentBL;
	fl_HdrFtrSectionLayout *    m_pHFSL;
};

#endif /* SECTIONLAYOUT_H */
