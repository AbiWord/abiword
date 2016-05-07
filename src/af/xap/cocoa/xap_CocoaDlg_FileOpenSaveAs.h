/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003, 2009 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_FILEOPENSAVEAS_H
#define XAP_COCOADIALOG_FILEOPENSAVEAS_H

#import <Cocoa/Cocoa.h>

#include "xap_Dlg_FileOpenSaveAs.h"

class XAP_CocoaFrame;
class XAP_CocoaDialog_FileOpenSaveAs;


@interface XAP_OpenSavePanel_AccessoryController 
	: NSObject<NSOpenSavePanelDelegate>
{
	IBOutlet NSTextField *		oFTLabel;
	IBOutlet NSPopUpButton *	oFTPopUp;
	IBOutlet NSView *			oFTAccessoryView;

	XAP_CocoaDialog_FileOpenSaveAs *	_xap;
}

- (id)initWithXAP:(XAP_CocoaDialog_FileOpenSaveAs*)xap;

- (NSView *)fileTypeAccessoryView;
- (void)setFileTypeLabel:(const std::string &)label;
- (void)setSelectedFileType:(int)type;

- (void)removeItemsOfFileTypesMenu;
- (void)addItemWithTitle:(NSString *)title fileType:(int)type;

- (IBAction)selectFileType:(id)sender;
@end

/*****************************************************************/

class XAP_CocoaDialog_FileOpenSaveAs : public XAP_Dialog_FileOpenSaveAs
{
public:
	XAP_CocoaDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_FileOpenSaveAs(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	void	_setSelectedFileType (UT_sint32 type);

private:
	NSSavePanel * _makeOpenPanel();
	NSSavePanel * _makeSavePanel(const std::string & label);

	XAP_OpenSavePanel_AccessoryController *	m_accessoryViewsController;

	NSSavePanel	*							m_panel;
	NSMutableArray *						m_fileTypes;

	const char *							m_szFileTypeDescription;
	UT_uint32								m_szFileTypeCount;

	bool									m_bPanelActive;
	bool									m_bOpenPanel;
	bool									m_bIgnoreCancel;
};

#endif /* XAP_COCOADIALOG_FILEOPENSAVEAS_H */
