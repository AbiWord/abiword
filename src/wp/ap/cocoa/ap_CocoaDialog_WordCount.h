/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001, 2003-2005 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_WORDCOUNT_H
#define AP_COCOADIALOG_WORDCOUNT_H

#import <Cocoa/Cocoa.h>

#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaWidget.h"

#include "ap_Dialog_WordCount.h"

class UT_Timer;

class AP_CocoaDialog_WordCount;

@interface AP_CocoaDialog_WordCountController : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSTextField *_titleLabel;
    IBOutlet NSTextField *_charNoSpaceCount;
    IBOutlet NSTextField *_charNoSpaceLabel;
    IBOutlet NSTextField *_charSpaceCount;
    IBOutlet NSTextField *_charSpaceLabel;
    IBOutlet NSTextField *_linesCount;
    IBOutlet NSTextField *_linesLabel;
    IBOutlet NSTextField *_pageCount;
    IBOutlet NSTextField *_pageLabel;
    IBOutlet NSTextField *_paraCount;
    IBOutlet NSTextField *_paraLabel;
    IBOutlet NSTextField *_wordCount;
    IBOutlet NSTextField *_wordLabel;
	IBOutlet NSTextField *_wordNoFNCount;
	IBOutlet NSTextField *_wordNoFNLabel;

	AP_CocoaDialog_WordCount *	_xap;
}
- (XAP_CocoaWidget *)getWidget:(int)wid;
@end

/*****************************************************************/

class AP_CocoaDialog_WordCount: public AP_Dialog_WordCount
{
public:
	AP_CocoaDialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_WordCount(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	static void			autoupdateWC(UT_Worker * pTimer);
	// callbacks can fire these events

	virtual void			event_Update(void);
	void 					event_CloseWindow(void);
protected:
	virtual XAP_Widget *getWidget(xap_widget_id wid)
	{
		return [m_dlg getWidget:((int) wid)];
	}

private:
	UT_Timer * m_pAutoUpdateWC;

	// Handshake variables
	bool m_bDestroy_says_stopupdating;
	bool m_bAutoUpdate_happening_now;
	AP_CocoaDialog_WordCountController*	m_dlg;
};

#endif /* AP_COCOADIALOG_WORDCOUNT_H */








