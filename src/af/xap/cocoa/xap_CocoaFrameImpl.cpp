/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2004 Hubert Figuiere
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

#import "ut_debugmsg.h"
#import "ut_assert.h"
#import "ut_sleep.h"

#import "fv_View.h"
#import "ev_EditMethod.h"
#import "ev_CocoaKeyboard.h"
#import "ev_CocoaMouse.h"
#import "ev_CocoaMenu.h"
#import "ev_CocoaMenuBar.h"
#import "ev_CocoaMenuPopup.h"
#import "ev_CocoaToolbar.h"
#import "ev_Toolbar.h"
#import "gr_CocoaCairoGraphics.h"
#import "xap_App.h"
#import "xap_CocoaApp.h"
#import "xap_CocoaAppController.h"
#import "xap_CocoaFrameImpl.h"
#import "xap_CocoaTextView.h"
#import "xap_CocoaTimer.h"
#import "xap_CocoaToolbarWindow.h"
#import "xap_CocoaToolPalette.h"
#import "xap_FrameImpl.h"
#import "xap_Frame.h"
#import "xap_FrameNSWindow.h"

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)


NSString* XAP_CocoaFrameImpl::XAP_FrameNeedToolbar = @"XAP_FrameNeedToolbar";
NSString* XAP_CocoaFrameImpl::XAP_FrameReleaseToolbar = @"XAP_FrameReleaseToolbar";


/*****************************************************************/

XAP_CocoaFrameImpl::XAP_CocoaFrameImpl(XAP_Frame* frame)
	: XAP_FrameImpl (frame),
	  m_dialogFactory(XAP_App::getApp(), frame),
	  m_pCocoaPopup(NULL),
	  m_frameController(nil)
{
//	m_pView = NULL;

#if 0
	// dirty hack to make sure the frame is compiled in as the class is only 
	// referenced from a nib.
	XAP_FrameNSWindow *p = nil;
	if (0) {
		p = [[[XAP_FrameNSWindow alloc] init] autorelease];
	}
#endif
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??
/*
XAP_CocoaFrameImpl::XAP_CocoaFrameImpl(XAP_CocoaFrameImpl * f)
	: XAP_FrameImpl(f),
	  m_dialogFactory(XAP_App::getApp(), f->m_pFrame),
	  m_pCocoaMenu(NULL),
	  m_pCocoaPopup(NULL),
	  m_frameController(nil)
{
	m_pView = NULL;
}
*/

XAP_CocoaFrameImpl::~XAP_CocoaFrameImpl(void)
{
	// only delete the things we created...
	if 	(m_frameController != nil) {
		[m_frameController release];
	}

	DELETEP(m_pCocoaPopup);
}

/*
bool XAP_CocoaFrameImpl::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
								  const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
								  const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
								  const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
								  const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue)
{
	bool bResult;

	// invoke our base class first.
	
	bResult = XAP_Frame::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue,
									szMenuLayoutKey, szMenuLayoutDefaultValue,
									szMenuLabelSetKey, szMenuLabelSetDefaultValue,
									szToolbarLayoutsKey, szToolbarLayoutsDefaultValue,
									szToolbarLabelSetKey, szToolbarLabelSetDefaultValue);
	UT_ASSERT(bResult);
*/
void XAP_CocoaFrameImpl::_initialize()
{
   	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.

	EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_CocoaKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);
	
	m_pMouse = new EV_CocoaMouse(pEEM);
	UT_ASSERT(m_pMouse);
}

void XAP_CocoaFrameImpl::notifyViewChanged(AV_View * pView) // called from XAP_Frame::setView(pView)
{
	if (XAP_Frame * pFrame = getFrame())
		{
			XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
			[pController resetCurrentView:pView inFrame:pFrame];
		}
}

UT_sint32 XAP_CocoaFrameImpl::_setInputMode(const char * szName)
{
	UT_sint32 result = XAP_App::getApp()->setInputMode(szName);
	if (result == 1)
	{
		// if it actually changed we need to update keyboard and mouse

		EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
		UT_ASSERT(pEEM);

		m_pKeyboard->setEditEventMap(pEEM);
		m_pMouse->setEditEventMap(pEEM);
	}

	return result;
}

NSWindow * XAP_CocoaFrameImpl::getTopLevelWindow(void) const
{
	UT_ASSERT (m_frameController);
	return [m_frameController window];
}

