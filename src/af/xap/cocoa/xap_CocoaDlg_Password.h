/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_PASSWORD_H
#define XAP_COCOADIALOG_PASSWORD_H

#include <Cocoa/Cocoa.h>
#include "xap_Dlg_Password.h"

class XAP_CocoaFrame;
@class XAP_CocoaDlg_PasswordController;

/*****************************************************************/

class XAP_CocoaDialog_Password: public XAP_Dialog_Password
{
public:
	XAP_CocoaDialog_Password(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_Password(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	void event_Ok ();
	void event_Cancel ();
private:
	XAP_CocoaDlg_PasswordController* m_dlg;
};

@interface XAP_CocoaDlg_PasswordController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSButton *_okBtn;
    IBOutlet NSTextField *_passwordData;
    IBOutlet NSTextField *_passwordLabel;
	XAP_CocoaDialog_Password *_xap;
}
- (NSString*)password;
- (IBAction)cancelAction:(id)sender;
- (IBAction)okAction:(id)sender;
@end
#endif /* XAP_COCOADIALOG_PASSWORD_H */
