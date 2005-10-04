/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
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

#ifndef XAP_COCOADIALOG_IMAGE_H
#define XAP_COCOADIALOG_IMAGE_H

#include "xap_Dlg_Image.h"

class XAP_CocoaFrame;
@class XAP_CocoaDialog_ImageController;

/*****************************************************************/

class XAP_CocoaDialog_Image: public XAP_Dialog_Image
{
public:
	XAP_CocoaDialog_Image(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~XAP_CocoaDialog_Image(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	
	void	event_Ok ();
	void	event_Cancel ();
	void	aspectCheckbox();
	void	doWidthSpin(bool bIncrement);
	void	doHeightSpin(bool bIncrement);
	void	doHeightEntry(void);
	void	setHeightEntry(void);
	void	setWidthEntry(void);
	void	doWidthEntry(void);
	void	adjustHeightForAspect(void);
	void	adjustWidthForAspect(void);

private:
	double		m_dHeightWidth;

	XAP_CocoaDialog_ImageController *m_dlg;
};


@interface XAP_CocoaDialog_ImageController : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSFormCell *		_titleCell;
	IBOutlet NSFormCell *		_descriptionCell;
	IBOutlet NSFormCell *		_widthCell;
	IBOutlet NSFormCell *		_heightCell;

	IBOutlet NSStepper *		_widthNumStepper;
	IBOutlet NSStepper *		_heightNumStepper;

	IBOutlet NSButton *			_preserveAspectBtn;

	IBOutlet NSTextField *		_textWrapLabel;
	IBOutlet NSMatrix *			_textWrapMatrix;
	IBOutlet NSButtonCell *		_textWrapInline;
	IBOutlet NSButtonCell *		_textWrapRight;
	IBOutlet NSButtonCell *		_textWrapLeft;
	IBOutlet NSButtonCell *		_textWrapBoth;

	IBOutlet NSTextField *		_imagePlaceLabel;
	IBOutlet NSMatrix *			_imagePlaceMatrix;
	IBOutlet NSButtonCell *		_imagePlaceNearest;
	IBOutlet NSButtonCell *		_imagePlaceColumn;
	IBOutlet NSButtonCell *		_imagePlacePage;

	IBOutlet NSButton *			_cancelBtn;
	IBOutlet NSButton *			_okBtn;

	XAP_CocoaDialog_Image *		_xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)heightChanged:(id)sender;
- (IBAction)heightNumStepperChanged:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)preserveAction:(id)sender;
- (IBAction)widthChanged:(id)sender;
- (IBAction)widthNumStepperChanged:(id)sender;
- (IBAction)wrapAction:(id)sender;

- (NSString*)titleEntry;
- (NSString*)altEntry;
- (NSString*)widthEntry;
- (void)setWidthEntry:(NSString*)entry;
- (NSString*)heightEntry;
- (void)setHeightEntry:(NSString*)entry;
- (BOOL)preserveRatio;
- (void)setPreserveRatio:(BOOL)val;

- (void)setTextWrap:(WRAPPING_TYPE)textWrap isEnabled:(BOOL)enabled;
- (WRAPPING_TYPE)textWrap;

- (void)setImagePlacement:(POSITION_TO)imagePlacement isEnabled:(BOOL)enabled;
- (POSITION_TO)imagePlacement;
@end

#endif /* XAP_COCOADIALOG_IMAGE_H */
