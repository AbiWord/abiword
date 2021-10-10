/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003, 2005, 2009, 2011 Hubert Figuiere
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

#include <stdlib.h>

#include <string>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "gr_CocoaGraphics.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fv_View.h"
#include "pd_Style.h"
#include "ut_string_class.h"

#include "ap_CocoaDialog_Styles.h"

@interface AP_CocoaDialog_StylesController
    : NSWindowController <XAP_CocoaDialogProtocol, NSTableViewDelegate>
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
- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)editor;
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
@property (readonly) XAP_CocoaNSView* preview;
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


XAP_Dialog * AP_CocoaDialog_Styles::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Styles * p = new AP_CocoaDialog_Styles(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_Styles::AP_CocoaDialog_Styles(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
  : AP_Dialog_Styles(pDlgFactory, dlgid), 
	m_pParaPreviewWidget(NULL),
	m_pCharPreviewWidget(NULL),
	m_pAbiPreviewWidget(NULL),
	m_whichRow(-1), 
	m_whichType(AP_CocoaDialog_Styles::USED_STYLES)
{
}

AP_CocoaDialog_Styles::~AP_CocoaDialog_Styles(void)
{
	DELETEP (m_pParaPreviewWidget);
	DELETEP (m_pCharPreviewWidget);
	DELETEP (m_pAbiPreviewWidget);
}

/*****************************************************************/
void AP_CocoaDialog_Styles::runModal(XAP_Frame * pFrame)
{

//
// Get View and Document pointers. Place them in member variables
//

	setFrame(pFrame);
	setView((FV_View *) pFrame->getCurrentView());
	UT_ASSERT(getView());

	setDoc(getView()->getLayout()->getDocument());

	UT_ASSERT(getDoc());

	m_dlg = [[AP_CocoaDialog_StylesController alloc] initFromNib];
	[m_dlg setXAPOwner:this];
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);



    // populate the member variables for the  previews

	_populatePreviews(false);

	// make a new Cocoa GC
	DELETEP (m_pParaPreviewWidget);
	XAP_CocoaNSView * preview = m_dlg->_paraPreview;
	{
		GR_CocoaAllocInfo ai(preview);
		m_pParaPreviewWidget = (GR_CocoaGraphics*)XAP_App::getApp()->newGraphics(ai);
	}
	
        // let the widget materialize
	NSSize size = preview.frame.size;
	_createParaPreviewFromGC(m_pParaPreviewWidget,
							 (UT_uint32)rintf(size.width), (UT_uint32)rintf(size.height));
	preview.drawable = m_pParaPreview;

	// make a new Cocoa GC
	DELETEP (m_pCharPreviewWidget);
	preview = m_dlg->_charPreview;
	{
		GR_CocoaAllocInfo ai(preview);
		m_pCharPreviewWidget = (GR_CocoaGraphics*)XAP_App::getApp()->newGraphics(ai);
	}

	// let the widget materialize
	size = preview.frame.size;
	_createCharPreviewFromGC(m_pCharPreviewWidget,
							 (UT_uint32)rintf(size.width), (UT_uint32)rintf(size.height));
	preview.drawable = m_pCharPreview;
//
// Draw the previews!!
//
	// Populate the window's data items
	_populateWindowData();

	event_paraPreviewExposed();
	event_charPreviewExposed();

	[NSApp runModalForWindow:window];
	
	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	DELETEP (m_pParaPreviewWidget);
	DELETEP (m_pCharPreviewWidget);
	m_dlg = nil;
}

/*****************************************************************/

void AP_CocoaDialog_Styles::event_Apply(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Styles::a_OK;
	const gchar * szStyle = getCurrentStyle();
	if(szStyle && *szStyle)
	{
		getView()->setStyle(szStyle);
	}
}

void AP_CocoaDialog_Styles::event_Close(void)
{
	m_answer = AP_Dialog_Styles::a_CANCEL;
	[NSApp stopModal];
}


void AP_CocoaDialog_Styles::event_paraPreviewExposed(void)
{
	if(m_pParaPreview)
		m_pParaPreview->queueDraw();
}


void AP_CocoaDialog_Styles::event_charPreviewExposed(void)
{
	if(m_pCharPreview)
		event_charPreviewUpdated();
}

void AP_CocoaDialog_Styles::event_DeleteClicked(NSString* data)
{
	if (data)  {
        const char * style = [data UTF8String];

		UT_DEBUGMSG(("Hub: attempting to delete style %s\n", style));


		if (!getDoc()->removeStyle(style)) // actually remove the style
		{
			const XAP_StringSet * pSS = m_pApp->getStringSet();
			const gchar * msg = pSS->getValue (AP_STRING_ID_DLG_Styles_ErrStyleCantDelete);
		
			getFrame()->showMessageBox ((const char *)msg,
										XAP_Dialog_MessageBox::b_O,
										XAP_Dialog_MessageBox::a_OK);
			return;
		}

		getFrame()->repopulateCombos();
		_populateWindowData(); // force a refresh
		getDoc()->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
    }
}

void AP_CocoaDialog_Styles::event_NewClicked(void)
{
	setIsNew(true);
	modifyRunModal();
	/*moved to sheet did end*/
}

void AP_CocoaDialog_Styles::event_ClistClicked(int row)
{
	m_whichRow = row;

	// refresh the previews
	_populatePreviews(false);
}

void AP_CocoaDialog_Styles::event_ListFilterClicked(const StyleType which)
{
	m_whichType = which;
	_populateWindowData();
}

/*****************************************************************/
void AP_CocoaDialog_Styles::_populateCList(void)
{
	XAP_StringListDataSource* dataSource;

	size_t nStyles = getDoc()->getStyleCount();
	UT_DEBUGMSG(("DOM: we have %zu styles\n", nStyles));

	dataSource = m_dlg->m_stylesDataSource;
	[dataSource removeAllStrings];
	m_whichRow = -1;	// make sure it is no longer selected

	UT_GenericVector<PD_Style*>* pStyles = NULL;
	getDoc()->enumStyles(pStyles);
	for (UT_uint32 i = 0; i < nStyles; i++)
	{
		const PD_Style * pStyle = pStyles->getNthItem(i);

		// style has been deleted probably
		if (!pStyle)
			continue;

		const char * name = pStyle->getName();

	    // all of this is safe to do... append should take a const char **

	    if ((m_whichType == ALL_STYLES) || 
			(m_whichType == USED_STYLES && pStyle->isUsed()) ||
			(m_whichType == USER_STYLES && pStyle->isUserDefined()))
		{
			[dataSource addString:[NSString stringWithUTF8String:name]];
		}
	}
	DELETEP(pStyles);
	[m_dlg->_availStylesList reloadData];
}

void AP_CocoaDialog_Styles::_populateWindowData(void)
{
	_populateCList();
	_populatePreviews(false);
}

void AP_CocoaDialog_Styles::setDescription(const char * desc) const
{
	[m_dlg setStyleDescription:[NSString stringWithUTF8String:desc]];
}

const char * AP_CocoaDialog_Styles::getCurrentStyle (void) const
{
	if (m_whichRow < 0) {
		return NULL;
	}
	NSString * szStyle = [[m_dlg->m_stylesDataSource array] objectAtIndex:m_whichRow];

	return [szStyle UTF8String];
}


void AP_CocoaDialog_Styles::event_Modify_OK(void)
{
	// force the update, it might not have lost focus
	new_styleName();

	if (!m_newStyleName || !strlen (m_newStyleName))
    {
		// error message!
		const XAP_StringSet * pSS = m_pApp->getStringSet ();
		std::string label;
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrBlankName, label);

		getFrame()->showMessageBox (label,
			  XAP_Dialog_MessageBox::b_O,
			  XAP_Dialog_MessageBox::a_OK);

		return;
    }

	// TODO save out state of radio items
	m_answer = AP_Dialog_Styles::a_OK;
	[NSApp endSheet:[m_modifyDlg window]];
}

