/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifndef PAGE_H
#define PAGE_H

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_SectionLayout.h"
#include "fp_PageSize.h"
#include "fp_ContainerObject.h"

class FL_DocLayout;
class fp_Column;
class fp_Container;
class fp_ShadowContainer;
class fp_FootnoteContainer;
class fp_AnnotationContainer;
class fp_FrameContainer;
class fl_DocSectionLayout;
class fl_HdrFtrSectionLayout;
class fl_HdrFtrShadow;
class fl_FootnoteLayout;
class fl_AnnotationLayout;
class fl_FrameLayout;
class FV_View;
class GR_Graphics;
class fp_TableContainer;
class fl_TOCLayout;
struct dg_DrawArgs;

class ABI_EXPORT _BL
	{
	public:
		_BL(fl_BlockLayout * pBL,fp_Line * pL) :
			m_pBL(pBL),
			m_pL(pL)
			{
			}
		fl_BlockLayout * m_pBL;
		fp_Line * m_pL;
	};

// ----------------------------------------------------------------
class ABI_EXPORT fp_Page
{
public:
	fp_Page(FL_DocLayout*,
			FV_View*,
			const fp_PageSize& pageSize,
			fl_DocSectionLayout* pOwner
		);
	~fp_Page();

	UT_sint32			getWidth(void) const;
	const fp_PageSize&	getPageSize() const;
	UT_sint32			getHeight(void) const;
	UT_sint32			getBottom(void) const;
	fp_Page*			getNext(void) const;
	fp_Page*			getPrev(void) const;
	void				setNext(fp_Page*);
	void				setPrev(fp_Page*);
	void                markAllDirty(void) {m_bNeedsRedraw = true;}
	UT_sint32			getColumnGap(void) const;
	FL_DocLayout*		getDocLayout() const;
	void				setView(FV_View*);
    bool                isOnScreen(void);

	inline fl_DocSectionLayout* getOwningSection(void) const { return m_pOwner; }

