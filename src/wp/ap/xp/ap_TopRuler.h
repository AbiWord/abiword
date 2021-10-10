/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001 Tomas Frydrych
 * Copyright (C) 2004-2021 Hubert Figuière
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

#pragma once

#include "xap_Features.h"
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
#include "xap_CustomWidget.h"

class XAP_App;
class XAP_Frame;
class XAP_Prefs;
class AV_ScrollObj;
class UT_Timer;
class fp_CellContainer;
class  AP_TopRulerTableInfo;
/*****************************************************************/



/*****************************************************************/
class ABI_EXPORT AP_TopRulerTableInfo
{
public:
	UT_sint32 m_iLeftCellPos;
	UT_sint32 m_iLeftSpacing;
	UT_sint32 m_iRightCellPos;
	UT_sint32 m_iRightSpacing;
	fp_CellContainer * m_pCell;
};

/*****************************************************************/

/*****************************************************************/

class ABI_EXPORT AP_TopRulerInfo
{
public:
	typedef enum _mode { TRI_MODE_COLUMNS, TRI_MODE_TABLE, TRI_MODE_FRAME } Mode;

	AP_TopRulerInfo(void) :
							m_mode(TRI_MODE_COLUMNS),
							m_xPaperSize(0),
							m_xPageViewMargin(0),
							m_xrPoint(0),
							m_xrLeftIndent(0),
							m_xrFirstLineIndent(0),
							m_xrRightIndent(0),
							m_xrTabStop(0),
							m_pfnEnumTabStops(NULL),
							m_pVoidEnumTabStopsData(NULL),
							m_iTabStops(0),
							m_iDefaultTabInterval(0),
							m_pszTabStops(NULL),
							m_iCurrentColumn(0),
							m_iNumColumns(0),
							m_vecTableColInfo (NULL),
							m_vecFullTable(NULL),
							m_iTablePadding(0),
							m_iCells(0),
							m_iCurCell(0)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Creating AP_TopRulerInfo %x \n",this));

			u.c.m_xaLeftMargin  = 0;
			u.c.m_xaRightMargin = 0;
			u.c.m_xColumnGap    = 0;
			u.c.m_xColumnWidth  = 0;
		}
	virtual ~AP_TopRulerInfo(void)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Deleting AP_TopRulerInfo %x \n",this));
			if(m_vecTableColInfo)
			{
				UT_sint32 count = m_vecTableColInfo->getItemCount();
				UT_sint32 i =0;
				for(i=0; i< count; i++)
				{
					delete m_vecTableColInfo->getNthItem(i);
				}
				delete m_vecTableColInfo;
			}
			if(m_vecFullTable)
			{
				UT_sint32 count = m_vecFullTable->getItemCount();
				UT_sint32 i =0;
				for(i=0; i< count; i++)
				{
					delete m_vecFullTable->getNthItem(i);
				}
				delete m_vecFullTable;
				m_vecFullTable = NULL;
			}
		}

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

// Column information for current table

	UT_GenericVector<AP_TopRulerTableInfo *> * m_vecTableColInfo;
	UT_GenericVector<AP_TopRulerTableInfo *> * m_vecFullTable;
	UT_sint32               m_iTablePadding;
	UT_sint32               m_iCells;
	UT_sint32               m_iCurCell;
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

class ABI_EXPORT AP_TopRuler : public AP_Ruler, public AV_Listener
{
public:
	AP_TopRuler(XAP_Frame * pFrame);
	virtual ~AP_TopRuler(void);

