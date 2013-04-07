/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2002 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuière
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

#ifndef XAP_COCOADIALOG_HTMLOPTIONS_H
#define XAP_COCOADIALOG_HTMLOPTIONS_H

#import <Cocoa/Cocoa.h>

#include "ut_types.h"

#include "xap_Dialog.h"

#include "xap_Dlg_HTMLOptions.h"

class XAP_Frame;
@class XAP_CocoaDialog_HTMLOptions_Controller;
@protocol XAP_CocoaDialogProtocol;

class XAP_CocoaDialog_HTMLOptions : public XAP_Dialog_HTMLOptions
{
public:
	XAP_CocoaDialog_HTMLOptions (XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~XAP_CocoaDialog_HTMLOptions (void);

	virtual void			runModal (XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor (XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	void			toggle_Is4 ();
	void			toggle_AbiWebDoc ();
	void			toggle_DeclareXML ();
	void			toggle_AllowAWML ();
	void			toggle_EmbedCSS ();
	void			toggle_EmbedImages ();

	void			refreshStates ();

	typedef enum {
		IS4,
		ABIWEBDOC,
		DECLAREXML,
		ALLOWAWML,
		EMBEDCSS,
		EMBEDIMAGES
	} options;

	void			event_OK (void);
	void			event_SaveSettings (void);
	void			event_RestoreSettings (void);
	void			event_Cancel (void);
 private:
	XAP_CocoaDialog_HTMLOptions_Controller* m_dlg;
};


@interface XAP_CocoaDialog_HTMLOptions_Controller : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_allowExtraMarkupBtn;
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSButton *_declareXMLBtn;
    IBOutlet NSButton *_embedCSSBtn;
    IBOutlet NSButton *_embedImagesBtn;
    IBOutlet NSButton *_exportHTMLBtn;
    IBOutlet NSBox *_exportOptionsBox;
    IBOutlet NSButton *_exportPHPBtn;
    IBOutlet NSButton *_okBtn;
    IBOutlet NSButton *_restoreBtn;
    IBOutlet NSButton *_saveBtn;
	XAP_CocoaDialog_HTMLOptions*	_xap;
}
- (IBAction)allowExtraMarkupAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (IBAction)declareXMLAction:(id)sender;
- (IBAction)embedCSSAction:(id)sender;
- (IBAction)embedImageAction:(id)sender;
- (IBAction)exportHTMLAction:(id)sender;
- (IBAction)exportPHPAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)restoreAction:(id)sender;
- (IBAction)saveAction:(id)sender;

- (void)toggle:(XAP_CocoaDialog_HTMLOptions::options)btn withValue:(bool)value;
- (void)enable:(XAP_CocoaDialog_HTMLOptions::options)btn withValue:(bool)value;
- (bool)valueOf:(XAP_CocoaDialog_HTMLOptions::options)btn;
@end

#endif /* XAP_COCOADIALOG_HTMLOPTIONS_H */



