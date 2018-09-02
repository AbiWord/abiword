/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuière
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

#ifndef AP_LEFTRULER_H
#define AP_LEFTRULER_H

#include "xap_Features.h"
// Class for dealing with the horizontal ruler at the top of
// a document window.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "ut_hash.h"
#include "xav_Listener.h"

#include "gr_Graphics.h"
#include "ev_EditBits.h"
#include "pt_Types.h"
#include "xap_Strings.h"
#include "xap_Prefs.h"
#include "xap_CustomWidget.h"

class XAP_App;
class XAP_Frame;
class XAP_Prefs;
class AV_ScrollObj;
class GR_Graphics;
class ap_RulerTicks;
class fp_CellContainer;
class fp_TableContainer;
/*****************************************************************/


/*****************************************************************/
class ABI_EXPORT AP_LeftRulerTableInfo
{
public:
	UT_sint32 m_iTopCellPos;
	UT_sint32 m_iTopSpacing;
	UT_sint32 m_iBotCellPos;
	UT_sint32 m_iBotSpacing;
	fp_CellContainer * m_pCell;
};

/*****************************************************************/

/*****************************************************************/

class ABI_EXPORT AP_LeftRulerInfo
{
public:
	typedef enum _mode { TRI_MODE_COLUMNS,
						 TRI_MODE_TABLE,
						 TRI_MODE_FRAME } Mode;

	AP_LeftRulerInfo(void) : 	m_mode(TRI_MODE_COLUMNS),
								m_yPageStart(0),
								m_yPageSize(0),
								m_yPoint(0),
								m_yTopMargin(0),
								m_yBottomMargin(0),
								m_iNumRows(0),
								m_iCurrentRow(0),
								m_iTablePadding(0),
								m_vecTableRowInfo(NULL)
		{
		}
	virtual ~AP_LeftRulerInfo(void)
		{
			if(m_vecTableRowInfo)
			{
				UT_sint32 count = m_vecTableRowInfo->getItemCount();
				UT_sint32 i =0;
				for(i=0; i< count; i++)
				{
					delete m_vecTableRowInfo->getNthItem(i);
				}
				DELETEP(m_vecTableRowInfo);
			}
		}

	Mode					m_mode;

	/* all values are in layout units */

	UT_uint32				m_yPageStart;		/* absolute coord of start of page */
	UT_uint32				m_yPageSize;		/* absolute page size for the current page */
	UT_uint32				m_yPoint;			/* absolute coord of current insertion point */
	UT_sint32				m_yTopMargin;		/* content start relative to top of page */
	UT_sint32				m_yBottomMargin;	/* content end relative to top of page */

// Things we need for Tables

	UT_sint32               m_iNumRows;
	UT_sint32               m_iCurrentRow;
	UT_sint32               m_iTablePadding;
	UT_GenericVector<AP_LeftRulerTableInfo *> * m_vecTableRowInfo;
};

/*****************************************************************/

class ABI_EXPORT AP_LeftRuler : public AV_Listener, public XAP_CustomWidgetLU
{
public:
	AP_LeftRuler(XAP_Frame * pFrame);
	virtual ~AP_LeftRuler(void);

	virtual void		setView(AV_View * pView);
	void				setView(AV_View* pView, UT_uint32 iZoom);
	void				setViewHidden(AV_View* pView);
	void                setZoom(UT_uint32 iZoom);
	bool                isHidden(void) const
		{ return m_bIsHidden;}
	AV_View *           getView(void) const
		{return m_pView;}
	void				setHeight(UT_uint32 iHeight);
	UT_uint32			getHeight(void) const;
	void				setWidth(UT_uint32 iWidth);
	UT_uint32			getWidth(void) const;
	void				scrollRuler(UT_sint32 yoff, UT_sint32 ylimit);

	void			    mouseMotion(EV_EditModifierState ems, UT_sint32 x, UT_sint32 y);
	void                mousePress(EV_EditModifierState ems, EV_EditMouseButton emb, UT_uint32 x, UT_uint32 y);

	void                mouseRelease(EV_EditModifierState ems, EV_EditMouseButton emb, UT_sint32 x, UT_sint32 y);
	UT_sint32           setTableLineDrag(PT_DocPosition pos, UT_sint32 & iFixed, UT_sint32 y);
	/* used with AV_Listener */
	virtual bool		notify(AV_View * pView, const AV_ChangeMask mask);
    virtual  AV_ListenerType getType(void) { return AV_LISTENER_LEFTRULER;}