NSView * XAP_CocoaFrameImpl::getVBoxWidget(void) const
{
	UT_ASSERT (m_frameController);
	return [m_frameController getMainView];
}

XAP_DialogFactory * XAP_CocoaFrameImpl::_getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_CocoaFrameImpl::_nullUpdate() const
{
	/* FIXME: do this more propperly if there is any chance...*/
	[[m_frameController window] displayIfNeeded];
}

static UT_uint32	s_iNewFrameOffsetX = 0;
static UT_uint32	s_iNewFrameOffsetY = 0;

void XAP_CocoaFrameImpl::_createTopLevelWindow(void)
{
	// create a top-level window for us.
	m_frameController = _createController();
	UT_ASSERT (m_frameController);

	NSWindow * theWindow = [m_frameController window];
	UT_ASSERT (theWindow);
	[theWindow setTitle:[NSString stringWithUTF8String:XAP_App::getApp()->getApplicationTitleForTitleBar()]];

	// create a toolbar instance for each toolbar listed in our base class.

	_createToolbars();

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).
	_createDocumentWindow();

	// Let the app-specific frame code create the status bar
	// if it wants to.  we will put it below the document
	// window (a peer with toolbars and the overall sunkenbox)
	// so that it will appear outside of the scrollbars.
	_createStatusBarWindow([m_frameController getStatusBar]);

	// set the icon
	_setWindowIcon();

	// set geometry hints as the user requested
	int x, y;
	UT_uint32 width, height;
	XAP_CocoaApp::windowGeometryFlags f;

	dynamic_cast<XAP_CocoaApp*>(XAP_App::getApp())->getGeometry(&x, &y, &width, &height, &f);

	// Set the size if requested
	NSSize userSize;
	// userSize.width  = (width  < 300.0f) ? 300.0f : static_cast<float>(width);
	// userSize.height = (height < 300.0f) ? 300.0f : static_cast<float>(height);

	if (f & XAP_CocoaApp::GEOMETRY_FLAG_SIZE)
	{
		NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];

		screenFrame.size.height -= [[XAP_CocoaToolbarWindow_Controller sharedToolbar] height];

		if ([XAP_CocoaToolPalette instantiated])
		{
			NSWindow * palettePanel = [[XAP_CocoaToolPalette instance:theWindow] window];
			if ([palettePanel isVisible])
			{
				NSRect paletteFrame = [palettePanel frame];
				if (paletteFrame.origin.x < screenFrame.origin.x + 10.0f) // panel at left
				{
					screenFrame.origin.x   += paletteFrame.size.width;
					screenFrame.size.width -= paletteFrame.size.width;
				}
				else if (paletteFrame.origin.x + paletteFrame.size.width > screenFrame.origin.x + screenFrame.size.width - 10.0f) // panel at right
				{
					screenFrame.size.width -= paletteFrame.size.width;
				}
			}
		}

		userSize.height = screenFrame.size.height;
		userSize.width  = rintf(screenFrame.size.height * 0.9f);

		if (XAP_App::getApp()->getFrameCount() == 1)
		{
			s_iNewFrameOffsetX = 0;
			s_iNewFrameOffsetY = 0;
		}

		screenFrame.origin.x    += s_iNewFrameOffsetX;
		screenFrame.size.height -= s_iNewFrameOffsetY;

		s_iNewFrameOffsetX = (s_iNewFrameOffsetX <= 128) ? (s_iNewFrameOffsetX + 32) : 0;
		s_iNewFrameOffsetY = (s_iNewFrameOffsetY <  128) ? (s_iNewFrameOffsetY + 32) : 0;

		NSRect windowFrame;
		windowFrame.size.width  = UT_MIN(screenFrame.size.width,  userSize.width ); // 813);
		windowFrame.size.height = UT_MIN(screenFrame.size.height, userSize.height); // 836);
		windowFrame.origin.x = screenFrame.origin.x;
		windowFrame.origin.y = screenFrame.origin.y + screenFrame.size.height - windowFrame.size.height;

		[theWindow setFrame:windowFrame display:YES];
	}
}


