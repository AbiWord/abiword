/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_COCOADIALOG_INSERTHYPERLINK_H
#define AP_COCOADIALOG_INSERTHYPERLINK_H

#include "ap_Dialog_InsertHyperlink.h"
#import "xap_Cocoa_NSTableUtils.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_InsertHyperlinkController;
@protocol XAP_CocoaDialogProtocol;

/*****************************************************************/

class AP_CocoaDialog_InsertHyperlink: public AP_Dialog_InsertHyperlink
{
public:
	AP_CocoaDialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_CocoaDialog_InsertHyperlink(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	void event_OK(void);
	void event_Cancel(void);
private:
	XAP_StringListDataSource*	m_pBookmarks;
	AP_CocoaDialog_InsertHyperlinkController* m_dlg;
};


#endif /* AP_COCOADIALOG_INSERTBOOKMARK_H */
