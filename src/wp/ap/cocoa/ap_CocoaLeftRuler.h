/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2021 Hubert Figuière
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

// Class for dealing with the horizontal ruler at the left of
// a document window.

/*****************************************************************/

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ap_LeftRuler.h"

class XAP_Frame;
@class AP_CocoaLeftRulerDelegate;

/*****************************************************************/

class AP_CocoaLeftRuler : public AP_LeftRuler
{
public:
	AP_CocoaLeftRuler(XAP_Frame * pFrame);
	virtual ~AP_CocoaLeftRuler(void);

	XAP_CocoaNSView *		createWidget(void);
	virtual void	setView(AV_View * pView);

	// cheats for the callbacks
	void				getWidgetPosition(int * x, int * y);
	XAP_CocoaNSView * 		getWidget(void) { return m_wLeftRuler; };

//	void _ruler_style_changed (void);
protected:
#if 0
	virtual void		_drawMarginProperties(const UT_Rect * pClipRect,
											  AP_LeftRulerInfo * pInfo,
											  GR_Graphics::GR_Color3D clr);
	virtual void		_drawCellMark(UT_Rect *prDrag, bool bUp);
#endif
private:
	XAP_CocoaNSView *		m_wLeftRuler;
	AP_CocoaLeftRulerDelegate* m_delegate;
};
