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

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"

class FL_DocLayout;
class fl_BlockLayout;
class fb_LineBreaker;
class fp_Column;
class PD_Document;
class PP_AttrProp;
class PX_ChangeRecord_StruxChange;

class fl_SectionLayout : public fl_Layout
{
	friend class fl_DocListener;

public:
	fl_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex ap);
	~fl_SectionLayout();

	FL_DocLayout*		getDocLayout() const;
	fp_Column*			getNewColumn();
	fp_Column*			getFirstColumn() const;
	fp_Column*			getLastColumn() const;
	int					format();

	inline UT_sint32			getLeftMargin(void) const { return m_iLeftMargin; }
	inline UT_sint32			getRightMargin(void) const { return m_iRightMargin; }
	inline UT_sint32			getTopMargin(void) const { return m_iTopMargin; }
	inline UT_sint32			getBottomMargin(void) const { return m_iBottomMargin; }
	
	UT_uint32			getNumColumns(void) const;
	UT_uint32			getColumnGap(void) const;

	fl_BlockLayout *	getFirstBlock(void) const;
	fl_BlockLayout *	getLastBlock(void) const;
	fl_BlockLayout *	appendBlock(PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP);
	fl_BlockLayout *	insertBlock(PL_StruxDocHandle sdh, fl_BlockLayout * pPrev, PT_AttrPropIndex indexAP);
	fl_BlockLayout *	removeBlock(fl_BlockLayout * pBL);

	void				deleteEmptyColumns(void);
	UT_Bool 			doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);

protected:
	void				_purgeLayout();
	fb_LineBreaker *	_getLineBreaker(void);
	void				_lookupProperties(void);

	FL_DocLayout*		m_pLayout;
	fb_LineBreaker*		m_pLB;

	fl_BlockLayout*		m_pFirstBlock;
	fl_BlockLayout*		m_pLastBlock;

	UT_uint32			m_iNumColumns;
	UT_uint32			m_iColumnGap;

	UT_sint32			m_iLeftMargin;
	UT_sint32			m_iRightMargin;
	UT_sint32			m_iTopMargin;
	UT_sint32			m_iBottomMargin;

	UT_Bool				m_bForceNewPage;

	fp_Column*			m_pFirstColumn;
	fp_Column*			m_pLastColumn;
};

#endif /* SECTIONLAYOUT_H */
