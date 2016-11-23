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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef FV_VISUALINLINEIMAGE_H
#define FV_VISUALINLINEIMAGE_H

#include "pt_Types.h"
#include "fl_FrameLayout.h"
#include "ut_string_class.h"
#include "fv_FrameEdit.h" // for FV_Base


typedef enum _FV_InlineDragMode
{
	FV_InlineDrag_NOT_ACTIVE,
	FV_InlineDrag_WAIT_FOR_MOUSE_CLICK,
	FV_InlineDrag_WAIT_FOR_MOUSE_DRAG,
	FV_InlineDrag_DRAGGING,
	FV_InlineDrag_RESIZE,
	FV_InlineDrag_START_DRAGGING
} FV_InlineDragMode;

class GR_Graphics;
class GR_Image;
class FV_View;
class PP_AttrProp;
class UT_ByteBuf;

class ABI_EXPORT FV_VisualInlineImage : public FV_Base
{
	friend class fv_View;

public:

	FV_VisualInlineImage (FV_View * pView);
	~FV_VisualInlineImage();
	bool                  isActive(void) const;
    void                      setMode(FV_InlineDragMode iInlineDragMode);
	FV_InlineDragMode     getInlineDragMode(void) const
		{ return m_iInlineDragMode;}
	void                  setDragType(UT_sint32 x,UT_sint32 y, bool bDrawImage);
	FV_DragWhat           mouseMotion(UT_sint32 x, UT_sint32 y);
	void                  mouseLeftPress(UT_sint32 x, UT_sint32 y);
	void                  mouseCut(UT_sint32 x, UT_sint32 y);
	void                  mouseCopy(UT_sint32 x, UT_sint32 y);
	void                  mouseRelease(UT_sint32 x, UT_sint32 y);
	bool                  drawImage(void);
	void                  getImageFromSelection(UT_sint32 x, UT_sint32 y,PP_AttrProp ** pAP = NULL );
	PP_AttrProp *         getImageAPFromXY(UT_sint32 x, UT_sint32 y);
	PT_DocPosition        getPosFromXY(UT_sint32 x, UT_sint32 y) const;
	void                  drawCursor(PT_DocPosition newPos);
	static void 		  _actuallyScroll(UT_Worker * pTimer);
	static void 		  _autoScroll(UT_Worker * pTimer);
	void                  clearCursor(void);
	void                  cleanUP(void);
	void                  abortDrag(void);
	const char *          getPNGImage(UT_ConstByteBufPtr & pBuf) const;
	UT_sint32             getImageSelBoxSize() const; // in device units!
	void                  setSelectionDrawn(bool bSelectionDrawn);
protected:
	virtual void          _mouseDrag(UT_sint32 x, UT_sint32 y);
private:
	FV_InlineDragMode     m_iInlineDragMode;
	GR_Image *            m_pDragImage;
	UT_sint32             m_iLastX;
	UT_sint32             m_iLastY;
	UT_sint32             m_iInitialOffX;
	UT_sint32             m_iInitialOffY;
	bool                  m_bTextCut;
	GR_Image *            m_pDocUnderCursor;
	bool                  m_bCursorDrawn;
	UT_Rect               m_recCursor;

	// autoscroll stuff
	UT_Timer *			  m_pAutoScrollTimer;

	bool                  m_bDoingCopy;
	PP_AttrProp *         m_pImageAP;
	GR_Image *            m_screenCache;
	UT_UTF8String         m_sCopyName;
	bool                  m_bIsEmbedded;
	bool				  m_bEmbedCanResize;
	UT_UTF8String         m_sDataId;
	bool                  m_bSelectionDrawn;
};

#endif /* FV_VISUALINLINEIMAGE_H */