/*!
 * fill the properties vector with the values the given style.
 */
void AP_CocoaDialog_Styles::new_styleName(void)
{
	static char message[200];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const char * psz = [[m_modifyDlg->_styleNameData stringValue] UTF8String];
	std::string label;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefNone, label);
	if(psz && (label == psz))
	{
			// TODO: do a real error dialog
		std::string label1, label2;
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrNotTitle1, label1);
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrNotTitle2, label2);
		sprintf(message,"%s%s%s",label1.c_str(),	psz, label2.c_str());
		//FIXME
		//messageBoxOK((const char *) message);
		return;
	}
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefCurrent, label);
	if(psz && (label == psz))
	{
			// TODO: do a real error dialog
		std::string label1, label2;
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrNotTitle1, label1);
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrNotTitle2, label2);
		sprintf(message,"%s%s%s",label1.c_str(),	psz, label2.c_str());
		//FIXME
		//messageBoxOK((const char *) message);
		return;
	}

	snprintf(m_newStyleName, sizeof(m_newStyleName), "%s", psz);
	PP_addOrSetAttribute(PT_NAME_ATTRIBUTE_NAME, getNewStyleName(), m_vecAllAttribs);
}

/*!
 * Remove the property from the current style shown in the remove combo box
 */
void AP_CocoaDialog_Styles::event_RemoveProperty(void)
{
	const char * psz = [[m_modifyDlg->_removePropCombo stringValue] UTF8String];
	PP_removeAttribute(psz, m_vecAllProps);
	rebuildDeleteProps();
	updateCurrentStyle();
}

