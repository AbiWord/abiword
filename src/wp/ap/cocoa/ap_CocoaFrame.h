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

#ifndef AP_COCOAFRAME_H
#define AP_COCOAFRAME_H

#import <Cocoa/Cocoa.h>
#import "gr_CocoaGraphics.h"
#import "xap_CocoaFrame.h"

class AP_CocoaFrame;
class FL_DocLayout;

#include "ap_Frame.h"
#include "ie_types.h"
#include "ut_assert.h"

/*****************************************************************/


@interface AP_CocoaFrameController : XAP_CocoaFrameController
{
    IBOutlet XAP_CocoaNSView *hRuler;
    IBOutlet XAP_CocoaNSView *vRuler;
}
+ (XAP_CocoaFrameController*)createFrom:(XAP_Frame *)frame;
- (id)initWith:(XAP_Frame *)frame;
- (IBAction)rulerClick:(id)sender;
- (XAP_CocoaNSView *)getVRuler;
- (XAP_CocoaNSView *)getHRuler;
@end

/*****************************************************************/
class AP_CocoaFrameHelper : public XAP_CocoaFrameHelper
{
 public:
	AP_CocoaFrameHelper(AP_CocoaFrame *pCocoaFrame, XAP_CocoaApp *pCocoaApp); 
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
	virtual	void	_createDocView(GR_CocoaGraphics* &pG);

	NSScroller *				m_hScrollbar;
	NSScroller *				m_vScrollbar;
	XAP_CocoaNSView *			m_docAreaGRView;
private:
	static bool					_graphicsUpdateCB(NSRect * aRect, GR_CocoaGraphics *pG, void* param);
};


class AP_CocoaFrame : public XAP_Frame
{
public:
	AP_CocoaFrame(XAP_CocoaApp * app);
	AP_CocoaFrame(AP_CocoaFrame * f);
	virtual ~AP_CocoaFrame(void);

	virtual bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void);
	virtual	XAP_Frame *			buildFrame(XAP_Frame * pClone);
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft);
	virtual UT_Error                        loadDocument(const char * szFilename, int ieft, bool createNew);
	virtual UT_Error            importDocument(const char * szFilename, int ieft, bool markClean);
	virtual bool				initFrameData(void);
	virtual void				killFrameData(void);

	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y);
	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage(void);
	virtual void				setStatusMessage(const char * szMsg);

	virtual void				toggleRuler(bool bRulerOn);
	virtual void                            toggleTopRuler(bool bRulerOn);
	virtual void                            toggleLeftRuler(bool bRulerOn);
	virtual void				toggleBar(UT_uint32 iBarNb, bool bBarOn);
	virtual void				toggleStatusBar(bool bStatusBarOn);

protected:
	UT_Error   					_loadDocument(const char * szFilename, IEFileType ieft, bool createNew);
	virtual UT_Error            _importDocument(const char * szFilename, int ieft, bool markClean);
	UT_Error   					_showDocument(UT_uint32 iZoom=100);
	static void					_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void					_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
	UT_Error					_replaceDocument(AD_Document * pDoc);

#if 0	
	GtkAdjustment *				m_pVadj;
	GtkAdjustment *				m_pHadj;
	GtkWidget *					m_table;
	GtkWidget *					m_innertable;
	GtkWidget *					m_topRuler;
	GtkWidget *					m_leftRuler;
	GtkWidget *					m_wSunkenBox;
#endif
};

#endif /* AP_COCOAFRAME_H */

