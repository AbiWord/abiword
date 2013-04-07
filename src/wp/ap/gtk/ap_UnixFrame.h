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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_UNIXFRAME_H
#define AP_UNIXFRAME_H

class GR_Graphics;

#include "xap_Frame.h"
#include "ap_Frame.h"
#include "ie_types.h"

class XAP_UnixApp;
class AP_UnixFrame;

/*****************************************************************/

class AP_UnixFrame : public AP_Frame
{
public:
	AP_UnixFrame();
	AP_UnixFrame(AP_UnixFrame * f);
	virtual ~AP_UnixFrame(void);

	virtual	XAP_Frame *			cloneFrame(void);
	virtual bool				initialize(XAP_FrameMode frameMode=XAP_NormalFrame);

	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y);
	virtual void				setStatusMessage(const char * szMsg);

	virtual void				toggleRuler(bool bRulerOn);
	virtual void                            toggleTopRuler(bool bRulerOn);
	virtual void                            toggleLeftRuler(bool bRulerOn);
	virtual void				toggleBar(UT_uint32 iBarNb, bool bBarOn);
	virtual void				toggleStatusBar(bool bStatusBarOn);
	virtual UT_sint32 getDocumentAreaXoff();
	virtual UT_sint32 getDocumentAreaYoff();

protected:
	friend class AP_UnixFrameImpl;

	// implementation of helper methods for AP_Frame::_showDocument
	virtual bool _createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom);
	virtual void _bindToolbars(AV_View *pView);
	virtual void _setViewFocus(AV_View *pView);
	virtual bool _createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj,
					       ap_ViewListener *& pViewListener, ap_Scrollbar_ViewListener *& pScrollbarViewListener,
					       AV_ListenerId &lid, AV_ListenerId &lidScrollbarViewListener);
	virtual UT_sint32 _getDocumentAreaWidth();
	virtual UT_sint32 _getDocumentAreaHeight();

	// scrolling function
	static void _scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void _scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);

};

#endif /* AP_UNIXFRAME_H */