void AP_CocoaDialog_Styles::rebuildDeleteProps(void)
{
	NSComboBox * delCombo = m_modifyDlg->_removePropCombo;

	[delCombo removeAllItems];

	for (auto iter = m_vecAllProps.cbegin(); iter != m_vecAllProps.cend();
		 ++iter)
	{
		const std::string & value = *iter;
		NSString *str =  [[NSString alloc] initWithUTF8String:value.c_str()];
		[delCombo addItemWithObjectValue:str];
		[str release];
		++iter;
		if (iter == m_vecAllProps.cend()) {
			break;
		}
	}
}

/*!
 * Update the properties and Attributes vector given the new basedon name
 */
void AP_CocoaDialog_Styles::event_basedOn(void)
{
	const char * psz = [[m_modifyDlg->_basedOnCombo stringValue] UTF8String];
	snprintf((char *) m_basedonName, sizeof(m_basedonName), "%s", psz);
	PP_addOrSetAttribute("basedon", getBasedonName(), m_vecAllAttribs);
	fillVecWithProps(getBasedonName(),false);
	updateCurrentStyle();
}


/*!
 * Update the Attributes vector given the new followedby name
 */
void AP_CocoaDialog_Styles::event_followedBy(void)
{
	const char * psz = [[m_modifyDlg->_followStyleCombo stringValue] UTF8String];
	snprintf((char *) m_followedbyName, sizeof(m_followedbyName), "%s", psz);
	PP_addOrSetAttribute("followedby", getFollowedbyName(), m_vecAllAttribs);
}


/*!
 * Update the Attributes vector given the new Style Type
 */
void AP_CocoaDialog_Styles::event_styleType(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const char * psz = [[m_modifyDlg->_styleTypeCombo stringValue] UTF8String];
	snprintf((char *) m_styleType, sizeof(m_styleType), "%s", psz);
	const gchar * pszSt = "P";
	std::string label;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyCharacter, label);
	if(m_styleType != label) {
		pszSt = "C";
	}
	PP_addOrSetAttribute("type", pszSt, m_vecAllAttribs);
}

void AP_CocoaDialog_Styles::event_Modify_Cancel(void)
{
	m_answer = AP_Dialog_Styles::a_CANCEL;
	[NSApp endSheet:[m_modifyDlg window]];
}


void  AP_CocoaDialog_Styles::modifyRunModal(void)
{
	m_modifyDlg = [[AP_CocoaDialog_StylesModifyController alloc] initFromNib];
	[m_modifyDlg setXAPOwner:this];
	NSWindow* window = m_modifyDlg.window;
	UT_ASSERT(window);

	if (_populateModify()) {
		// make a new Cocoa GC
		DELETEP (m_pAbiPreviewWidget);
		XAP_CocoaNSView* preview = m_modifyDlg.preview;
		GR_CocoaAllocInfo ai(preview);
		m_pAbiPreviewWidget = (GR_CocoaGraphics*)XAP_App::getApp()->newGraphics(ai);

		
			// let the widget materialize
	
		NSSize size =  preview.frame.size;
		_createAbiPreviewFromGC(m_pAbiPreviewWidget,
								(UT_uint32)rintf(size.width), (UT_uint32)rintf(size.height));
		preview.drawable = m_pAbiPreview;
		_populateAbiPreview(isNew());
		event_ModifyPreviewExposed();
	
		[NSApp beginSheet:window modalForWindow:m_dlg.window modalDelegate:m_modifyDlg
				didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:) contextInfo:nil];
	}
}