	virtual void	setView(AV_View * pView);
    void	        setViewHidden(AV_View * pView);
	void			setView(AV_View* pView, UT_uint32 iZoom);
	AV_View *       getView(void) const { return m_pView;}
	void			setOffsetLeftRuler(UT_uint32 iLeftRulerWidth);
	void            setZoom(UT_uint32 iZoom);
	virtual void	setHeight(UT_uint32 iHeight) override;
	UT_uint32		getHeight(void) const;
	virtual void	setWidth(UT_uint32 iWidth) override;
	UT_uint32		getWidth(void) const;
	virtual GR_Graphics*	getGraphics(void) const override { return m_pG; }
	virtual XAP_Frame* getFrame() const override {  return m_pFrame; }
	bool            isHidden(void) const
		{ return m_bIsHidden;}
	void			scrollRuler(UT_sint32 xoff, UT_sint32 xlimit);

	UT_sint32       setTableLineDrag(PT_DocPosition pos, UT_sint32 x, UT_sint32 & iFixed);
	virtual void	mouseMotion(EV_EditModifierState ems, UT_sint32 x, UT_sint32 y) override;
	virtual void	mousePress(EV_EditModifierState ems, EV_EditMouseButton emb, UT_uint32 x, UT_uint32 y) override;
	virtual void	mouseRelease(EV_EditModifierState ems, EV_EditMouseButton emb, UT_sint32 x, UT_sint32 y) override;

	bool            isMouseOverTab(UT_uint32 x, UT_uint32 y);
	/* used with AV_Listener */
	virtual bool notify(AV_View * pView, const AV_ChangeMask mask) override;
	virtual AV_ListenerType getType(void) override { return AV_LISTENER_TOPRULER;}

	/* used with AV_ScrollObj */
	static void		_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void		_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);

	/* for use with the prefs listener top_ruler_prefs_listener */
	UT_Dimension	getDimension() const { return m_dim; }
	void			setDimension( UT_Dimension newdim );

	UT_uint32       getTabToggleAreaWidth() const;

	static UT_uint32 getFixedWidth(){return s_iFixedWidth;}

protected:
	/* implement XAP_CustomWidgetLU::drawImmediateLU */
	virtual void drawImmediateLU(const UT_Rect *clip) override;

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

	UT_sint32		_findTabStop(AP_TopRulerInfo * pInfo, UT_uint32 x, UT_uint32 y, UT_sint32 & anchor, eTabType & iType, eTabLeader & iLeader);
	const char *	_getTabStopString(AP_TopRulerInfo * pInfo, UT_sint32 k);
	void			_getTabZoneRect(AP_TopRulerInfo * pInfo, UT_Rect &rZone);
	void			_setTabStops(ap_RulerTicks tick, UT_sint32 iTab, eTabLeader iLeader, bool bDelete);

	UT_sint32	_getColumnMarkerXRightEnd(AP_TopRulerInfo * pInfo, UT_uint32 kCol);
	void		_getColumnMarkerRect(AP_TopRulerInfo * pInfo, UT_uint32 kCol, UT_sint32 xCenter,
									 UT_Rect * prCol);
	void		_drawColumnProperties(const UT_Rect * pClipRect,
									  AP_TopRulerInfo * pInfo,
									  UT_uint32 kCol);
	void		_getCellMarkerRect(AP_TopRulerInfo * pInfo, UT_sint32 kCell,
								   UT_Rect * prCell);
	void		_drawCellProperties(const UT_Rect * pClipRect,
									  AP_TopRulerInfo * pInfo,
									  UT_uint32 kCol, bool bDrawAll);
	void		_drawCellProperties(const UT_Rect * pClipRect,
									AP_TopRulerInfo * pInfo, bool bDrawAll);
	void        _drawCellGap( AP_TopRulerInfo * pInfo, UT_sint32 iCell);

	virtual void	_drawCellMark(UT_Rect * prDrag, bool bUp);

	void		_getMarginMarkerRects(AP_TopRulerInfo * pInfo, UT_Rect &rLeft, UT_Rect &rRight);

	virtual void	_drawMarginProperties(const UT_Rect * pClipRect,
									  AP_TopRulerInfo * pInfo, GR_Graphics::GR_Color3D clr);

	void		_xorGuide(bool bClear=false);

	void		_ignoreEvent(bool bDone);
	double		_scalePixelDistanceToUnits(UT_sint32 xColRel, ap_RulerTicks & tick);
	double		_getUnitsFromRulerLeft(UT_sint32 xColRel, ap_RulerTicks & tick);
	UT_sint32	_getFirstPixelInColumn(AP_TopRulerInfo * pInfo, UT_uint32 kCol);
	UT_sint32	_snapPixelToGrid(UT_sint32 xDist, ap_RulerTicks & tick);

	virtual void	_drawLeftIndentMarker(UT_Rect & r, bool bFilled);
	virtual void	_drawRightIndentMarker(UT_Rect & r, bool bFilled);
	virtual void	_drawFirstLineIndentMarker(UT_Rect & r, bool bFilled);

	void		_drawTabStop(UT_Rect & r, eTabType iType, bool bFilled);

	virtual void	_drawColumnGapMarker(UT_Rect & r);

	bool		_isInBottomBoxOfLeftIndent(UT_uint32 y);
	void		_displayStatusMessage(XAP_String_Id messageID, const ap_RulerTicks &tick, double dValue);
	void		_displayStatusMessage(XAP_String_Id messageID, const ap_RulerTicks &tick, double dValue1, double dValue2);
	void		_displayStatusMessage(XAP_String_Id FormatMessageID, UT_sint32 iCol, const char * format);
	void		_displayStatusMessage(XAP_String_Id FormatMessageID);

	virtual void _refreshView(void) override;

	// must be static so that I can pass as a functional arg - shack
	static void _prefsListener( XAP_Prefs *pPrefs, const XAP_PrefsChangeSet *phChanges, void *data );

	// autoscroll stuff
	static void			_autoScroll(UT_Worker * pTimer);

	XAP_Frame *			m_pFrame;
	GR_Graphics *		m_pG;
	UT_uint32			m_iLeftRulerWidth; // device
	UT_sint32			m_xScrollOffset;
	UT_sint32			m_xScrollLimit;

	static UT_uint32	s_iFixedHeight /* =32 */;	/* size we draw stuff w/o regard to window size: device */
	static UT_uint32	s_iFixedWidth  /* =32 */;	/* minimum width of non-scrolling area on left: device */

