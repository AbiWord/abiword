/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2012 Hubert Figuiere
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

#ifndef _AP_QT_FRAME_H_
#define _AP_QT_FRAME_H_

#include "ap_Frame.h"

class AP_QtFrame : public AP_Frame
{
public:
	AP_QtFrame();
	AP_QtFrame(AP_QtFrame * f);
	virtual ~AP_QtFrame();

	virtual	XAP_Frame *			cloneFrame(void);
	virtual bool				initialize(XAP_FrameMode frameMode = XAP_NormalFrame);

	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);

	virtual void				setStatusMessage(const char * szMsg);

	virtual void                            toggleTopRuler(bool bRulerOn);
	virtual void                            toggleLeftRuler(bool bRulerOn);

protected:
	virtual bool _createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom);
	virtual void _bindToolbars(AV_View *pView);
	virtual void _setViewFocus(AV_View *pView);
	virtual bool _createScrollBarListeners(AV_View * pView,
										   AV_ScrollObj *& pScrollObj,
										   ap_ViewListener *& pViewListener,
										   ap_Scrollbar_ViewListener *& pScrollbarViewListener,
										   AV_ListenerId &lid,
										   AV_ListenerId &lidScrollbarViewListener);

	virtual UT_sint32 _getDocumentAreaWidth();
	virtual UT_sint32 _getDocumentAreaHeight();
};

#endif

