/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#ifndef EV_COCOATOOLBAR_H
#define EV_COCOATOOLBAR_H

#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Toolbar.h"
#include "xav_Listener.h"
#include "ap_Toolbar_Id.h"
#include "ap_CocoaToolbar_StyleCombo.h"

#import <Cocoa/Cocoa.h>

class XAP_CocoaApp;
class AP_CocoaFrame;
class AP_CocoaToolbar_Icons;
class EV_CocoaToolbar_ViewListener;
class EV_Toolbar_Label;

@interface EV_CocoaToolbarTarget : NSObject
{
}
- (id)toolbarSelected:(id)sender;
@end

class _wd;

class EV_CocoaToolbar : public EV_Toolbar
{
public:
	EV_CocoaToolbar(XAP_CocoaApp * pCocoaApp, AP_CocoaFrame * pCocoaFrame,
				   const char * szToolbarLayoutName,
				   const char * szToolbarLabelSetName);
	
	virtual ~EV_CocoaToolbar(void);

	bool toolbarEvent(_wd * wd, UT_UCS2Char * pData, UT_uint32 dataLength);
	virtual bool synthesize(void);
	bool bindListenerToView(AV_View * pView);
	virtual bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);
	virtual bool repopulateStyles(void);
	UT_sint32 destroy(void);
	void      rebuildToolbar(UT_sint32 oldpos);
	XAP_CocoaApp *	getApp(void);
	AP_CocoaFrame * getFrame(void);
//	void setCurrentEvent(GdkEvent * event) {m_eEvent = event;}
	virtual void show(void);
	virtual void hide(void);
	
protected:
	void							_releaseListener(void);
	
	XAP_CocoaApp *					m_pCocoaApp;
	AP_CocoaFrame *					m_pCocoaFrame;
	EV_CocoaToolbar_ViewListener *	m_pViewListener;
	AV_ListenerId					m_lid;	/* view listener id */

//	GdkEvent *                      m_eEvent;
	NSView *						m_wToolbar;
	NSView *						m_superView;
	AP_CocoaToolbar_Icons *			m_pCocoaToolbarIcons;
	UT_Vector						m_vecToolbarWidgets;
private:
	NSButton * _makeToolbarButton (int type, EV_Toolbar_Label * pLabel, _wd * wd, NSView *parent,
												float & btnX);
	EV_CocoaToolbarTarget * 		m_target;
	bool							m_hidden;
};

#endif /* EV_COCOATOOLBAR_H */