private:
	AV_ScrollObj *		m_pScrollObj;
protected:
	AV_View *			m_pView;
private:
	UT_Dimension		m_dim;

	UT_uint32			m_iHeight;		/* size of window: device */
	UT_uint32			m_iWidth;		/* size of window: device */

	UT_Timer *			m_pAutoScrollTimer;
	char				m_aScrollDirection; // 'L' == left   'R' == right

	UT_sint32			m_minColumnWidth; // logical

	UT_sint32			m_iMinCellPos;	/* cell marker left border while dragging: logical */
	UT_sint32			m_iMaxCellPos;  /* cell marker right border while dragging: logical */

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
								 DW_TABSTOP,
								 DW_TABTOGGLE,
								 DW_CELLMARK
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
	UT_sint32			m_oldX; /* Only for dragging; used to see if object has moved */

	eTabType			m_iDefaultTabType;
	UT_sint32           m_draggingCell; // index of cell being dragged
	bool				m_bGuide;	/* true ==> guide line XORed onscreen */
	UT_sint32			m_xGuide;	/* valid iff m_bGuide */
	UT_sint32			m_xOtherGuide;

	AV_ListenerId		m_lidTopRuler;		/* need to save the view/listenerID so we can removeListener in destructor */
	UT_sint32           m_iCellContainerLeftPos; // position of the left side of the container
                                                 // holding the cell
	bool                m_bIsHidden;
	UT_sint32           m_iOrigPosition;
#if XAP_DONTUSE_XOR
	UT_Rect				m_guideCacheRect;
	UT_Rect				m_otherGuideCacheRect;
	GR_Image*			m_guideCache;
	GR_Image*			m_otherGuideCache;
#endif
};
