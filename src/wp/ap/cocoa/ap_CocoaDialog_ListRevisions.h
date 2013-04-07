/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_COCOADIALOG_LISTREVISIONS_H
#define AP_COCOADIALOG_LISTREVISIONS_H

#include "ap_Dialog_ListRevisions.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_ListRevisionsController;
@class AP_ListRevisions_DataSource;

/*****************************************************************/

class AP_CocoaDialog_ListRevisions: public AP_Dialog_ListRevisions
{
public:
	AP_CocoaDialog_ListRevisions(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_ListRevisions(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	void event_OK();
	void event_Cancel();
	void event_Select(int idx);
private:
	AP_CocoaDialog_ListRevisionsController *m_dlg;
	AP_ListRevisions_DataSource *m_dataSource;
};


@interface AP_CocoaDialog_ListRevisionsController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSTextField *_label;
    IBOutlet NSTableView *_list;
    IBOutlet NSButton *_okBtn;
	AP_CocoaDialog_ListRevisions	*_xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)listAction:(id)sender;
- (IBAction)okAction:(id)sender;

- (void)setDataSource:(AP_ListRevisions_DataSource*)ds;
@end

#endif /* AP_COCOADIALOG_LISTREVISIONS_H */
