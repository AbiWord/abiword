/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
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


#import <Cocoa/Cocoa.h>

#import "ut_types.h"
#import "ut_debugmsg.h"
#import "ut_assert.h"
#import "gr_CocoaCairoGraphics.h"
#import "ev_CocoaToolbar.h"
#import "ev_CocoaMouse.h"

#import "xav_View.h"
#import "xap_CocoaCompat.h"
#import "xap_CocoaApp.h"
#import "xap_CocoaTextView.h"
#import "xap_CocoaToolbarWindow.h"

#import "ap_FrameData.h"
#import "ap_CocoaFrame.h"
#import "ap_CocoaFrameImpl.h"
#import "ap_CocoaTopRuler.h"
#import "ap_CocoaLeftRuler.h"
#import "ap_CocoaStatusBar.h"

/*!
	custom wrapper for NSScrollers.
 */
@interface XAP_NSScroller : NSScroller
{
}
-(id)initWithFrame:(NSRect)frame andController:(NSWindowController*)controller vertical:(BOOL)vertical;

@end

/*****************************************************************/
AP_CocoaFrameImpl::AP_CocoaFrameImpl(AP_CocoaFrame *pCocoaFrame)
	: XAP_CocoaFrameImpl (pCocoaFrame),
		m_hScrollbar(nil),
		m_vScrollbar(nil),
		m_docAreaGRView(nil),
		m_HMinScroll(0),
		m_HMaxScroll(0),
		m_HCurrentScroll(0),
		m_VMinScroll(0),
		m_VMaxScroll(0),
		m_VCurrentScroll(0)
{
}

XAP_FrameImpl * AP_CocoaFrameImpl::createInstance(XAP_Frame *pFrame)
{
	return new AP_CocoaFrameImpl(static_cast<AP_CocoaFrame*>(pFrame));
}

void AP_CocoaFrameImpl::_setHScrollValue(UT_sint32 value)
{
	if (m_HCurrentScroll != value) {
		m_HCurrentScroll = value;
		if (m_HCurrentScroll < m_HMinScroll) {
			m_HCurrentScroll = m_HMinScroll;
		}
		if (m_HCurrentScroll > m_HMaxScroll) {
			m_HCurrentScroll = m_HMaxScroll;
		}
		UT_DEBUGMSG(("_setHScrollValue : %d\n", value));
		_setHScrollbarValues();
	}
}

void AP_CocoaFrameImpl::_setHScrollMin(UT_sint32 value)
{
	if (m_HMinScroll != value) {
		m_HMinScroll = value;
		if (m_HCurrentScroll < m_HMinScroll) {
			m_HCurrentScroll = m_HMinScroll;
		}
		UT_DEBUGMSG(("_setHScrollMin : %d\n", value));
		_setHScrollbarValues();
	}
}

void AP_CocoaFrameImpl::_setHScrollMax(UT_sint32 value)
{
	if (m_HMaxScroll != value) {
		m_HMaxScroll = value;
		if (m_HCurrentScroll > m_HMaxScroll) {
			m_HCurrentScroll = m_HMaxScroll;
		}
		UT_DEBUGMSG(("_setHScrollMax : %d\n", value));
		_setHScrollbarValues();
	}
}

void AP_CocoaFrameImpl::_setHVisible(UT_sint32 value)
{
	if (m_HVisible != value) {
		m_HVisible = value;
		_setHScrollbarValues();
	}
}

void AP_CocoaFrameImpl::_setVScrollValue(UT_sint32 value)
{
	if (m_VCurrentScroll != value) {
		m_VCurrentScroll = value;
		if (m_VCurrentScroll < m_VMinScroll) {
			m_VCurrentScroll = m_VMinScroll;
		}
		if (m_VCurrentScroll > m_VMaxScroll) {
			m_VCurrentScroll = m_VMaxScroll;
		}
		UT_DEBUGMSG(("_setVScrollValue : %d\n", value));
		_setVScrollbarValues();
	}
}

