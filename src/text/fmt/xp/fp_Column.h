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

#ifndef COLUMN_H
#define COLUMN_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fp_Page.h"

class fl_EndnoteSectionLayout;
class fl_HdrFtrSectionLayout;
class fl_DocSectionLayout;
class fl_SectionLayout;
class fp_Line;
class fp_Page;
class PP_AttrProp;
class GR_Graphics;
struct dg_DrawArgs;
struct fp_Sliver;

typedef enum {
	FP_CONTAINER_COLUMN,
	FP_CONTAINER_SHADOW,
	FP_CONTAINER_HDRFTR,
	FP_CONTAINER_ENDNOTE
} FP_ContainerType;

class fp_Container
{
public:
	fp_Container(FP_ContainerType iType, fl_SectionLayout* pSectionLayout);
	virtual ~fp_Container();
	/*!
	  Return container type
	  \return Type
	*/
	inline FP_ContainerType	getType(void) const { return m_iType; }
	
	void				setPage(fp_Page*);

	void				setWidth(UT_sint32);
	void				setWidthInLayoutUnits(UT_sint32);
	void                setMaxLineHeight(UT_sint32 iLineHeight);
	UT_sint32           getMaxLineHeight(void) const;
	void				setHeight(UT_sint32);
	void				setMaxHeight(UT_sint32);
	void				setMaxHeightInLayoutUnits(UT_sint32);

	void				setX(UT_sint32);
	void				setY(UT_sint32);
	/*!
	  Get container's max height
	  \return Max height
	*/
	inline UT_sint32	getMaxHeight(void) const
 		{ return m_iMaxHeight; }
	/*!
	  Get container's max height in layout units
	  \return Max height in layout units
	*/
	inline UT_sint32	getMaxHeightInLayoutUnits(void) const
		{ return m_iMaxHeightLayoutUnits; }
	/*!
	  Get container's width
	  \return Width
	*/
	inline UT_sint32	getWidth(void) const
		{ return m_iWidth; }
	/*!
	  Get container's width in layout units
	  \return Width in layout units
	*/
	inline UT_sint32	getWidthInLayoutUnits(void) const
		{ UT_ASSERT(m_iWidthLayoutUnits); return m_iWidthLayoutUnits; }
	/*!
	  Get container's X position
	  \return X position
	*/
	inline UT_sint32	getX(void) const
		{ return m_iX; }
	/*!
	  Get container's Y position
	  \return Y position
	*/
	inline UT_sint32	getY(void) const
		{ return m_iY; }
	/*!
	  Get page container is located on
	  \return Page
	*/
	inline fp_Page*		getPage(void) const
		{ return m_pPage; }
	/*!
	  Get section container is contained in
	  \return Section
	*/
	inline fl_SectionLayout* getSectionLayout(void) const
		{ return m_pSectionLayout; }
	/*!
	  Get container's height
	  \return Height
	*/
	inline UT_sint32	getHeight(void) const
		{ return m_iHeight; }
	/*!
	  Get container's height in layout units
	  \return Height in layout units
	*/
	inline UT_sint32	getHeightInLayoutUnits(void) const
		{ return m_iHeightLayoutUnits; }
	/*!
	  Get column gap from page the container is located on
	  \return Column gap
	*/
	inline UT_sint32	getColumnGap(void) const
		{ return m_pPage->getColumnGap(); }
	
	/*!
	  Get container's intentionally empty flag
	  \return Empty
	*/
	inline bool			getIntentionallyEmpty(void) const
		{ return m_bIntentionallyEmpty; }
	/*!
	  Set container's intentionally empty flag
	*/
	inline void			setIntentionallyEmpty(bool b)
		{ m_bIntentionallyEmpty = b; }

	fp_Line*			getFirstLine(void) const;
	fp_Line*			getLastLine(void) const;
	
	bool				insertLineAfter(fp_Line* pNewLine, fp_Line*	pAfterLine);
	bool				insertLine(fp_Line*);
	bool				addLine(fp_Line*);
	void				removeLine(fp_Line*);

	bool				isEmpty(void) const;
	
	UT_uint32 			distanceFromPoint(UT_sint32 x, UT_sint32 y);

	void				mapXYToPosition(UT_sint32 xPos, 
										UT_sint32 yPos, 
										PT_DocPosition& pos, 
										bool& bBOL, bool& bEOL);

	void		 		getOffsets(fp_Line* pLine, UT_sint32& xoff, 
								   UT_sint32& yoff);

	void		 		getScreenOffsets(fp_Line* pLine, UT_sint32& xoff, 
										 UT_sint32& yoff);

	void				draw(dg_DrawArgs*);

