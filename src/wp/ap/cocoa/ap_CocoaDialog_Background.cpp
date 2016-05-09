/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004 Francis James Franklin
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
#include <stdio.h>

#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaCompat.h"

#include "ap_CocoaDialog_Background.h"
#include "ap_Dialog_Id.h"
#include "ap_Strings.h"

XAP_Dialog * AP_CocoaDialog_Background::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Background * p = new AP_CocoaDialog_Background(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_Background::AP_CocoaDialog_Background(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	AP_Dialog_Background(pDlgFactory, dlgid)
{
	// 
}

AP_CocoaDialog_Background::~AP_CocoaDialog_Background(void)
{
	// 
}

void AP_CocoaDialog_Background::runModal(XAP_Frame * /*pFrame*/)
{
	if (AP_CocoaDialog_Background_Controller * dlg = [[AP_CocoaDialog_Background_Controller alloc] initFromNib])
		{
			[dlg setXAPOwner:this];

			[NSApp runModalForWindow:[dlg window]];

			[dlg close];
			[dlg release];
			dlg = nil;
		}
}

@implementation AP_CocoaDialog_Background_Controller

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_Background"]) {
		_xap = NULL;
	}
	return self;
}

- (void)dealloc
{
	// 
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = static_cast<AP_CocoaDialog_Background *>(owner);
}

- (void)discardXAP
{
	_xap = 0;
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

//	LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Background_TitleFore);
//	LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Background_TitleHighlight);
	LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Background_Title);

//	LocalizeControl(oClear,  pSS, AP_STRING_ID_DLG_Background_ClearHighlight);
	LocalizeControl(oClear,  pSS, AP_STRING_ID_DLG_Background_ClearClr);

	LocalizeControl(oCancel, pSS, XAP_STRING_ID_DLG_Cancel);
	LocalizeControl(oOK,     pSS, XAP_STRING_ID_DLG_OK);

	const gchar * pszC = 0;

	if (_xap)
		{
			pszC = _xap->getColor();
		}

	bool bTransparent = false;

	if (!pszC)
		{
			bTransparent = true;
		}
	else if (strcmp (pszC, "transparent") == 0)
		{
			bTransparent = true;
		}

	float r = 1;
	float g = 1;
	float b = 1;
	float a = 0;

	if (!bTransparent)
		{
			UT_RGBColor c(255,255,255);
			UT_parseColor(pszC, c);

			r = c.m_red / 255.0f;
			g = c.m_grn / 255.0f;
			b = c.m_blu / 255.0f;
			a = 1;
		}
	[oColorWell setColor:[NSColor colorWithDeviceRed:r green:g blue:b alpha:a]];

	if ([[NSColorPanel sharedColorPanel] isVisible])
		{
			[oColorWell activate:YES];
		}
}

- (IBAction)aColor:(id)sender
{
	UT_UNUSED(sender);
	NSColor * color = [oColorWell color];

	XAP_CGFloat red;
	XAP_CGFloat green;
	XAP_CGFloat blue;
	XAP_CGFloat alpha;

	[color getRed:&red green:&green blue:&blue alpha:&alpha]; // TODO: is color necessarily RGBA? if not, could be a problem...

	int r = static_cast<int>(lrintf(red   * 255));	r = (r < 0) ? 0 : r;	r = (r > 255) ? 255 : r;
	int g = static_cast<int>(lrintf(green * 255));	g = (g < 0) ? 0 : g;	g = (g > 255) ? 255 : g;
	int b = static_cast<int>(lrintf(blue  * 255));	b = (b < 0) ? 0 : b;	b = (b > 255) ? 255 : b;

	UT_RGBColor rgb(r, g, b);

	if (_xap)
		_xap->setColor(rgb);
}

- (IBAction)aClear:(id)sender
{
	UT_UNUSED(sender);
	float r = 1;
	float g = 1;
	float b = 1;
	float a = 0;

	[oColorWell setColor:[NSColor colorWithDeviceRed:r green:g blue:b alpha:a]];

	if (_xap)
		_xap->setColor(0);
}

- (IBAction)aCancel:(id)sender
{
	UT_UNUSED(sender);
	if (_xap)
		_xap->_setAnswer(AP_Dialog_Background::a_CANCEL);

	[[NSColorPanel sharedColorPanel] orderOut:self];

	[NSApp stopModal];
}

- (IBAction)aOK:(id)sender
{
	UT_UNUSED(sender);
	if (_xap)
		_xap->_setAnswer(AP_Dialog_Background::a_OK);

	[[NSColorPanel sharedColorPanel] orderOut:self];

	[NSApp stopModal];
}

@end
