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
	virtual UT_sint32	getWidth(void) const = 0;
	virtual UT_sint32	getWidthInLayoutUnits(void) const = 0;
	virtual UT_sint32	getX(void) const = 0;
	virtual UT_sint32	getY(void) const = 0;
	inline fl_SectionLayout* getSectionLayout(void) const
		{ return m_pSectionLayout; }
	inline void         setSectionLayout(fl_SectionLayout * pSL)
		{ m_pSectionLayout = pSL; }
	virtual UT_sint32	getHeight(void) const = 0;
	virtual UT_sint32	getHeightInLayoutUnits(void) const = 0;
	virtual void		draw(dg_DrawArgs*) = 0;
	virtual void		clearScreen(void) = 0;
    GR_Graphics*        getGraphics(void) 
		{ return m_pG;}
    virtual fp_ContainerObject * getNext(void) = 0;
    virtual fp_ContainerObject * getPrev(void) = 0;
    virtual void        setNext(fp_ContainerObject * pNext) = 0;
    virtual void        setPrev(fp_ContainerObject * pNext) = 0;
	virtual bool        isVBreakable(void) = 0;
	virtual bool        isHBreakable(void) = 0;
	virtual UT_sint32   wantVBreakAt(UT_sint32) = 0;
	virtual UT_sint32   wantHBreakAt(UT_sint32) = 0;
	virtual fp_ContainerObject * VBreakAt(UT_sint32) =0;
	virtual fp_ContainerObject * HBreakAt(UT_sint32) = 0;
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
      Graphics frawing area
    */
	GR_Graphics *           m_pG;
};

#endif /*  CONTAINEROBJECT_H */