void XAP_CocoaFrameImpl::_rebuildMenus()
{
#if 0
	// destroy old menu
	m_pUnixMenu->destroy();
	DELETEP(m_pUnixMenu);
	
	// build new one.
	m_pUnixMenu = new EV_CocoaMenuBar(static_cast<XAP_UnixApp*>(XAP_App::getApp()), getFrame(),
					 m_szMenuLayoutName,
					 m_szMenuLabelSetName);
	UT_return_if_fail(m_pUnixMenu);
	bool bResult = m_pUnixMenu->rebuildMenuBar();
	UT_ASSERT_HARMLESS(bResult);
#endif
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}


/*!
 * This code is used by the dynamic toolbar API to rebuild a toolbar after a
 * a change in the toolbar structure.
 */
void XAP_CocoaFrameImpl::_rebuildToolbar(UT_uint32 ibar)
{
	XAP_Frame* pFrame = getFrame();
//
// Destroy the old toolbar
//
	EV_Toolbar * pToolbar = (EV_Toolbar *) m_vecToolbars.getNthItem(ibar);
	const char * szTBName = (const char *) m_vecToolbarLayoutNames.getNthItem(ibar);
	EV_CocoaToolbar * pUTB = static_cast<EV_CocoaToolbar *>( pToolbar);
	UT_sint32 oldpos = pUTB->destroy();
//
// Delete the old class
//
	delete pToolbar;
	if(oldpos < 0)
	{
		return;
	}
//
// Build a new one.
//
	pToolbar = _newToolbar(pFrame, szTBName,
						   (const char *) m_szToolbarLabelSetName);
	static_cast<EV_CocoaToolbar *>(pToolbar)->rebuildToolbar(oldpos);
	m_vecToolbars.setNthItem(ibar, pToolbar, NULL);
//
// Refill the framedata pointers
//
	pFrame->refillToolbarsInFrameData();
	pFrame->repopulateCombos();
}

bool XAP_CocoaFrameImpl::_close()
{
	UT_DEBUGMSG (("XAP_CocoaFrame::close()\n"));
	[m_frameController close];
	[[NSNotificationCenter defaultCenter] postNotificationName:XAP_FrameReleaseToolbar object:m_frameController];
	return true;
}

bool XAP_CocoaFrameImpl::_raise()
{
	UT_DEBUGMSG (("XAP_CocoaFrame::raise()\n"));
	[[m_frameController window] makeKeyAndOrderFront:m_frameController];
	[[NSNotificationCenter defaultCenter] postNotificationName:XAP_FrameNeedToolbar object:m_frameController];
	return true;
}

bool XAP_CocoaFrameImpl::_show()
{
	UT_DEBUGMSG (("XAP_CocoaFrame::show()\n"));
	[[m_frameController window] makeKeyAndOrderFront:m_frameController];
	[[NSNotificationCenter defaultCenter] postNotificationName:XAP_FrameNeedToolbar object:m_frameController];

	XAP_CocoaTextView * textView = (XAP_CocoaTextView *) [m_frameController textView];
	[textView hasBeenResized:nil];

	return true;
}


UT_RGBColor XAP_CocoaFrameImpl::getColorSelBackground () const
{
	/* this code is disabled as the NSColor returned is not RGB compatible. */
	static UT_RGBColor * c = NULL;
	if (c == NULL) {
		c = new UT_RGBColor();
		GR_CocoaCairoGraphics::_utNSColorToRGBColor ([[NSColor selectedTextBackgroundColor] 
					colorUsingColorSpaceName:NSCalibratedRGBColorSpace], *c);
	}
	return *c;
}


bool XAP_CocoaFrameImpl::_updateTitle()
{
	if (!XAP_FrameImpl::_updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	NSWindow * theWindow = [m_frameController window];
	UT_ASSERT (theWindow);
	if (theWindow)
		{
			const char * szTitle    = getFrame()->getNonDecoratedTitle();
			const char * szFilename = getFrame()->getFilename();

			[theWindow setTitle:[NSString stringWithUTF8String:szTitle]];
			if (szFilename)
				[theWindow setRepresentedFilename:[NSString stringWithUTF8String:szFilename]];

			[theWindow setDocumentEdited:(getFrame()->isDirty() ? YES : NO)];
		}
	return true;
}

/*****************************************************************/
bool XAP_CocoaFrameImpl::_runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
										   UT_sint32 /*x*/, UT_sint32 /*y*/)
{
	bool bResult = true;

	UT_ASSERT(!m_pCocoaPopup);

	m_pCocoaPopup = new EV_CocoaMenuPopup(szMenuName, m_szMenuLabelSetName);
	if (m_pCocoaPopup && m_pCocoaPopup->synthesizeMenuPopup()) {
		NSEvent *evt = [NSApp currentEvent];
		[NSMenu popUpContextMenu:m_pCocoaPopup->getMenuHandle() withEvent:evt forView:[m_frameController getMainView]];
	}
	XAP_Frame * pFrame = (XAP_Frame *)getFrame();
	if (pFrame->getCurrentView()) {
		pFrame->getCurrentView()->focusChange( AV_FOCUS_HERE);
	}
	DELETEP(m_pCocoaPopup);
	return bResult;
}

