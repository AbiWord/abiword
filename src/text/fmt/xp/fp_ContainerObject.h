/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior
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

#ifndef CONTAINEROBJECT_H
#define CONTAINEROBJECT_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_misc.h"
#include "ut_types.h"
#include "gr_Graphics.h"
#include "fl_SectionLayout.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fp_Page.h"
#include "fribidi_types.h"
class GR_Graphics;
struct dg_DrawArgs;
struct fp_Sliver;

struct dg_DrawArgs;

typedef enum {
	FP_CONTAINER_RUN,
	FP_CONTAINER_LINE,
	FP_CONTAINER_VERTICAL,
	FP_CONTAINER_ROW,
	FP_CONTAINER_TABLE,
	FP_CONTAINER_CELL,
	FP_CONTAINER_COLUMN,
	FP_CONTAINER_HDRFTR,
	FP_CONTAINER_ENDNOTE,
	FP_CONTAINER_FOOTNOTE,
	FP_CONTAINER_COLUMN_POSITIONED,
	FP_CONTAINER_COLUMN_SHADOW
} FP_ContainerType;

class ABI_EXPORT fp_ContainerObject
{
public:
	fp_ContainerObject(FP_ContainerType iType, fl_SectionLayout* pSectionLayout);
	virtual ~fp_ContainerObject();
	/*!
	  Return container type
	  \return Type
	*/
	inline FP_ContainerType	getContainerType(void) const { return m_iType; }
	
	virtual void		setWidth(UT_sint32) = 0;
	virtual void		setWidthInLayoutUnits(UT_sint32) =0;
	virtual void		setHeight(UT_sint32) = 0 ;
	virtual void        setHeightLayoutUnits(UT_sint32 ihLayout) =0;
	virtual void		setX(UT_sint32) = 0;
	virtual void		setY(UT_sint32) = 0;
	virtual void		setYInLayoutUnits(UT_sint32) = 0;
	virtual UT_sint32	getWidth(void) const = 0;
	virtual UT_sint32	getWidthInLayoutUnits(void) const = 0;
	virtual UT_sint32	getX(void) const = 0;
	virtual UT_sint32	getY(void) const = 0;
	inline fl_SectionLayout* getSectionLayout(void) const
		{ return m_pSectionLayout; }
	inline void         setSectionLayout(fl_SectionLayout * pSL)
		{ m_pSectionLayout = pSL; }
	virtual inline FriBidiCharType getDirection(void) 
		{ return m_iDirection;}
	virtual inline void setDirection(FriBidiCharType c) {m_iDirection = c;}
	virtual UT_sint32	getHeight(void) const = 0;
	virtual UT_sint32	getHeightInLayoutUnits(void) const = 0;
	virtual void		draw(dg_DrawArgs*) = 0;
	virtual void		draw(GR_Graphics*) = 0;
	virtual void		clearScreen(void) = 0;
    GR_Graphics*        getGraphics(void) 
		{ return m_pG;}
    virtual fp_ContainerObject * getNext(void) const = 0;
    virtual fp_ContainerObject * getPrev(void) const = 0;
    virtual void        setNext(fp_ContainerObject * pNext) = 0;
    virtual void        setPrev(fp_ContainerObject * pNext) = 0;
	virtual bool        isVBreakable(void) = 0;
	virtual bool        isHBreakable(void) = 0;
	virtual UT_sint32   wantVBreakAt(UT_sint32) = 0;
	virtual UT_sint32   wantHBreakAt(UT_sint32) = 0;
	virtual fp_ContainerObject * VBreakAt(UT_sint32) =0;
	virtual fp_ContainerObject * HBreakAt(UT_sint32) = 0;
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL) = 0;
	virtual fp_Container * getNextContainerInSection(void) const = 0;
	virtual fp_Container * getPrevContainerInSection(void) const = 0;
private:
	/*!
	  Container type
	*/
	FP_ContainerType		m_iType;
	/*!
	  Section layout type used for this container
	*/
	fl_SectionLayout*		m_pSectionLayout;
	/*!
      Graphics drawing area
    */
	GR_Graphics *           m_pG;
	FriBidiCharType         m_iDirection;
};


class ABI_EXPORT fp_Container : public fp_ContainerObject
{
public:
	fp_Container(FP_ContainerType iType, fl_SectionLayout* pSectionLayout);
	virtual ~fp_Container();

	void                   setContainer(fp_Container * pContainer);
    fp_Container *         getContainer(void) const;
	fp_Container *         getColumn(void) const;
	fp_Page *              getPage(void) const;
	virtual UT_sint32      getMarginBefore(void) const =0;
	virtual UT_sint32      getMarginAfter(void) const =0;
	virtual UT_sint32      getMarginBeforeInLayoutUnits(void) const =0;
	virtual UT_sint32      getMarginAfterInLayoutUnits(void) const =0;
    virtual fp_ContainerObject * getNext(void) const {return m_pNext;}
    virtual fp_ContainerObject * getPrev(void) const {return m_pPrev;}
    virtual void        setNext(fp_ContainerObject * pNext) 
		{m_pNext = pNext;}
    virtual void        setPrev(fp_ContainerObject * pPrev)
		{m_pPrev = pPrev;}
	void                   clearCons(void) 
		{ m_vecContainers.clear();}
	fp_ContainerObject *   getNthCon(UT_uint32 i) const
		{ return (fp_ContainerObject *) m_vecContainers.getNthItem(i);}
	void                   addCon(fp_ContainerObject * pCon) 
		{m_vecContainers.addItem((void *) pCon);}
	UT_uint32              countCons(void) const
		{return m_vecContainers.getItemCount();}
	UT_sint32              findCon(fp_ContainerObject * pCon) const
		{return m_vecContainers.findItem((void *) pCon);}
	void                   deleteNthCon(UT_sint32 i)
		{m_vecContainers.deleteNthItem(i);}
	void                   insertConAt(fp_ContainerObject * pCon, UT_sint32 i)
		{m_vecContainers.insertItemAt(pCon,i);}
	bool                   isEmpty(void) const
		{return m_vecContainers.getItemCount() == 0;}
	virtual UT_uint32 	distanceFromPoint(UT_sint32 x, UT_sint32 y) =0;
	virtual void        recalcMaxWidth(void) = 0;
	virtual void        setAssignedScreenHeight(UT_sint32 iY) =0;
private:
	fp_Container*          m_pContainer;
	fp_ContainerObject *   m_pNext;
	fp_ContainerObject *   m_pPrev;
	UT_Vector              m_vecContainers;
};

#endif /*  CONTAINEROBJECT_H */






