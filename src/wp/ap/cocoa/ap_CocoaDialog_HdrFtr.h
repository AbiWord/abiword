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

#ifndef AP_COCOADIALOG_HDRFTR_H
#define AP_COCOADIALOG_HDRFTR_H

#include <Cocoa/Cocoa.h>
#include "ap_Dialog_HdrFtr.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_HdrFtrController;

/*****************************************************************/

class AP_CocoaDialog_HdrFtr: public AP_Dialog_HdrFtr
{
public:
	AP_CocoaDialog_HdrFtr(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_HdrFtr(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	virtual void eventOk(void);
	virtual void eventCancel(void);
	void         RestartSpinChanged(UT_sint32 RestartValue);
	void         CheckChanged(id checkbox);

private:
	AP_CocoaDialog_HdrFtrController*	m_dlg;
};



@interface AP_CocoaDialog_HdrFtrController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *cancelBtn;
    IBOutlet NSBox *footerBox;
    IBOutlet NSButton *footerFacingBtn;
    IBOutlet NSButton *footerFirstBtn;
    IBOutlet NSButton *footerLastBtn;
    IBOutlet NSBox *headerBox;
    IBOutlet NSButton *headerFacingBtn;
    IBOutlet NSButton *headerFirstBtn;
    IBOutlet NSButton *headerLastBtn;
    IBOutlet NSButton *okBtn;
    IBOutlet NSBox *pageNumberBox;
    IBOutlet NSTextField *restartAtData;
    IBOutlet NSTextField *restartAtLabel;
    IBOutlet NSButton *restartPgNumberBtn;
    IBOutlet NSStepper *restartStepper;
	AP_CocoaDialog_HdrFtr*	_xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)btnAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)restartAction:(id)sender;
- (IBAction)restartBtnAction:(id)sender;
- (IBAction)restartStepperAction:(id)sender;
@end

#endif /* AP_COCOADIALOG_HDRFTR_H */