void AP_CocoaDialog_Styles::event_modifySheetDidEnd(int /*code*/)
{
	if(m_answer == AP_Dialog_Styles::a_OK)
	{
		if (isNew()) {
			createNewStyle(getNewStyleName());
			_populateCList();
		}
		else {
			applyModifiedStyleToDoc();
			getDoc()->updateDocForStyleChange(getCurrentStyle(),true);
			getDoc()->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
		}
	}
	else
	{
//
// Do other stuff
//
	}

	destroyAbiPreview();
	DELETEP(m_pAbiPreviewWidget);
	
	[m_modifyDlg close];
	[m_modifyDlg release];
}

void AP_CocoaDialog_Styles::event_ModifyPreviewExposed(void)
{
	invalidatePreview();
}

void AP_CocoaDialog_Styles::event_ModifyClicked(void)
{
	PD_Style * pStyle = NULL;
	const char * szCurrentStyle = getCurrentStyle ();
	
	if(szCurrentStyle)
		getDoc()->getStyle(szCurrentStyle, &pStyle);
	
	if (!pStyle)
	{
		// TODO: error message - nothing selected
		return;
	}
//
// Allow built-ins to be modified
//
#if 0
	if (!pStyle->isUserDefined ())
	{
		// can't change builtin, error message
		const XAP_StringSet * pSS = m_pApp->getStringSet();
		const gchar * msg = pSS->getValue (AP_STRING_ID_DLG_Styles_ErrStyleBuiltin);
		
		getFrame()->showMessageBox ((const char *)msg,
									XAP_Dialog_MessageBox::b_O,
									XAP_Dialog_MessageBox::a_OK);
		return;
	}	
#endif
	
//
// fill the data structures needed for the Modify dialog
//
	setIsNew(false);
	
	modifyRunModal();
	/*moved to sheetDidEnd */
//  
// Restore the values in the main dialog
//
}

void  AP_CocoaDialog_Styles::setModifyDescription( const char * desc)
{
	[m_dlg setStyleDescription:[NSString stringWithUTF8String:desc]];
}

