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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_COCOADIALOG_FORMATTOC_H
#define AP_COCOADIALOG_FORMATTOC_H

#include <Cocoa/Cocoa.h>

#include "ap_Dialog_FormatTOC.h"


@class AP_CocoaDialog_FormatTOC_Controller;
@protocol XAP_CocoaDialogProtocol;

/*****************************************************************/

class AP_CocoaDialog_FormatTOC: public AP_Dialog_FormatTOC
{
public:
	AP_CocoaDialog_FormatTOC(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_FormatTOC(void);
	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	virtual void            setTOCPropsInGUI(void);
    virtual void            setSensitivity(bool bSensitive);
	virtual void            destroy(void);
	virtual void            activate(void);	
	void			_createLevelItems(NSPopUpButton*);
private:
	void			_populateWindowData(void);
	AP_CocoaDialog_FormatTOC_Controller *m_dlg;
};


@interface AP_CocoaDialog_FormatTOC_Controller : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_applyBtn;
    IBOutlet NSTextField *_displayStyleLabel;
    IBOutlet NSPopUpButton *_displayStylePopup;
    IBOutlet NSTextField *_fillStyleLabel;
    IBOutlet NSPopUpButton *_fillStylePopup;
    IBOutlet NSBox *_hasHeadingBox;
    IBOutlet NSButton *_hasHeadingBtn;
    IBOutlet NSButton *_hasLabelBtn;
	IBOutlet NSPopUpButton *_headingStyleData;
	IBOutlet NSTextField *_headingStyleLabel;
	IBOutlet NSTextField *_headingTextData;
	IBOutlet NSTextField *_headingTextLabel;
    IBOutlet NSTextField *_indentData;
    IBOutlet NSTextField *_indentLabel;
    IBOutlet NSButton *_inheritLabelBtn;
    IBOutlet NSBox *_labelDefBox;
    IBOutlet NSPopUpButton *_layoutLevelPopup;
    IBOutlet NSPopUpButton *_mainLevelPopup;
    IBOutlet NSBox *_mainPropBox;
    IBOutlet NSPopUpButton *_numberingTypeData;
    IBOutlet NSTextField *_numberingTypeLabel;
    IBOutlet NSPopUpButton *_pageNumberingData;
    IBOutlet NSTextField *_pageNumberingLabel;
    IBOutlet NSTextField *_startAtData;
    IBOutlet NSTextField *_startAtLabel;
    IBOutlet NSPopUpButton *_tabLeadersData;
    IBOutlet NSTextField *_tabLeadersLabel;
    IBOutlet NSBox *_tabsAndPageNumbBox;
    IBOutlet NSTabView *_tabView;
    IBOutlet NSTextField *_textAfterData;
    IBOutlet NSTextField *_textAfterLabel;
    IBOutlet NSTextField *_textBeforeData;
    IBOutlet NSTextField *_textBeforeLabel;
	AP_CocoaDialog_FormatTOC *_xap;
}
- (IBAction)mainLevelAction:(id)sender;
- (IBAction)detailLevelAction:(id)sender;
- (IBAction)applyAction:(id)sender;
- (void)enableApply:(BOOL)enable;
@end

#endif /* AP_COCOADIALOG_FORMATOC_H */
