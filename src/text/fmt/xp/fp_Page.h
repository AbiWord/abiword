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
class fp_HdrFtrContainer;
class fl_HdrFtrSectionLayout;
class fl_DocSectionLayout;
class FV_View;
class GR_Graphics;
struct dg_DrawArgs;

// ----------------------------------------------------------------
class fp_Page
{
public:
	fp_Page(FL_DocLayout*,
			FV_View*,
			const fp_PageSize& pageSize,
			fl_DocSectionLayout* pOwner
		);
	~fp_Page();

	UT_sint32			getWidth(void) const;
	UT_sint32			getWidthInLayoutUnits(void) const;
	const fp_PageSize&	getPageSize() const;
	UT_sint32			getHeight(void) const;
	UT_sint32			getHeightInLayoutUnits(void) const;
	UT_sint32			getBottom(void) const;
	fp_Page*			getNext(void) const;
	fp_Page*			getPrev(void) const;
	void				setNext(fp_Page*);
	void				setPrev(fp_Page*);

	UT_sint32			getColumnGap(void) const {return getOwningSection()->getColumnGap(); }
						
	FL_DocLayout*		getDocLayout();
	void				setView(FV_View*);

	inline fl_DocSectionLayout* getOwningSection(void) const { return m_pOwner; }

	PT_DocPosition		getFirstLastPos(bool bFirst) const;
	void				mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	void				mapXYToPositionClick(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, fl_HdrFtrShadow *& pShadow,  bool& bBOL, bool& bEOL);
	void				getOffsets(fp_Container*, UT_sint32& xoff, UT_sint32& yoff);
	void				getScreenOffsets(fp_Container*, UT_sint32& xoff, UT_sint32& yoff);
						
	void				draw(dg_DrawArgs*, bool bAlaysUseWhiteBackground=false);
	bool				needsRedraw(void) const;
						
	void 				columnHeightChanged(fp_Column* pLeader);
	UT_uint32 			countColumnLeaders(void) const;
	fp_Column*			getNthColumnLeader(UT_sint32 n) const;
	bool				insertColumnLeader(fp_Column* pLeader, fp_Column* pAfter);
	void				removeColumnLeader(fp_Column* pLeader);
	bool				isEmpty(void) const;
	void                            removeHeader(void);
	void                            removeFooter(void);
	fp_HdrFtrContainer*             getHeaderP(void) const { return m_pHeader;}
	fp_HdrFtrContainer*             getFooterP(void) const { return m_pFooter;}
	fp_HdrFtrContainer*	getHeaderContainer(fl_HdrFtrSectionLayout*);
	fp_HdrFtrContainer*	getFooterContainer(fl_HdrFtrSectionLayout*);
	
#ifdef FMT_TEST
	void				__dump(FILE * fp) const;
#endif				  
	
protected:
    void                _drawCropMarks(dg_DrawArgs*);
	void				_reformat(void);

	FL_DocLayout*		m_pLayout;
	FV_View*			m_pView;
	fp_Page*			m_pNext;
	fp_Page*			m_pPrev;

	fp_PageSize			m_pageSize;
	UT_uint32			m_iResolution;	// in points per inch

	bool				m_bNeedsRedraw;

	UT_Vector			m_vecColumnLeaders;

	fl_DocSectionLayout*	m_pOwner;

	fp_HdrFtrContainer* m_pFooter;
	fp_HdrFtrContainer* m_pHeader;
};

#endif /* PAGE_H */
