/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include "ut_units.h"
#include "xav_Listener.h"
#include "ap_Ruler.h"
#include "ev_EditBits.h"
#include "gr_Graphics.h"
#include "fl_BlockLayout.h"
#include "xap_Strings.h"

class XAP_App;
class XAP_Frame;
class XAP_Prefs;
class UT_AlphaHashTable;
class AV_ScrollObj;
class UT_Timer;

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
	UT_sint32				m_xrTabStop;

	// tab stop information

	bool					(*m_pfnEnumTabStops)(void * pData, UT_uint32 k, fl_TabStop *pTabInfo);
	void *					m_pVoidEnumTabStopsData;
	UT_sint32				m_iTabStops;
	UT_sint32				m_iDefaultTabInterval;
	const char *			m_pszTabStops;

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
	void			setView(AV_View* pView, UT_uint32 iZoom);
	void			setOffsetLeftRuler(UT_uint32 iLeftRulerWidth);
	void			setHeight(UT_uint32 iHeight);
	UT_uint32		getHeight(void) const;
	void			setWidth(UT_uint32 iWidth);
	UT_uint32		getWidth(void) const;
	void			draw(const UT_Rect * pClipRect, AP_TopRulerInfo * pUseInfo = NULL);
	void			scrollRuler(UT_sint32 xoff, UT_sint32 xlimit);

	void			mouseMotion(EV_EditModifierState ems, UT_sint32 x, UT_sint32 y);
	void			mousePress(EV_EditModifierState ems, EV_EditMouseButton emb, UT_uint32 x, UT_uint32 y);
	void			mouseRelease(EV_EditModifierState ems, EV_EditMouseButton emb, UT_sint32 x, UT_sint32 y);

	/* used with AV_Listener */
	virtual bool	notify(AV_View * pView, const AV_ChangeMask mask);

	/* used with AV_ScrollObj */
	static void		_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void		_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);

	/* for use with the prefs listener top_ruler_prefs_listener */
	UT_Dimension	getDimension() const { return m_dim; }
	void			setDimension( UT_Dimension newdim );
	