void XAP_CocoaFrameImpl::setTimeOfLastEvent(NSTimeInterval timestamp)
{
	dynamic_cast<XAP_CocoaApp*>(XAP_App::getApp())->setTimeOfLastEvent(timestamp);
}

EV_Toolbar * XAP_CocoaFrameImpl::_newToolbar(XAP_Frame *frame,
											 const char *szLayout,
											 const char *szLanguage)
{
	return (new EV_CocoaToolbar((AP_CocoaFrame *)frame, 
								szLayout, szLanguage));
}

void XAP_CocoaFrameImpl::_queue_resize()
{
	UT_DEBUGMSG(("XAP_CocoaFrameImpl::queue_resize\n"));	
	UT_ASSERT (UT_NOT_IMPLEMENTED);
//	gtk_widget_queue_resize(m_wTopLevelWindow);
}


EV_Menu* XAP_CocoaFrameImpl::_getMainMenu()
{
	return dynamic_cast<XAP_CocoaApp*>(XAP_App::getApp())->getCocoaMenuBar();
}


void XAP_CocoaFrameImpl::_setController (XAP_CocoaFrameController * ctrl)
{
	if ((m_frameController) && (ctrl != m_frameController)){
		[m_frameController release];
	}
	m_frameController = ctrl; 
}

void XAP_CocoaFrameImpl::setToolbarRect(const NSRect &r)
{
	UT_uint32 frameCount = XAP_App::getApp()->getFrameCount();
	UT_uint32 i;
	
	for (i = 0; i < frameCount; i++) {
		XAP_CocoaFrameImpl* impl = dynamic_cast<XAP_CocoaFrameImpl*>(XAP_App::getApp()->getFrame(i)->getFrameImpl());
		XAP_CocoaFrameController* ctrl  = impl->_getController();
		NSRect frame = [[ctrl window] frame];
		if (NSIntersectsRect(frame, r)) {
			if (frame.origin.y + frame.size.height > r.origin.y) {
				UT_DEBUGMSG(("original frame is %f %f %f %f\n", frame.origin.x, frame.origin.y,
									frame.size.width, frame.size.height));
				frame.size.height = r.origin.y - frame.origin.y;
				[[ctrl window] setFrame:frame display:YES];
				UT_DEBUGMSG(("resized frame is %f %f %f %f\n", frame.origin.x, frame.origin.y,
									frame.size.width, frame.size.height));
			}
		}
	}
}


/* Objective C section */

@implementation XAP_CocoaFrameController

- (BOOL)windowShouldClose:(id)sender
{
	UT_UNUSED(sender);
	UT_DEBUGMSG (("shouldCloseDocument\n"));
	UT_ASSERT (m_frame);
	XAP_App * pApp = XAP_App::getApp();
	UT_ASSERT(pApp);

	const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
	UT_ASSERT(pEMC);
	
	const EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindowX");
	UT_ASSERT(pEM);
	
	if (pEM)
	{
		XAP_Frame * pFrame = m_frame->getFrame ();
		UT_ASSERT(pFrame);
		AV_View * pView = pFrame->getCurrentView();	
		UT_ASSERT(pView);
		if (pEM->Fn(m_frame->getFrame()->getCurrentView(),NULL))
		{
			// returning YES means close the window
			// but first let's warn the app controller not to assume our existence
			
			XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
			[pController unsetCurrentView:pView inFrame:pFrame];

			return YES;
		}
	}
		
	// returning NO means do NOT close the window
	return NO;
}

