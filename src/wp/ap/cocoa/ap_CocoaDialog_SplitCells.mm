/* AbiWord
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 
#include "xap_App.h"

#include "xap_CocoaDialog_Utilities.h"
#include "ap_Strings.h"
#include "ap_CocoaDialog_SplitCells.h"


XAP_Dialog * AP_CocoaDialog_SplitCells::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id dlgid)
{
	return new AP_CocoaDialog_SplitCells(pFactory,dlgid);
}

AP_CocoaDialog_SplitCells::AP_CocoaDialog_SplitCells(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id dlgid)
	: AP_Dialog_SplitCells(pDlgFactory,dlgid)
{
}

AP_CocoaDialog_SplitCells::~AP_CocoaDialog_SplitCells(void)
{
}

void AP_CocoaDialog_SplitCells::runModeless(XAP_Frame * pFrame)
{
	m_dlg = [[AP_CocoaDialog_SplitCellsController alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	NSWindow* window = [m_dlg window];

	// Populate the window's data items
	_populateWindowData();

	[window orderFront:m_dlg];
	startUpdater();
}

void AP_CocoaDialog_SplitCells::setSensitivity(AP_Dialog_SplitCells::SplitType mergeThis, bool bSens)
{
	[m_dlg setEnableButton:mergeThis to:bSens];
}

void AP_CocoaDialog_SplitCells::event_Close(void)
{
	m_answer = AP_Dialog_SplitCells::a_CANCEL;
}

void AP_CocoaDialog_SplitCells::destroy(void)
{
	finalize();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}
void AP_CocoaDialog_SplitCells::activate(void)
{
	ConstructWindowName();
	NSWindow* window = [m_dlg window];
	[window setTitle:[NSString stringWithUTF8String:m_WindowName]];
	setAllSensitivities();
	[window orderFront:m_dlg];
}

void AP_CocoaDialog_SplitCells::notifyActiveFrame(XAP_Frame *pFrame)
{
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:m_WindowName]];
	setAllSensitivities();
}

/*****************************************************************/

void AP_CocoaDialog_SplitCells::_populateWindowData(void)
{
   setAllSensitivities();
}

void AP_CocoaDialog_SplitCells::_storeWindowData(void)
{
}


@implementation AP_CocoaDialog_SplitCellsController


- (id)initFromNib
{
	self = [super initWithWindowNibName:@"ap_CocoaDialog_SplitCells"];
	return self;
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
	_xap = dynamic_cast<AP_CocoaDialog_SplitCells*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		_xap->ConstructWindowName();
		[[self window] setTitle:[NSString stringWithUTF8String:_xap->getWindowName()]];
		LocalizeControl(_splitCellsBox, pSS, AP_STRING_ID_DLG_SplitCells_Frame);
		LocalizeControl(_splitLeftBtn, pSS, AP_STRING_ID_DLG_SplitCells_Left);
		[_splitLeftBtn setImage:[NSImage imageNamed:@"tb_SplitLeft"]];
		LocalizeControl(_splitMiddleHBtn, pSS, AP_STRING_ID_DLG_SplitCells_HoriMid);
		[_splitMiddleHBtn setImage:[NSImage imageNamed:@"tb_SplitHoriMid"]];
		LocalizeControl(_splitRightBtn, pSS, AP_STRING_ID_DLG_SplitCells_Right);
		[_splitRightBtn setImage:[NSImage imageNamed:@"tb_SplitRight"]];
		LocalizeControl(_splitTopBtn, pSS, AP_STRING_ID_DLG_SplitCells_Above);
		[_splitTopBtn setImage:[NSImage imageNamed:@"tb_SplitAbove"]];
		LocalizeControl(_splitMiddleVBtn, pSS, AP_STRING_ID_DLG_SplitCells_VertMid);
		[_splitMiddleVBtn setImage:[NSImage imageNamed:@"tb_SplitVertMid"]];
		LocalizeControl(_splitBottomBtn, pSS, AP_STRING_ID_DLG_SplitCells_Below);
		[_splitBottomBtn setImage:[NSImage imageNamed:@"tb_SplitBelow"]];
	}
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	_xap->event_Close();
}

- (IBAction)splitLeft:(id)sender
{
	_xap->setSplitType(AP_Dialog_SplitCells::hori_left);
	_xap->onSplit();
}

- (IBAction)splitMiddleH:(id)sender
{
	_xap->setSplitType(AP_Dialog_SplitCells::hori_mid);
	_xap->onSplit();
}

- (IBAction)splitRight:(id)sender
{
	_xap->setSplitType(AP_Dialog_SplitCells::hori_right);
	_xap->onSplit();
}

- (IBAction)splitTop:(id)sender
{
	_xap->setSplitType(AP_Dialog_SplitCells::vert_above);
	_xap->onSplit();
}

- (IBAction)splitMiddleV:(id)sender
{
	_xap->setSplitType(AP_Dialog_SplitCells::vert_mid);
	_xap->onSplit();
}

- (IBAction)splitBottom:(id)sender
{
	_xap->setSplitType(AP_Dialog_SplitCells::vert_below);
	_xap->onSplit();
}

- (void)setEnableButton:(AP_Dialog_SplitCells::SplitType)btn to:(bool)val
{
	switch(btn)
	{
	case AP_Dialog_SplitCells::hori_left:
		[_splitLeftBtn setEnabled:(val?YES:NO)];
		break;
	case AP_Dialog_SplitCells::hori_mid:
		[_splitMiddleHBtn setEnabled:(val?YES:NO)];
		break;
	case AP_Dialog_SplitCells::hori_right:
		[_splitRightBtn setEnabled:(val?YES:NO)];
		break;
	case AP_Dialog_SplitCells::vert_above:
		[_splitTopBtn setEnabled:(val?YES:NO)];
		break;
	case AP_Dialog_SplitCells::vert_mid:
		[_splitMiddleVBtn setEnabled:(val?YES:NO)];
		break;
	case AP_Dialog_SplitCells::vert_below:
		[_splitBottomBtn setEnabled:(val?YES:NO)];
		break;
	default:
		break;
	}
}


@end
