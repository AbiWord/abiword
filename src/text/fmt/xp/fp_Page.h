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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef PAGE_H
#define PAGE_H

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_SectionLayout.h"
#include "fp_PageSize.h"


class FL_DocLayout;
class fp_Column;
class fp_Container;
class fp_ShadowContainer;
class fp_FootnoteContainer;
class fl_DocSectionLayout;
class fl_HdrFtrSectionLayout;
class fl_HdrFtrShadow;
class fl_FootnoteLayout;
class FV_View;
class GR_Graphics;
struct dg_DrawArgs;

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
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32			getWidthInLayoutUnits(void) const;
	UT_sint32			getHeightInLayoutUnits(void) const;
#else
	UT_sint32			getWidthInLayoutUnits(void) const;
	UT_sint32			getHeightInLayoutUnits(void) const;
#endif
	UT_sint32			getBottom(void) const;
	fp_Page*			getNext(void) const;
	fp_Page*			getPrev(void) const;
	void				setNext(fp_Page*);
	void				setPrev(fp_Page*);
	void                markAllDirty(void) {m_bNeedsRedraw = true;}
	UT_sint32			getColumnGap(void) const;
	FL_DocLayout*		getDocLayout() const;
	void				setView(FV_View*);

	inline fl_DocSectionLayout* getOwningSection(void) const { return m_pOwner; }

	PT_DocPosition		getFirstLastPos(bool bFirst) const;
	void				mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool bUseHdrFtr = false, fl_HdrFtrShadow ** pShadow = NULL);
	void				getScreenOffsets(fp_Container*, UT_sint32& xoff, UT_sint32& yoff) const;

	void				draw(dg_DrawArgs*, bool bAlaysUseWhiteBackground=false);
	bool				needsRedraw(void) const;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
    UT_sint32           getFilledHeightInLayoutUnits(fp_Container * prevContainer) const;
	UT_sint32           getAvailableHeightInLayoutUnits(void) const;
#else
    UT_sint32           getFilledHeight(fp_Container * prevContainer) const;
#endif
	UT_sint32           getAvailableHeight(void) const;

	// Leader (e.g. column) functions.
	void 				columnHeightChanged(fp_Column* pLeader);
	bool                breakPage(void);
	UT_uint32 			countColumnLeaders(void) const;
	fp_Column*			getNthColumnLeader(UT_sint32 n) const;
	bool				insertColumnLeader(fp_Column* pLeader, fp_Column* pAfter);
	void				removeColumnLeader(fp_Column* pLeader);
	bool				isEmpty(void) const;

	// Header/Footer functions.
	void                removeHdrFtr(HdrFtrType hfType);
	fp_ShadowContainer* getHdrFtrP(HdrFtrType hfType) const;
	fp_ShadowContainer*	getHdrFtrContainer(fl_HdrFtrSectionLayout*);
	fp_ShadowContainer*	buildHdrFtrContainer(fl_HdrFtrSectionLayout*, 
											 HdrFtrType hfType);

	// Footnote functions.
	UT_uint32			countFootnoteContainers(void) const;
	fp_FootnoteContainer* getNthFootnoteContainer(UT_sint32 n) const; 
	bool				insertFootnoteContainer(fp_FootnoteContainer * pFC, 
												fp_FootnoteContainer * pAfter);
	void				removeFootnoteContainer(fp_FootnoteContainer * pFC);

#ifdef FMT_TEST
	void				__dump(FILE * fp) const;
#endif

protected:
    void                _drawCropMarks(dg_DrawArgs*);
	void				_reformat(void);
	void				_reformatColumns(void);
	void				_reformatFootnotes(void);

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

	UT_Vector			m_vecColumnLeaders;

	fl_DocSectionLayout*	m_pOwner;

	fp_ShadowContainer* m_pFooter;
	fp_ShadowContainer* m_pHeader;

	UT_Vector			m_vecFootnotes;
};

#endif /* PAGE_H */