	void				clearScreen(void);

protected:
	/*!
	  Container type
	*/
	FP_ContainerType		m_iType;
	/*!
	  Page this container is located on
	*/
	fp_Page*				m_pPage;
	/*!
	  Width of the container
	*/
	UT_sint32 				m_iWidth;
	/*!
	  Width in layout units of the container
	*/
	UT_sint32 				m_iWidthLayoutUnits;
	/*!
	  Height of the container
	*/
	UT_sint32 				m_iHeight;
	/*!
	  Maximum height of the container
	*/
	UT_sint32				m_iMaxHeight;
	/*!
	  Height in layout units of the container
	*/
	UT_sint32 				m_iHeightLayoutUnits;
	/*!
	  Maximum height in layout units of the container
	*/
	UT_sint32				m_iMaxHeightLayoutUnits;
	/*!
	  X coordinate of container
	*/
	UT_sint32				m_iX;
	/*!
	  Y coordinate of container
	*/
	UT_sint32				m_iY;
	/*!
	  Vector of lines (fp_Line) in containter
	*/
	UT_Vector				m_vecLines;
	/*!
	  Section layout type used for this container
	*/
	fl_SectionLayout*		m_pSectionLayout;
	/*!
	  GR_Graphics this container is drawn on
	*/
	GR_Graphics*			m_pG;

	virtual void			_drawBoundaries(dg_DrawArgs* pDA);

	/*!
	  Set if this container is intentionally left empty

	  The breakSection function that does page layout sometimes
	  decides to leave sections empty for one reason or another. This
	  needs to be flagged, or fl_DocSectionLayout::deleteEmptyColumns
	  will delete the container.
	 */
	bool					m_bIntentionallyEmpty;
private:
	UT_sint32               m_imaxLineHeight;
};

class fp_Column : public fp_Container
{
public:
	fp_Column(fl_SectionLayout* pSectionLayout);
	~fp_Column();

	fl_DocSectionLayout*	getDocSectionLayout(void) const;
	
	inline void			setLeader(fp_Column* p) { m_pLeader = p; }
	inline void			setFollower(fp_Column* p) { m_pFollower = p; }
	inline void 		setNext(fp_Column* p) { m_pNext = p; }
	inline void 		setPrev(fp_Column* p) { m_pPrev = p; }

	inline fp_Column*	getLeader(void) const 			{ return m_pLeader; }
	inline fp_Column*	getFollower(void) const 		{ return m_pFollower; }
	inline fp_Column*	getNext(void) const				{ return m_pNext; }
	inline fp_Column*	getPrev(void) const				{ return m_pPrev; }

	void				layout(void);
	
	void 				bumpLines(fp_Line* pLastLineToKeep);
	
#ifdef FMT_TEST
	void				__dump(FILE * fp) const;
#endif	

protected:
	UT_uint32 				_getBottomOfLastLine(void) const;

	void					_drawBoundaries(dg_DrawArgs* pDA);

private:
	fp_Column*				m_pNext;
	fp_Column*				m_pPrev;

	fp_Column*				m_pLeader;
	fp_Column*				m_pFollower;
};

class fp_ShadowContainer : public fp_Container
{
public:
	fp_ShadowContainer(UT_sint32 iX, UT_sint32 iY, 
					   UT_sint32 iWidth, UT_sint32 iHeight,
					   UT_sint32 iWidthLayout, UT_sint32 iHeightLayout, 
					   fl_SectionLayout* pSL);
	~fp_ShadowContainer();

	fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const;
	fl_HdrFtrShadow *   getShadow();
 	void				draw(dg_DrawArgs*);
  	void				layout(void);
 	void				clearScreen(void);
	void                clearHdrFtrBoundaries(void);
protected:
	void                _drawHdrFtrBoundaries(dg_DrawArgs * pDA);
	bool                m_bHdrFtrBoxDrawn;
	UT_sint32           m_ixoffBegin;
	UT_sint32           m_iyoffBegin;
	UT_sint32           m_ixoffEnd;
	UT_sint32           m_iyoffEnd;

};


class fp_HdrFtrContainer : public fp_Container
{
public:
	fp_HdrFtrContainer( UT_sint32 iWidth,
					   UT_sint32 iWidthLayout, 
					   fl_SectionLayout* pSL);
	~fp_HdrFtrContainer();

	fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const;
 	void				draw(dg_DrawArgs*);
  	void				layout(void);
 	void				clearScreen(void);
	void		 		getScreenOffsets(fp_Line* pLine, UT_sint32& xoff, 
										 UT_sint32& yoff);
	
protected:
};

class fp_EndnoteSectionContainer : public fp_Container
{
public:
	fp_EndnoteSectionContainer(fl_SectionLayout* pSectionLayout);
	~fp_EndnoteSectionContainer();

	fl_EndnoteSectionLayout*	getEndnoteSectionLayout(void) const;
	
	void 				setNext(fp_EndnoteSectionContainer* p) { m_pNext = p; }
	void 				setPrev(fp_EndnoteSectionContainer* p) { m_pPrev = p; }

	inline fp_EndnoteSectionContainer* getNext(void) const { return m_pNext; }
	inline fp_EndnoteSectionContainer* getPrev(void) const { return m_pPrev; }

 	void				draw(dg_DrawArgs*);
  	void				layout(void);
 	void				clearScreen(void);
protected:
private:
	fp_EndnoteSectionContainer*	m_pNext;
	fp_EndnoteSectionContainer* m_pPrev;
};

#endif /* COLUMN_H */
