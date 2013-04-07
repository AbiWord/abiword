/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_DOCCOMPARISON_H
#define XAP_COCOADIALOG_DOCCOMPARISON_H

#import <Cocoa/Cocoa.h>

#include "xap_Dlg_DocComparison.h"
#include "xap_CocoaDialogFactory.h"
#include "xap_Dialog_Id.h"
#include "xap_Dialog.h"

@class XAP_CocoaDialog_DocComparisonController;
@protocol XAP_CocoaDialogProtocol;

class XAP_Frame;


/*****************************************************************/

class XAP_CocoaDialog_DocComparison: public XAP_Dialog_DocComparison
{
public:
	XAP_CocoaDialog_DocComparison(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_DocComparison(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

private:

	void _populateWindowData();
	XAP_CocoaDialog_DocComparisonController*	m_dlg;
};


@interface XAP_CocoaDialog_DocComparisonController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSTextField *_contentData;
    IBOutlet NSTextField *_contentLabel;
    IBOutlet NSTextField *_doc1;
    IBOutlet NSTextField *_doc2;
    IBOutlet NSBox *_docCmpBox;
    IBOutlet NSTextField *_formatData;
    IBOutlet NSTextField *_formatLabel;
    IBOutlet NSButton *_okBtn;
    IBOutlet NSTextField *_relationshipData;
    IBOutlet NSTextField *_relationshipLabel;
    IBOutlet NSBox *_resultsBox;
    IBOutlet NSTextField *_stylesData;
    IBOutlet NSTextField *_stylesLabel;
	XAP_CocoaDialog_DocComparison* _xap;
}
- (IBAction)okAction:(id)sender;
- (void)populate;
@end


#endif /* XAP_COCOADLG_DOCCOMPARISON */
