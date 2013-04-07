/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2005 Martin Sevior
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

#ifndef AP_COCOADIALOG_LATEX_H
#define AP_COCOADIALOG_LATEX_H

#import <Cocoa/Cocoa.h>

#include "ap_Dialog_Latex.h"
#include "xap_CocoaDialog_Utilities.h"

@class AP_CocoaDialog_LatexController;

/*****************************************************************/

class AP_CocoaDialog_Latex : public AP_Dialog_Latex
{
public:
	AP_CocoaDialog_Latex(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~AP_CocoaDialog_Latex(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events

	void			event_Insert(void);
	void			event_Close(void);
	void			event_WindowDelete(void);

	virtual void	setLatexInGUI(void);
	virtual bool	getLatexFromGUI(void);

protected:
	AP_CocoaDialog_LatexController *	m_dlg;
};

@interface AP_CocoaDialog_LatexController : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSButton *		oClose;
	IBOutlet NSButton *		oInsert;

	IBOutlet NSTextView *	oEditor;

	IBOutlet NSTextField *	oHeadingText;
	IBOutlet NSTextField *	oExampleText;

	AP_CocoaDialog_Latex *	_xap;
}
- (id)initFromNib;
- (void)dealloc;

- (void)setXAPOwner:(XAP_Dialog *)owner;
- (void)discardXAP;

- (void)windowDidLoad;
- (void)windowWillClose:(NSNotification *)aNotification;

- (IBAction)aClose:(id)sender;
- (IBAction)aInsert:(id)sender;

- (void)setEditorText:(NSString *)text;
- (NSString *)editorText;
@end

#endif /* AP_COCOADIALOG_Latex_H */
