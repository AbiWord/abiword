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

class XAP_Frame;
class AV_ScrollObj;
class GR_Graphics;

/*****************************************************************/

class AP_LeftRuler : public AV_Listener
{
public:
	AP_LeftRuler(XAP_Frame * pFrame);
	virtual ~AP_LeftRuler(void);

	virtual void		setView(AV_View * pView);
	void				setView(AV_View* pView, UT_uint32 iZoom);
	void				setOffsetPageViewTopMargin(UT_uint32 iPageViewLeftMargin);
	void				setHeight(UT_uint32 iHeight);
	UT_uint32			getHeight(void) const;
	void				setWidth(UT_uint32 iWidth);
	UT_uint32			getWidth(void) const;
	void				draw(const UT_Rect * pClipRect);
	void				scrollRuler(UT_sint32 yoff, UT_sint32 ylimit);

	/* used with AV_Listener */
	virtual UT_Bool		notify(AV_View * pView, const AV_ChangeMask mask);

	/* used with AV_ScrollObj */
	static void			_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void			_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
	
protected:

//	void				_draw3DFrame(const UT_Rect * pClipRect, AP_TopRulerInfo * pInfo,
//									 UT_sint32 x, UT_sint32 h);
	
	XAP_Frame *			m_pFrame;
	AV_View *			m_pView;
	AV_ScrollObj *		m_pScrollObj;
	GR_Graphics *		m_pG;
	UT_Dimension		m_dim;
	UT_uint32			m_iHeight;		/* size of window */
	UT_uint32			m_iWidth;		/* size of window */
	UT_uint32			m_iPageViewTopMargin;
	UT_sint32			m_yScrollOffset;
	UT_sint32			m_yScrollLimit;

	/* static const*/ UT_uint32	s_iFixedWidth  /* =32 */;	/* width we draw stuff regardless of window width */

	// a collection of standard colors for drawing

	UT_RGBColor			m_clrWhite;				/* constant used for highlights */
	UT_RGBColor			m_clrBlack;				/* constant used for ticks/text, shadows */
	UT_RGBColor			m_clrDarkGray;			/* constant used for default tab stops, shadows */
	UT_RGBColor			m_clrLiteGray;
	
	UT_RGBColor			m_clrBackground;		/* used for background flood fill */

	UT_RGBColor			m_clrMarginArea;		/* used for flood fill of ruler area where margins are */
	UT_RGBColor			m_clrDocumentArea;		/* used for flood fill of ruler where document is */
};

#endif /* AP_LEFTRULER_H */
