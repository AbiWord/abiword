/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001,2003 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_PARAGRAPH_H
#define AP_COCOADIALOG_PARAGRAPH_H

#include "ap_Dialog_Paragraph.h"

class GR_CocoaCairoGraphics;
class XAP_CocoaFrame;
class AP_CocoaDialog_Paragraph;

@interface AP_CocoaDialog_ParagraphController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSTextField *_alignmentLabel;
    IBOutlet NSPopUpButton *_alignmentPopup;
    IBOutlet NSTextField *_atData;
    IBOutlet NSTextField *_atLabel;
    IBOutlet NSTextField *_byData;
    IBOutlet NSTextField *_byLabel;
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSButton *_dontHyphenBtn;
    IBOutlet NSBox *_indentationBox;
    IBOutlet NSFormCell *_indentLeftFormCell;
    IBOutlet NSFormCell *_indentRightFormCell;
    IBOutlet NSButton *_keepNextBtn;
    IBOutlet NSButton *_keepsLinesBtn;
    IBOutlet NSTextField *_lineSpacingLabel;
    IBOutlet NSPopUpButton *_lineSpacingPopup;
    IBOutlet NSTabView *_mainTab;
    IBOutlet NSButton *_okBtn;
    IBOutlet NSButton *_pageBreakBtn;
    IBOutlet NSBox *_paginationBox;
    IBOutlet XAP_CocoaNSView *_preview;
    IBOutlet NSBox *_previewBox;
    IBOutlet NSButton *_rtlDominantBtn;
    IBOutlet NSFormCell *_spacingAfterFormCell;
    IBOutlet NSFormCell *_spacingBeforeFormCell;
    IBOutlet NSBox *_spacingBox;
    IBOutlet NSTextField *_specialLabel;
    IBOutlet NSPopUpButton *_specialPopup;
    IBOutlet NSButton *_suppressLineNumBtn;
    IBOutlet NSButton *_tabsBtn;
    IBOutlet NSButton *_widowOrphanBtn;
	AP_CocoaDialog_Paragraph* _xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)tabAction:(id)sender;
- (IBAction)checkBoxAction:(id)sender;
- (IBAction)menuAction:(id)sender;
- (IBAction)editAction:(id)sender;
- (NSPopUpButton*)specialPopup;
- (NSPopUpButton*)lineSpacingPopup;
- (NSPopUpButton*)alignmentPopup;
- (XAP_CocoaNSView*)preview;
- (id)_getWidget:(AP_Dialog_Paragraph::tControl) widget;
@end

/*****************************************************************/

class AP_CocoaDialog_Paragraph: public AP_Dialog_Paragraph
{
public:
	AP_CocoaDialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_CocoaDialog_Paragraph(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void event_OK(void);
	virtual void event_Cancel(void);
	virtual void event_Tabs(void);
	virtual void event_MenuChanged(id sender);
	virtual void event_EditChanged(id sender);
	virtual void event_CheckToggled(id sender);
	virtual void event_PreviewAreaExposed(void);

#if 0
	// all data controls are of three types in this dialog; the static
	// functions pass in widget pointers which are mapped into
	// base class "tControl" IDs for data operations.

		// menus take a "changed" event

		// spin buttons can take "increment", "decrement", and "changed"
		virtual void event_SpinIncrement(GtkWidget * widget);
		virtual void event_SpinDecrement(GtkWidget * widget);
		virtual void event_SpinFocusOut(GtkWidget * widget);

		// checks are just "toggled"

	// Preview
#endif
void	_createGC(XAP_CocoaNSView* owner);
void 	_deleteGC(void);
 protected:

	GR_CocoaCairoGraphics	* 		m_pGraphics;

	void                 _populateWindowData(void);

	virtual void         _syncControls(tControl changed, bool bAll = false);
private:
	int _tCheckStateToNS(AP_CocoaDialog_Paragraph::tCheckState x);
	id _getWidget(AP_Dialog_Paragraph::tControl widget)
	{
		return [m_dlg _getWidget:widget];
	};

	AP_CocoaDialog_ParagraphController* m_dlg;
};

#endif /* XAP_COCOADIALOG_PARAGRAPH_H */
