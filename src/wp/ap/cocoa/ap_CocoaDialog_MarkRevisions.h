/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
 * Copyright (C) 2003 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_MARKREVISIONS_H
#define AP_COCOADIALOG_MARKREVISIONS_H

#include "ap_Dialog_MarkRevisions.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_MarkRevisionsController;


/*****************************************************************/

class AP_CocoaDialog_MarkRevisions: public AP_Dialog_MarkRevisions
{
public:
	AP_CocoaDialog_MarkRevisions(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_MarkRevisions(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	void event_OK();
	void event_Cancel();
	void event_FocusToggled () ;

private:
	AP_CocoaDialog_MarkRevisionsController	*m_dlg;
};


@interface AP_CocoaDialog_MarkRevisionsController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSTextField *_commentData;
	IBOutlet NSTextField *_label1;
    IBOutlet NSTextField *_label2;
    IBOutlet NSButton *_okBtn;
    IBOutlet NSButton *_radio1;
    IBOutlet NSButton *_radio2;
	AP_CocoaDialog_MarkRevisions *_xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)radio1Action:(id)sender;
- (IBAction)radio2Action:(id)sender;

- (NSString*)comment;
- (int)toggled;
- (void)setItems2Enabled:(bool)state;
@end

#endif /* AP_COCOADIALOG_MARKREVISIONS_H */
