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

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fp_Page.h"

class fl_HdrFtrSectionLayout;
class fl_DocSectionLayout;
class fl_SectionLayout;
class fp_Line;
class fp_Page;
class PP_AttrProp;
class GR_Graphics;
struct dg_DrawArgs;
struct fp_Sliver;

#define FP_CONTAINER_COLUMN		1
#define FP_CONTAINER_HDRFTR		2

class fp_Container
{
public:
/*!
  Create container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
*/
	fp_Container(UT_uint32 iType, fl_SectionLayout* pSectionLayout);

/*!
  Destruct container
  \note The lines (fp_Line) in the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		fl_BlockLayout), not the physical hierarchy.
*/
	~fp_Container();

/*!
  Return container type
  \return Type
*/
	inline UT_uint32	getType(void) const { return m_iType; }
	
/*!
  Set page
  \param pPage Page container is located on
*/
	void				setPage(fp_Page*);

/*!
  Set width
  \param iWidth Width of container
  \todo Should force re-line-break operations on all blocks in the
        container
*/
	void				setWidth(UT_sint32);

/*!
 Set width in layout units
 \param iWidth Width in layout units of container
*/
	void				setWidthInLayoutUnits(UT_sint32);

/*!
 Set maximum height
 \param iMaxHeight Maximum height of container
*/
	void				setMaxHeight(UT_sint32);

/*!
 Set maximum height in layout units
 \param iMaxHeight Maximum height in layout units of container
*/
	void				setMaxHeightInLayoutUnits(UT_sint32);

/*!
 Set height
 \param iHeight Height of container
 \bug This function does not appear to have any use as it asserts if
      the height of the container is ever attempted changed. 
*/
	void				setHeight(UT_sint32);

/*!
 Set X position of container
 \param iX New X position

 Before the postition of the container is changed, its content is
 first cleared from the screen.
*/
	void				setX(UT_sint32);

/*!
 Set Y position of container
 \param iY New Y position

 Before the postition of the container is changed, its content is
 first cleared from the screen.
*/
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
 Return first line in the container
 \return The first line, or NULL if the container is empty
*/
	fp_Line*			getFirstLine(void) const;

/*!
 Return last line in the container
 \return The last line, or NULL if the container is empty
*/
	fp_Line*			getLastLine(void) const;
	
/*!
 Insert line in container after specified line
 \param pNewLine   Line to be inserted
 \param pAfterLine After this line
 \todo This function has been hacked to handle the case where
       pAfterLine is NULL. That case should not happen. Bad callers
       should be identified and fixed, and this function should be
       cleaned up.
*/
	UT_Bool				insertLineAfter(fp_Line* pNewLine, fp_Line*	pAfterLine);

/*!
 Insert line at the front/top of the container
 \param pNewLine Line
*/
	UT_Bool				insertLine(fp_Line*);

/*!
 Append line at the end/bottom of the container
 \param pNewLine Line
*/
	UT_Bool				addLine(fp_Line*);

/*!
 Remove line from container
 \param pLine Line
 \note The line is not destructed, as it is owned by the logical
       hierarchy.
*/
	void				removeLine(fp_Line*);

/*!
  Determine if container is empty
 \return True if container is empty, otherwise false.
*/
	UT_Bool				isEmpty(void) const;
	
/*!
 Compute the distance from point to the container's circumference
 \param x X coordinate of point
 \param y Y coordinate of point
 \return Distance between container's circumference and point
*/
	UT_uint32 			distanceFromPoint(UT_sint32 x, UT_sint32 y);

/*!
  Find document position from X and Y coordinates
 \param  x X coordinate
 \param  y Y coordinate
 \retval pos Document position
 \retval bBOL True if position is at begining of line, otherwise false
 \retval bEOL True if position is at end of line, otherwise false
*/
	void				mapXYToPosition(UT_sint32 xPos, 
										UT_sint32 yPos, 
										PT_DocPosition& pos, 
										UT_Bool& bBOL, UT_Bool& bEOL);

/*!
  Get line's offsets relative to this container
 \param  pLine Line
 \retval xoff Line's X offset relative to container
 \retval yoff Line's Y offset relative to container
*/
	void		 		getOffsets(fp_Line* pLine, UT_sint32& xoff, 
								   UT_sint32& yoff);

/*!
  Get line's offsets relative to the screen
 \param  pLine Line
 \retval xoff Line's X offset relative the screen
 \retval yoff Line's Y offset relative the screen
*/
	void		 		getScreenOffsets(fp_Line* pLine, UT_sint32& xoff, 
										 UT_sint32& yoff);

/*!
 Draw container content
 \param pDA Draw arguments
*/
	void				draw(dg_DrawArgs*);

/*!
  Clear container content from screen.
*/
	void				clearScreen(void);

protected:
/*!
  Container type
  \bug Surely this should be an enum?!?
*/
	UT_uint32				m_iType;
	
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

/*!
 Draw container outline
 \param pDA Draw arguments
*/
    void                    _drawBoundaries(dg_DrawArgs* pDA);
};

class fp_Column : public fp_Container
{
public:
	fp_Column(fl_SectionLayout* pSectionLayout);
	~fp_Column();

	void				setLeader(fp_Column*);
	void				setFollower(fp_Column*);
	void 				setNext(fp_Column*);
	void 				setPrev(fp_Column*);

	fl_DocSectionLayout*	getDocSectionLayout(void) const;
	
	inline				fp_Column*			getLeader(void) const 			{ return m_pLeader; }
	inline				fp_Column*			getFollower(void) const 		{ return m_pNextFollower; }
	inline				fp_Column*			getNext(void) const				{ return m_pNext; }
	inline				fp_Column*			getPrev(void) const				{ return m_pPrev; }

	void				layout(void);
	
	void 				bumpLines(fp_Line* pLastLineToKeep);
	
protected:
	UT_uint32 				_getBottomOfLastLine(void) const;

	fp_Column*				m_pNext;
	fp_Column*				m_pPrev;

	fp_Column*				m_pLeader;
	fp_Column*				m_pNextFollower;

};

class fp_HdrFtrContainer : public fp_Container
{
public:
	fp_HdrFtrContainer(UT_sint32 iX, UT_sint32 iY, 
							UT_sint32 iWidth, UT_sint32 iHeight,
							UT_sint32 iWidthLayout, UT_sint32 iHeightLayout, 
							fl_SectionLayout* pSL);
	~fp_HdrFtrContainer();

	fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const;
	
	void				layout(void);
	
protected:

};

#endif /* COLUMN_H */
