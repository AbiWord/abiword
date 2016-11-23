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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
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
#include "gr_Image.h"
#include "fg_Graphic.h"
#include "pp_PropertyMap.h"

class fp_ContainerObject;

typedef enum {
	FG_FILL_TRANSPARENT,
	FG_FILL_COLOR,
	FG_FILL_IMAGE
} FG_Fill_Type;

#define INITIAL_OFFSET -99999999

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
class fl_DocSectionLayout;
class FL_DocLayout;
class fp_Page;
class fp_Container;
struct dg_DrawArgs;
struct fp_Sliver;
struct dg_DrawArgs;

typedef enum {
	FP_CONTAINER_RUN,      //0
	FP_CONTAINER_LINE,     //1
	FP_CONTAINER_VERTICAL, //2
	FP_CONTAINER_ROW,      //3
	FP_CONTAINER_TABLE,    // 4
	FP_CONTAINER_CELL,     // 5
	FP_CONTAINER_COLUMN,   // 6
	FP_CONTAINER_HDRFTR,   // 7
	FP_CONTAINER_ENDNOTE,  // 8
	FP_CONTAINER_FOOTNOTE, // 9
	FP_CONTAINER_COLUMN_POSITIONED, // 10
	FP_CONTAINER_COLUMN_SHADOW, // 11
	FP_CONTAINER_FRAME, // 12
	FP_CONTAINER_TOC, // 13
	FP_CONTAINER_ANNOTATION // 14

} FP_ContainerType;

class ABI_EXPORT fg_FillType
{
	friend class fp_Run;
public:
	fg_FillType(fg_FillType *pParent, fp_ContainerObject * pContainer, FG_Fill_Type iType);
	virtual ~ fg_FillType(void);
	void           setParent(fg_FillType * pParent);
	void           setColor(UT_RGBColor & color);
	void           setColor(const char * pszColor);
	void           setTransColor(UT_RGBColor & color);
	void           setTransColor(const char * pszColor);
	void           setImage(FG_Graphic * pGraphic, GR_Image * pImage,GR_Graphics * pG, UT_sint32 width, UT_sint32 height);
	void           setTransparent(void);
	void           setWidthHeight(GR_Graphics * pG, UT_sint32 width, UT_sint32 height, bool doImage = false);
	void           setWidth(GR_Graphics * pG, UT_sint32 width);
	void           setHeight(GR_Graphics * pG, UT_sint32 height);
	void           setDocLayout(FL_DocLayout * pDocLayout);
	void           markTransparentForPrint(void);
	void           Fill(GR_Graphics * pG, UT_sint32 & srcX, UT_sint32 & srcY, UT_sint32 x, UT_sint32 y, UT_sint32 width, UT_sint32 height);
	fg_FillType *  getParent(void) const;
	FG_Fill_Type   getFillType(void) const;
	const FL_DocLayout * getDocLayout(void) const;
	const UT_RGBColor *  getColor(void) const;
	void           setImagePointer(const FG_SharedGraphicPtr & pDocGraphic, GR_Image ** pDocImage);
	void           setIgnoreLineLevel(bool b);
private:
    void        	     _regenerateImage(GR_Graphics * pG);
	fg_FillType *        m_pParent;
	fp_ContainerObject * m_pContainer;
	FL_DocLayout *       m_pDocLayout;
	FG_Fill_Type         m_FillType;
	GR_Image *           m_pImage;
	FG_Graphic *         m_pGraphic;
	UT_uint32            m_iGraphicTick;
	bool                 m_bTransparentForPrint;
	UT_RGBColor          m_color;
	UT_RGBColor          m_TransColor;
	bool                 m_bTransColorSet;
	bool                 m_bColorSet;
	UT_sint32            m_iWidth;
	UT_sint32            m_iHeight;
	GR_Image **          m_pDocImage;
	FG_SharedGraphicPtr  m_pDocGraphic;
	bool                 m_bIgnoreLineLevel;
};


class ABI_EXPORT fp_ContainerObject
{
public:
	fp_ContainerObject(FP_ContainerType iType, fl_SectionLayout* pSectionLayout);
	virtual ~fp_ContainerObject();
	/*!
	  Return container type
	  \return Type
	*/
	FP_ContainerType	getContainerType(void) const { return m_iConType; }
	bool                isColumnType(void) const;
	virtual void		setWidth(UT_sint32) = 0;
	virtual void		setHeight(UT_sint32) = 0 ;
	virtual void		setX(UT_sint32, bool bDontClearIfNeeded = false) = 0;
	virtual void		setY(UT_sint32) = 0;
	virtual UT_sint32	getWidth(void) const = 0;

	// all containers that may wish to draw stuff outwith the normal
	// drawing region, such as the pilcrow on a line, should overwrite this
	virtual UT_sint32   getDrawingWidth(void) const {return getWidth();}

