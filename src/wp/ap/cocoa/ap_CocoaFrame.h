/* AbiWord
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

#ifndef AP_COCOAFRAME_H
#define AP_COCOAFRAME_H

#import <Cocoa/Cocoa.h>
#import "XAP_CocoaFrame.h"

class GR_Graphics;
class AP_CocoaFrame;

#include "ie_types.h"

/*****************************************************************/


@interface AP_CocoaFrameController : XAP_CocoaFrameController
{
    IBOutlet NSControl *hRuler;
    IBOutlet NSControl *vRuler;
}
+ (AP_CocoaFrameController*)createFrom:(AP_CocoaFrame *)frame;
- (id)initWith:(XAP_CocoaFrame *)frame;
- (IBAction)rulerClick:(id)sender;
- (NSControl *)getVRuler;
- (NSControl *)getHRuler;
@end


class AP_CocoaFrame : public XAP_CocoaFrame
{
public:
	AP_CocoaFrame(XAP_CocoaApp * app);
	AP_CocoaFrame(AP_CocoaFrame * f);
	virtual ~AP_CocoaFrame(void);

	virtual bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void);
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
	virtual NSString *			_getNibName ();
	virtual XAP_CocoaFrameController *_createController();

protected:
	virtual void			_createDocumentWindow(void);
	virtual void				_createStatusBarWindow(NSView *);
	virtual void				_setWindowIcon(void);
	UT_Error   					_loadDocument(const char * szFilename, IEFileType ieft, bool createNew);
	virtual UT_Error            _importDocument(const char * szFilename, int ieft, bool markClean);
	UT_Error   					_showDocument(UT_uint32 iZoom=100);
	static void					_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void					_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
	UT_Error					_replaceDocument(AD_Document * pDoc);
	virtual void				_showOrHideToolbars(void);
	virtual void				_showOrHideStatusbar(void);

#if 0	
	GtkAdjustment *				m_pVadj;
	GtkAdjustment *				m_pHadj;
	GtkWidget *					m_hScroll;
	GtkWidget *					m_vScroll;
	GtkWidget *					m_dArea;
	GtkWidget *					m_table;
	GtkWidget *					m_innertable;
	GtkWidget *					m_topRuler;
	GtkWidget *					m_leftRuler;
	GtkWidget *					m_wSunkenBox;
#endif
};

#endif /* AP_COCOAFRAME_H */

