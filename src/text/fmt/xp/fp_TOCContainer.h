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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
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
	virtual double      getHeight(void);
	virtual void		clearScreen(void);
	virtual void		draw(dg_DrawArgs*);
	virtual void		draw(GR_Graphics*);
	virtual void        setContainer(fp_Container * pContainer);
	virtual void        setY(double iY);
	virtual fp_Container * getNextContainerInSection(void) const;
	virtual fp_Container * getPrevContainerInSection(void) const;
	fp_Column *         getBrokenColumn(void);
	virtual bool        isVBreakable(void);
	virtual bool        isHBreakable(void) {return false;}
	virtual double      wantVBreakAt(double);
	virtual double      wantHBreakAt(double) {return 0;}
	virtual fp_ContainerObject * VBreakAt(UT_sint32);
	virtual fp_ContainerObject * HBreakAt(UT_sint32) {return NULL;}

	fl_DocSectionLayout * getDocSectionLayout(void);
	void                  setSelected(bool bIsSelected);
	fp_TOCContainer *   getMasterTOC(void) const
		{ return m_pMasterTOC; }
	bool                isThisBroken(void) const
		{ return m_bIsBroken;}
	void                setYBreakHere(double iBreakHere);
	void                setYBottom(double iBotContainer);
	bool                isInBrokenTOC(fp_Container * pCon); 
//
// This is the smallest Y value of the TOC allowed in this 
// broken TOC
//
	double           getYBreak(void) const
		{return m_iYBreakHere;}
//
// This is the largest Y value of the TOC allowed in this broken TOC
//
	double           getYBottom(void) const
		{return m_iYBottom;}
	fp_TOCContainer *   getFirstBrokenTOC(void) const;
	fp_TOCContainer *   getLastBrokenTOC(void) const;
	void                setFirstBrokenTOC(fp_TOCContainer * pBroke);
	void                setLastBrokenTOC(fp_TOCContainer * pBroke);
	void                deleteBrokenTOCs(bool bClearFirst);
	void                adjustBrokenTOCs(void);
	double              getBrokenTop(void);
	double              getBrokenBot(void);
	void                setBrokenTop(double iTop) 
		{ m_iBrokenTop = iTop;}
	void                setBrokenBot(double iBot) 
		{ m_iBrokenBottom = iBot;}
	UT_sint32           getBrokenNumber(void);
	void setLastWantedVBreak(double iBreakAt)
	{
		m_iLastWantedVBreak = iBreakAt;
	}
	double getLastWantedVBreak(void) const
	{
		return m_iLastWantedVBreak;
	}

private:
//
// Variables for TOC's broken across Vertical Containers.
//
	fp_TOCContainer *     m_pFirstBrokenTOC;
	fp_TOCContainer *     m_pLastBrokenTOC;
	bool                    m_bIsBroken;
	fp_TOCContainer *     m_pMasterTOC;
	double                  m_iYBreakHere;
	double                  m_iYBottom;
	double                  m_iBrokenTop;
	double                  m_iBrokenBottom;
	double	                m_iLastWantedVBreak;
};

#endif /* TOCCONTAINER_H */
