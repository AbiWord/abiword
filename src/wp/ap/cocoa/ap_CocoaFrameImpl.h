/* AbiWord
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

#import <Cocoa/Cocoa.h>

#import "xap_CocoaFrameImpl.h"


class AP_CocooApp;
class XAP_CocoaFrame;

@class XAP_CocoaNSView;

@interface AP_CocoaFrameController : XAP_CocoaFrameController
{
    IBOutlet XAP_CocoaNSView *hRuler;
    IBOutlet XAP_CocoaNSView *vRuler;
}
+ (XAP_CocoaFrameController*)createFrom:(XAP_CocoaFrameImpl *)frame;
- (id)initWith:(XAP_CocoaFrameImpl *)frame;
- (IBAction)rulerClick:(id)sender;
- (XAP_CocoaNSView *)getVRuler;
- (XAP_CocoaNSView *)getHRuler;
@end

/*****************************************************************/
class AP_CocoaFrameImpl : public XAP_CocoaFrameImpl
{
 public:
	AP_CocoaFrameImpl(AP_CocoaFrame *pCocoaFrame, XAP_CocoaApp *pCocoaApp); 
	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame, XAP_App *pApp) { return NULL; };

	virtual NSString *			_getNibName (); /* must be public to be called from Obj-C */

 protected:
	void _showOrHideStatusbar(void);

	void _showOrHideToolbars(void);
	virtual void _refillToolbarsInFrameData();
	void _bindToolbars(AV_View * pView);

	virtual void _createDocumentWindow();
	virtual void _createStatusBarWindow(XAP_CocoaNSStatusBar *);

	friend class AP_CocoaFrame;
	virtual void _setWindowIcon();
	/* Cocoa specific stuff */
	virtual XAP_CocoaFrameController *_createController();
	virtual	void	_createDocView(GR_Graphics* &pG);

	NSScroller *				m_hScrollbar;
	NSScroller *				m_vScrollbar;
	XAP_CocoaNSView *			m_docAreaGRView;
private:
	static bool					_graphicsUpdateCB(NSRect * aRect, GR_CocoaGraphics *pG, void* param);
};