void AP_CocoaFrameImpl::_setVScrollMin(UT_sint32 value)
{
	if (m_VMinScroll != value) {
		m_VMinScroll = value;
		if (m_VCurrentScroll < m_VMinScroll) {
			m_VCurrentScroll = m_VMinScroll;
		}
		UT_DEBUGMSG(("_setVScrollMin : %d\n", value));
		_setVScrollbarValues();
	}
}

void AP_CocoaFrameImpl::_setVScrollMax(UT_sint32 value)
{
	if (m_VMaxScroll != value) {
		m_VMaxScroll = value;
		if (m_VCurrentScroll > m_VMaxScroll) {
			m_VCurrentScroll = m_VMaxScroll;
		}
		UT_DEBUGMSG(("_setVScrollMax : %d\n", value));
		_setVScrollbarValues();
	}
}

void AP_CocoaFrameImpl::_setVVisible(UT_sint32 value)
{
	if (m_VVisible != value) {
		m_VVisible = value;
		_setVScrollbarValues();
	}
}

void AP_CocoaFrameImpl::_setHScrollbarValues()
{
	float value;
	CGFloat knob;
	if (m_HMaxScroll == 0) {
		value = 0.0;
	}
	else {
		value = ((float)m_HCurrentScroll / (float)m_HMaxScroll);
	}
	if (m_HMaxScroll == 0) {
		knob = 1.0;
	}
	else {
		knob = ((CGFloat)m_HVisible / (CGFloat)(m_HVisible + m_HMaxScroll));
	}
	UT_DEBUGMSG(("_setHScrollbarValues(), max = %d, current = %d, visible = %d\n", m_HMaxScroll, m_HCurrentScroll, m_HVisible));
	UT_DEBUGMSG(("_setHScrollbarValues(), value = %f, knob = %f\n", value, knob));
	if (knob >= 1.0) {
		[m_hScrollbar setEnabled:NO];
	}
	else {
		[m_hScrollbar setEnabled:YES];
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1050
		[m_hScrollbar setKnobProportion:knob];
		[m_hScrollbar setDoubleValue:value];
#else
		[m_hScrollbar setFloatValue:value knobProportion:knob];
#endif
	}
	[m_hScrollbar setNeedsDisplay:YES];
	[[(AP_CocoaFrameController*)_getController() getHRuler] setNeedsDisplay:YES];
}


void AP_CocoaFrameImpl::_setVScrollbarValues()
{
	float value;
	CGFloat knob;
	if (m_VMaxScroll == 0) {
		value = 0.0;
	}
	else {
		value = ((float)m_VCurrentScroll / (float)m_VMaxScroll);
	}
	if (m_VMaxScroll == 0) {
		knob = 1.0;
	}
	else {
		knob = ((CGFloat)m_VVisible / (CGFloat)(m_VVisible + m_VMaxScroll));
	}
	UT_DEBUGMSG(("_setVScrollbarValues(), max = %d, current = %d, visible = %d\n", m_VMaxScroll, m_VCurrentScroll, m_VVisible));
	UT_DEBUGMSG(("_setVScrollbarValues(), value = %f, knob = %f\n", value, knob));
	if (knob >= 1.0) {
		[m_vScrollbar setEnabled:NO];
	}
	else {
		[m_vScrollbar setEnabled:YES];
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1050
		[m_hScrollbar setKnobProportion:knob];
		[m_hScrollbar setDoubleValue:value];
#else
		[m_hScrollbar setFloatValue:value knobProportion:knob];
#endif
	}
	[m_vScrollbar setNeedsDisplay:YES];
	// [[(_getController()) getVRuler] setNeedsDisplay:YES]; // ??
}


