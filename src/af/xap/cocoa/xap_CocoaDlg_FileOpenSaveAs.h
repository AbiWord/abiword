/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef XAP_COCOADIALOG_FILEOPENSAVEAS_H
#define XAP_COCOADIALOG_FILEOPENSAVEAS_H

#import <Cocoa/Cocoa.h>

#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Strings.h"

class XAP_CocoaFrame;
class XAP_CocoaDialog_FileOpenSaveAs;


@interface XAP_OpenSavePanel_AccessoryController : NSObject
{
	IBOutlet NSTextField*		_fileTypeLabel;
	IBOutlet NSPopUpButton*	_fileTypePopup;
	IBOutlet NSView*			_fileTypeAcessoryView;
//	IBOutlet NSImageView*		_imagePreview;
	XAP_CocoaDialog_FileOpenSaveAs*	_xap;
}

-(id)initWithXAP:(XAP_CocoaDialog_FileOpenSaveAs*)xap;
-(void)awakeFromNib;

-(NSView*)fileTypeAcessoryView;
-(void)setFileTypeLabel:(NSString*)label;
-(void)setSelectedFileType:(int)type;
-(NSMenu*)fileTypesMenu;
-(void)removeItemsOfFileTypesMenu;

-(IBAction)selectFileType:(id)sender;

@end

/*****************************************************************/

class XAP_CocoaDialog_FileOpenSaveAs : public XAP_Dialog_FileOpenSaveAs
{
public:
	XAP_CocoaDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_FileOpenSaveAs(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	int previewPicture (void);

	void	_setSelectedFileType (UT_sint32 type);
protected:
	NSSavePanel	*m_panel;
#if 0
	bool					_run_gtk_main(XAP_Frame * pFrame, void * pFSvoid,
										  bool bCheckWritePermission,
										  GtkWidget * filetypes_pulldown);
	void 					_notifyError_OKOnly(XAP_Frame * pFrame,
												XAP_String_Id sid);
	void 					_notifyError_OKOnly(XAP_Frame * pFrame,
												XAP_String_Id sid,
												const char * sz1);
	bool 				_askOverwrite_YesNo(XAP_Frame * pFrame,
												const char * fileName);

	GtkFileSelection * m_FS;
	GtkWidget * m_preview;
#endif
	XAP_OpenSavePanel_AccessoryController*	m_accessoryViewsController;
	XAP_CocoaFrame *						m_pCocoaFrame;
};

#endif /* XAP_COCOADIALOG_FILEOPENSAVEAS_H */
