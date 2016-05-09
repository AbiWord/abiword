/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_CocoaDialog_Field.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_Field::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Field * p = new AP_CocoaDialog_Field(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_Field::AP_CocoaDialog_Field(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) : 
	AP_Dialog_Field(pDlgFactory, dlgid),
	m_typeList(nil),
	m_fieldList(nil),
	m_dlg(nil)
{
	// 
}

AP_CocoaDialog_Field::~AP_CocoaDialog_Field(void)
{
	// 
}

/*****************************************************************/

void AP_CocoaDialog_Field::runModal(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_FieldController alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];

	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);

	m_fieldList = [[XAP_StringListDataSource alloc] init];
	m_typeList  = [[XAP_StringListDataSource alloc] init];

	_populateCategories();

	[m_dlg setTypeList:m_typeList andFieldList:m_fieldList];

	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;

	[m_typeList release];
	[m_fieldList release];
}

void AP_CocoaDialog_Field::event_OK(void)
{
	int idx;

	idx = [m_dlg selectedType];

	// if there is no selection
	// is empty, return cancel.  GTK can make this happen.
	if (idx == -1)
	{
		m_answer = AP_Dialog_Field::a_CANCEL;
		[NSApp stopModal];
		return;
	}
	m_iTypeIndex = idx;
	
	// find item selected in the Field list box, save it to m_iFormatIndex
	idx = [m_dlg selectedField];

	if (idx == -1)
	{
		m_answer = AP_Dialog_Field::a_CANCEL;
		[NSApp stopModal];
		return;
	}

	m_iFormatIndex = 0;

	fp_FieldTypesEnum FType = fp_FieldTypes[m_iTypeIndex].m_Type;

	for (int i = 0; fp_FieldFmts[i].m_Tag != NULL; i++) {
		if((fp_FieldFmts[i].m_Num != FPFIELD_endnote_anch ) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_endnote_ref  ) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_footnote_anch) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_footnote_ref ))
			{ 
				if (fp_FieldFmts[i].m_Type == FType) {
					if (!idx) {
						m_iFormatIndex = i;
						break;
					}
					--idx;
			}
		}
	}

	setParameter([[m_dlg extraParam] UTF8String]);

	m_answer = AP_Dialog_Field::a_OK;
	[NSApp stopModal];
}

void AP_CocoaDialog_Field::event_Cancel(void)
{
	m_answer = AP_Dialog_Field::a_CANCEL;
	[NSApp stopModal];
}

void AP_CocoaDialog_Field::types_changed(int row)
{
	// Update m_iTypeIndex with the row number
	m_iTypeIndex = row;

	// Update the fields list with this new Type
	setFieldsList();
}

void AP_CocoaDialog_Field::_populateCategories(void)
{
	// Fill in the two lists
	setTypesList();
	setFieldsList();
}

void AP_CocoaDialog_Field::setTypesList(void)
{
	UT_ASSERT(m_typeList);

	[m_typeList removeAllStrings];

	for (int i = 0; fp_FieldTypes[i].m_Desc != NULL; i++) {
		[m_typeList addString:[NSString stringWithUTF8String:fp_FieldTypes[i].m_Desc]];
	}
	
	m_iTypeIndex = 0;
}

void AP_CocoaDialog_Field::setFieldsList(void)
{
	UT_ASSERT(m_fieldList);

	fp_FieldTypesEnum FType = fp_FieldTypes[m_iTypeIndex].m_Type;

	[m_fieldList removeAllStrings];

	for (int i = 0; fp_FieldFmts[i].m_Tag != NULL; i++) {
		if((fp_FieldFmts[i].m_Num != FPFIELD_endnote_anch ) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_endnote_ref  ) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_footnote_anch) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_footnote_ref ))
			{ 
				if (fp_FieldFmts[i].m_Type == FType) {
					[m_fieldList addString:[NSString stringWithUTF8String:fp_FieldFmts[i].m_Desc]];
			}
		}
	}
}

/*****************************************************************/

@implementation AP_CocoaDialog_FieldController

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_Field"]) {
		_xap = NULL;
	}
	return self;
}

-(void)dealloc
{
	// 
	[super dealloc];
}

-(void)discardXAP
{
	_xap = 0;
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = static_cast<AP_CocoaDialog_Field *>(owner);
}

-(void)windowDidLoad
{
	if (_xap)
		{
			const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

			LocalizeControl([self window],    pSS, AP_STRING_ID_DLG_Field_FieldTitle);

			LocalizeControl(_okBtn,           pSS, XAP_STRING_ID_DLG_OK);
			LocalizeControl(_cancelBtn,       pSS, XAP_STRING_ID_DLG_Cancel);

			LocalizeControl(_typesLabel,      pSS,  AP_STRING_ID_DLG_Field_Types);
			LocalizeControl(_fieldsLabel,     pSS,  AP_STRING_ID_DLG_Field_Fields);
			LocalizeControl(_extraParamLabel, pSS,  AP_STRING_ID_DLG_Field_Parameters);

			[_typesList  setDelegate:self];
			[_typesList  setAction:@selector(typesAction:)];
			[_fieldsList setDoubleAction:@selector(okAction:)];
		}
}

- (int)selectedType
{
	return [_typesList selectedRow];
}

- (int)selectedField
{
	return [_fieldsList selectedRow];
}

- (void)setTypeList:(XAP_StringListDataSource *)tl andFieldList:(XAP_StringListDataSource *)fl
{
	[_fieldsList setDataSource:fl];
	[_typesList  setDataSource:tl];
}

- (NSString *)extraParam
{
	return [_extraParamData stringValue];
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	_xap->types_changed([_typesList selectedRow]);
	[_fieldsList reloadData];
}

- (void)typesAction:(id)sender
{
	UT_UNUSED(sender);
	// _xap->types_changed([_typesList selectedRow]);
	// [_fieldsList reloadData];
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_OK();
}

@end
