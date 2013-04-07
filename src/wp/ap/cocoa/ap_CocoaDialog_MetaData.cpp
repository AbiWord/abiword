/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#import <Cocoa/Cocoa.h>

#include <stdlib.h>
#include <time.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MetaData.h"
#include "ap_CocoaDialog_MetaData.h"

#define SET_ENTRY_TXT(name) { \
  UT_UTF8String prop(get##name ()) ; \
  if ( prop.size () > 0 ) { \
    [m_dlg setGUI##name:prop] ; \
  }}
  
#define GRAB_ENTRY_TEXT(name) { \
	NSString* str;\
	str = [m_dlg GUI##name] ; \
	if(str){ \
	set##name ( [str UTF8String] );} \
}

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_MetaData::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_MetaData * p = new AP_CocoaDialog_MetaData(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_MetaData::AP_CocoaDialog_MetaData(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_MetaData(pDlgFactory,dlgid)
{
}

AP_CocoaDialog_MetaData::~AP_CocoaDialog_MetaData(void)
{
}

void AP_CocoaDialog_MetaData::runModal(XAP_Frame * /*pFrame*/)
{
	NSWindow* window;
	//	UT_ASSERT(pFrame);
	m_dlg = [[AP_CocoaDialog_MetaDataController alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	window = [m_dlg window];

	SET_ENTRY_TXT(Title);
	SET_ENTRY_TXT(Subject);
	SET_ENTRY_TXT(Author);
	SET_ENTRY_TXT(Publisher);
	SET_ENTRY_TXT(CoAuthor);
	SET_ENTRY_TXT(Category);
	SET_ENTRY_TXT(Keywords);
	SET_ENTRY_TXT(Languages);
	SET_ENTRY_TXT(Source);
	SET_ENTRY_TXT(Relation);
	SET_ENTRY_TXT(Coverage);
	SET_ENTRY_TXT(Rights);
	SET_ENTRY_TXT(Description);
	
	[NSApp runModalForWindow:window];

	[m_dlg close];
	[m_dlg release];
}

void AP_CocoaDialog_MetaData::okAction(void)
{
	setAnswer(AP_Dialog_MetaData::a_OK);

	GRAB_ENTRY_TEXT(Title);
	GRAB_ENTRY_TEXT(Subject);
	GRAB_ENTRY_TEXT(Author);
	GRAB_ENTRY_TEXT(Publisher);  
	GRAB_ENTRY_TEXT(CoAuthor);
	GRAB_ENTRY_TEXT(Category);
	GRAB_ENTRY_TEXT(Keywords);
	GRAB_ENTRY_TEXT(Languages);
	GRAB_ENTRY_TEXT(Source);
	GRAB_ENTRY_TEXT(Relation);
	GRAB_ENTRY_TEXT(Coverage);
	GRAB_ENTRY_TEXT(Rights);
	GRAB_ENTRY_TEXT(Description);

	[NSApp stopModal];
}


void AP_CocoaDialog_MetaData::cancelAction(void)
{
	setAnswer(AP_Dialog_MetaData::a_CANCEL);
	[NSApp stopModal];
}


@implementation AP_CocoaDialog_MetaDataController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_MetaData"];	
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<AP_CocoaDialog_MetaData*>(owner);
	UT_ASSERT(_xap);
}


- (void)discardXAP
{
	_xap = nil;
}


- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	LocalizeControl([self window], pSS, AP_STRING_ID_DLG_MetaData_Title);
	
	LocalizeControl(m_titleLabel, pSS, AP_STRING_ID_DLG_MetaData_Title_LBL);
	LocalizeControl(m_subjectLabel, pSS, AP_STRING_ID_DLG_MetaData_Subject_LBL);
	LocalizeControl(m_authorLabel, pSS, AP_STRING_ID_DLG_MetaData_Author_LBL);
	LocalizeControl(m_publisherLabel, pSS, AP_STRING_ID_DLG_MetaData_Publisher_LBL);
	LocalizeControl(m_coAuthorLabel, pSS, AP_STRING_ID_DLG_MetaData_CoAuthor_LBL);
	LocalizeControl(m_categoryLabel, pSS, AP_STRING_ID_DLG_MetaData_Category_LBL);
	LocalizeControl(m_keywordLabel, pSS, AP_STRING_ID_DLG_MetaData_Keywords_LBL);
	LocalizeControl(m_languageLabel, pSS, AP_STRING_ID_DLG_MetaData_Languages_LBL);
	LocalizeControl(m_descriptionLabel, pSS, AP_STRING_ID_DLG_MetaData_Description_LBL);
	LocalizeControl(m_sourceLabel, pSS, AP_STRING_ID_DLG_MetaData_Source_LBL);
	LocalizeControl(m_relationLabel, pSS, AP_STRING_ID_DLG_MetaData_Relation_LBL);
	LocalizeControl(m_coverageLabel, pSS, AP_STRING_ID_DLG_MetaData_Coverage_LBL);
	LocalizeControl(m_rightsLabel, pSS, AP_STRING_ID_DLG_MetaData_Rights_LBL);
	LocalizeControl([m_tabs tabViewItemAtIndex:0], pSS, AP_STRING_ID_DLG_MetaData_TAB_General);
	LocalizeControl([m_tabs tabViewItemAtIndex:1], pSS, AP_STRING_ID_DLG_MetaData_TAB_Summary);
	LocalizeControl([m_tabs tabViewItemAtIndex:2], pSS, AP_STRING_ID_DLG_MetaData_TAB_Permission);
}

- (IBAction)cancelBtnAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->cancelAction();
}


- (IBAction)okBtnAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->okAction();
}


- (NSString*)GUITitle
{
	return [m_titleData stringValue];
}

- (void)setGUITitle:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_titleData setStringValue:s];
	[s release];
}


