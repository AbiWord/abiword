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

#ifndef AP_LEFTRULER_H
#define AP_LEFTRULER_H

// Class for dealing with the horizontal ruler at the top of
// a document window.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "xav_Listener.h"

#include "gr_Graphics.h"
#include "ev_EditBits.h"

class XAP_App;
class XAP_Frame;
class XAP_Prefs;
class UT_AlphaHashTable;
class AV_ScrollObj;
class GR_Graphics;
class ap_RulerTicks;

/*****************************************************************/
/*****************************************************************/

class AP_LeftRulerInfo
{
public:
	typedef enum _mode { TRI_MODE_COLUMNS, TRI_MODE_TABLE } Mode;
		
	Mode					m_mode;

	/* all values are in pixels */

	UT_uint32				m_yPageStart;		/* absolute coord of start of page */
	UT_uint32				m_yPageSize;		/* absolute page size for the current page */
	UT_uint32				m_yPoint;			/* absolute coord of current insertion point */
	UT_sint32				m_yTopMargin;		/* content start relative to top of page */
	UT_sint32				m_yBottomMargin;	/* content end relative to top of page */
};
	
/*****************************************************************/

class AP_LeftRuler : public AV_Listener
{
public:
	AP_LeftRuler(XAP_Frame * pFrame);
	virtual ~AP_LeftRuler(void);

	virtual void		setView(AV_View * pView);
	void				setView(AV_View* pView, UT_uint32 iZoom);
	void				setHeight(UT_uint32 iHeight);
	UT_uint32			getHeight(void) const;
	void				setWidth(UT_uint32 iWidth);
	UT_uint32			getWidth(void) const;
	void				draw(const UT_Rect * pClipRect);
	void				draw(const UT_Rect * pClipRect, AP_LeftRulerInfo & lfi);
	void				scrollRuler(UT_sint32 yoff, UT_sint32 ylimit);

	void			    mouseMotion(EV_EditModifierState ems, UT_sint32 x, UT_sint32 y);
	void                mousePress(EV_EditModifierState ems, EV_EditMouseButton emb, UT_uint32 x, UT_uint32 y);

	void                mouseRelease(EV_EditModifierState ems, EV_EditMouseButton emb, UT_sint32 x, UT_sint32 y);

	/* used with AV_Listener */
	virtual bool		notify(AV_View * pView, const AV_ChangeMask mask);

	/* used with AV_ScrollObj */
	static void			_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void			_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
	
	/* for use with the prefs listener top_ruler_prefs_listener */
	UT_Dimension	    getDimension() const { return m_dim; }
	void			    setDimension( UT_Dimension newdim );
	
protected:
	void                _refreshView(void);

//	void				_draw3DFrame(const UT_Rect * pClipRect, AP_TopRulerInfo * pInfo,
//									 UT_sint32 x, UT_sint32 h);

	// must be static so that I can pass as a functional arg - shack
	static void _prefsListener( XAP_App *pApp, XAP_Prefs *pPrefs, UT_AlphaHashTable *phChanges, void *data );
	
	XAP_Frame *			m_pFrame;
	GR_Graphics *		m_pG;

	/* static const*/ UT_uint32	s_iFixedHeight /* =32 */;	/* size we draw stuff w/o regard to window size */
	/* static const*/ UT_uint32	s_iFixedWidth  /* =32 */;	/* minimum width of non-scrolling area on left */

private:
	UT_sint32           _snapPixelToGrid(UT_sint32 xDist, ap_RulerTicks & tick);
	double              _scalePixelDistanceToUnits(UT_sint32 yDist, ap_RulerTicks & tick);
	void                _ignoreEvent(bool bDone);

	void                _getMarginMarkerRects(AP_LeftRulerInfo * pInfo, UT_Rect &rTop, UT_Rect &rBottom);
	void		        _drawMarginProperties(const UT_Rect * pClipRect,
											  AP_LeftRulerInfo * pInfo, 
											  GR_Graphics::GR_Color3D clr);

	void                _xorGuide(bool bClear=false);

	AP_LeftRulerInfo	m_lfi;					/* the values we last drew with */

	// scrolling objects
	AV_ScrollObj *		m_pScrollObj;
	UT_sint32			m_yScrollOffset;
	UT_sint32			m_yScrollLimit;

	AV_ListenerId		m_lidLeftRuler;

	// misc info

	AV_View *			m_pView;
	UT_Dimension		m_dim;
	UT_uint32			m_iHeight;		/* size of window */
	UT_uint32			m_iWidth;		/* size of window */

	AP_LeftRulerInfo	m_infoCache;

	UT_sint32			m_oldY; /* Only for dragging; used to see if object has moved */

	typedef enum _draggingWhat { DW_NOTHING,
								 DW_TOPMARGIN,
								 DW_BOTTOMMARGIN
	} DraggingWhat;

	DraggingWhat		m_draggingWhat;
	UT_sint32			m_draggingCenter; /* center of primary thing being dragged */
	bool				m_bBeforeFirstMotion;

	bool				m_bGuide;	/* true ==> guide line XORed onscreen */
	UT_sint32			m_yGuide;	/* valid iff m_bGuide */

	bool				m_bValidMouseClick;
	bool				m_bEventIgnored;

	UT_sint32           m_minPageLength;
};

#endif /* AP_LEFTRULER_H */
