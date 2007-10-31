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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef FV_FRAME_EDIT_H
#define FV_FRAME_EDIT_H

#include "pt_Types.h"
#include "fl_FrameLayout.h"

typedef enum _FV_FrameEditMode
{
	FV_FrameEdit_NOT_ACTIVE,
	FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT,
	FV_FrameEdit_RESIZE_INSERT,
    FV_FrameEdit_RESIZE_SELECTED_EXISTING,
	FV_FrameEdit_RESIZE_EXISTING,
	FV_FrameEdit_DRAG_EXISTING,
	FV_FrameEdit_EXISTING_SELECTED
} FV_FrameEditMode;

typedef enum _FV_FrameEditDragWhat
{
	FV_FrameEdit_DragNothing,
    FV_FrameEdit_DragTopLeftCorner,
    FV_FrameEdit_DragTopRightCorner,
    FV_FrameEdit_DragBotLeftCorner,
    FV_FrameEdit_DragBotRightCorner,
    FV_FrameEdit_DragLeftEdge,
    FV_FrameEdit_DragTopEdge,
    FV_FrameEdit_DragRightEdge,
    FV_FrameEdit_DragBotEdge,
    FV_FrameEdit_DragWholeFrame
} FV_FrameEditDragWhat;

class FL_DocLayout;
class PD_Document;
class GR_Graphics;
class FV_View;
class GR_Image;
class fp_Page;

/**
 * Base class for (currently) FV_FrameEdit and FV_VisualInlineImage
 */
class ABI_EXPORT FV_Base
{
public:
	FV_Base( FV_View* pView );
	virtual ~FV_Base();
	PD_Document *         getDoc(void) const;
	FL_DocLayout *        getLayout(void) const;
	GR_Graphics *         getGraphics(void) const;
	inline FV_View *      getView(void) { return m_pView;}

protected:
	FV_View *             m_pView;
};

class ABI_EXPORT FV_FrameEdit : public FV_Base
{
	friend class fv_View;

public:

	FV_FrameEdit (FV_View * pView);
	~FV_FrameEdit ();
	bool                  isActive(void) const;
	void                  abortDrag(void);
	UT_sint32             haveDragged(void) const;
    void                  setMode(FV_FrameEditMode iEditMode);
	FV_FrameEditMode      getFrameEditMode(void) const 
		{ return m_iFrameEditMode;}
	FV_FrameEditDragWhat  getFrameEditDragWhat(void) const 
		{ return m_iDraggingWhat;}
	virtual void          mouseDrag(UT_sint32 x, UT_sint32 y);
	void                  mouseLeftPress(UT_sint32 x, UT_sint32 y);
	void                  mouseRelease(UT_sint32 x, UT_sint32 y);
	FV_FrameEditDragWhat  mouseMotion(UT_sint32 x, UT_sint32 y);
	void                  drawFrame(bool bWithHandles);
	void                  deleteFrame(fl_FrameLayout * pFL = NULL);
	void                  setDragType(UT_sint32 x,UT_sint32 y, bool bDrawFrame);
	bool                  getFrameStrings(UT_sint32 x, UT_sint32 y,
					      UT_String & sXpos,
					      UT_String & sYpos,
					      UT_String & sWidth,
					      UT_String & sHeight,
					      UT_String & sColXpos,
					      UT_String & sColYpos,
					      UT_String & sPageXpos,
					      UT_String & sPageYpos,
					      fl_BlockLayout ** pCloseBL,
					      fp_Page ** pPage);
	fl_FrameLayout *      getFrameLayout(void)
		{ return m_pFrameLayout;}
	const char *          getPNGImage(const UT_ByteBuf ** ppByteBuf);
	void                  setPointInside(void);
	fp_FrameContainer *   getFrameContainer(void) { return m_pFrameContainer;}
	static void 		  _actuallyScroll(UT_Worker * pTimer);
	static void 		  _autoScroll(UT_Worker * pTimer);
	void                  _beginGlob();
	void                  _endGlob();
	UT_sint32             getGlobCount(void);
        bool                  isImageWrapper(void) const;  	
protected:
	virtual void          _mouseDrag(UT_sint32 x, UT_sint32 y);

private:
	FV_FrameEditMode      m_iFrameEditMode;
	fl_FrameLayout *      m_pFrameLayout;
	fp_FrameContainer *   m_pFrameContainer;
	FV_FrameEditDragWhat  m_iDraggingWhat;
	UT_sint32             m_iLastX;
	UT_sint32             m_iLastY;
	UT_Rect               m_recCurFrame;
	UT_sint32             m_iInitialDragX;
	UT_sint32             m_iInitialDragY;
	bool                  m_bFirstDragDone;
	bool                  m_bInitialClick;
	GR_Image *            m_pFrameImage;

	// autoscroll stuff
	UT_Timer *			  m_pAutoScrollTimer;
	UT_sint32			  m_xLastMouse;
	UT_sint32			  m_yLastMouse;

	UT_sint32             m_iFirstEverX;
	UT_sint32             m_iFirstEverY;
	UT_sint32             m_iGlobCount;
	//
	UT_sint32             m_iInitialFrameX;
	UT_sint32             m_iInitialFrameY;
};

#endif /* FV_FRAME_EDIT_H */