	PT_DocPosition		getFirstLastPos(bool bFirst) const;
	void				mapXYToPosition(bool bNotFrames,UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL,bool & isTOC, bool bUseHdrFtr = false, fl_HdrFtrShadow ** pShadow = NULL) const;
	void				mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC,bool bUseHdrFtr = false, fl_HdrFtrShadow ** pShadow = NULL) const;
	void				getScreenOffsets(fp_Container*, UT_sint32& xoff, UT_sint32& yoff) const;

	void				draw(dg_DrawArgs*, bool bAlaysUseWhiteBackground=false);
	bool				needsRedraw(void) const;
    UT_sint32           getFilledHeight(fp_Container * prevContainer) const;
	UT_sint32           getAvailableHeight(void) const;
	UT_sint32           getAvailableHeightForColumn(const fp_Column * pColumn) const;
	fp_TableContainer * getContainingTable(PT_DocPosition pos);
	void                clearCountWrapNumber(void);
	// Leader (e.g. column) functions.
	void 				columnHeightChanged(fp_Column* pLeader);
	bool                breakPage(void);
	UT_sint32 			countColumnLeaders(void) const;
	fp_Column*			getNthColumnLeader(UT_sint32 n) const;
	bool				insertColumnLeader(fp_Column* pLeader, fp_Column* pAfter);
	void				removeColumnLeader(fp_Column* pLeader);
	fp_Container*                   getNthColumn(UT_uint32 n,fl_DocSectionLayout *pSection) const;
	bool				isEmpty(void) const;
	bool                            containsPageBreak(void) const;
	fp_Container *      updatePageForWrapping(fp_Column *& pNextCol);
	// Header/Footer functions.
	void                removeHdrFtr(HdrFtrType hfType);
	fp_ShadowContainer* getHdrFtrP(HdrFtrType hfType) const;
	fp_ShadowContainer*	getHdrFtrContainer(fl_HdrFtrSectionLayout*);
	fp_ShadowContainer*	buildHdrFtrContainer(fl_HdrFtrSectionLayout*,
						     HdrFtrType hfType);

	// Footnote functions.
	void 				footnoteHeightChanged(void);
	UT_sint32			countFootnoteContainers(void) const;
	fp_FootnoteContainer* getNthFootnoteContainer(UT_sint32 n) const;
	bool				insertFootnoteContainer(fp_FootnoteContainer * pFC);
	void				removeFootnoteContainer(fp_FootnoteContainer * pFC);
	UT_sint32           findFootnoteContainer(fp_FootnoteContainer * pFC) const;
	void                clearScreenFootnotes(void);
	UT_sint32           getFootnoteHeight(void) const;


	// Annotation functions.
	void 				annotationHeightChanged(void);
	UT_sint32			countAnnotationContainers(void) const;
	fp_AnnotationContainer* getNthAnnotationContainer(UT_sint32 n) const;
	bool				insertAnnotationContainer(fp_AnnotationContainer * pFC);
	void				removeAnnotationContainer(fp_AnnotationContainer * pFC);
	UT_sint32           findAnnotationContainer(fp_AnnotationContainer * pFC) const;
	void                clearScreenAnnotations(void);
	UT_sint32           getAnnotationHeight(void) const;
	UT_sint32           getAnnotationPos( UT_uint32 pid) const;

	// Frame functions.
	void 				frameHeightChanged(void);
	UT_sint32			countAboveFrameContainers(void) const;
	UT_sint32			countBelowFrameContainers(void) const;
	fp_FrameContainer*  getNthAboveFrameContainer(UT_sint32 n) const;
	fp_FrameContainer*  getNthBelowFrameContainer(UT_sint32 n) const;
	bool				insertFrameContainer(fp_FrameContainer * pFC);
	void				removeFrameContainer(fp_FrameContainer * pFC);
	UT_sint32           findFrameContainer(fp_FrameContainer * pFC) const;
	void                clearScreenFrames(void);
	void                markDirtyOverlappingRuns(fp_FrameContainer * pFC);
    void                expandDamageRect(UT_sint32 x, UT_sint32 y,
										 UT_sint32 width, UT_sint32 height);
        bool                intersectsDamagedRect(fp_ContainerObject * pObj);
	void                redrawDamagedFrames(dg_DrawArgs* pDA);
	bool                overlapsWrappedFrame(fp_Line * pLine);
	bool                overlapsWrappedFrame(UT_Rect & rec);
	void                setPageNumberInFrames(void);
	UT_sint32           getPageNumber(void); // TODO make const
	UT_sint32           getFieldPageNumber(void) const;
	void                setFieldPageNumber(UT_sint32 iPageNum);
	void                resetFieldPageNumber(void);
	bool                TopBotMarginChanged(void);
	void                setLastMappedTOC(fl_TOCLayout * pTOCL)
		{ m_pLastMappedTOC = pTOCL;}
	fl_TOCLayout *      getLastMappedTOC(void) const
		{ return m_pLastMappedTOC;}
	fg_FillType &       getFillType(void);
	const fg_FillType & getFillType(void) const;
	void                getAllLayouts(UT_GenericVector<fl_ContainerLayout *> & AllLayouts) const;

#ifdef FMT_TEST
	void				__dump(FILE * fp) const;
#endif

	void                updateColumnX();
protected:
    void                _drawCropMarks(dg_DrawArgs*);
	void				_reformat(void);
	void				_reformatColumns(void);
	void				_reformatFootnotes(void);
	void				_reformatAnnotations(void);

private:
	// don't allow copying
	fp_Page(const fp_Page&);		// no impl.
	void operator=(const fp_Page&);	// no impl.

	FL_DocLayout*		m_pLayout;
	FV_View*			m_pView;
	fp_Page*			m_pNext;
	fp_Page*			m_pPrev;

	fp_PageSize			m_pageSize;
	UT_uint32			m_iResolution;	// in points per inch

	bool				m_bNeedsRedraw;

	UT_GenericVector<fp_Column *> m_vecColumnLeaders;

	fl_DocSectionLayout*	m_pOwner;

	fp_ShadowContainer* m_pFooter;
	fp_ShadowContainer* m_pHeader;

	UT_GenericVector<fp_FootnoteContainer *> m_vecFootnotes;
	UT_GenericVector<fp_AnnotationContainer *> m_vecAnnotations;
	fg_FillType         m_FillType;
	UT_GenericVector<fp_FrameContainer *> m_vecAboveFrames;
	UT_GenericVector<fp_FrameContainer *> m_vecBelowFrames;
	fl_TOCLayout *      m_pLastMappedTOC;

	UT_Rect             m_rDamageRect;
	UT_sint32           m_iCountWrapPasses;
	UT_sint32           m_iFieldPageNumber;
};

#endif /* PAGE_H */
