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

class XAP_Frame;
class AV_ScrollObj;
class DG_Graphics;

/*****************************************************************/

class AP_TopRuler : public AV_Listener
{
public:
	AP_TopRuler(XAP_Frame * pFrame);
	virtual ~AP_TopRuler(void);

	virtual void		setView(AV_View * pView);
	void				setOffsetLeftRuler(UT_uint32 iLeftRulerWidth);
	void				setOffsetPageViewLeftMargin(UT_uint32 iPageViewLeftMargin);
	void				setHeight(UT_uint32 iHeight);
	UT_uint32			getHeight(void) const;
	void				setWidth(UT_uint32 iWidth);
	UT_uint32			getWidth(void) const;
	void				draw(const UT_Rect * pClipRect);
	void				scrollRuler(UT_sint32 xoff);

	/* used with AV_Listener */
	virtual UT_Bool		notify(AV_View * pView, const AV_ChangeMask mask);

	/* used with AV_ScrollObj */
	static void			_scrollFuncX(void * pData, UT_sint32 xoff);
	static void			_scrollFuncY(void * pData, UT_sint32 yoff);
	
protected:
	XAP_Frame *			m_pFrame;
	AV_View *			m_pView;
	AV_ScrollObj *		m_pScrollObj;
	DG_Graphics *		m_pG;
	UT_uint32			m_iHeight;		/* size of window */
	UT_uint32			m_iWidth;		/* size of window */
	UT_uint32			m_iLeftRulerWidth;
	UT_uint32			m_iPageViewLeftMargin;
	UT_sint32			m_xScrollOffset;

	/* static const*/ UT_uint32	s_iFixedHeight /* =32 */;	/* size we draw stuff w/o regard to window size */
	/* static const*/ UT_uint32	s_iFixedWidth  /* =32 */;	/* minimum width of non-scrolling area on left */
};

#endif /* AP_TOPRULER_H */