	/* used with AV_ScrollObj */
	static void			_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void			_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);

	/* for use with the prefs listener top_ruler_prefs_listener */
	UT_Dimension	    getDimension() const { return m_dim; }
	void			    setDimension( UT_Dimension newdim );
	GR_Graphics *       getGraphics(void) const { return m_pG;}
protected:
	void                _refreshView(void);

	/* don't call this function directly, use XAP_CustomWidget::queueDraw() instead */
	virtual void		drawLU(const UT_Rect *clip);

//	void				_draw3DFrame(const UT_Rect * pClipRect, AP_TopRulerInfo * pInfo,
//									 UT_sint32 x, UT_sint32 h);

	// must be static so that I can pass as a functional arg - shack
	static void _prefsListener( XAP_Prefs *pPrefs, const XAP_PrefsChangeSet *phChanges, void *data );

	XAP_Frame *			m_pFrame;
	GR_Graphics *		m_pG;

	// These are in device units.
	/* static const*/ UT_uint32	s_iFixedHeight /* =32 */;	/* size we draw stuff w/o regard to window size */
	/* static const*/ UT_uint32	s_iFixedWidth  /* =32 */;	/* minimum width of non-scrolling area on left */

private:
	UT_sint32           _snapPixelToGrid(UT_sint32 xDist, ap_RulerTicks & tick);
	double              _scalePixelDistanceToUnits(UT_sint32 yDist, ap_RulerTicks & tick);
	void                _ignoreEvent(bool bDone);
protected:
	void                _getMarginMarkerRects(const AP_LeftRulerInfo * pInfo, UT_Rect &rTop, UT_Rect &rBottom);

	virtual void		_drawMarginProperties(const UT_Rect * pClipRect,
											  const AP_LeftRulerInfo * pInfo,
											  GR_Graphics::GR_Color3D clr);
private:

	void                _getCellMarkerRects(const AP_LeftRulerInfo * pInfo, UT_sint32 iCell, UT_Rect &rCell, fp_TableContainer * pBroke=NULL);
	void		        _drawCellProperties(const AP_LeftRulerInfo * pInfo);
protected:
	virtual void		_drawCellMark(UT_Rect *prDrag, bool bUp);
private:
	void                _xorGuide(bool bClear=false);
	void				_displayStatusMessage(XAP_String_Id messageID, const ap_RulerTicks &tick, double dValue);

	AP_LeftRulerInfo *	m_lfi; /* the values we last drew with */

	// scrolling objects
	AV_ScrollObj *		m_pScrollObj;
	UT_sint32			m_yScrollOffset;
	UT_sint32			m_yScrollLimit;

	AV_ListenerId		m_lidLeftRuler;

	// misc info

	AV_View *			m_pView;
	UT_Dimension		m_dim;
	UT_uint32			m_iHeight;		/* size of window, in device units */
	UT_uint32			m_iWidth;		/* size of window, in device units */

	AP_LeftRulerInfo 	m_infoCache;
	UT_sint32			m_oldY; /* Only for dragging; used to see if object has moved */

	typedef enum _draggingWhat { DW_NOTHING,
								 DW_TOPMARGIN,
								 DW_BOTTOMMARGIN,
								 DW_CELLMARK
	} DraggingWhat;

	DraggingWhat		m_draggingWhat;
	UT_sint32			m_draggingCenter; /* center of primary thing being dragged */
	bool				m_bBeforeFirstMotion;
	UT_sint32           m_draggingCell;
	bool				m_bGuide;	/* true ==> guide line XORed onscreen */
	UT_sint32			m_yGuide;	/* valid iff m_bGuide */

	bool				m_bValidMouseClick;
	bool				m_bEventIgnored;
	UT_Rect             m_draggingRect;
	UT_sint32           m_minPageLength;
	PT_DocPosition       m_draggingDocPos;
	bool                m_bIsHidden;
#if XAP_DONTUSE_XOR
	UT_Rect				m_guideCacheRect;
	GR_Image*			m_guideCache;
#endif
};

#endif /* AP_LEFTRULER_H */