bool  AP_CocoaDialog_Styles::_populateModify(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	setModifyDescription( m_curStyleDesc.c_str());
//
// Get Style name and put in in the text entry
//
	const char * szCurrentStyle = NULL;
	if(!isNew())
	{
		szCurrentStyle= getCurrentStyle();
		if(!szCurrentStyle)
		{
			// TODO: change me to use a real messagebox
			//messageBoxOK( pSS->getValue(AP_STRING_ID_DLG_Styles_ErrNoStyle));
			m_answer = AP_Dialog_Styles::a_CANCEL;
			return false;
		}
		[m_modifyDlg->_styleNameData setStringValue:[NSString stringWithUTF8String:getCurrentStyle()]];
		[m_modifyDlg->_styleNameData setEditable:NO];
	}
	else {
		[m_modifyDlg->_styleNameData setEditable:YES];
	}
//
// Next interogate the current style and find the based on and followed by
// Styles
//
	const char * szBasedOn = NULL;
	const char * szFollowedBy = NULL;
	PD_Style * pBasedOnStyle = NULL;
	PD_Style * pFollowedByStyle = NULL;
	if(!isNew())
	{
		PD_Style * pStyle = NULL;
		if(szCurrentStyle)
			getDoc()->getStyle(szCurrentStyle,&pStyle);
		if(!pStyle)
		{
			// TODO: do a real error dialog
//			messageBoxOK( pSS->getValue(AP_STRING_ID_DLG_Styles_ErrStyleNot));
			m_answer = AP_Dialog_Styles::a_CANCEL;
			return false;
		}
//
// Valid style get the Based On and followed by values
//
	    pBasedOnStyle = pStyle->getBasedOn();
		pFollowedByStyle = pStyle->getFollowedBy();
	}
//
// Next make a glists of all styles and attach them to the BasedOn and FollowedBy
//
	size_t nStyles = getDoc()->getStyleCount();
	
	[m_modifyDlg->_basedOnCombo removeAllItems];
	[m_modifyDlg->_followStyleCombo removeAllItems];
	[m_modifyDlg->_styleTypeCombo removeAllItems];

	UT_GenericVector<PD_Style*>* pStyles = NULL;
	getDoc()->enumStyles(pStyles);
	for (UT_uint32 i = 0; i < nStyles; i++)
	{
		const char * name = NULL;
		const PD_Style * pcStyle = pStyles->getNthItem(i);

		if(pcStyle) {
			name = pcStyle->getName();
		}
		if(pBasedOnStyle && pcStyle == pBasedOnStyle)
		{
			szBasedOn = name;
		}
		if(pFollowedByStyle && pcStyle == pFollowedByStyle)
			szFollowedBy = name;
		if(szCurrentStyle && strcmp(name,szCurrentStyle) != 0) {
			[m_modifyDlg->_basedOnCombo addItemWithObjectValue:[NSString stringWithUTF8String:name]];
		}
		else if(szCurrentStyle == NULL) {
			[m_modifyDlg->_basedOnCombo addItemWithObjectValue:[NSString stringWithUTF8String:name]];
		}
		[m_modifyDlg->_followStyleCombo addItemWithObjectValue:[NSString stringWithUTF8String:name]];
	}
	DELETEP(pStyles);

	[m_modifyDlg->_followStyleCombo addItemWithObjectValue:LocalizedString(pSS, AP_STRING_ID_DLG_Styles_DefCurrent)];
	[m_modifyDlg->_basedOnCombo addItemWithObjectValue:LocalizedString(pSS, AP_STRING_ID_DLG_Styles_DefNone)];
	[m_modifyDlg->_styleTypeCombo addItemWithObjectValue:LocalizedString(pSS, AP_STRING_ID_DLG_Styles_ModifyParagraph)];
	[m_modifyDlg->_styleTypeCombo addItemWithObjectValue:LocalizedString(pSS, AP_STRING_ID_DLG_Styles_ModifyCharacter)];
 
//
// Set the popdown list
//
	if(isNew())
	{
//		gtk_combo_set_popdown_strings( GTK_COMBO(m_wStyleTypeCombo),m_gStyleType);
	}
//
// OK here we set intial values for the basedOn and followedBy
//
	if(!isNew())
	{
		if(pBasedOnStyle != NULL)
			[m_modifyDlg->_basedOnCombo setStringValue:[NSString stringWithUTF8String:szBasedOn]];
		else
			[m_modifyDlg->_basedOnCombo setStringValue:LocalizedString(pSS, AP_STRING_ID_DLG_Styles_DefNone)];
		if(pFollowedByStyle != NULL)
			[m_modifyDlg->_followStyleCombo setStringValue:[NSString stringWithUTF8String:szFollowedBy]];
		else
			[m_modifyDlg->_followStyleCombo setStringValue:LocalizedString(pSS, AP_STRING_ID_DLG_Styles_DefCurrent)];
		const std::string & type = PP_getAttribute("type", m_vecAllAttribs);
		if(type.find("P") != std::string::npos)
		{
			[m_modifyDlg->_styleTypeCombo setStringValue:
								LocalizedString(pSS, AP_STRING_ID_DLG_Styles_ModifyParagraph)];
		}
		else
		{
			[m_modifyDlg->_styleTypeCombo setStringValue:
								LocalizedString(pSS, AP_STRING_ID_DLG_Styles_ModifyCharacter)];
		}
	}
	else
	{
//
// Hardwire defaults for "new"
//
		[m_modifyDlg->_basedOnCombo setStringValue:LocalizedString(pSS, AP_STRING_ID_DLG_Styles_DefNone)];
		[m_modifyDlg->_followStyleCombo setStringValue:LocalizedString(pSS, AP_STRING_ID_DLG_Styles_DefCurrent)];
		[m_modifyDlg->_styleTypeCombo setStringValue:
								LocalizedString(pSS, AP_STRING_ID_DLG_Styles_ModifyParagraph)];
	}

//
// Set these in our attributes vector
//
	event_basedOn();
	event_followedBy();
	event_styleType();
	if(isNew())
	{
		fillVecFromCurrentPoint();
	}
	else
	{
		fillVecWithProps(szCurrentStyle,true);
	}
//
// Now set the list of properties which can be deleted.
//
	rebuildDeleteProps();
	return true;
}

void   AP_CocoaDialog_Styles::event_ModifyParagraph()
{
//
// Can do all this in XP land.
//
	ModifyParagraph();
	rebuildDeleteProps();

//
// This applies the changes to current style and displays them
//
	updateCurrentStyle();
}