void AP_CocoaFrameImpl::_scrollAction(id sender)
{
	float newValue = [sender floatValue];
	NSScrollerPart part = [sender hitPart];
	AV_View * pView = getFrame()->getCurrentView();
	GR_Graphics * pGr = pView->getGraphics();

	UT_DEBUGMSG(("newValue = %f\n", newValue));
	switch (part) {
	case NSScrollerNoPart:
		UT_DEBUGMSG(("No Scroll\n"));
		return;

    case NSScrollerDecrementPage:
		UT_DEBUGMSG(("NSScrollerDecrementPage\n"));
		if (sender == m_vScrollbar) {
			_setVScrollValue(m_VCurrentScroll - m_VVisible);
		}
		else {
			_setHScrollValue(m_HCurrentScroll - m_HVisible);
		}
		break;
    case NSScrollerIncrementPage:
		UT_DEBUGMSG(("NSScrollerIncrementPage\n"));
		if (sender == m_vScrollbar) {
			_setVScrollValue(m_VCurrentScroll + m_VVisible);
		}
		else {
			_setHScrollValue(m_HCurrentScroll + m_HVisible);
		}
		break;
    case NSScrollerDecrementLine:
		UT_DEBUGMSG(("NSScrollerDecrementLine\n"));
		if (sender == m_vScrollbar) {
			_setVScrollValue(m_VCurrentScroll - pGr->tlu(20));
		}
		else {
			_setHScrollValue(m_HCurrentScroll - pGr->tlu(20));
		}
		break;
    case NSScrollerIncrementLine:
		UT_DEBUGMSG(("NSScrollerIncrementLine\n"));
		if (sender == m_vScrollbar) {
			_setVScrollValue(m_VCurrentScroll + pGr->tlu(20));
		}
		else {
			_setHScrollValue(m_HCurrentScroll + pGr->tlu(20));
		}
		break;
    case NSScrollerKnob	:
    case NSScrollerKnobSlot	:
		UT_DEBUGMSG(("NSScrollerKnob\n"));
		if (sender == m_vScrollbar) {
			m_VCurrentScroll = lrintf(newValue * m_VMaxScroll);
			_setVScrollbarValues();
		}
		else {
			m_HCurrentScroll = lrintf(newValue * m_HMaxScroll);
			_setHScrollbarValues();
		}
		break;
	default:
		break;
	}
	if (sender == m_vScrollbar) {
		pView->sendVerticalScrollEvent(m_VCurrentScroll);
	}
	else {
		pView->sendHorizontalScrollEvent(m_HCurrentScroll);	
	}
}


void AP_CocoaFrameImpl::_showTopRulerNSView(void)
{
	XAP_CocoaFrameController* ctrl = _getController();
	UT_ASSERT([ctrl isKindOfClass:[XAP_CocoaFrameController class]]);
	AP_CocoaFrameController* apCtrl = (AP_CocoaFrameController*)ctrl;
	XAP_CocoaNSView* ruler = [apCtrl getHRuler];
	XAP_CocoaNSView* vRuler = [apCtrl getVRuler];
	NSView* mainView = [ctrl getMainView];
	
	UT_ASSERT([ruler superview] == nil);
	// make sure it is not visible
	if ([ruler superview]) {
		NSLog(@"AP_CocoaFrameImpl::_showTopRulerNSView attempted to show already visible ruler");
		return;
	}
	
	UT_ASSERT(mainView);
	NSRect mainFrame = [mainView frame];
	NSRect rulerFrame = [ruler frame];
	mainFrame.size.height -= rulerFrame.size.height;
	[mainView setFrame:mainFrame];
	rulerFrame.origin.y = mainFrame.origin.y + mainFrame.size.height;
	rulerFrame.size.width = mainFrame.size.width + mainFrame.origin.x;
	[ruler setFrame:rulerFrame];
	if ([vRuler superview]) {
		NSRect vRulerFrame = [vRuler frame];
		vRulerFrame.size.height = mainFrame.size.height;
		[vRuler setFrame:vRulerFrame];
	}
	[[mainView superview] addSubview:ruler];
	[ruler release];	/* when we hide the ruler we retain the object before the removeFromSuperview */
}


