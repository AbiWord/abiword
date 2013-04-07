/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003, 2005 Hubert Figuiere
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

#import "xap_CocoaFrame.h"
#import "xap_CocoaFrameImpl.h"


class AP_CocooApp;

@class XAP_CocoaTextView, XAP_NSScroller;

@interface AP_DocViewDelegate : NSObject <XAP_MouseEventDelegate>
{
}
@end

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
- (IBAction)scrollAction:(id)sender;
@end

/*****************************************************************/
class AP_CocoaFrameImpl : public XAP_CocoaFrameImpl
{
 public:
	AP_CocoaFrameImpl(AP_CocoaFrame *pCocoaFrame);
	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame);

	virtual NSString *			_getNibName (); /* must be public to be called from Obj-C */

	UT_sint32 _getHScrollValue()	{ return m_HCurrentScroll; };
	UT_sint32 _getHScrollMin()	{ return m_HMinScroll; };
	UT_sint32 _getHScrollMax()	{ return m_HMaxScroll; };
	UT_sint32 _getHVisible()	{ return m_HVisible; };
	void _setHScrollValue(UT_sint32);
	void _setHScrollMin(UT_sint32);
	void _setHScrollMax(UT_sint32);
	void _setHVisible(UT_sint32);
	UT_sint32 _getVScrollValue()	{ return m_VCurrentScroll; };
	UT_sint32 _getVScrollMin()	{ return m_VMinScroll; };
	UT_sint32 _getVScrollMax()	{ return m_VMaxScroll; };
	UT_sint32 _getVVisible()	{ return m_VVisible; };
	void _setVScrollValue(UT_sint32);
	void _setVScrollMin(UT_sint32);
	void _setVScrollMax(UT_sint32);
	void _setVVisible(UT_sint32);
	void _scrollAction(id sender);

	void _showTopRulerNSView(void);
	void _hideTopRulerNSView(void);
	void _showLeftRulerNSView(void);
	void _hideLeftRulerNSView(void);
	XAP_CocoaTextView *_getDocAreaGRView(void)
		{ return m_docAreaGRView; }
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

	virtual void _hideMenuScroll(bool /*bHideMenuScroll*/) { UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED); }

	void giveFocus();
private:
	XAP_NSScroller*					m_hScrollbar;
	XAP_NSScroller*					m_vScrollbar;
	XAP_CocoaTextView*				m_docAreaGRView;
	static bool					_graphicsUpdateCB(NSRect * aRect, GR_CocoaCairoGraphics *pG, void* param);
private:
	UT_sint32					m_HMinScroll;
	UT_sint32					m_HMaxScroll;
	UT_sint32					m_HCurrentScroll;
	UT_sint32					m_HVisible;
	UT_sint32					m_VMinScroll;
	UT_sint32					m_VMaxScroll;
	UT_sint32					m_VCurrentScroll;
	UT_sint32					m_VVisible;
	/* called when updating */
	void _setHScrollbarValues();
	void _setVScrollbarValues();
};
