/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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

#ifndef EV_COCOATOOLBAR_H
#define EV_COCOATOOLBAR_H

#include "ut_types.h"
#include "xap_Types.h"
#include "ev_Toolbar.h"
#include "xav_Listener.h"
#include "ap_Toolbar_Id.h"
#include "ap_CocoaToolbar_StyleCombo.h"

#import <Cocoa/Cocoa.h>

class AP_CocoaFrame;
class XAP_CocoaToolbar_Icons;
class EV_CocoaToolbar_ViewListener;
class EV_Toolbar_Label;

class EV_CocoaToolbar;
@class EV_CocoaToolbarTarget;


class EV_CocoaToolbar : public EV_Toolbar
{
public:
	EV_CocoaToolbar(AP_CocoaFrame * pCocoaFrame,
				   const char * szToolbarLayoutName,
				   const char * szToolbarLabelSetName);

	virtual ~EV_CocoaToolbar(void);

	bool toolbarEvent(XAP_Toolbar_Id tlbrid, const UT_UCSChar * pData, UT_uint32 dataLength);
	virtual bool synthesize(void);
	bool bindListenerToView(AV_View * pView);
	virtual bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);
	virtual bool repopulateStyles(void);
	UT_sint32 destroy(void);
	void      rebuildToolbar(UT_sint32 oldpos);
	AP_CocoaFrame * getFrame(void);
	virtual void show(void);
	virtual void hide(void);

	static	float	getButtonWidth (void)
					{ return 28.0f; };
	static	float	getButtonHeight (void)
					{ return 28.0f; };

	static float  getToolbarHeight(void)
					{ return getButtonHeight(); };

	NSView*	_getToolbarView(void) const
				{ return m_wToolbar; };
protected:
	void							_releaseListener(void);

	AP_CocoaFrame *					m_pCocoaFrame;
	EV_CocoaToolbar_ViewListener *	m_pViewListener;
	AV_ListenerId					m_lid;	/* view listener id */

	NSView *						m_wToolbar;
	NSView *						m_superView;
	XAP_CocoaToolbar_Icons *		m_pCocoaToolbarIcons;
private:
	NSButton * _makeToolbarButton (int type, EV_Toolbar_Label * pLabel,
												XAP_Toolbar_Id tlbrid, NSView *parent,
												float & btnX);
	EV_CocoaToolbarTarget * 		m_target;
};

#endif /* EV_COCOATOOLBAR_H */









