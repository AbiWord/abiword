/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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


#ifndef XAP_COCOAFRAME_H
#define XAP_COCOAFRAME_H

#import <Cocoa/Cocoa.h>

#import "gr_CocoaGraphics.h"

#include "xap_Frame.h"
#include "ut_vector.h"
#include "xap_CocoaDialogFactory.h"

class XAP_CocoaApp;
class EV_CocoaKeyboard;
class EV_CocoaMouse;
class EV_CocoaMenuBar;
class EV_CocoaMenuPopup;

/*****************************************************************
******************************************************************
** This file defines the cocoa-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** cocoa-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class XAP_CocoaFrameHelper;
class GR_CocoaGraphics;
class FV_View;

// TODO should figure out if need default values
@interface XAP_CocoaNSView : NSView
{
	XAP_Frame 		*m_pFrame;
	GR_CocoaGraphics	*m_pGR;
}
//- (id)initWith:(XAP_CocoaFrame *)frame;
- (id)initWith:(XAP_Frame *)frame andFrame:(NSRect)windowFrame;
- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (void)setXAPFrame:(XAP_Frame *)frame;
- (void)setGraphics:(GR_CocoaGraphics *)gr;
- (void)drawRect:(NSRect)aRect;
- (BOOL)isFlipped;
- (BOOL)isOpaque;
@end

@class XAP_CocoaNSStatusBar;

@interface XAP_CocoaFrameController : NSWindowController
{
	XAP_Frame *m_frame;
    IBOutlet NSView *mainView;
    IBOutlet XAP_CocoaNSStatusBar *statusBar;
	IBOutlet NSMenu *menuBar;
	IBOutlet NSMenuItem *m_preferenceMenu;
	IBOutlet NSMenuItem *m_aboutMenu;
        IBOutlet NSMenuItem *m_quitMenu;
}
+ (XAP_CocoaFrameController*)createFrom:(XAP_Frame *)frame;
- (id)initWith:(XAP_Frame *)frame;
- (NSView *)getMainView;
- (NSMenu *)getMenuBar;
- (XAP_CocoaNSStatusBar *)getStatusBar;
- (NSMenuItem *)_aboutMenu;
- (NSMenuItem *)_preferenceMenu;
- (NSMenuItem *)_quitMenu;
@end

class XAP_CocoaFrameHelper : public XAP_FrameHelper
{
public:
	XAP_CocoaFrameHelper(XAP_Frame* frame, XAP_CocoaApp * app);
	friend class XAP_Frame;
//	XAP_CocoaFrameHelper(XAP_CocoaFrame * f);
	virtual ~XAP_CocoaFrameHelper();
/*
	virtual bool				initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
										   const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
										   const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
										   const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
										   const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue);
*/
	virtual void _initialize();
	virtual	XAP_Frame *			cloneFrame() = 0;
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft) = 0;
	virtual UT_Error                        loadDocument(const char * szFilename, int ieft, bool createNew) = 0;
	virtual bool				_close();
	virtual bool				_raise();
	virtual bool				_show();
	virtual void setFullScreen(bool isFullScreen) {}
	virtual bool				openURL(const char * szURL);
	virtual bool				_updateTitle();
	virtual UT_sint32			_setInputMode(const char * szName);
	virtual void                _nullUpdate () const;
	virtual void                _setCursor(GR_Graphics::Cursor c) {}

	NSWindow *					getTopLevelWindow() const;
	NSView *					getVBoxWidget() const;
	virtual XAP_DialogFactory *	_getDialogFactory();
	virtual void				setXScrollRange() = 0;
	virtual void				setYScrollRange() = 0;
	virtual bool				_runModalContextMenu(AV_View * pView, const char * szMenuName,
													UT_sint32 x, UT_sint32 y);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y) = 0;
	virtual void				setStatusMessage(const char * szMsg) = 0;

	void						setTimeOfLastEvent(NSTimeInterval timestamp);
	
	virtual void				toggleRuler(bool bRulerOn) = 0;
	virtual void				queue_resize();
	virtual EV_Menu*			_getMainMenu();
	virtual void                rebuildMenus(void);
    virtual void                rebuildToolbar(UT_uint32 ibar);
	void                        _setController (XAP_CocoaFrameController * ctrl);
	XAP_CocoaFrameController *	_getController () { return m_frameController; };
	XAP_CocoaApp *				_getApp () { return m_pCocoaApp; };
	virtual NSString *			_getNibName () = 0;
	virtual XAP_CocoaFrameController *_createController() = 0;
	
	NSMenuItem	*				_getPreferenceMenuItem () { return [m_frameController _preferenceMenu]; };
	NSMenuItem  *				_getAboutMenuItem () { return [m_frameController _aboutMenu]; };
	NSMenuItem  *				_getQuitMenuItem () { return [m_frameController _quitMenu]; };
protected:
	virtual void				_createDocumentWindow() = 0;
	virtual void				_createStatusBarWindow(XAP_CocoaNSStatusBar *) = 0;
	virtual void				_createTopLevelWindow();
	virtual void				_setWindowIcon() = 0;
	virtual	void				_createDocView(GR_CocoaGraphics* &pG) = 0; /* Cocoa specific */

	virtual EV_Toolbar *		_newToolbar(XAP_App *app, XAP_Frame *frame, const char *, const char *);
private:
	AP_CocoaDialogFactory		m_dialogFactory;
	XAP_CocoaApp *				m_pCocoaApp;
	EV_CocoaMenuBar *			m_pCocoaMenu;
	EV_CocoaMenuPopup *			m_pCocoaPopup; /* only valid while a context popup is up */
	
	XAP_CocoaFrameController *		m_frameController;

	UT_uint32					m_iAbiRepaintID;

protected:
	class _fe
	{
	friend class XAP_Frame;
	public:
		static int abi_expose_repaint(void * p);
	};
	friend class _fe;
};

#endif /* XAP_COCOAFRAME_H */
