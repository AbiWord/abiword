/* AbiWord
 * Copyright (c) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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
 */

#ifndef FV_VISUALDRAGTEXT_H
#define FV_VISUALDRAGTEXT_H

#include "pt_Types.h"
#include "fl_FrameLayout.h"

typedef enum _FV_VisualDragMode
{
	FV_VisualDrag_NOT_ACTIVE,
	FV_VisualDrag_WAIT_FOR_MOUSE_CLICK,
	FV_VisualDrag_WAIT_FOR_MOUSE_DRAG,
	FV_VisualDrag_DRAGGING,
	FV_VisualDrag_START_DRAGGING
} FV_VisualDragMode;

class GR_Graphics;
class GR_Image;
class FV_View;

class ABI_EXPORT FV_VisualDragText
{
	friend class fv_View;

public:

	FV_VisualDragText (FV_View * pView);
	virtual ~FV_VisualDragText();
	GR_Graphics *         getGraphics(void) const ;
	bool                  isActive(void) const;
    void                  setMode(FV_VisualDragMode iVisualDragMode);
	FV_VisualDragMode      getVisualDragMode(void) const
		{ return m_iVisualDragMode;}
	virtual void          mouseDrag(UT_sint32 x, UT_sint32 y);
	void                  mouseCut(UT_sint32 x, UT_sint32 y);
	void                  mouseCopy(UT_sint32 x, UT_sint32 y);
	void                  mouseRelease(UT_sint32 x, UT_sint32 y);
	void                  drawImage(void);
	void                  getImageFromSelection(UT_sint32 x, UT_sint32 y);
	bool                  reposOffsets(UT_sint32 x, UT_sint32 y);
	PT_DocPosition        getPosFromXY(UT_sint32 x, UT_sint32 y);
	void                  drawCursor(PT_DocPosition newPos);
	static void 		  _actuallyScroll(UT_Worker * pTimer);
	static void 		  _autoScroll(UT_Worker * pTimer);
	void                  clearCursor(void);
	void                  abortDrag(void);
	bool                  isNotdraggingImage(void)
	  { return m_bNotDraggingImage;}
	bool                  isDoingCopy(void)
	  { return m_bDoingCopy;}
	UT_Rect *             getCurFrame(void)
	  { return &m_recCurFrame;}
 protected:
	void                  _mouseDrag(UT_sint32 x, UT_sint32 y);
	FV_View *             m_pView;
private:
	FV_VisualDragMode     m_iVisualDragMode;
	GR_Image *            m_pDragImage;
	UT_sint32             m_iLastX;
	UT_sint32             m_iLastY;
	UT_Rect               m_recCurFrame;
	UT_sint32             m_iInitialOffX;
	UT_sint32             m_iInitialOffY;
	UT_Rect               m_recOrigLeft;
	UT_Rect               m_recOrigRight;
	bool                  m_bTextCut;
	GR_Image *            m_pDocUnderCursor;
	bool                  m_bCursorDrawn;
	UT_Rect               m_recCursor;
	UT_Rect               m_recDoc;

	// autoscroll stuff
	UT_Timer *			  m_pAutoScrollTimer;
	UT_sint32			  m_xLastMouse;
	UT_sint32			  m_yLastMouse;

	bool                  m_bDoingCopy;
	bool                  m_bNotDraggingImage;
	bool                  m_bSelectedRow;
};

#endif /* FV_VISUALDRAGTEXT_H */
