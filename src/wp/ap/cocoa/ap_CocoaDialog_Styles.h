/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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

#ifndef AP_CocoaDialog_Styles_H
#define AP_CocoaDialog_Styles_H

#import <Cocoa/Cocoa.h>
#include "ap_Dialog_Columns.h"
#include "gr_CocoaGraphics.h"

#include "ut_types.h"
#include "ut_string.h"
#import "xap_Cocoa_NSTableUtils.h"
#include "ap_Dialog_Styles.h"


class XAP_CocoaFrame;
@class AP_CocoaDialog_StylesController;
@class AP_CocoaDialog_StylesModifyController;

/*****************************************************************/

class AP_CocoaDialog_Styles: public AP_Dialog_Styles
{
public:
	typedef enum _StyleType 
	  {USED_STYLES, ALL_STYLES, USER_STYLES} StyleType;

	AP_CocoaDialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Styles(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events

	void                            event_paraPreviewExposed(void);
	void                            event_charPreviewExposed(void);
	
	virtual void			event_Apply(void);
	virtual void			event_Close(void);

	virtual void event_DeleteClicked(NSString* data);
	virtual void event_NewClicked(void);
	virtual void event_ModifyClicked(void);
	virtual void event_ClistClicked(int row);
	virtual void event_ListFilterClicked(const StyleType which);
	void new_styleName(void);

/////////////////////////////////////////////////////////////////////////
// Modify window
/////////////////////////////////////////////////////////////////////////

	void         event_Modify_OK(void);
	void         event_Modify_Cancel(void);
	void         event_ModifyParagraph();
	void         event_ModifyFont();
	void         event_ModifyNumbering();
	void         event_ModifyTabs();
	void         event_ModifyLanguage();
	void         event_ModifyPreviewExposed();
	void         event_RemoveProperty(void);
	void         rebuildDeleteProps(void);
	void         event_basedOn(void);
	void         event_followedBy(void);
	void         event_styleType(void);
	void         modifyRunModal(void);
	void         event_modifySheetDidEnd(int code);
	void         setIsNew(bool isNew) {m_bIsNew = isNew;}
	const bool   isNew(void) const { return m_bIsNew;}
	XML_Char *   getNewStyleName(void) const {return (XML_Char *) m_newStyleName;}
	XML_Char *   getBasedonName(void) const {return (XML_Char *) m_basedonName;}
	XML_Char *   getFollowedbyName(void) const {return (XML_Char *) m_followedbyName;}
	XML_Char *   getStyleType(void) const {return (XML_Char *) m_styleType;}

private:
	void				_populateWindowData(void);
	void                            _populateCList(void);
	void 				_storeWindowData(void) const;
	virtual const char * getCurrentStyle (void) const;
	virtual void setDescription (const char * desc) const;

	GR_CocoaGraphics	* 		m_pParaPreviewWidget;
	GR_CocoaGraphics	* 		m_pCharPreviewWidget;

	virtual void setModifyDescription( const char * desc);
	bool        _populateModify(void);

	XML_Char    m_newStyleName[40];
	XML_Char    m_basedonName[40];
	XML_Char    m_followedbyName[40];
	XML_Char    m_styleType[40];
	GR_CocoaGraphics	* 		m_pAbiPreviewWidget;
	int m_whichRow;
	StyleType m_whichType;
	bool m_bIsNew;
	AP_CocoaDialog_StylesController* m_dlg;
	AP_CocoaDialog_StylesModifyController* m_modifyDlg;
};


#if defined(INTERNAL_OBJC)

@interface AP_CocoaDialog_StylesController : NSWindowController <XAP_CocoaDialogProtocol>
{
@public
    IBOutlet NSButton *_applyBtn;
    IBOutlet NSBox *_availStylesBox;
    IBOutlet NSTableView *_availStylesList;
    IBOutlet XAP_CocoaNSView *_charPreview;
    IBOutlet NSBox *_charPreviewBox;
    IBOutlet NSButton *_closeBtn;
    IBOutlet NSButton *_deleteBtn;
    IBOutlet NSBox *_descriptionBox;
    IBOutlet NSTextField *_descriptionData;
    IBOutlet NSPopUpButton *_listCombo;
    IBOutlet NSTextField *_listLabel;
    IBOutlet NSButton *_modifyBtn;
    IBOutlet NSButton *_newBtn;
    IBOutlet XAP_CocoaNSView *_paraPreview;
    IBOutlet NSBox *_paraPreviewBox;
	
	XAP_StringListDataSource* m_stylesDataSource;
	AP_CocoaDialog_Styles* _xap;
}
- (IBAction)applyAction:(id)sender;
- (IBAction)closeAction:(id)sender;
- (IBAction)deleteAction:(id)sender;
- (IBAction)listSelectedAction:(id)sender;
- (IBAction)listFilterSelectedAction:(id)sender;
- (IBAction)modifyAction:(id)sender;
- (IBAction)newAction:(id)sender;

- (void)setStyleDescription:(NSString*)desc;
@end

@interface AP_CocoaDialog_StylesModifyController : NSWindowController <XAP_CocoaDialogProtocol>
{
@public
    IBOutlet NSComboBox *_basedOnCombo;
    IBOutlet NSTextField *_basedOnLabel;
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSTextField *_desc;
    IBOutlet NSBox *_descBox;
    IBOutlet NSComboBox *_followStyleCombo;
    IBOutlet NSTextField *_followStyleLabel;
    IBOutlet NSPopUpButton *_formatPopupBtn;
    IBOutlet NSButton *_okBtn;
    IBOutlet XAP_CocoaNSView *_preview;
    IBOutlet NSBox *_previewBox;
    IBOutlet NSButton *_removeBtn;
    IBOutlet NSComboBox *_removePropCombo;
    IBOutlet NSTextField *_removePropLabel;
    IBOutlet NSButton *_shortcutBtn;
    IBOutlet NSTextField *_styleNameData;
    IBOutlet NSTextField *_styleNameLabel;
    IBOutlet NSComboBox *_styleTypeCombo;
    IBOutlet NSTextField *_styleTypeLabel;
	AP_CocoaDialog_Styles* _xap;
}
- (IBAction)basedOnAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (IBAction)followStyleAction:(id)sender;
- (IBAction)formatAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)removeAction:(id)sender;
- (IBAction)shortcutAction:(id)sender;
- (IBAction)styleNameAction:(id)sender;
- (IBAction)styleTypeAction:(id)sender;

- (void)sheetDidEnd:(NSWindow*)sheet returnCode:(int)returnCode contextInfo:(void  *)c;
@end

#endif


#endif /* AP_CocoaDialog_Styles_H */