void AP_CocoaFrameImpl::_hideTopRulerNSView(void)
{
	XAP_CocoaFrameController* ctrl = _getController();
	UT_ASSERT([ctrl isKindOfClass:[XAP_CocoaFrameController class]]);
	AP_CocoaFrameController* apCtrl = (AP_CocoaFrameController*)ctrl;
	XAP_CocoaNSView* ruler = [apCtrl getHRuler];
	XAP_CocoaNSView* vRuler = [apCtrl getVRuler];
	NSView* mainView = [ctrl getMainView];
	
	UT_ASSERT([ruler superview]);
	// make sure it is not visible
	if ([ruler superview] == nil) {
		NSLog(@"AP_CocoaFrameImpl::_hideTopRulerNSView attempted to hide already hidden ruler");
		return;
	}
	
	UT_ASSERT(mainView);
	NSRect mainFrame = [mainView frame];
	NSRect rulerFrame = [ruler frame];
	mainFrame.size.height += rulerFrame.size.height;
	[mainView setFrame:mainFrame];
	if ([vRuler superview]) {
		NSRect vRulerFrame = [vRuler frame];
		vRulerFrame.size.height = mainFrame.size.height;
		[vRuler setFrame:vRulerFrame];
	}
//	rulerFrame.origin.y = mainFrame.origin.y + mainFrame.size.height + rulerFrame.size.height;
//	[ruler setFrame:rulerFrame];
	[ruler retain];	
	[ruler removeFromSuperview];
}


void AP_CocoaFrameImpl::_showLeftRulerNSView(void)
{
	XAP_CocoaFrameController* ctrl = _getController();
	UT_ASSERT([ctrl isKindOfClass:[XAP_CocoaFrameController class]]);
	AP_CocoaFrameController* apCtrl = (AP_CocoaFrameController*)ctrl;
	XAP_CocoaNSView* ruler = [apCtrl getVRuler];
	NSView* mainView = [ctrl getMainView];
	
	UT_ASSERT([ruler superview] == nil);
	// make sure it is not visible
	if ([ruler superview]) {
		NSLog(@"AP_CocoaFrameImpl::_showLeftRulerNSView attempted to show already visible ruler");
		return;
	}
	
	UT_ASSERT(mainView);
	NSRect mainFrame = [mainView frame];
	NSRect rulerFrame = [ruler frame];
	mainFrame.size.width -= rulerFrame.size.width;
	mainFrame.origin.x = rulerFrame.size.width;
	[mainView setFrame:mainFrame];
	rulerFrame.origin.x = 0;
	rulerFrame.origin.y = mainFrame.origin.y;
	rulerFrame.size.height = mainFrame.size.height;
	[ruler setFrame:rulerFrame];
	[[mainView superview] addSubview:ruler];
	[ruler release];	/* when we hide the ruler we retain the object before the removeFromSuperview */
}


void AP_CocoaFrameImpl::_hideLeftRulerNSView(void)
{
	XAP_CocoaFrameController* ctrl = _getController();
	UT_ASSERT([ctrl isKindOfClass:[XAP_CocoaFrameController class]]);
	AP_CocoaFrameController* apCtrl = (AP_CocoaFrameController*)ctrl;
	XAP_CocoaNSView* ruler = [apCtrl getVRuler];
	NSView* mainView = [ctrl getMainView];
	
	UT_ASSERT([ruler superview]);
	// make sure it is not visible
	if ([ruler superview] == nil) {
		NSLog(@"AP_CocoaFrameImpl::_hideLeftRulerNSView attempted to hide already hidden ruler");
		return;
	}
	
	UT_ASSERT(mainView);
	NSRect mainFrame = [mainView frame];
	mainFrame.size.width += mainFrame.origin.x;
	mainFrame.origin.x = 0;
	[mainView setFrame:mainFrame];
	[ruler retain];	
	[ruler removeFromSuperview];
}

