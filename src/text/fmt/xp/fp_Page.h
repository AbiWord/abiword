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



#ifndef PAGE_H
#define PAGE_H

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class FL_DocLayout;
class fp_Column;
class fl_SectionLayout;
class FV_View;
class DG_Graphics;
struct dg_DrawArgs;

// ----------------------------------------------------------------
class fp_Page
{
 public:
	fp_Page(FL_DocLayout*,
			FV_View*,
			UT_uint32 iWidth,
			UT_uint32 iHeight);
	~fp_Page();

	UT_sint32		getWidth(void) const;
	UT_sint32		getHeight(void) const;
	UT_sint32		getBottom(void) const;
	fp_Page*		getNext(void) const;
	fp_Page*		getPrev(void) const;
	void			setNext(fp_Page*);
	void			setPrev(fp_Page*);
	
	FL_DocLayout*	getDocLayout();
	void            setView(FV_View*);

	void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	void			getOffsets(fp_Column*, UT_sint32& xoff, UT_sint32& yoff);
	void			getScreenOffsets(fp_Column*, UT_sint32& xoff, UT_sint32& yoff);

	void			draw(dg_DrawArgs*);
	UT_Bool			needsRedraw(void) const;

	void 			columnHeightChanged(fp_Column* pLeader);
	UT_uint32 		countColumnLeaders(void) const;
	fp_Column*		getNthColumnLeader(UT_sint32 n) const;
	UT_Bool			insertColumnLeader(fp_Column* pLeader, fp_Column* pAfter);
#if 0
	UT_Bool			addColumnLeader(fp_Column* pLeader);
#endif
	void			removeColumnLeader(fp_Column* pLeader);
	UT_Bool			isEmpty(void) const;
	
#ifdef FMT_TEST
	void			__dump(FILE * fp) const;
#endif
	
protected:
	void				_reformat(void);

	FL_DocLayout*		m_pLayout;
	FV_View*			m_pView;
	fp_Page*			m_pNext;
	fp_Page*			m_pPrev;

	UT_sint32			m_iWidth;
	UT_sint32			m_iHeight;

	UT_Bool				m_bNeedsRedraw;

	UT_Vector			m_vecColumnLeaders;
};

#endif /* PAGE_H */