void   AP_CocoaDialog_Styles::event_ModifyFont()
{
//
// Can do all this in XP land.
//
	ModifyFont();
	rebuildDeleteProps();
//
// This applies the changes to current style and displays them
//
	updateCurrentStyle();
}

void AP_CocoaDialog_Styles::event_ModifyLanguage()
{
	ModifyLang();
	rebuildDeleteProps();

	updateCurrentStyle();
}

void   AP_CocoaDialog_Styles::event_ModifyNumbering()
{

//
// Can do all this in XP land.
//
	ModifyLists();
	rebuildDeleteProps();

//
// This applies the changes to current style and displays them
//
	updateCurrentStyle();

}


void   AP_CocoaDialog_Styles::event_ModifyTabs()
{
//
// Can do all this in XP land.
//
	ModifyTabs();
	rebuildDeleteProps();

//
// This applies the changes to current style and displays them
//
	updateCurrentStyle();
}


@implementation AP_CocoaDialog_StylesController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_Styles"];
}

-(void)discardXAP
{
	_xap = NULL;
}

-(void)dealloc
{
	[m_stylesDataSource release];
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_Styles*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Styles_StylesTitle);
		LocalizeControl(_applyBtn, pSS, XAP_STRING_ID_DLG_Apply);
		LocalizeControl(_closeBtn, pSS, XAP_STRING_ID_DLG_Close);
		LocalizeControl(_availStylesBox, pSS, AP_STRING_ID_DLG_Styles_List);
		[_listCombo removeAllItems];
		AppendLocalizedMenuItem(_listCombo, pSS, AP_STRING_ID_DLG_Styles_LBL_InUse, AP_CocoaDialog_Styles::USED_STYLES);
		AppendLocalizedMenuItem(_listCombo, pSS, AP_STRING_ID_DLG_Styles_LBL_All, AP_CocoaDialog_Styles::ALL_STYLES);
		AppendLocalizedMenuItem(_listCombo, pSS, AP_STRING_ID_DLG_Styles_LBL_UserDefined, AP_CocoaDialog_Styles::USER_STYLES);
		LocalizeControl(_paraPreviewBox, pSS, AP_STRING_ID_DLG_Styles_ParaPrev);
		LocalizeControl(_charPreviewBox, pSS, AP_STRING_ID_DLG_Styles_CharPrev);
		LocalizeControl(_descriptionBox, pSS, AP_STRING_ID_DLG_Styles_Description);
		LocalizeControl(_newBtn, pSS, AP_STRING_ID_DLG_Styles_New);
		LocalizeControl(_modifyBtn, pSS, AP_STRING_ID_DLG_Styles_Modify);
		LocalizeControl(_deleteBtn, pSS, AP_STRING_ID_DLG_Styles_Delete);
		
		m_stylesDataSource = [[XAP_StringListDataSource alloc] init];
		[_availStylesList setDataSource:m_stylesDataSource];
		[_availStylesList setDelegate:self];
		[_availStylesList setAction:@selector(listSelectedAction:)];
		[_availStylesList setTarget:self];
	}
}


- (IBAction)applyAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Apply();
}

- (IBAction)closeAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Close();
}

- (IBAction)deleteAction:(id)sender
{
	UT_UNUSED(sender);
	NSString *str;
	NSInteger row = [_availStylesList selectedRow];
	if (row >= 0) {
		str = [[m_stylesDataSource array] objectAtIndex:row];
		_xap->event_DeleteClicked(str);
	}
}

- (IBAction)listSelectedAction:(id)sender
{
	_xap->event_ClistClicked([sender selectedRow]);
}

- (IBAction)listFilterSelectedAction:(id)sender
{
	_xap->event_ListFilterClicked(static_cast<AP_CocoaDialog_Styles::StyleType>([[sender selectedItem] tag]));
}

- (IBAction)modifyAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_ModifyClicked();
}

- (IBAction)newAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_NewClicked();
}


- (void)setStyleDescription:(NSString*)desc
{
	[_descriptionData setStringValue:desc];
}

- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)editor
{
	UT_UNUSED(editor);
	if(control == _availStylesList) {
		return NO;
	}
	return YES;
}
@end


@implementation AP_CocoaDialog_StylesModifyController

@synthesize preview = _preview;

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_StylesModify"];
}

-(void)discardXAP
{
	if (_xap) {
		_xap = NULL;
	}
}