void AP_CocoaFrameImpl::_createDocView(GR_Graphics* &pG)
{
	XAP_Frame*	pFrame = getFrame();
	NSView*		docArea = [_getController() getMainView];

    NSArray* docAreaSubviews = [[docArea subviews] copy];
    if ([docAreaSubviews count] > 0) {
		[docAreaSubviews makeObjectsPerformSelector:@selector(removeFromSuperviewWithoutNeedingDisplay)];
        m_hScrollbar = NULL;
        m_vScrollbar = NULL;
        m_docAreaGRView = NULL;
    }
	[docAreaSubviews release];

	NSRect frame = [docArea bounds];
	NSRect controlFrame;
	
	/* vertical scrollbar */
	controlFrame.origin.y = [NSScroller scrollerWidth];
	controlFrame.size.width = [NSScroller scrollerWidth];
	controlFrame.size.height = frame.size.height - controlFrame.origin.y;
	controlFrame.origin.x = frame.size.width - controlFrame.size.width;
	m_vScrollbar = [[XAP_NSScroller alloc] initWithFrame:controlFrame andController:_getController()
						vertical:YES];
	[docArea addSubview:m_vScrollbar];
	[m_vScrollbar setEnabled:YES];
	[m_vScrollbar release];
	
	/* horizontal scrollbar */
	controlFrame.origin.x = 0;
	controlFrame.origin.y = 0;
	controlFrame.size.height = [NSScroller scrollerWidth];
	controlFrame.size.width = frame.size.width - controlFrame.size.height;
	m_hScrollbar = [[XAP_NSScroller alloc] initWithFrame:controlFrame andController:_getController()
						vertical:NO];
	[docArea addSubview:m_hScrollbar];
	[m_hScrollbar setEnabled:YES];
	[m_hScrollbar release];

	/* doc view */
	controlFrame.origin.x = 0;
	controlFrame.origin.y = [NSScroller scrollerWidth];
	controlFrame.size.height = frame.size.height - controlFrame.origin.y;
	controlFrame.size.width = frame.size.width - [NSScroller scrollerWidth];
	m_docAreaGRView = [[XAP_CocoaTextView alloc] initWith:pFrame andFrame:controlFrame];
	[docArea addSubview:m_docAreaGRView];
	[m_docAreaGRView setAutoresizingMask:(NSViewHeightSizable | NSViewWidthSizable)];
	[m_docAreaGRView setEventDelegate:[[[AP_DocViewDelegate alloc] init] autorelease]];
	[_getController() setTextView:m_docAreaGRView];
	[[_getController() window] makeFirstResponder:m_docAreaGRView];
	[m_docAreaGRView release];
	
	GR_CocoaCairoAllocInfo ai(m_docAreaGRView);
	pG = (GR_CocoaCairoGraphics*)XAP_App::getApp()->newGraphics(ai);

	static_cast<GR_CocoaCairoGraphics *>(pG)->_setUpdateCallback (&_graphicsUpdateCB, (void *)pFrame);
}


void AP_CocoaFrameImpl::_bindToolbars(AV_View * pView)
{
	int nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (int k = 0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.
		
		EV_CocoaToolbar * pCocoaToolbar = (EV_CocoaToolbar *)m_vecToolbars.getNthItem(k);
		pCocoaToolbar->bindListenerToView(pView);
	}
}

// Does the initial show/hide of statusbar (based on the user prefs).
// Idem.
void AP_CocoaFrameImpl::_showOrHideStatusbar()
{
	XAP_Frame*	pFrame = getFrame();
	bool bShowStatusBar = static_cast<AP_FrameData*> (pFrame->getFrameData())->m_bShowStatusBar;
	static_cast<AP_CocoaFrame *>(pFrame)->toggleStatusBar(bShowStatusBar);
}

// Does the initial show/hide of toolbars (based on the user prefs).
// This is needed because toggleBar is called only when the user
// (un)checks the show {Stantandard,Format,Extra} toolbar checkbox,
// and thus we have to manually call this function at startup.
void AP_CocoaFrameImpl::_showOrHideToolbars()
{
	XAP_Frame*	pFrame = getFrame();
	bool *bShowBar = static_cast<AP_FrameData*> (pFrame->getFrameData())->m_bShowBar;
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	XAP_CocoaToolbarWindow_Controller* tlbr = [XAP_CocoaToolbarWindow_Controller sharedToolbar];
	[tlbr lock];
	for (UT_uint32 i = 0; i < cnt; i++)
	{
		// TODO: The two next lines are here to bind the EV_Toolbar to the
		// AP_FrameData, but their correct place are next to the toolbar creation (JCA)
		EV_CocoaToolbar * pCocoaToolbar = static_cast<EV_CocoaToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*> (pFrame->getFrameData())->m_pToolbar[i] = pCocoaToolbar;
		static_cast<AP_CocoaFrame *>(pFrame)->toggleBar(i, bShowBar[i]);
	}
	[tlbr unlock];
	[tlbr redisplayToolbars:_getController()];
}


