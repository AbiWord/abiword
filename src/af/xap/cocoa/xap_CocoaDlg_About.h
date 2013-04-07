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

#ifndef XAP_COCOADIALOG_ABOUT_H
#define XAP_COCOADIALOG_ABOUT_H


#import <Cocoa/Cocoa.h>

#include "xap_Dlg_About.h"
#include "xap_CocoaFrame.h"

class XAP_CocoaDialog_About;
@protocol XAP_CocoaDialogProtocol;

@interface XAP_CocoaDlg_AboutController : NSWindowController <XAP_CocoaDialogProtocol>
{
	XAP_CocoaDialog_About *m_xap;
    IBOutlet NSImageView *m_imageView;
    IBOutlet NSTextView *m_licenseText;
    IBOutlet NSButton *m_okBtn;
    IBOutlet NSTextField *m_versionLabel;
    IBOutlet NSTextField *m_appName;
    IBOutlet NSButton *m_webBtn;
}
- (IBAction)okBtnAction:(id)sender;
- (IBAction)webBtnAction:(id)sender;
@end

/*****************************************************************/

class XAP_CocoaDialog_About: public XAP_Dialog_About
{
public:
	XAP_CocoaDialog_About(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_About(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	virtual void			runModal(XAP_Frame * pFrame);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_URL(void);
private:
	XAP_CocoaDlg_AboutController * m_dlg;
};


#endif /* XAP_COCOADIALOG_ABOUT_H */
