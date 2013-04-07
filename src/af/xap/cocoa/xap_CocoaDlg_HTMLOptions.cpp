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

#import <Cocoa/Cocoa.h>

#include "xap_Frame.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaDlg_HTMLOptions.h"

XAP_Dialog * XAP_CocoaDialog_HTMLOptions::static_constructor (XAP_DialogFactory * pDF,
															 XAP_Dialog_Id dlgid)
{
	return new XAP_CocoaDialog_HTMLOptions(pDF,dlgid);
}

XAP_CocoaDialog_HTMLOptions::XAP_CocoaDialog_HTMLOptions (XAP_DialogFactory * pDlgFactory,
														XAP_Dialog_Id dlgid)
	: XAP_Dialog_HTMLOptions(pDlgFactory,dlgid)
{
	// 
}

XAP_CocoaDialog_HTMLOptions::~XAP_CocoaDialog_HTMLOptions ()
{
	// 
}


void XAP_CocoaDialog_HTMLOptions::runModal (XAP_Frame * /*pFrame*/)
{
	m_dlg = [[XAP_CocoaDialog_HTMLOptions_Controller alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);

	refreshStates ();
	[NSApp runModalForWindow:window];

	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void XAP_CocoaDialog_HTMLOptions::toggle_Is4 ()
{
	bool on = [m_dlg valueOf:IS4];
	set_HTML4 (on);
	refreshStates ();
}

void XAP_CocoaDialog_HTMLOptions::toggle_AbiWebDoc ()
{
	bool on = [m_dlg valueOf:ABIWEBDOC];
	set_PHTML (on);
	refreshStates ();
}

void XAP_CocoaDialog_HTMLOptions::toggle_DeclareXML ()
{
	bool on = [m_dlg valueOf:DECLAREXML];
	set_Declare_XML (on);
	refreshStates ();
}

void XAP_CocoaDialog_HTMLOptions::toggle_AllowAWML ()
{
	bool on = [m_dlg valueOf:ALLOWAWML];
	set_Allow_AWML (on);
	refreshStates ();
}

void XAP_CocoaDialog_HTMLOptions::toggle_EmbedCSS ()
{
	bool on = [m_dlg valueOf:EMBEDCSS];
	set_Embed_CSS (on);
	refreshStates ();
}

void XAP_CocoaDialog_HTMLOptions::toggle_EmbedImages ()
{
	bool on = [m_dlg valueOf:EMBEDIMAGES];
	set_Embed_Images (on);
	refreshStates ();
}

void XAP_CocoaDialog_HTMLOptions::refreshStates ()
{
	bool on;

	on = get_HTML4 ();
	[m_dlg toggle:IS4 withValue:on];

	on = get_PHTML ();
	[m_dlg toggle:ABIWEBDOC withValue:on];

	on = get_Declare_XML ();
	[m_dlg toggle:DECLAREXML withValue:on];

	on = can_set_Declare_XML ();
	[m_dlg enable:DECLAREXML withValue:on];

	on = get_Allow_AWML ();
	[m_dlg toggle:ALLOWAWML withValue:on];

	on = can_set_Allow_AWML ();
	[m_dlg enable:ALLOWAWML withValue:on];

	on = get_Embed_CSS ();
	[m_dlg toggle:EMBEDCSS withValue:on];

	on = can_set_Embed_CSS ();
	[m_dlg enable:EMBEDCSS withValue:on];

	on = get_Embed_Images ();
	[m_dlg toggle:EMBEDIMAGES withValue:on];

	on = can_set_Embed_Images ();
	[m_dlg enable:EMBEDIMAGES withValue:on];
}

void XAP_CocoaDialog_HTMLOptions::event_OK ()
{
	m_bShouldSave = true;
	[NSApp stopModal];
}

void XAP_CocoaDialog_HTMLOptions::event_SaveSettings ()
{
	saveDefaults ();
	refreshStates ();
}

void XAP_CocoaDialog_HTMLOptions::event_RestoreSettings ()
{
	restoreDefaults ();
	refreshStates ();
}

void XAP_CocoaDialog_HTMLOptions::event_Cancel ()
{
	m_bShouldSave = false;
	[NSApp stopModal];
}


@implementation XAP_CocoaDialog_HTMLOptions_Controller

- (id)initFromNib
{
	return [super initWithWindowNibName:@"xap_CocoaDlg_HTMLOptions"];
}

-(void)discardXAP
{
	_xap = NULL; 
}

-(void)dealloc
{
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<XAP_CocoaDialog_HTMLOptions*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpTitle);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_saveBtn, pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpSave);
		LocalizeControl(_restoreBtn, pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpRestore);

		LocalizeControl(_exportOptionsBox, pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpLabel);

		LocalizeControl(_exportHTMLBtn, pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpIs4);
		LocalizeControl(_exportPHPBtn, pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpAbiWebDoc);
		LocalizeControl(_declareXMLBtn, pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpDeclareXML);
		LocalizeControl(_allowExtraMarkupBtn, pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpAllowAWML);
		LocalizeControl(_embedCSSBtn, pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedCSS);
		LocalizeControl(_embedImagesBtn, pSS, XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedImages);
	}
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