/*!
 * Refills the framedata class with pointers to the current toolbars. We 
 * need to do this after a toolbar icon and been dragged and dropped.
 */
void AP_CocoaFrameImpl::_refillToolbarsInFrameData(void)
{
	XAP_Frame*	pFrame = getFrame();
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	for (UT_uint32 i = 0; i < cnt; i++)
	{
		EV_CocoaToolbar * pCocoaToolbar = static_cast<EV_CocoaToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*> (pFrame->getFrameData())->m_pToolbar[i] = pCocoaToolbar;
	}
}

void AP_CocoaFrameImpl::_createDocumentWindow()
{
	XAP_Frame*	pFrame = getFrame();
	AP_FrameData* pData = static_cast<AP_FrameData*> (pFrame->getFrameData());
	bool bShowRulers = pData->m_bShowRuler;

	// create the rulers
	AP_CocoaTopRuler * pCocoaTopRuler = NULL;
	AP_CocoaLeftRuler * pCocoaLeftRuler = NULL;

	if ( bShowRulers )
	{
		pCocoaTopRuler = new AP_CocoaTopRuler(pFrame);
		UT_ASSERT(pCocoaTopRuler);
		
		if (pData->m_pViewMode == VIEW_PRINT) {
		    pCocoaLeftRuler = new AP_CocoaLeftRuler(pFrame);
		    UT_ASSERT(pCocoaLeftRuler);
		    pCocoaLeftRuler->createWidget();
		    // get the width from the left ruler and stuff it into the top ruler.
		    pCocoaTopRuler->setOffsetLeftRuler(pCocoaLeftRuler->getWidth());
		}
		else {
			_hideLeftRulerNSView();
		    pCocoaTopRuler->setOffsetLeftRuler(0);
		}
	}
	else {
		_hideTopRulerNSView();
		_hideLeftRulerNSView();
	}

	pData->m_pTopRuler = pCocoaTopRuler;
	pData->m_pLeftRuler = pCocoaLeftRuler;
}

void AP_CocoaFrameImpl::_createStatusBarWindow(XAP_CocoaNSStatusBar * /*statusBar*/)
{
	XAP_Frame*	pFrame = getFrame();
	
	UT_DEBUGMSG (("AP_CocoaFrame::_createStatusBarWindow ()\n"));
	// TODO: pass the NSView instead of the whole frame
	AP_CocoaStatusBar * pCocoaStatusBar = new AP_CocoaStatusBar(pFrame);
	UT_ASSERT(pCocoaStatusBar);

	static_cast<AP_FrameData *>(pFrame->getFrameData())->m_pStatusBar = pCocoaStatusBar;
	pCocoaStatusBar->createWidget();
}

void AP_CocoaFrameImpl::_setWindowIcon()
{
	// this is NOT needed. Just need to the the title.
}

NSString *	AP_CocoaFrameImpl::_getNibName ()
{
	return @"ap_CocoaFrame";
}

/*!
	Create and intialize the controller.
 */
XAP_CocoaFrameController *AP_CocoaFrameImpl::_createController()
{
	UT_DEBUGMSG (("AP_CocoaFrame::_createController()\n"));
	return [[AP_CocoaFrameController createFrom:this] retain];
}



