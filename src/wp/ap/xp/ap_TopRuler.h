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

#ifndef AP_TOPRULER_H
#define AP_TOPRULER_H

// Class for dealing with the horizontal ruler at the top of
// a document window.

#include "ut_types.h"
#include "ut_misc.h"
#include "xav_Listener.h"
#include "ap_Ruler.h"
#include "ev_EditBits.h"

class XAP_Frame;
class AV_ScrollObj;
class GR_Graphics;

/*****************************************************************/
/*****************************************************************/

class AP_TopRulerInfo
{
public:
	typedef enum _mode { TRI_MODE_COLUMNS, TRI_MODE_TABLE } Mode;
		
	Mode					m_mode;
	UT_uint32				m_xPaperSize;
	UT_uint32				m_xPageViewMargin;
	
	// current caret position -- relative to the current column
	
	UT_sint32				m_xrPoint;
	UT_sint32				m_xrLeftIndent;
	UT_sint32				m_xrFirstLineIndent;
	UT_sint32				m_xrRightIndent;

	// current column number and the number of columns
	
	UT_uint32				m_iCurrentColumn;
	UT_uint32				m_iNumColumns;

	union _u {

		struct _c {

			// page absolute document margins
			
			UT_sint32		m_xaLeftMargin;
			UT_sint32		m_xaRightMargin;

			// column width and spacing -- currently we only support
			// uniform gaps and widths for columns
			
			UT_uint32		m_xColumnGap;
			UT_uint32		m_xColumnWidth;

		} c;									/* valid when column mode */

		struct _t {

			int foo;
			
		} t;									/* valid when table mode */

	} u;
};

/*****************************************************************/

class AP_TopRuler : public AV_Listener
{
public:
	AP_TopRuler(XAP_Frame * pFrame);
	virtual ~AP_TopRuler(void);

	virtual void	setView(AV_View * pView);
	void			setOffsetLeftRuler(UT_uint32 iLeftRulerWidth);
	void			setHeight(UT_uint32 iHeight);
	UT_uint32		getHeight(void) const;
	void			setWidth(UT_uint32 iWidth);
	UT_uint32		getWidth(void) const;
	void			draw(const UT_Rect * pClipRect, AP_TopRulerInfo * pUseInfo = NULL);
	void			scrollRuler(UT_sint32 xoff, UT_sint32 xlimit);

	void			mouseMotion(EV_EditModifierState ems, UT_uint32 x, UT_uint32 y);
	void			mousePress(EV_EditModifierState ems, EV_EditMouseButton emb, UT_uint32 x, UT_uint32 y);
	void			mouseRelease(EV_EditModifierState ems, EV_EditMouseButton emb, UT_uint32 x, UT_uint32 y);

	/* used with AV_Listener */
	virtual UT_Bool	notify(AV_View * pView, const AV_ChangeMask mask);

	/* used with AV_ScrollObj */
	static void		_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void		_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
	
protected:
	void	_draw(const UT_Rect * pClipRect, AP_TopRulerInfo * pUseInfo);
	void	_drawBar(const UT_Rect * pClipRect, AP_TopRulerInfo * pInfo,
					 UT_RGBColor &clr, UT_sint32 x, UT_sint32 w);
	void	_drawTickMark(const UT_Rect * pClipRect,
						  AP_TopRulerInfo * pInfo, ap_RulerTicks &tick,
						  UT_RGBColor &clr, GR_Font * pFont,
						  UT_sint32 k, UT_sint32 xTick);
	void	_drawTicks(const UT_Rect * pClipRect,
					   AP_TopRulerInfo * pInfo, ap_RulerTicks &tick,
					   UT_RGBColor &clr, GR_Font * pFont,
					   UT_sint32 xOrigin, UT_sint32 xFrom, UT_sint32 xTo);

