/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
/* GTK - The GIMP Toolkit
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
#include "gr_Graphics.h"
#include "fl_TableLayout.h"

class fl_TableLayout;

class fp_TableRowColumn
{
public:
	fp_TableRowColumn(void);
	virtual ~fp_TableRowColumn(void);
	double requisition;
	double allocation;
	double spacing;
	bool need_expand;
	bool need_shrink;
	bool expand;
	bool shrink;
	bool empty;
};

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
class fp_TableContainer;
struct dg_DrawArgs;
struct fp_Sliver;

class ABI_EXPORT fp_CellContainer : public fp_VerticalContainer
{
public:
	fp_CellContainer(fl_SectionLayout* pSectionLayout);
	virtual ~fp_CellContainer();

	void                sizeRequest(fp_Requisition * pRequest);
	void                sizeAllocate(fp_Allocation * pAllocate);
	void				layout(void);
	fp_Container *      drawSelectedCell(fp_Line * pLine);
	bool			    isSelected(void) const
	{ return m_bIsSelected; }
	void                clearSelection(void) 
	{ m_bIsSelected = false; 
	  m_bLinesDrawn = true;
	}
	bool                doesOverlapBrokenTable(fp_TableContainer * pBroke);
	void		        drawBroken(dg_DrawArgs* pDa, fp_TableContainer * pTab);
	virtual void		clearScreen(void);
	void		        clearScreen(bool bNoRecursive);
	void                drawLines(fp_TableContainer * pBroke,GR_Graphics * pG);
	void                drawLinesAdjacent(void);
	void                draw(fp_Line * pLine);
	fp_TableContainer * getBrokenTable(fp_Container * pCon);
	fp_VerticalContainer * getColumn(fp_Container *pCon);
	double              tweakBrokenTable(fp_TableContainer * pBroke);
	virtual void		draw(dg_DrawArgs*);
	virtual void		draw(GR_Graphics*) {}
	virtual void        setContainer(fp_Container * pContainer);
	virtual void        setWidth(double iWidth);
	virtual void        setHeight(double iHeight);
	        void        _drawBoundaries(dg_DrawArgs* pDA, fp_TableContainer *pBroke);
	virtual bool        isVBreakable(void);
	virtual bool        isHBreakable(void) {return false;}
	virtual double		wantVBreakAt(double);
	virtual double		wantHBreakAt(double) {return 0;}
	virtual fp_ContainerObject * VBreakAt(double);
	virtual fp_ContainerObject * HBreakAt(double) {return NULL;}
	void                recalcMaxWidth(bool bDontClearIfNeeded = false) {}
	virtual void        setAssignedScreenHeight(double) {}
	virtual fp_Container * getNextContainerInSection(void) const;
	virtual fp_Container * getPrevContainerInSection(void) const;
	fp_TableContainer * getTopmostTable(void) const;
	double				getCellX(fp_Line * pLine) const; 
	double				getCellY(fp_Line * pLine) const;
	double				getSpannedHeight(void);
	void                setLineMarkers(void);
	bool                containsFootnoteReference(void);
	void                deleteBrokenTables(bool bClearFirst=true);
	bool                getFootnoteContainers(UT_GenericVector<fp_FootnoteContainer*>* pvecFoots);
	void                getLeftTopOffsets(double & xoff, double & yoff);
   UT_sint32           getLeftAttach(void) const
		{ return m_iLeftAttach;}
    UT_sint32           getRightAttach(void) const
		{ return m_iRightAttach;}
    UT_sint32           getTopAttach(void) const
		{ return m_iTopAttach;}
    UT_sint32           getBottomAttach(void) const
		{ return m_iBottomAttach;}
    void                setLeftAttach(UT_sint32 i)
		{ m_iLeftAttach = i;}
    void                setRightAttach(UT_sint32 i)
		{ m_iRightAttach = i;}
    void                setTopAttach(UT_sint32 i)
		{ m_iTopAttach = i;}
    void                setBottomAttach(UT_sint32 i)
		{ m_iBottomAttach = i;}
	void                setToAllocation(void);
	double				getLeftPad(void) const
		{ return m_iLeftPad;}
	double				getRightPad(void) const
		{ return m_iRightPad;}
	double				getTopPad(void) const
		{ return m_iTopPad;}
	double				getBotPad(void) const
		{ return m_iBotPad;}
	void                setLeftPad(double i)
		{ m_iLeftPad = i;}
	void                setRightPad(double i)
		{ m_iRightPad = i;}
	void                setTopPad(double i)
		{ m_iTopPad = i;}
	void                setBotPad(double i)
		{ m_iBotPad = i;}
		
	PP_PropertyMap::Background getBackground () const;

	void setBackground (const PP_PropertyMap::Background & style);


	PP_PropertyMap::Line getBottomStyle (const fl_TableLayout * table) const;
	PP_PropertyMap::Line getLeftStyle   (const fl_TableLayout * table) const;
	PP_PropertyMap::Line getRightStyle  (const fl_TableLayout * table) const;
	PP_PropertyMap::Line getTopStyle    (const fl_TableLayout * table) const;

	void setBottomStyle (const PP_PropertyMap::Line & style) { m_lineBottom = style; }
	void setLeftStyle   (const PP_PropertyMap::Line & style) { m_lineLeft   = style; }
	void setRightStyle  (const PP_PropertyMap::Line & style) { m_lineRight  = style; }
	void setTopStyle    (const PP_PropertyMap::Line & style) { m_lineTop    = style; }

	bool                getXexpand(void) const
		{ return m_bXexpand;}
	bool                getYexpand(void) const
		{ return m_bYexpand;}
	bool                getXshrink(void) const
		{ return m_bXshrink;}
	bool                getYshrink(void) const
		{ return m_bYshrink;}
	void                setXexpand(bool b)
		{ m_bXexpand = b;}
	void                setYexpand(bool b)
		{ m_bYexpand = b;}
	void                setXshrink(bool b)
		{ m_bXshrink = b;}
	void                setYshrink(bool b)
		{ m_bYshrink = b;}
	bool                getXfill(void) const
		{return m_bXfill;}
	bool                getYfill(void)const
		{return m_bYfill;}
	void                setXfill(bool b)
		{ m_bXfill = b;}
	void                setYfill(bool b)
		{ m_bYfill = b;}
	double           	getStartY(void) const
		{ return m_iTopY;}
	double           	getStopY(void) const
		{ return m_iBotY;}
	double           	getLeftPos(void) const
		{  return m_iLeft; }
	double           	getRightPos(void) const
		{  return m_iRight; }
	void                markAsDirty(void) { m_bDirty = true;}
	bool                isDirty(void) const
	    {  return m_bDirty;}
	bool                doesIntersectClip(fp_TableContainer * pBroke, UT_Rect * rClip);
	bool                isInNestedTable(void);
	bool                containsNestedTables(void);
#ifdef FMT_TEST
	void				__dump(FILE * fp) const;
#endif

private:
		
	void                _clear(fp_TableContainer * pBroke);
	void				_drawLine(const PP_PropertyMap::Line & style,
								  double left, double top, double right, double bot, GR_Graphics * pG);
	void				_getBrokenRect(fp_TableContainer * pBroke, fp_Page* &pPage, UT_Rect &bRec, GR_Graphics * pG);
		
//
// These variables describe where the cell is attached to the table.
// The first cell in the Table is at (0,0)
//
// m_iLeftAttach is the leftmost column containing the cell.
//  
	UT_sint32           m_iLeftAttach;

// m_iRightAttach is the first column to the right of the cell.

	UT_sint32           m_iRightAttach;

// m_iTopAttach is the topmost row containing the cell.

	UT_sint32           m_iTopAttach;

// m_iBottomAttach is the row immediately below the cell.

	UT_sint32           m_iBottomAttach;
	
// a default color to use the a cell border style is set to "off"

	UT_RGBColor			m_borderColorNone;
	
//
// Padding left,right, top and bottom
//
	double           	m_iLeftPad;
	double           	m_iRightPad;
	double           	m_iTopPad;
	double           	m_iBotPad;
//
// Needed since a cell can span multiple pages.
//
	fp_CellContainer *  m_pNextInTable;
	fp_CellContainer *  m_pPrevInTable;
//
// Should the cell expand or shrink in the x and y directitions.
//
	bool                m_bXexpand;
	bool                m_bYexpand;
	bool                m_bXshrink;
	bool                m_bYshrink;
//
// Should we fill the container in x and y?
//
	bool                m_bXfill;
	bool                m_bYfill;

// Local size request and allocation.

	fp_Allocation       m_MyAllocation;
	fp_Requisition      m_MyRequest;

// Coordinates of the cell used for drawing lines around it.

	double				m_iLeft;
	double				m_iRight;
	double				m_iTopY;
	double				m_iBotY;
	bool                m_bDrawLeft;
	bool                m_bDrawTop;
	bool                m_bDrawBot;
	bool                m_bDrawRight;
	bool                m_bLinesDrawn;

// bool to see if the background needs to be redrawn
	bool				m_bBgDirty;
	
// cell-background properties
	PP_PropertyMap::Background	m_background;

// cell-border properties
	PP_PropertyMap::Line   m_lineBottom;
	PP_PropertyMap::Line   m_lineLeft;
	PP_PropertyMap::Line   m_lineRight;
	PP_PropertyMap::Line   m_lineTop;

// Flag to see if this cell is drawn "selected"
	bool		           m_bIsSelected;
	
	
	bool                   m_bDirty;
};

class ABI_EXPORT fp_TableContainer : public fp_VerticalContainer
{
public:
	fp_TableContainer(fl_SectionLayout* pSectionLayout);
	fp_TableContainer(fl_SectionLayout* pSectionLayout, fp_TableContainer * pMaster);
	~fp_TableContainer();

	void                sizeRequest(fp_Requisition * pRequest);
	void                sizeAllocate(fp_Allocation * pAllocate);
    virtual void        mapXYToPosition(double x, double y, 
										PT_DocPosition& pos,
										bool& bBOL, bool& bEOL, bool &isTOC);
	virtual fp_Page *   getPage(void);
	fp_Line *           getFirstLineInColumn(fp_Column * pCol);
	fp_Line *           getLastLineInColumn(fp_Column * pCol);
	void				layout(void);
	virtual void        setY(double iY);
	virtual double      getHeight(void);
	virtual void        setHeight(double iHeight);
	virtual void        setContainer(fp_Container * pContainer);
	virtual void		draw(dg_DrawArgs*);
	virtual void		draw(GR_Graphics*) {}
	virtual double      getMarginBefore(void) const;
	virtual double      getMarginAfter(void) const;
	fp_Column *         getBrokenColumn(void);
	void                drawLines();
	bool                containsFootnoteReference(void);
	bool                getFootnoteContainers(UT_GenericVector<fp_FootnoteContainer*>* pvecFoots);
	UT_sint32           tweakBrokenTable(fp_TableContainer * pBroke);
    virtual void        clearScreen(void);
	virtual bool        isVBreakable(void);
	virtual bool        isHBreakable(void) {return false;}
	virtual double      wantVBreakAt(double);
	virtual double      wantHBreakAt(double) {return 0;}
	virtual fp_ContainerObject * VBreakAt(double);
	void                breakCellsAt(double vpos);
	virtual fp_ContainerObject * HBreakAt(double) {return NULL;}
	UT_sint32           getBrokenNumber(void);
	void                setToAllocation(void);
	void                tableAttach(fp_CellContainer * pCell);
	void                setHomogeneous (bool bIsHomogeneous);
	void                setColSpacings (double spacing);
	void                setRowSpacings (double spacing);
	void                setColSpacing(UT_sint32 column, double spacing);
	void                setRowSpacing (UT_sint32 row, double spacing);
	void                resize(UT_sint32 n_rows, UT_sint32 n_cols);
	void                setBorderWidth(double i);
	double              getBorderWidth(void) const
		{ return m_iBorderWidth;}
	void                setLeftOffset(double iLeftOff)
		{ m_iLeftOffset = iLeftOff;}
	void                setRightOffset(double iRightOff)
		{ m_iRightOffset = iRightOff;}
	void                setTopOffset(double iTopOff)
		{ m_iTopOffset = iTopOff;}
	void                setBottomOffset(double iBotOff)
		{ m_iBottomOffset = iBotOff;}
	void                setLineThickness(double iLineThickness)
		{ m_iLineThickness = iLineThickness;}
	double              getLineThickness(void)
		{ return m_iLineThickness;}
	void                queueResize(void);
	double              getYOfRow(UT_sint32 row);
	fp_CellContainer *  getCellAtRowColumn(UT_sint32 row, UT_sint32 column);
	fp_CellContainer *  getCellAtRowColumnLinear(UT_sint32 row, UT_sint32 column);
	virtual fp_Container * getNextContainerInSection(void) const;
	virtual fp_Container * getPrevContainerInSection(void) const;
	fp_TableContainer * getMasterTable(void) const
		{ return m_pMasterTable; }
	bool                isThisBroken(void) const
		{ return m_bIsBroken;}
	void                setYBreakHere(double iBreakHere);
	void                setYBottom(double iBotContainer);
	bool                isInBrokenTable(fp_CellContainer * pCell, 
										fp_Container * pCon);

//
// This is the smallest Y value of the Table allowed in this 
// broken Table.
//
	double           getYBreak(void) const
		{return m_iYBreakHere;}
//
// This is the largest Y value of the Table allowed in this broken table
//
	double              getYBottom(void) const
		{return m_iYBottom;}
	fp_TableContainer * getFirstBrokenTable(void) const;
	fp_TableContainer * getLastBrokenTable(void) const;
	void                setFirstBrokenTable(fp_TableContainer * pBroke);
	void                setLastBrokenTable(fp_TableContainer * pBroke);
	void                deleteBrokenTables(bool bClearFirst, bool bRecurseUp = true);
	void                adjustBrokenTables(void);
	UT_sint32               getBrokenTop(void);
	UT_sint32               getBrokenBot(void);
	void                    setBrokenTop(UT_sint32 iTop) 
		{ m_iBrokenTop = iTop;}
	void                    setBrokenBot(UT_sint32 iBot) 
		{ m_iBrokenBottom = iBot;}
	UT_sint32           getNumRows(void) const;
	UT_sint32           getNumCols(void) const;
	double              getRowHeight(UT_sint32 iRow, double iMeasHeight);
	void                setRedrawLines(void)
		{ m_bRedrawLines = true;}
	bool                doRedrawLines(void) const
		{ return m_bRedrawLines;}
	fp_TableRowColumn *     getNthCol(UT_sint32 i);
	fp_TableRowColumn *     getNthRow(UT_sint32 i);
	bool                    containsNestedTables(void);
	void setRowHeightType(FL_RowHeightType iType)
		{
	      m_iRowHeightType = iType;
		}		
	void setRowHeight(double iHeight)
		{
	      m_iRowHeight = iHeight;
		}		
	void setLastWantedVBreak(double iBreakAt)
	{
		m_iLastWantedVBreak = iBreakAt;
	}
	double getLastWantedVBreak(void) const
	{
		return m_iLastWantedVBreak;
	}
#ifdef FMT_TEST
	void				__dump(FILE * fp) const;
#endif

private:
	void                    _size_request_init(void);
	void                    _size_request_pass1(void);
	void                    _size_request_pass2(void);
	void                    _size_request_pass3(void);

    void                    _size_allocate_init(void);
    void                    _size_allocate_pass1(void);
	void                    _size_allocate_pass2(void);
	UT_uint32 				_getBottomOfLastContainer(void) const;
	void					_drawBoundaries(dg_DrawArgs* pDA);
	void					_drawBrokenBoundaries(dg_DrawArgs* pDA);
	void					_brokenDraw(dg_DrawArgs* pDA);

	UT_sint32               m_iRows;
	UT_sint32               m_iCols;
	double                  m_iBorderWidth;
	bool                    m_bIsHomogeneous;

	UT_GenericVector<fp_TableRowColumn *> m_vecRows;
	UT_GenericVector<fp_TableRowColumn *> m_vecColumns;

// Local size request and allocation.

	fp_Allocation           m_MyAllocation;
	fp_Requisition          m_MyRequest;

	double                  m_iRowSpacing;
	double                  m_iColSpacing;
//
// Variables for Tables broken across Vertical Containers.
//
	fp_TableContainer *     m_pFirstBrokenTable;
	fp_TableContainer *     m_pLastBrokenTable;
	bool                    m_bIsBroken;
	fp_TableContainer *     m_pMasterTable;
	double                  m_iYBreakHere;
	double                  m_iYBottom;
	UT_sint32               m_iBrokenTop;
	UT_sint32               m_iBrokenBottom;
	bool                    m_bRedrawLines;
//
// Global Table properties
//
	double                  m_iLeftOffset;
	double                  m_iRightOffset;
	double                  m_iTopOffset;
	double                  m_iBottomOffset;
	double                  m_iLineThickness;

// Global row height type
	FL_RowHeightType        m_iRowHeightType;

// Global row height
	double                  m_iRowHeight;

// Last requested vbreak height

	double                  m_iLastWantedVBreak;

// Cache the first and last cells of a broken table

	fp_CellContainer *  m_pFirstBrokenCell;
	fp_CellContainer *  m_pLastBrokenCell;
	bool                m_bRecursiveClear;
 };

#endif /* TABLECONTAINER_H */
