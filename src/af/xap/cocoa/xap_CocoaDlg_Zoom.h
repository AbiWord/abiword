/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003, 2009 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_ZOOM_H
#define XAP_COCOADIALOG_ZOOM_H

#import <Cocoa/Cocoa.h>

#include "xap_Dlg_Zoom.h"

class XAP_CocoaFrame;
class XAP_CocoaDialog_Zoom;

@interface XAP_CocoaDlg_ZoomController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButtonCell *_pageWidthBtn;
    IBOutlet NSButtonCell *_percentBtn;
    IBOutlet NSTextField *_percentField;
	IBOutlet NSStepper *_percentStepper;
    IBOutlet NSButtonCell *_wholePageBtn;
    IBOutlet NSButtonCell *_zoom100Btn;
    IBOutlet NSButtonCell *_zoom200Btn;
    IBOutlet NSButtonCell *_zoom75Btn;
    IBOutlet NSBox *_zoomBox;
	IBOutlet NSMatrix	*_zoomMatrix;
	IBOutlet NSButton *_closeBtn;
	XAP_CocoaDialog_Zoom*	_xap;
}
- (IBAction)closeAction:(id)sender;
- (IBAction)stepperAction:(id)sender;
- (IBAction)zoom100Action:(id)sender;
- (IBAction)zoom200Action:(id)sender;
- (IBAction)zoom75Action:(id)sender;
- (IBAction)zoomPageWidthAction:(id)sender;
- (IBAction)zoomWholePageAction:(id)sender;
- (IBAction)zoomPercentAction:(id)sender;
- (IBAction)zoomChangedAction:(id)sender;

- (NSMatrix*)zoomMatrix;
- (int)percentValue;
- (void)setPercentValue:(int)value;
- (void)_enablePercentSpin:(BOOL)enable;
@end

/*****************************************************************/

class XAP_CocoaDialog_Zoom: public XAP_Dialog_Zoom
{
public:
	XAP_CocoaDialog_Zoom(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_Zoom(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	virtual void			event_Close(void);

	virtual void			event_Radio200Clicked(void);
	virtual void			event_Radio100Clicked(void);
	virtual void			event_Radio75Clicked(void);
	virtual void			event_RadioPageWidthClicked(void);
	virtual void			event_RadioWholePageClicked(void);

	virtual void			event_RadioPercentClicked(void);
	virtual void			event_SpinPercentChanged(void);
private:
	void		_populateWindowData(void);
	void 		_storeWindowData(void);
	XAP_CocoaDlg_ZoomController*	m_dlg;
};

#endif /* XAP_COCOADIALOG_ZOOM_H */
