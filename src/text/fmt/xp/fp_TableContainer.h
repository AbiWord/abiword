/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
 *
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

#ifndef TABLECONTAINER_H
#define TABLECONTAINER_H

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

struct fp_Requisition;
struct fp_Allocation;

class fp_VerticalContainer;
class fp_Column;
class fl_EndnoteSectionLayout;
class fl_HdrFtrSectionLayout;
class fl_DocSectionLayout;
class fl_SectionLayout;
class fl_HdrFtrShadow;
class fp_Page;
class PP_AttrProp;
class GR_Graphics;
struct dg_DrawArgs;
struct fp_Sliver;

class ABI_EXPORT fp_CellContainer : public fp_VerticalContainer
{
public:
	fp_CellContainer(FP_ContainerType iType, fl_SectionLayout* pSectionLayout);
	virtual ~fp_CellContainer();

	void                sizeRequest(fp_Request * pRequest);
	void                sizeAllocate(fp_Allocate * pAllocate);

	virtual void		draw(dg_DrawArgs*);
	virtual void		draw(GR_Graphics*) {}

	virtual bool        isVBreakable(void);
	virtual bool        isHBreakable(void) {return false;}
	virtual UT_sint32   wantVBreakAt(UT_sint32);
	virtual UT_sint32   wantHBreakAt(UT_sint32) {return 0;}
	virtual fp_ContainerObject * VBreakAt(UT_sint32);
	virtual fp_ContainerObject * HBreakAt(UT_sint32) {return NULL;}
	void                recalcMaxWidth(bool bDontClearIfNeeded = false) {}
	virtual UT_sint32   getMarginBefore(void) const;
	virtual UT_sint32   getMarginAfter(void) const;
	virtual UT_sint32   getMarginBeforeInLayoutUnits(void) const;
	virtual UT_sint32   getMarginAfterInLayoutUnits(void) const;
	virtual void        setAssignedScreenHeight(UT_sint32) {}
	virtual fp_Container * getNextContainerInSection(void) const;
	virtual fp_Container * getPrevContainerInSection(void) const;

private:
	

};

class ABI_EXPORT fp_Column : public fp_VerticalContainer
{
public:
	fp_Column(fl_SectionLayout* pSectionLayout);
	~fp_Column();

	fl_DocSectionLayout*	getDocSectionLayout(void) const;

	inline void			setLeader(fp_Column* p) { m_pLeader = p; }
	inline void			setFollower(fp_Column* p) { m_pFollower = p; }
	inline fp_Column*	getLeader(void) const 			{ return m_pLeader; }
	inline fp_Column*	getFollower(void) const 		{ return m_pFollower; }
	/*!
	  Get page container is located on
	  \return Page
	*/

	void				setPage(fp_Page* pPage) {m_pPage = pPage ;}
	virtual inline fp_Page*		getPage(void) const
		{ return m_pPage; }

	void				layout(void);


#ifdef FMT_TEST
	void				__dump(FILE * fp) const;
#endif

protected:
	UT_uint32 				_getBottomOfLastContainer(void) const;

	void					_drawBoundaries(dg_DrawArgs* pDA);

private:

	fp_Column*				m_pLeader;
	fp_Column*				m_pFollower;
	fp_Page*				m_pPage;
};

class ABI_EXPORT fp_ShadowContainer : public fp_VerticalContainer
{
public:
	fp_ShadowContainer(UT_sint32 iX, UT_sint32 iY,
					   UT_sint32 iWidth, UT_sint32 iHeight,
					   UT_sint32 iWidthLayout, UT_sint32 iHeightLayout,
					   fl_SectionLayout* pSL);
	~fp_ShadowContainer();

	fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const;
	fl_HdrFtrShadow *   getShadow();
 	virtual void		draw(dg_DrawArgs*);
 	virtual void		draw(GR_Graphics*) {};
  	virtual void		layout(void);
 	virtual void		clearScreen(void);
	void                clearHdrFtrBoundaries(void);
	void				setPage(fp_Page* pPage) {m_pPage = pPage ;}
	virtual inline fp_Page*		getPage(void) const
		{ return m_pPage; }
protected:
	void                _drawHdrFtrBoundaries(dg_DrawArgs * pDA);
private:
	bool                m_bHdrFtrBoxDrawn;
	UT_sint32           m_ixoffBegin;
	UT_sint32           m_iyoffBegin;
	UT_sint32           m_ixoffEnd;
	UT_sint32           m_iyoffEnd;
	fp_Page*			m_pPage;

};


class ABI_EXPORT fp_HdrFtrContainer : public fp_VerticalContainer
{
public:
	fp_HdrFtrContainer( UT_sint32 iWidth,
					   UT_sint32 iWidthLayout,
					   fl_SectionLayout* pSL);
	~fp_HdrFtrContainer();

	fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const;
 	virtual void		draw(dg_DrawArgs*);
  	virtual void		layout(void);
 	virtual void		clearScreen(void);
	void		 		getScreenOffsets(fp_ContainerObject* pContainer, UT_sint32& xoff,
										 UT_sint32& yoff);

protected:
};

class fp_EndnoteSectionContainer : public fp_VerticalContainer
{
public:
	fp_EndnoteSectionContainer(fl_SectionLayout* pSectionLayout);
	~fp_EndnoteSectionContainer();

	fl_EndnoteSectionLayout*	getEndnoteSectionLayout(void) const;

 	virtual void		draw(dg_DrawArgs*);
  	virtual void		layout(void);
 	virtual void		clearScreen(void);
	virtual inline fp_Page*		getPage(void) const
		{ return m_pPage; }
protected:
private:
	fp_Page*				m_pPage;
};

#endif /* COLUMN_H */
