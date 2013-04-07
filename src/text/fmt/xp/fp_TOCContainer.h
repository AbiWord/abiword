/* AbiWord
 * Copyright (C) 2004 Martin Sevior <msevior@physics.unimelb.edu.au>
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
 *
 */

#ifndef TOCCONTAINER_H
#define TOCCONTAINER_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fp_Page.h"
#include "fp_ContainerObject.h"
#include "fp_Column.h"
#include "gr_Graphics.h"

class fl_DocSectionLayout;

class ABI_EXPORT fp_TOCContainer : public fp_VerticalContainer
{
public:
	fp_TOCContainer(fl_SectionLayout* pSectionLayout);
	fp_TOCContainer(fl_SectionLayout* pSectionLayout,fp_TOCContainer * pMaster);
	virtual ~fp_TOCContainer();
    virtual void        mapXYToPosition(UT_sint32 x, UT_sint32 y,
										PT_DocPosition& pos,
										bool& bBOL, bool& bEOL, bool &isTOC);
	UT_sint32           getValue(void);
	void				layout(void);
	void		        forceClearScreen(void);
	UT_sint32		getTotalTOCHeight(void) const;
	virtual UT_sint32   getHeight(void) const;
	virtual void		clearScreen(void);
	virtual void		draw(dg_DrawArgs*);
	virtual void		draw(GR_Graphics*);
	virtual void        setContainer(fp_Container * pContainer);
	virtual void        setY(UT_sint32 iY);
	virtual fp_Container * getNextContainerInSection(void) const;
	virtual fp_Container * getPrevContainerInSection(void) const;
	fp_Column *         getBrokenColumn(void);
	virtual bool        isVBreakable(void);
	virtual bool        isHBreakable(void) {return false;}
	virtual UT_sint32   wantVBreakAt(UT_sint32);
	virtual UT_sint32   wantHBreakAt(UT_sint32) {return 0;}
	virtual fp_ContainerObject * VBreakAt(UT_sint32);
	virtual fp_ContainerObject * HBreakAt(UT_sint32) {return NULL;}

	fl_DocSectionLayout * getDocSectionLayout(void);
	void                  setSelected(bool bIsSelected);
	fp_TOCContainer *   getMasterTOC(void) const
		{ return m_pMasterTOC; }
	bool                isThisBroken(void) const
		{ return m_bIsBroken;}
	void                setYBreakHere(UT_sint32 iBreakHere);
	void                setYBottom(UT_sint32 iBotContainer);
	bool                isInBrokenTOC(fp_Container * pCon);
//
// This is the smallest Y value of the TOC allowed in this
// broken TOC
//
	UT_sint32           getYBreak(void) const
		{return m_iYBreakHere;}
//
// This is the largest Y value of the TOC allowed in this broken TOC
//
	UT_sint32           getYBottom(void) const
		{return m_iYBottom;}
	fp_TOCContainer *   getFirstBrokenTOC(void) const;
	fp_TOCContainer *   getLastBrokenTOC(void) const;
	void                setFirstBrokenTOC(fp_TOCContainer * pBroke);
	void                setLastBrokenTOC(fp_TOCContainer * pBroke);
	void                deleteBrokenTOCs(bool bClearFirst);
	void                adjustBrokenTOCs(void);
	UT_sint32           getBrokenTop(void);
	UT_sint32           getBrokenBot(void);
	void                setBrokenTop(UT_sint32 iTop)
		{ m_iBrokenTop = iTop;}
	void                setBrokenBot(UT_sint32 iBot)
		{ m_iBrokenBottom = iBot;}
	UT_sint32           getBrokenNumber(void);
	virtual void setLastWantedVBreak(UT_sint32 iBreakAt)
		{m_iLastWantedVBreak = iBreakAt;}
	virtual UT_sint32 getLastWantedVBreak(void) const
		{return m_iLastWantedVBreak;}
	virtual fp_Container * getFirstBrokenContainer() const;
	virtual void    deleteBrokenAfter(bool bClearFirst);

private:
//
// Variables for TOC's broken across Vertical Containers.
//
	fp_TOCContainer *     m_pFirstBrokenTOC;
	fp_TOCContainer *     m_pLastBrokenTOC;
	bool                    m_bIsBroken;
	fp_TOCContainer *     m_pMasterTOC;
	UT_sint32               m_iYBreakHere;
	UT_sint32               m_iYBottom;
	UT_sint32               m_iBrokenTop;
	UT_sint32               m_iBrokenBottom;
	UT_sint32	            m_iLastWantedVBreak;
};

#endif /* TOCCONTAINER_H */