- (IBAction)restoreAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_RestoreSettings();
}

- (IBAction)saveAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_SaveSettings();
}

- (IBAction)allowExtraMarkupAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->toggle_AllowAWML();
}

- (IBAction)declareXMLAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->toggle_DeclareXML();
}

- (IBAction)embedCSSAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->toggle_EmbedCSS();
}

- (IBAction)embedImageAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->toggle_EmbedImages();
}

- (IBAction)exportHTMLAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->toggle_Is4();
}

- (IBAction)exportPHPAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->toggle_AbiWebDoc();
}


- (void)toggle:(XAP_CocoaDialog_HTMLOptions::options)btn withValue:(bool)value
{
	NSButton *ctrl = nil;
	switch(btn) {
	case XAP_CocoaDialog_HTMLOptions::IS4:
		ctrl = _exportHTMLBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::ABIWEBDOC:
		ctrl = _exportPHPBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::DECLAREXML:
		ctrl = _declareXMLBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::ALLOWAWML:
		ctrl = _allowExtraMarkupBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::EMBEDCSS:
		ctrl = _embedCSSBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::EMBEDIMAGES:
		ctrl = _embedImagesBtn;
		break;
	default:
		UT_ASSERT_NOT_REACHED();
	}
	[ctrl setState:(value?NSOnState:NSOffState)];
}


- (void)enable:(XAP_CocoaDialog_HTMLOptions::options)btn withValue:(bool)value
{
	NSButton *ctrl = nil;
	switch(btn) {
	case XAP_CocoaDialog_HTMLOptions::IS4:
		ctrl = _exportHTMLBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::ABIWEBDOC:
		ctrl = _exportPHPBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::DECLAREXML:
		ctrl = _declareXMLBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::ALLOWAWML:
		ctrl = _allowExtraMarkupBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::EMBEDCSS:
		ctrl = _embedCSSBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::EMBEDIMAGES:
		ctrl = _embedImagesBtn;
		break;
	default:
		UT_ASSERT_NOT_REACHED();
	}
	[ctrl setEnabled:(value?YES:NO)];
}


- (bool)valueOf:(XAP_CocoaDialog_HTMLOptions::options)btn
{
	NSButton *ctrl = nil;
	switch(btn) {
	case XAP_CocoaDialog_HTMLOptions::IS4:
		ctrl = _exportHTMLBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::ABIWEBDOC:
		ctrl = _exportPHPBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::DECLAREXML:
		ctrl = _declareXMLBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::ALLOWAWML:
		ctrl = _allowExtraMarkupBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::EMBEDCSS:
		ctrl = _embedCSSBtn;
		break;
	case XAP_CocoaDialog_HTMLOptions::EMBEDIMAGES:
		ctrl = _embedImagesBtn;
		break;
	default:
		UT_ASSERT_NOT_REACHED();
	}
	return ([ctrl state] == NSOnState?true:false);
}


@end
