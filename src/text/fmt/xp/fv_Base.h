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

#ifndef FV_BASE_H
#define FV_BASE_H

typedef enum _FV_DragWhat
{
	FV_DragNothing,
    FV_DragTopLeftCorner,
    FV_DragTopRightCorner,
    FV_DragBotLeftCorner,
    FV_DragBotRightCorner,
    FV_DragLeftEdge,
    FV_DragTopEdge,
    FV_DragRightEdge,
    FV_DragBotEdge,
    FV_DragWhole
} FV_DragWhat;

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
	PD_Document *			getDoc(void) const;
	FL_DocLayout *			getLayout(void) const;
	GR_Graphics *			getGraphics(void) const;
	inline FV_View *		getView(void) const
		{ return m_pView;}
	UT_sint32				getGlobCount(void) const;
	void          			mouseDrag(UT_sint32 x, UT_sint32 y);	// non virtual calling virtual _mouseDrag
	FV_DragWhat				getDragWhat(void) const
		{ return m_iDraggingWhat; }
	void					setDragWhat( FV_DragWhat iDragWhat )
		{ m_iDraggingWhat = iDragWhat; }

protected:
	FV_View *				m_pView;
	UT_sint32				m_iGlobCount;
	UT_Rect					m_recCurFrame;
	bool					m_bFirstDragDone;
	UT_sint32				m_iFirstEverX;
	UT_sint32				m_iFirstEverY;
	UT_sint32				m_xLastMouse;
	UT_sint32				m_yLastMouse;

	void					_beginGlob();
	void					_endGlob();
	virtual void			_mouseDrag(UT_sint32 x, UT_sint32 y) = 0;
	void					_doMouseDrag(UT_sint32 x, UT_sint32 y, UT_sint32& dx, UT_sint32& dy, UT_Rect& expX, UT_Rect& expY);
	void					_checkDimensions();

private:
	FV_DragWhat				m_iDraggingWhat;	// made private on purpose
};

#endif