protected:
	void	_draw(const UT_Rect * pClipRect, AP_TopRulerInfo * pUseInfo);
	void	_drawBar(const UT_Rect * pClipRect, AP_TopRulerInfo * pInfo,
					 GR_Graphics::GR_Color3D clr3d, UT_sint32 x, UT_sint32 w);
	void	_drawTickMark(const UT_Rect * pClipRect,
						  AP_TopRulerInfo * pInfo, ap_RulerTicks &tick,
						  GR_Graphics::GR_Color3D clr3d, GR_Font * pFont,
						  UT_sint32 k, UT_sint32 xTick);
	void	_drawTicks(const UT_Rect * pClipRect,
					   AP_TopRulerInfo * pInfo, ap_RulerTicks &tick,
					   GR_Graphics::GR_Color3D clr3d, GR_Font * pFont,
					   UT_sint32 xOrigin, UT_sint32 xFrom, UT_sint32 xTo);

	void	_getParagraphMarkerXCenters(AP_TopRulerInfo * pInfo,
										UT_sint32 * pLeft, UT_sint32 * pRight, UT_sint32 * pFirstLine);
	void	_getParagraphMarkerRects(AP_TopRulerInfo * pInfo,
									 UT_sint32 leftCenter, UT_sint32 rightCenter, UT_sint32 firstLineCenter,
									 UT_Rect * prLeftIndent, UT_Rect * prRightIndent, UT_Rect * prFirstLineIndent);
	void	_drawParagraphProperties(const UT_Rect * pClipRect,
									 AP_TopRulerInfo * pInfo,
									 bool bDrawAll = true);

	void	_getTabToggleRect(UT_Rect * prToggle);
	void	_drawTabToggle(const UT_Rect * pClipRect, bool bErase);

	void	_getTabStopXAnchor(AP_TopRulerInfo * pInfo, UT_sint32 k, UT_sint32 * pTab, eTabType & iType, eTabLeader & iLeader);
	void	_getTabStopRect(AP_TopRulerInfo * pInfo, UT_sint32 anchor, UT_Rect * pRect);
	void	_drawTabProperties(const UT_Rect * pClipRect,
								   AP_TopRulerInfo * pInfo,
								   bool bDrawAll = true);

	UT_sint32		_findTabStop(AP_TopRulerInfo * pInfo, UT_uint32 x, UT_uint32 y, eTabType & iType, eTabLeader & iLeader);
	const char *	_getTabStopString(AP_TopRulerInfo * pInfo, UT_sint32 k);
	void			_getTabZoneRect(AP_TopRulerInfo * pInfo, UT_Rect &rZone);
	void			_setTabStops(ap_RulerTicks tick, UT_sint32 iTab, eTabLeader iLeader, bool bDelete);

	UT_sint32	_getColumnMarkerXRightEnd(AP_TopRulerInfo * pInfo, UT_uint32 kCol);
	void		_getColumnMarkerRect(AP_TopRulerInfo * pInfo, UT_uint32 kCol, UT_sint32 xCenter,
									 UT_Rect * prCol);
	void		_drawColumnProperties(const UT_Rect * pClipRect,
									  AP_TopRulerInfo * pInfo,
									  UT_uint32 kCol);

	void		_getMarginMarkerRects(AP_TopRulerInfo * pInfo, UT_Rect &rLeft, UT_Rect &rRight);
	void		_drawMarginProperties(const UT_Rect * pClipRect,
									  AP_TopRulerInfo * pInfo, GR_Graphics::GR_Color3D clr);
	void		_xorGuide(bool bClear=false);

	void		_ignoreEvent(bool bDone);
	double		_scalePixelDistanceToUnits(UT_sint32 xColRel, ap_RulerTicks & tick);
	double		_getUnitsFromRulerLeft(UT_sint32 xColRel, ap_RulerTicks & tick);
	UT_sint32	_getFirstPixelInColumn(AP_TopRulerInfo * pInfo, UT_uint32 kCol);
	UT_sint32	_snapPixelToGrid(UT_sint32 xDist, ap_RulerTicks & tick);
	void		_drawLeftIndentMarker(UT_Rect & r, bool bFilled);
	void		_drawRightIndentMarker(UT_Rect & r, bool bFilled);
	void		_drawFirstLineIndentMarker(UT_Rect & r, bool bFilled);
	void		_drawTabStop(UT_Rect & r, eTabType iType, bool bFilled);
	void		_drawColumnGapMarker(UT_Rect & r);
	bool		_isInBottomBoxOfLeftIndent(UT_uint32 y);
	void		_displayStatusMessage(XAP_String_Id messageID, const ap_RulerTicks &tick, double dValue);
	void		_displayStatusMessage(XAP_String_Id messageID, const ap_RulerTicks &tick, double dValue1, double dValue2);

	// must be static so that I can pass as a functional arg - shack
	static void _prefsListener( XAP_App *pApp, XAP_Prefs *pPrefs, UT_AlphaHashTable *phChanges, void *data );

	// autoscroll stuff
	static void			_autoScroll(UT_Timer * pTimer);
	UT_Timer *			m_pAutoScrollTimer;
	char				m_aScrollDirection; // 'L' == left   'R' == right
	
	XAP_Frame *			m_pFrame;
	AV_View *			m_pView;
	AV_ScrollObj *		m_pScrollObj;
	GR_Graphics *		m_pG;
	UT_Dimension		m_dim;
	UT_uint32			m_iHeight;		/* size of window */
	UT_uint32			m_iWidth;		/* size of window */
	UT_uint32			m_iLeftRulerWidth;
	UT_sint32			m_xScrollOffset;
	UT_sint32			m_xScrollLimit;

	UT_sint32			m_minColumnWidth;

	AP_TopRulerInfo		m_infoCache;
	bool				m_bValidMouseClick;
	bool				m_bEventIgnored;

	typedef enum _draggingWhat { DW_NOTHING,
								 DW_LEFTMARGIN,
								 DW_RIGHTMARGIN,
								 DW_COLUMNGAP,
								 DW_COLUMNGAPLEFTSIDE,
								 DW_LEFTINDENT,
								 DW_RIGHTINDENT,
								 DW_FIRSTLINEINDENT,
								 DW_LEFTINDENTWITHFIRST,
								 DW_TABSTOP
	} DraggingWhat;

	DraggingWhat		m_draggingWhat;
	UT_sint32			m_draggingCenter; /* center of primary thing being dragged */
	UT_Rect				m_draggingRect;	/* rectangle of primary thing being dragged */
	UT_sint32			m_dragging2Center; /* center of drag-along */
	UT_Rect				m_dragging2Rect; /* rect of drag-along */
	UT_sint32			m_draggingTab;	/* index of tab being dragged */
	eTabType			m_draggingTabType;
	eTabLeader			m_draggingTabLeader;
	UT_sint32			m_dragStart;
	bool				m_bBeforeFirstMotion;

	eTabType			m_iDefaultTabType;

	bool				m_bGuide;	/* true ==> guide line XORed onscreen */
	UT_sint32			m_xGuide;	/* valid iff m_bGuide */
	UT_sint32			m_xOtherGuide;
	
	UT_sint32			m_oldX; /* Only for dragging; used to see if object has moved */

	/* static const*/ UT_uint32	s_iFixedHeight /* =32 */;	/* size we draw stuff w/o regard to window size */
	/* static const*/ UT_uint32	s_iFixedWidth  /* =32 */;	/* minimum width of non-scrolling area on left */

	AV_ListenerId		m_lidTopRuler;		/* need to save the view/listenerID so we can removeListener in destructor */
};

#endif /* AP_TOPRULER_H */