- (void)keyDown:(NSEvent *)theEvent
{
	xxx_UT_DEBUGMSG(("keyDown in window '%s'\n", [[[self window] title] UTF8String]));
	[m_textView interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
#if 0
	XAP_Frame * pFrame = m_frame->getFrame();
//  	pFrame->setTimeOfLastEvent([theEvent timestamp]);
	AV_View * pView = pFrame->getCurrentView();
	ev_CocoaKeyboard * pCocoaKeyboard = static_cast<ev_CocoaKeyboard *>
		(pFrame->getKeyboard());

	if (pView)
		pCocoaKeyboard->keyPressEvent(pView, theEvent);
#endif
}

- (void)windowDidBecomeKey:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	UT_DEBUGMSG(("windowDidBecomeKey: '%s'\n", [[[self window] title] UTF8String]));

	XAP_Frame * pFrame = m_frame->getFrame ();
	AV_View * pView = pFrame->getCurrentView();

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	[pController setCurrentView:pView inFrame:pFrame];

	[[NSNotificationCenter defaultCenter] postNotificationName:XAP_CocoaFrameImpl::XAP_FrameNeedToolbar 
			object:self];
}

- (void)windowDidExpose:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	UT_DEBUGMSG(("windowDidExpose:\n"));
//	[[NSNotificationCenter defaultCenter] postNotificationName:XAP_CocoaFrameImpl::XAP_FrameNeedToolbar 
//			object:self];
}

- (void)windowDidResignKey:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	UT_DEBUGMSG(("windowDidResignKey: '%s'\n", [[[self window] title] UTF8String]));

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	[pController setCurrentView:0 inFrame:0];

//	[[NSNotificationCenter defaultCenter] postNotificationName:XAP_CocoaFrameImpl::XAP_FrameReleaseToolbar 
//			object:self];
}


/*!
	Returns an instance.
 */
+ (XAP_CocoaFrameController*)createFrom:(XAP_CocoaFrameImpl *)frame
{
	UT_DEBUGMSG (("Cocoa: createFrom:frame\n"));
	XAP_CocoaFrameController *obj = [[XAP_CocoaFrameController alloc] initWith:frame];
	return [obj autorelease];
}

- (id)initWith:(XAP_CocoaFrameImpl *)frame
{
	UT_DEBUGMSG (("Cocoa: @XAP_CocoaFrameController initWith:frame\n"));
	if(![self initWithWindowNibName:frame->_getNibName()]) {	/* this one will make the call to [super init]  */
		return nil;
	}
	m_frame = frame;
	[[self window] setAcceptsMouseMovedEvents:YES];		/* can't we set that from IB (FIXME) */
	[[self window] makeFirstResponder:self];
	return self;
}

- (NSView *)getMainView
{
	return mainView;
}


- (XAP_CocoaNSStatusBar *)getStatusBar
{
	return statusBar;
}

- (XAP_CocoaFrameImpl *)frameImpl
{
	return m_frame;
}

- (void)setTextView:(NSView <NSTextInput>*)tv
{
	m_textView = tv;
}

- (NSView <NSTextInput>*)textView
{
	return m_textView;
}

- (NSArray*)getToolbars
{
	NSMutableArray*	array = [NSMutableArray array];
	const UT_GenericVector<EV_Toolbar*> & toolbars = m_frame->_getToolbars();
	UT_uint32 count = toolbars.getItemCount();
	for (UT_uint32 i = 0; i < count; i++) {
		const EV_CocoaToolbar* tlbr = static_cast<const EV_CocoaToolbar*>(toolbars[i]);
		UT_ASSERT(tlbr);
		if (!tlbr->isHidden()) {
			[array addObject:tlbr->_getToolbarView()];
		}
	}
	return array;
}

- (NSString *)getToolbarSummaryID
{
	UT_UTF8String SummaryID;

	const UT_GenericVector<EV_Toolbar *> & toolbars = m_frame->_getToolbars();

	UT_uint32 count = toolbars.getItemCount();

	for (UT_uint32 i = 0; i < count; i++) {
		const EV_CocoaToolbar * tlbr = static_cast<const EV_CocoaToolbar *>(toolbars[i]);
		UT_ASSERT(tlbr);

		if (!tlbr->isHidden()) {
			SummaryID += "+";
		}
		else {
			SummaryID += "-";
		}
	}
	return [NSString stringWithUTF8String:(SummaryID.utf8_str())];
}

@end