- (NSString*)GUISubject
{
	return [m_subjectData stringValue];
}

- (void)setGUISubject:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_subjectData setStringValue:s];
	[s release];
}

- (NSString*)GUIAuthor
{
	return [m_authorData stringValue];
}

- (void)setGUIAuthor:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_authorData setStringValue:s];
	[s release];
}

- (NSString*)GUIPublisher
{
	return [m_publisherData stringValue];
}

- (void)setGUIPublisher:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_publisherData setStringValue:s];
	[s release];
}

- (NSString*)GUICoAuthor
{
	return [m_coAuthorData stringValue];
}

- (void)setGUICoAuthor:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_coAuthorData setStringValue:s];
	[s release];
}

- (NSString*)GUICategory
{
	return [m_categoryData stringValue];
}

- (void)setGUICategory:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_categoryData setStringValue:s];
	[s release];
}

- (NSString*)GUIKeywords
{
	return [m_keywordData stringValue];
}

- (void)setGUIKeywords:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_keywordData setStringValue:s];
	[s release];
}

- (NSString*)GUILanguages
{
	return [m_languageData stringValue];
}

- (void)setGUILanguages:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_languageData setStringValue:s];
	[s release];
}

- (NSString*)GUISource
{
	return [m_sourceData stringValue];
}

- (void)setGUISource:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_sourceData setStringValue:s];
	[s release];
}

- (NSString*)GUIRelation
{
	return [m_relationData stringValue];
}

- (void)setGUIRelation:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_relationData setStringValue:s];
	[s release];
}

- (NSString*)GUICoverage
{
	return [m_coverageData stringValue];
}

- (void)setGUICoverage:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_coverageData setStringValue:s];
	[s release];
}

- (NSString*)GUIRights
{
	return [m_rightsData stringValue];
}

- (void)setGUIRights:(const UT_UTF8String&)str
{
	NSString *s = [[NSString alloc] initWithUTF8String:str.utf8_str()];
	[m_rightsData setStringValue:s];
	[s release];
}

- (NSString*)GUIDescription
{
	return [m_descriptionData string];
}

- (void)setGUIDescription:(const UT_UTF8String&)str
{
	[m_descriptionData setString:[NSString stringWithUTF8String:str.utf8_str()]];
}


@end
