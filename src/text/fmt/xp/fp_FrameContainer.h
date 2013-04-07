/* AbiWord
 * Copyright (C) 2002 Patrick Lam <plam@mit.edu>
 * Copyright (C) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#ifndef FRAMECONTAINER_H
#define FRAMECONTAINER_H

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
#include "pp_PropertyMap.h"

class fl_TableLayout;
class fl_DocSectionLayout;
class fl_BlockLayout;

#define FRAME_HANDLE_SIZE 6

class ABI_EXPORT fp_FrameContainer : public fp_VerticalContainer
{
public:
	fp_FrameContainer(fl_SectionLayout* pSectionLayout);
	virtual ~fp_FrameContainer();
	void				layout(void);
	virtual void		clearScreen(void);
	virtual void		draw(dg_DrawArgs*);
	virtual void		draw(GR_Graphics*) {}
	void                drawHandles(dg_DrawArgs * pDA);
	void                drawBoundaries(dg_DrawArgs * pDA);
	virtual void        setContainer(fp_Container * pContainer);
	virtual fp_Container * getNextContainerInSection(void) const;
	virtual fp_Container * getPrevContainerInSection(void) const;
	virtual fp_Page *   getPage(void) { return m_pPage;}
	virtual UT_sint32   getX() const;
	virtual UT_sint32   getY() const;
	virtual UT_sint32   getWidth() const;
	virtual UT_sint32   getHeight() const;
	virtual void        setHeight(UT_sint32 iHeight);
	UT_sint32           getLeftPad(UT_sint32 y, UT_sint32 height);
	UT_sint32           getRightPad(UT_sint32 y, UT_sint32 height);
	virtual bool        isVBreakable(void)
		{ return false;}
	UT_sint32           getFullX() const;
	UT_sint32           getFullY() const;
	UT_sint32           getFullWidth() const;
	UT_sint32           getFullHeight() const;
	void                setXpad(UT_sint32 xPad)
		{m_iXpad = xPad;}
	void                setYpad(UT_sint32 yPad)
		{m_iYpad = yPad;}
	UT_sint32           getXPad(void) const { return m_iXpad;}
	UT_sint32           getYPad(void) const { return m_iYpad;}
	void                setPage(fp_Page * pPage);
	fl_DocSectionLayout * getDocSectionLayout(void);
	void                getBlocksAroundFrame(UT_GenericVector<fl_BlockLayout *> & vecBlocks);
	PP_PropertyMap::Background getBackground () const;
	void                setPreferedPageNo(UT_sint32 i);
	UT_sint32           getPreferedPageNo(void)
	{     return m_iPreferedPageNo;}
	void                setPreferedColumnNo(UT_sint32 i);
	UT_sint32           getPreferedColumnNo(void)
	{     return m_iPreferedColumnNo;}

	void setBackground (const PP_PropertyMap::Background & style);

	void setBottomStyle (const PP_PropertyMap::Line & style) { m_lineBottom = style; }
	void setLeftStyle   (const PP_PropertyMap::Line & style) { m_lineLeft   = style; }
	void setRightStyle  (const PP_PropertyMap::Line & style) { m_lineRight  = style; }
	void setTopStyle    (const PP_PropertyMap::Line & style) { m_lineTop    = style; }
	void                setOverWrote(void)
		{m_bOverWrote = true;}
	void                setWrapping(bool bWrapping)
		{m_bIsWrapped = bWrapping;}
	bool                isWrappingSet(void) const
		{ return m_bIsWrapped;}
	void                setTightWrapping( bool bTight)
	        { m_bIsTightWrapped = bTight;}
	bool                isTightWrapped(void) const
	        { return m_bIsTightWrapped;}
	bool                isTopBot(void) const
	  { return m_bIsTopBot; }
	void                setTopBot(bool b)
	  { m_bIsTopBot = b;}
	bool                isLeftWrapped(void) const
	  { return m_bIsLeftWrapped;}
	void                setLeftWrapped(bool b)
	  { m_bIsLeftWrapped = b;}
	bool                isRightWrapped(void) const
	  { return m_bIsRightWrapped;}
	void                setRightWrapped(bool b)
	  { m_bIsRightWrapped = b;}

	bool                overlapsRect(UT_Rect & rec);
	bool                isAbove(void);
	void                setAbove(bool bAbove)
	        { m_bIsAbove = bAbove;}
private:
	void                   _drawLine (const PP_PropertyMap::Line & style,
									  UT_sint32 left, UT_sint32 top,
									  UT_sint32 right, UT_sint32 bot,
									  GR_Graphics * pGr);
	fp_Page * m_pPage;
	UT_sint32 m_iXpad;
	UT_sint32 m_iYpad;
	bool      m_bNeverDrawn;

// cell-background properties
	PP_PropertyMap::Background	m_background;

// cell-border properties
	PP_PropertyMap::Line   m_lineBottom;
	PP_PropertyMap::Line   m_lineLeft;
	PP_PropertyMap::Line   m_lineRight;
	PP_PropertyMap::Line   m_lineTop;
	bool                   m_bOverWrote;
	bool                   m_bIsWrapped;
	bool                   m_bIsTightWrapped;
	bool                   m_bIsAbove;
	bool                   m_bIsTopBot;
	bool                   m_bIsLeftWrapped;
	bool                   m_bIsRightWrapped;
	UT_sint32              m_iPreferedPageNo;
	UT_sint32              m_iPreferedColumnNo;
};


#endif /* FRAMECONTAINER_H */
