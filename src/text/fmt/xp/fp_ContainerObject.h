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
 *
 * Code for allocation/requisition:
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef CONTAINEROBJECT_H
#define CONTAINEROBJECT_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_misc.h"
#include "ut_types.h"
#include "gr_Graphics.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fribidi_types.h"

typedef struct _fp_Requisition	  fp_Requisition;
typedef struct _fp_Allocation    fp_Allocation;

/*! 
 *  A requisition is a desired amount of space which a
 *  container may request.
 */
struct _fp_Requisition
{
  UT_sint32 width;
  UT_sint32 height;
};

/*!
 *  An allocation is a size and position. Where a container
 *  can ask for a desired size, it is actually given
 *  this amount of space at the specified position.
 */
struct _fp_Allocation
{
  UT_sint32 x;
  UT_sint32 y;
  UT_sint32 width;
  UT_sint32 height;
};

class GR_Graphics;
class fl_SectionLayout;
class fp_Page;
class fp_Container;
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
	FP_ContainerType	getContainerType(void) const { return m_iType; }
	bool                isColumnType(void) const;
	virtual void		setWidth(UT_sint32) = 0;
	virtual void		setHeight(UT_sint32) = 0 ;
	virtual void		setX(UT_sint32, bool bDontClearIfNeeded = false) = 0;
	virtual void		setY(UT_sint32) = 0;
	virtual UT_sint32	getWidth(void) const = 0;

	// all containers that may wish to draw stuff outwith the normal
	// drawing region, such as the pilcrow on a line, should overwrite this
	virtual UT_sint32   getDrawingWidth(void) const {return getWidth();}
#ifndef WITH_PANGO
	virtual void		setWidthInLayoutUnits(UT_sint32) =0;
	virtual void        setHeightLayoutUnits(UT_sint32 ihLayout) =0;
	virtual void		setYInLayoutUnits(UT_sint32) = 0;
	virtual UT_sint32	getWidthInLayoutUnits(void) const = 0;
	virtual UT_sint32	getHeightInLayoutUnits(void) const = 0;
#endif

	virtual UT_sint32	getX(void) const = 0;
	virtual UT_sint32	getY(void) const = 0;
	fl_SectionLayout*   getSectionLayout(void) const
		{ return m_pSectionLayout; }
    void         setSectionLayout(fl_SectionLayout * pSL)
		{ m_pSectionLayout = pSL; }
	virtual inline FriBidiCharType getDirection(void)
		{ return m_iDirection;}
	virtual inline void setDirection(FriBidiCharType c) {m_iDirection = c;}
	virtual UT_sint32	getHeight(void) const = 0;


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
	fl_SectionLayout*		        m_pSectionLayout;
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

	virtual void           setContainer(fp_Container * pContainer);
    fp_Container *         getContainer(void) const;
	fp_Container *         getColumn(void) const;
	fp_Page *              getPage(void) const;
	virtual UT_sint32      getMarginBefore(void) const =0;
	virtual UT_sint32      getMarginAfter(void) const =0;
#ifndef WITH_PANGO
	virtual UT_sint32      getMarginBeforeInLayoutUnits(void) const =0;
	virtual UT_sint32      getMarginAfterInLayoutUnits(void) const =0;
#endif
    virtual fp_ContainerObject * getNext(void) const {return m_pNext;}
    virtual fp_ContainerObject * getPrev(void) const {return m_pPrev;}
    virtual void        setNext(fp_ContainerObject * pNext)
		{m_pNext = pNext;}
    virtual void        setPrev(fp_ContainerObject * pPrev)
		{m_pPrev = pPrev;}
	void                   clearCons(void)
		{ m_vecContainers.clear();}
	fp_ContainerObject *   getNthCon(UT_uint32 i) const
		{ if(countCons() == 0) return NULL;
		   return (fp_ContainerObject *) m_vecContainers.getNthItem(i);}
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
	virtual void        recalcMaxWidth(bool bDontClearIfNeeded = false) = 0;
	virtual void        setAssignedScreenHeight(UT_sint32 iY) =0;
	bool                getPageRelativeOffsets(UT_Rect &r) const;
	bool                isOnScreen() const;
private:
	fp_Container*          m_pContainer;
	fp_ContainerObject *   m_pNext;
	fp_ContainerObject *   m_pPrev;
	UT_Vector              m_vecContainers;
};

#endif /*  CONTAINEROBJECT_H */






