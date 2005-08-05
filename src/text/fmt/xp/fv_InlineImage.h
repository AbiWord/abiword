/* AbiWord
 * Copyright (c) 2005 Martin Sevior <msevior@physics.unimelb.edu.au> 
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

#ifndef FV_VISUALINLINEIMAGE_H
#define FV_VISUALINLINEIMAGE_H

#include "pt_Types.h"
#include "fl_FrameLayout.h"
#include "ut_string_class.h"


typedef enum _FV_InlineDragMode
{
	FV_InlineDrag_NOT_ACTIVE,
	FV_InlineDrag_WAIT_FOR_MOUSE_CLICK,
	FV_InlineDrag_WAIT_FOR_MOUSE_DRAG,
	FV_InlineDrag_DRAGGING,
	FV_InlineDrag_RESIZE,
	FV_InlineDrag_START_DRAGGING
} FV_InlineDragMode;

typedef enum _FV_InlineDragWhat
{
	FV_Inline_DragNothing,
    FV_Inline_DragTopLeftCorner,
    FV_Inline_DragTopRightCorner,
    FV_Inline_DragBotLeftCorner,
    FV_Inline_DragBotRightCorner,
    FV_Inline_DragLeftEdge,
    FV_Inline_DragTopEdge,
    FV_Inline_DragRightEdge,
    FV_Inline_DragBotEdge,
    FV_Inline_DragWholeImage
} FV_InlineDragWhat;

class GR_Graphics;
class GR_Image;
class FV_View;
class PP_AttrProp;

class ABI_EXPORT FV_VisualInlineImage
{
	friend class fv_View;

public:

	FV_VisualInlineImage (FV_View * pView);
	~FV_VisualInlineImage();
	PD_Document *         getDoc(void) const;
	FL_DocLayout *        getLayout(void) const;
	GR_Graphics *         getGraphics(void) const ;
	bool                  isActive(void) const;
    void                      setMode(FV_InlineDragMode iInlineDragMode);
	FV_InlineDragMode     getInlineDragMode(void) const 
		{ return m_iInlineDragMode;}
	FV_InlineDragWhat     getInlineDragWhat(void) const 
		{ return       m_iDraggingWhat;;}
	void                  setDragType(UT_sint32 x,UT_sint32 y, bool bDrawImage);
	FV_InlineDragWhat     mouseMotion(UT_sint32 x, UT_sint32 y);
	void                  mouseLeftPress(UT_sint32 x, UT_sint32 y);
	void                  mouseDrag(UT_sint32 x, UT_sint32 y);
	void                  mouseCut(UT_sint32 x, UT_sint32 y);
	void                  mouseCopy(UT_sint32 x, UT_sint32 y);
	void                  mouseRelease(UT_sint32 x, UT_sint32 y);
	bool                  drawImage(void);
	void                  getImageFromSelection(UT_sint32 x, UT_sint32 y); 	
	PT_DocPosition        getPosFromXY(UT_sint32 x, UT_sint32 y);
	void                  drawCursor(PT_DocPosition newPos);
	static void 		  _autoScroll(UT_Worker * pTimer);
	void                  clearCursor(void);
	UT_sint32             getGlobCount(void);
	void                  _beginGlob();
	void                  _endGlob();
	void                  cleanUP(void);
private:
	FV_View *             m_pView;
	FV_InlineDragMode     m_iInlineDragMode;
	GR_Image *            m_pDragImage;
	UT_sint32             m_iLastX;
	UT_sint32             m_iLastY;
	UT_Rect               m_recCurFrame;
	UT_sint32             m_iInitialOffX;
	UT_sint32             m_iInitialOffY;
	UT_sint32             m_iFirstEverX;
	UT_sint32             m_iFirstEverY;
	bool                  m_bTextCut;
	GR_Image *            m_pDocUnderCursor;
	bool                  m_bCursorDrawn;
	UT_Rect               m_recCursor;

	// autoscroll stuff
	UT_Timer *			  m_pAutoScrollTimer;
	UT_sint32			  m_xLastMouse;
	UT_sint32			  m_yLastMouse;

	bool                  m_bDoingCopy;
	FV_InlineDragWhat     m_iDraggingWhat;
	UT_sint32             m_iGlobCount;
	PP_AttrProp *         m_pImageAP;
	GR_Image *            m_screenCache;
	bool                  m_bFirstDragDone;
	UT_UTF8String         m_sCopyName;
	bool                  m_bIsEmbedded;
};

#endif /* FV_VISUALINLINEIMAGE_H */