-(void)dealloc
{
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_Styles*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_shortcutBtn, pSS, AP_STRING_ID_DLG_Styles_ModifyShortCut);

		LocalizeControl(_styleNameLabel, pSS, AP_STRING_ID_DLG_Styles_ModifyName);
		LocalizeControl(_styleTypeLabel, pSS, AP_STRING_ID_DLG_Styles_ModifyType);
		LocalizeControl(_basedOnLabel, pSS, AP_STRING_ID_DLG_Styles_ModifyBasedOn);
		LocalizeControl(_followStyleLabel, pSS, AP_STRING_ID_DLG_Styles_ModifyFollowing);
		LocalizeControl(_previewBox, pSS, AP_STRING_ID_DLG_Styles_ModifyPreview);
		LocalizeControl(_descBox, pSS, AP_STRING_ID_DLG_Styles_ModifyDescription);
		LocalizeControl(_removePropLabel, pSS, AP_STRING_ID_DLG_Styles_RemoveLab);
		LocalizeControl(_removeBtn, pSS, AP_STRING_ID_DLG_Styles_RemoveButton);
/*
	if(isNew())
	{
		styleTypeCombo = gtk_combo_new ();
		gtk_widget_show (styleTypeCombo);
		gtk_table_attach (GTK_TABLE (comboTable), styleTypeCombo, 1, 2, 1, 2,
						  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						  (GtkAttachOptions) (0), 0, 0);

		styleTypeEntry = GTK_COMBO (styleTypeCombo)->entry;
		gtk_widget_show (styleTypeEntry);
		gtk_widget_set_usize (styleTypeEntry, 158, -2);
	}
	else
	{
		styleTypeEntry = gtk_entry_new ();
		gtk_widget_show (styleTypeEntry);
		gtk_table_attach (GTK_TABLE (comboTable), styleTypeEntry, 1, 2, 1, 2,
						  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						  (GtkAttachOptions) (0), 0, 0);
		gtk_widget_set_usize (styleTypeEntry, 158, -2);
	}

*/
		//LocalizeControl(_styleNameLabel, pSS, AP_STRING_ID_DLG_Styles_ModifyTemplate);
		//LocalizeControl(_styleNameLabel, pSS, AP_STRING_ID_DLG_Styles_ModifyAutomatic);
		[_formatPopupBtn removeAllItems];
		AppendLocalizedMenuItem(_formatPopupBtn, pSS, AP_STRING_ID_DLG_Styles_ModifyFormat, 0);
		AppendLocalizedMenuItem(_formatPopupBtn, pSS, AP_STRING_ID_DLG_Styles_ModifyParagraph, 1);
		AppendLocalizedMenuItem(_formatPopupBtn, pSS, AP_STRING_ID_DLG_Styles_ModifyFont, 2);
		AppendLocalizedMenuItem(_formatPopupBtn, pSS, AP_STRING_ID_DLG_Styles_ModifyTabs, 3);
		AppendLocalizedMenuItem(_formatPopupBtn, pSS, AP_STRING_ID_DLG_Styles_ModifyNumbering, 4);
		AppendLocalizedMenuItem(_formatPopupBtn, pSS, AP_STRING_ID_DLG_Styles_ModifyLanguage, 5);
	}
}


- (IBAction)basedOnAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_basedOn();
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Modify_Cancel();
}

- (IBAction)followStyleAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_followedBy();
}

- (IBAction)formatAction:(id)sender
{
	int choice = [[sender selectedItem] tag];
	switch (choice) {
	case 1:
		_xap->event_ModifyParagraph();
		break;
	case 2:
		_xap->event_ModifyFont();
		break;
	case 3:
		_xap->event_ModifyTabs();
		break;
	case 4:
		_xap->event_ModifyNumbering();
		break;
	case 5:
		_xap->event_ModifyLanguage();
		break;
	default:
		break;
	}
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Modify_OK();
}

- (IBAction)removeAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_RemoveProperty();
}

- (IBAction)shortcutAction:(id)sender
{
	UT_UNUSED(sender);
}

- (IBAction)styleNameAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->new_styleName();
}

- (IBAction)styleTypeAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_styleType();
}


- (void)sheetDidEnd:(NSWindow*)sheet returnCode:(int)returnCode contextInfo:(void  *)c
{
	UT_UNUSED(c);
	[sheet orderOut:self];
	_xap->event_modifySheetDidEnd(returnCode);	
}


@end