	virtual UT_sint32	getX(void) const = 0;
	virtual UT_sint32	getY(void) const = 0;
	fl_SectionLayout*   getSectionLayout(void) const
		{ return m_pSectionLayout; }
	fl_DocSectionLayout *   getDocSectionLayout(void);
    void         setSectionLayout(fl_SectionLayout * pSL)
		{ m_pSectionLayout = pSL; }
	virtual inline UT_BidiCharType getDirection(void) const
		{ return m_iDirection;}
	virtual inline void setDirection(UT_BidiCharType c) {m_iDirection = c;}
	virtual UT_sint32	getHeight(void) const = 0;


	virtual void		draw(dg_DrawArgs*) = 0;
	virtual void		draw(GR_Graphics*) = 0;
	virtual void		clearScreen(void) = 0;
    GR_Graphics*        getGraphics(void) const;
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
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& isTOC) = 0;
	virtual fp_Container * getNextContainerInSection(void) const = 0;
	virtual fp_Container * getPrevContainerInSection(void) const = 0;
    virtual UT_Rect *      getScreenRect() = 0;
    virtual void           markDirtyOverlappingRuns(UT_Rect & recScreen) = 0;
	const char *           getContainerString(void);
	void                   setAllowDelete(bool bDelete)
	{ m_bCanDelete = bDelete;}
	bool                   canDelete(void)
	{ return m_bCanDelete;}
    UT_sint32   getBreakTick(void) const
		{ return  m_iBreakTick;}
	void        setBreakTick(UT_sint32 iTick)
		{ m_iBreakTick = iTick;}
        void                ref(void)
	{ m_iRef++;}
	void                unref(void)
	{ m_iRef--;}
	UT_sint32           getRefCount(void)
	{ return m_iRef;}
private:
	/*!
	  Container type
	*/
	FP_ContainerType		m_iConType;
	/*!
	  Section layout type used for this container
	*/
	fl_SectionLayout*       m_pSectionLayout;
	UT_BidiCharType         m_iDirection;
	UT_sint32               m_iBreakTick;
	UT_sint32               m_iRef;
	bool                    m_bCanDelete;
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
    virtual fp_ContainerObject * getNext(void) const {return m_pNext;}
    virtual fp_ContainerObject * getPrev(void) const {return m_pPrev;}
    virtual void        setNext(fp_ContainerObject * pNext);
    virtual void        setPrev(fp_ContainerObject * pPrev);
	void                   clearCons(void)
		{ m_vecContainers.clear();}
	fp_ContainerObject *   getNthCon(UT_sint32 i) const;
	void                   addCon(fp_ContainerObject * pCon);
	UT_sint32              countCons(void) const;
	UT_sint32              findCon(fp_ContainerObject * pCon) const;
	void                   justRemoveNthCon(UT_sint32 i);
	void                   deleteNthCon(UT_sint32 i);
	void                   insertConAt(fp_ContainerObject * pCon, UT_sint32 i);
	bool                   isEmpty(void) const;
	virtual UT_uint32 	distanceFromPoint(UT_sint32 x, UT_sint32 y) =0;
	virtual void        recalcMaxWidth(bool bDontClearIfNeeded = false) = 0;
	virtual void        setAssignedScreenHeight(UT_sint32 iY) =0;
	bool                getPageRelativeOffsets(UT_Rect &r) const;
	bool                isOnScreen() const;
	fp_Container *      getMyBrokenContainer(void) const;
	void                setMyBrokenContainer(fp_Container * pMyBroken);
	void                clearBrokenContainers(void);
	UT_uint32           binarysearchCons(const void* key,int (*compar)(const void *,
																 const void *)) const;
	UT_uint32           getBrokenCount(void) { return m_cBrokenContainers; }
	void                incBrokenCount(void) { m_cBrokenContainers += 1; }
	void                decBrokenCount(void) { if (m_cBrokenContainers > 0) {
			m_cBrokenContainers -= 1; }}

        fg_FillType &       getFillType(void);
	const fg_FillType & getFillType(void) const;
  	void                drawLine(const PP_PropertyMap::Line & style,
				     UT_sint32 left, UT_sint32 top,
				     UT_sint32 right, UT_sint32 bot,
				     GR_Graphics * pGr);

private:
	fp_Container*          m_pContainer;
	fp_ContainerObject *   m_pNext;
	fp_ContainerObject *   m_pPrev;
	UT_GenericVector<fp_ContainerObject *> m_vecContainers;
	fp_Container *         m_pMyBrokenContainer;
	UT_uint32              m_cBrokenContainers;
    fg_FillType            m_FillType;
};


#endif /*  CONTAINEROBJECT_H */






