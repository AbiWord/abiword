/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004, 2009 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_HISTORY_H
#define XAP_COCOADIALOG_HISTORY_H

#include "xap_Dlg_History.h"

@class XAP_CocoaDialog_HistoryController;
@protocol XAP_CocoaDialogProtocol;
class XAP_Frame;

/*****************************************************************/

class XAP_CocoaDialog_History: public XAP_Dialog_History
{
public:
	XAP_CocoaDialog_History(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_History(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	void event_OK();
	void event_Cancel();
private:
	void        _populateWindowData(void);

	XAP_CocoaDialog_HistoryController *m_dlg;
};



@interface XAP_CocoaDialog_HistoryController
    : NSWindowController <XAP_CocoaDialogProtocol, NSTableViewDataSource>
{
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSTextField *_createdData;
    IBOutlet NSTextField *_createdLabel;
    IBOutlet NSTextField *_docNameData;
    IBOutlet NSTextField *_docNameLabel;
    IBOutlet NSTextField *_editTimeData;
    IBOutlet NSTextField *_editTimeLabel;
    IBOutlet NSBox *_historyBox;
    IBOutlet NSTableView *_historyList;
    IBOutlet NSTextField *_identifierData;
    IBOutlet NSTextField *_identifierLabel;
    IBOutlet NSTextField *_lastSavedData;
    IBOutlet NSTextField *_lastSavedLabel;
    IBOutlet NSButton *_okBtn;
    IBOutlet NSTextField *_versionData;
    IBOutlet NSTextField *_versionLabel;
	XAP_CocoaDialog_History* _xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)historySelect:(id)sender;
- (void)populate;

// data source
- (id)tableView:(NSTableView *)aTableView
    objectValueForTableColumn:(NSTableColumn *)aTableColumn
    row:(int)rowIndex;
- (int)numberOfRowsInTableView:(NSTableView *)aTableView;
@end

#endif /* XAP_COCOADIALOG_HISTORY_H */