	void	_getParagraphMarkerXCenters(AP_TopRulerInfo * pInfo,
										UT_sint32 * pLeft, UT_sint32 * pRight, UT_sint32 * pFirstLine);
	void	_getParagraphMarkerRects(AP_TopRulerInfo * pInfo,
									 UT_sint32 leftCenter, UT_sint32 rightCenter, UT_sint32 firstLineCenter,
									 UT_Rect * prLeftIndent, UT_Rect * prRightIndent, UT_Rect * prFirstLineIndent);
	void	_drawParagraphProperties(const UT_Rect * pClipRect,
									 AP_TopRulerInfo * pInfo,
									 UT_Bool bDrawAll = UT_TRUE);

	UT_sint32	_getColumnMarkerXRightEnd(AP_TopRulerInfo * pInfo, UT_uint32 kCol);
	void		_getColumnMarkerRect(AP_TopRulerInfo * pInfo, UT_uint32 kCol, UT_sint32 xCenter,
									 UT_Rect * prCol);
	void		_drawColumnProperties(const UT_Rect * pClipRect,
									  AP_TopRulerInfo * pInfo,
									  UT_uint32 kCol);

	void		_getMarginMarkerRects(AP_TopRulerInfo * pInfo, UT_Rect &rLeft, UT_Rect &rRight);
	void		_drawMarginProperties(const UT_Rect * pClipRect,
									  AP_TopRulerInfo * pInfo, UT_RGBColor &clr);

	void		_ignoreEvent(UT_RGBColor &clrBlack, UT_RGBColor &clrWhite);
	double		_scalePixelDistanceToUnits(UT_sint32 xColRel, ap_RulerTicks & tick);
	UT_sint32	_getFirstPixelInColumn(AP_TopRulerInfo * pInfo, UT_uint32 kCol);
	void		_drawHollowRect(UT_RGBColor &clrDark, UT_RGBColor &clrLight, UT_Rect &r);
	UT_sint32	_snapPixelToGrid(UT_sint32 xDist, ap_RulerTicks & tick);
	void		_drawSculptedRect(UT_Rect &r);
	void		_drawLeftIndentMarker(UT_Rect & r, UT_Bool bFilled);
	void		_drawRightIndentMarker(UT_Rect & r, UT_Bool bFilled);
	void		_drawFirstLineIndentMarker(UT_Rect & r, UT_Bool bFilled);
	void		_drawColumnGapMarker(UT_Rect & r);
	UT_Bool		_isInBottomBoxOfLeftIndent(UT_uint32 y);
		
	XAP_Frame *			m_pFrame;
	AV_View *			m_pView;
	AV_ScrollObj *		m_pScrollObj;
	GR_Graphics *		m_pG;
	UT_uint32			m_iHeight;		/* size of window */
	UT_uint32			m_iWidth;		/* size of window */
	UT_uint32			m_iLeftRulerWidth;
	UT_sint32			m_xScrollOffset;
	UT_sint32			m_xScrollLimit;

	AP_TopRulerInfo		m_infoCache;
	UT_Bool				m_bValidMouseClick;

	typedef enum _draggingWhat { DW_NOTHING,
								 DW_LEFTMARGIN,
								 DW_RIGHTMARGIN,
								 DW_COLUMNGAP,
								 DW_COLUMNGAPLEFTSIDE,
								 DW_LEFTINDENT,
								 DW_RIGHTINDENT,
								 DW_FIRSTLINEINDENT,
								 DW_LEFTINDENTWITHFIRST
	} DraggingWhat;

	DraggingWhat		m_draggingWhat;
	UT_sint32			m_draggingCenter; /* center of primary thing being dragged */
	UT_Rect				m_draggingRect;	/* rectangle of primary thing being dragged */
	UT_sint32			m_dragging2Center; /* center of drag-along */
	UT_Rect				m_dragging2Rect; /* rect of drag-along */
	UT_Bool				m_bBeforeFirstMotion;
	
	/* static const*/ UT_uint32	s_iFixedHeight /* =32 */;	/* size we draw stuff w/o regard to window size */
	/* static const*/ UT_uint32	s_iFixedWidth  /* =32 */;	/* minimum width of non-scrolling area on left */
};

#endif /* AP_TOPRULER_H */
