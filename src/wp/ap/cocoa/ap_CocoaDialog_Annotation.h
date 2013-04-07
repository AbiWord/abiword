/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_ANNOTATION_H
#define AP_COCOADIALOG_ANNOTATION_H

#include <Cocoa/Cocoa.h>

#include "xap_CocoaDialog_Utilities.h"
#include "ap_Dialog_Annotation.h"

class XAP_CocoaFrame;
class AP_CocoaDialog_Annotation;

@interface AP_CocoaDialog_AnnotationController : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSTextField * _titleLabel;
	IBOutlet NSTextField * _authorLabel;
	IBOutlet NSTextField * _descLabel;

	IBOutlet NSTextField * _titleText;
	IBOutlet NSTextField * _authorText;
	IBOutlet NSTextView * _descText;

	IBOutlet NSButton * _okBtn;
	IBOutlet NSButton * _cancelBtn;
	IBOutlet NSButton * _replaceBtn;

	AP_CocoaDialog_Annotation * _xap;
}
- (IBAction)okAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (IBAction)replaceAction:(id)sender;
- (void)setXAPOwner:(XAP_Dialog*)owner;

@end
/*****************************************************************/

class AP_CocoaDialog_Annotation
	: public AP_Dialog_Annotation
{
 public:
	AP_CocoaDialog_Annotation(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_CocoaDialog_Annotation(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

private:
	AP_CocoaDialog_AnnotationController * m_dlg;
};

#endif /* AP_COCOADIALOG_ANNOTATION_H */
