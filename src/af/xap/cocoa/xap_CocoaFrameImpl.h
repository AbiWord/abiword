/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef XAP_COCOAFRAMEIMPL_H
#define XAP_COCOAFRAMEIMPL_H

#import <Cocoa/Cocoa.h>
#import "xap_CocoaDialogFactory.h"
#import "xap_FrameImpl.h"
#import "xap_CocoaAppController.h"

@class XAP_CocoaNSStatusBar;

class XAP_CocoaFrameImpl;

@interface XAP_CocoaFrameController : NSWindowController
{
	XAP_CocoaFrameImpl *m_frame;
    IBOutlet NSView *mainView;
    IBOutlet XAP_CocoaNSStatusBar *statusBar;
	NSView <NSTextInput> *m_textView;
}
+ (XAP_CocoaFrameController*)createFrom:(XAP_CocoaFrameImpl *)frame;
- (id)initWith:(XAP_CocoaFrameImpl *)frame;
- (NSView *)getMainView;
- (XAP_CocoaNSStatusBar *)getStatusBar;
- (XAP_CocoaFrameImpl *)frameImpl;
- (void)setTextView:(NSView <NSTextInput>*)tv;
- (NSView <NSTextInput>*)textView;

- (NSArray *)getToolbars;
- (NSString *)getToolbarSummaryID;
@end


class EV_Menu;
class EV_CocoaMenuBar;
class EV_CocoaMenuPopup;
class XAP_DialogFactory;
class XAP_Frame;
class GR_CocoaGraphics;

class XAP_CocoaFrameImpl : public XAP_FrameImpl
{
public:
	static NSString* XAP_FrameNeedToolbar;
	static NSString* XAP_FrameReleaseToolbar;

	XAP_CocoaFrameImpl(XAP_Frame* frame);
	friend class XAP_Frame;
//	XAP_CocoaFrameImpl(XAP_CocoaFrame * f);
	virtual ~XAP_CocoaFrameImpl();
/*
	virtual bool				initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
										   const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
										   const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
										   const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
										   const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue);
*/
	virtual void _initialize();
	virtual void                notifyViewChanged(AV_View * pView);
//	virtual	XAP_Frame *			cloneFrame() = 0;
//	virtual UT_Error   			loadDocument(const char * szFilename, int ieft) = 0;
//	virtual UT_Error                        loadDocument(const char * szFilename, int ieft, bool createNew) = 0;
	virtual bool				_close();
	virtual bool				_raise();
	virtual bool				_show();
	virtual void 				_setFullScreen(bool /*isFullScreen*/ ) { UT_ASSERT(UT_NOT_IMPLEMENTED); }
	virtual bool				_updateTitle();
	virtual UT_sint32			_setInputMode(const char * szName);
	virtual void _hideMenuScroll(bool /*bHideMenuScroll*/)
		{ UT_ASSERT(UT_NOT_IMPLEMENTED); }
	virtual void                _nullUpdate () const;
	virtual void                _setCursor(GR_Graphics::Cursor /*c*/) { }

	NSWindow *					getTopLevelWindow() const;
	NSView *					getVBoxWidget() const;
	virtual XAP_DialogFactory *	_getDialogFactory();
//	virtual void				setXScrollRange() = 0;
//	virtual void				setYScrollRange() = 0;
	virtual bool				_runModalContextMenu(AV_View * pView, const char * szMenuName,
													UT_sint32 x, UT_sint32 y);
//	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y) = 0;
//	virtual void				setStatusMessage(const char * szMsg) = 0;

	void						setTimeOfLastEvent(NSTimeInterval timestamp);

//	virtual void				toggleRuler(bool bRulerOn) = 0;
	virtual void				_queue_resize();
	virtual EV_Menu*			_getMainMenu();

	virtual void                _rebuildMenus(void);
    virtual void                _rebuildToolbar(UT_uint32 ibar);
	void                        _setController (XAP_CocoaFrameController * ctrl);
	XAP_CocoaFrameController *	_getController () { return m_frameController; };
	virtual NSString *			_getNibName () = 0;
	virtual XAP_CocoaFrameController *_createController() = 0;
	static void				setToolbarRect(const NSRect &r);
protected:
	virtual void				_createDocumentWindow() = 0;
	virtual void				_createStatusBarWindow(XAP_CocoaNSStatusBar *) = 0;
	virtual void				_createTopLevelWindow();
	virtual void				_setWindowIcon() = 0;
	virtual	void				_createDocView(GR_Graphics* &pG) = 0; /* Cocoa specific */

	virtual EV_Toolbar *		_newToolbar(XAP_Frame *frame, const char *, const char *);

	virtual UT_RGBColor 		getColorSelBackground () const;
private:
	AP_CocoaDialogFactory		m_dialogFactory;
	EV_CocoaMenuPopup *			m_pCocoaPopup; /* only valid while a context popup is up */

	XAP_CocoaFrameController *		m_frameController;
};


#endif