bool AP_CocoaFrameImpl::_graphicsUpdateCB(NSRect * aRect, GR_CocoaCairoGraphics *pG, void* param)
{
	// a static function
	AP_CocoaFrame * pCocoaFrame = static_cast<AP_CocoaFrame *>(param);
	if (!pCocoaFrame)
		return false;
	FV_View * pView = static_cast<FV_View *>(pCocoaFrame->getCurrentView());

	UT_DEBUGMSG(("AP_CocoaFrameImpl::_graphicsUpdateCB()\n"));
	if(pView != NULL) {
		UT_Rect rClip;
		rClip.left = (UT_sint32)rint(pG->tluD(aRect->origin.x));
		rClip.top = (UT_sint32)rint(pG->tluD(aRect->origin.y));
		rClip.width = (UT_sint32)rint(pG->tluD(aRect->size.width));
		rClip.height = (UT_sint32)rint(pG->tluD(aRect->size.height));
		UT_DEBUGMSG(("Cocoa in frame expose painting area:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
		pView->draw(&rClip);
	}
	else
		return false;
	return true;
}


void AP_CocoaFrameImpl::giveFocus()
{
	[[_getController() window] makeFirstResponder:m_docAreaGRView];
}
/* Objective-C section */

@implementation AP_CocoaFrameController
+ (XAP_CocoaFrameController*)createFrom:(XAP_CocoaFrameImpl *)frame
{
	UT_DEBUGMSG (("Cocoa: @AP_CocoaFrameController createFrom:frame\n"));
	AP_CocoaFrameController *obj = [[AP_CocoaFrameController alloc] initWith:frame];
	return [obj autorelease];
}

- (id)initWith:(XAP_CocoaFrameImpl *)frame
{
	UT_DEBUGMSG (("Cocoa: @AP_CocoaFrameController initWith:frame\n"));
	return [super initWith:frame];
}


- (IBAction)rulerClick:(id)sender
{
	UT_UNUSED(sender);
	UT_DEBUGMSG(("ruler action"));
}

- (XAP_CocoaNSView *)getVRuler
{
	return vRuler;
}

- (XAP_CocoaNSView *)getHRuler
{
	return hRuler;
}

- (IBAction)scrollAction:(id)sender
{
	static_cast<AP_CocoaFrameImpl*>(m_frame)->_scrollAction(sender);
}

@end


@implementation XAP_NSScroller

-(id)initWithFrame:(NSRect)frame andController:(NSWindowController*)controller vertical:(BOOL)vertical
{
	if(![super initWithFrame:frame]) {
		return nil;
	}
	UT_DEBUGMSG (("x = %f, y = %f, w = %f, h = %f\n", frame.origin.x, frame.origin.y,
			frame.size.width, frame.size.height));
	if (vertical) {
		UT_DEBUGMSG(("Is vertical\n"));
		[self setAutoresizingMask:(NSViewMinXMargin |  NSViewHeightSizable)];
	}
	else {
		UT_DEBUGMSG(("Is horizontal\n"));
		[self setAutoresizingMask:(NSViewMaxYMargin |  NSViewWidthSizable)];
	}
	[self setTarget:controller];
	[self setAction:@selector(scrollAction:)];
	return self;
}

@end


@implementation AP_DocViewDelegate

- (void)mouseDown:(NSEvent *)theEvent from:(id)sender
{
//  	pFrame->setTimeOfLastEvent([theEvent timestamp]);
	XAP_Frame* pFrame = [(XAP_CocoaNSView*)sender xapFrame];
	AV_View * pView = pFrame->getCurrentView();
	EV_CocoaMouse * pCocoaMouse = (EV_CocoaMouse *)pFrame->getMouse();

	if (pView)
		pCocoaMouse->mouseClick (pView, theEvent, sender);
}

- (void)mouseDragged:(NSEvent *)theEvent from:(id)sender
{
//  	pFrame->setTimeOfLastEvent([theEvent timestamp]);
	XAP_Frame* pFrame = [(XAP_CocoaNSView*)sender xapFrame];
	AV_View * pView = pFrame->getCurrentView();
	EV_CocoaMouse * pCocoaMouse =(EV_CocoaMouse *)pFrame->getMouse();

	if (pView)
		pCocoaMouse->mouseMotion (pView, theEvent, sender);
}

- (void)mouseUp:(NSEvent *)theEvent from:(id)sender
{
//  	pFrame->setTimeOfLastEvent([theEvent timestamp]);
	XAP_Frame* pFrame = [(XAP_CocoaNSView*)sender xapFrame];
	AV_View * pView = pFrame->getCurrentView();
	EV_CocoaMouse * pCocoaMouse = (EV_CocoaMouse *)pFrame->getMouse();

	if (pView)
		pCocoaMouse->mouseUp (pView, theEvent, sender);
}

@end

