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
#include "ap_CocoaDialog_MergeCells.h"


XAP_Dialog * AP_CocoaDialog_MergeCells::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id dlgid)
{
	return new AP_CocoaDialog_MergeCells(pFactory,dlgid);
}

AP_CocoaDialog_MergeCells::AP_CocoaDialog_MergeCells(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id dlgid)
	: AP_Dialog_MergeCells(pDlgFactory,dlgid)
{
}

AP_CocoaDialog_MergeCells::~AP_CocoaDialog_MergeCells(void)
{
}

void AP_CocoaDialog_MergeCells::runModeless(XAP_Frame * pFrame)
{
	m_dlg = [[AP_CocoaDialog_MergeCellsController alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	NSWindow* window = [m_dlg window];

	// Populate the window's data items
	_populateWindowData();

	[window orderFront:m_dlg];
	startUpdater();
}

void AP_CocoaDialog_MergeCells::setSensitivity(AP_Dialog_MergeCells::mergeWithCell mergeThis, bool bSens)
{
	[m_dlg setEnableButton:mergeThis to:bSens];
}

void AP_CocoaDialog_MergeCells::event_Close(void)
{
	m_answer = AP_Dialog_MergeCells::a_CANCEL;
}

void AP_CocoaDialog_MergeCells::destroy(void)
{
	finalize();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}
void AP_CocoaDialog_MergeCells::activate(void)
{
	ConstructWindowName();
	NSWindow* window = [m_dlg window];
	[window setTitle:[NSString stringWithUTF8String:m_WindowName]];
	setAllSensitivities();
	[window orderFront:m_dlg];
}

void AP_CocoaDialog_MergeCells::notifyActiveFrame(XAP_Frame *pFrame)
{
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:m_WindowName]];
	setAllSensitivities();
}

/*****************************************************************/

void AP_CocoaDialog_MergeCells::_populateWindowData(void)
{
   setAllSensitivities();
}

void AP_CocoaDialog_MergeCells::_storeWindowData(void)
{
}


@implementation AP_CocoaDialog_MergeCellsController


- (id)initFromNib
{
	self = [super initWithWindowNibName:@"ap_CocoaDialog_MergeCells"];
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
	_xap = dynamic_cast<AP_CocoaDialog_MergeCells*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		_xap->ConstructWindowName();
		[[self window] setTitle:[NSString stringWithUTF8String:_xap->getWindowName()]];
		LocalizeControl(_mergeCellsBox, pSS, AP_STRING_ID_DLG_MergeCells_Frame);
		LocalizeControl(_mergeLeftBtn, pSS, AP_STRING_ID_DLG_MergeCells_Left);
		[_mergeLeftBtn setImage:[NSImage imageNamed:@"tb_MergeLeft"]];
		LocalizeControl(_mergeRightBtn, pSS, AP_STRING_ID_DLG_MergeCells_Right);
		[_mergeRightBtn setImage:[NSImage imageNamed:@"tb_MergeRight"]];
		LocalizeControl(_mergeAboveBtn, pSS, AP_STRING_ID_DLG_MergeCells_Above);
		[_mergeAboveBtn setImage:[NSImage imageNamed:@"tb_MergeAbove"]];
		LocalizeControl(_mergeBelowBtn, pSS, AP_STRING_ID_DLG_MergeCells_Below);
		[_mergeBelowBtn setImage:[NSImage imageNamed:@"tb_MergeBelow"]];
	}
}

- (void)windowWillClose:(NSNotification *)aNotification
{
	_xap->event_Close();
}

- (IBAction)mergeAbove:(id)sender
{
	_xap->setMergeType(AP_Dialog_MergeCells::radio_above);
	_xap->onMerge();
}

- (IBAction)mergeBelow:(id)sender
{
	_xap->setMergeType(AP_Dialog_MergeCells::radio_below);
	_xap->onMerge();
}

- (IBAction)mergeLeft:(id)sender
{
	_xap->setMergeType(AP_Dialog_MergeCells::radio_left);
	_xap->onMerge();
}

- (IBAction)mergeRight:(id)sender
{
	_xap->setMergeType(AP_Dialog_MergeCells::radio_right);
	_xap->onMerge();
}

- (void)setEnableButton:(AP_Dialog_MergeCells::mergeWithCell)btn to:(bool)val
{
	switch(btn)
	{
	case AP_Dialog_MergeCells::radio_left:
		[_mergeLeftBtn setEnabled:(val?YES:NO)];
		break;
	case AP_Dialog_MergeCells::radio_right:
		[_mergeRightBtn setEnabled:(val?YES:NO)];
		break;
	case AP_Dialog_MergeCells::radio_above:
		[_mergeAboveBtn setEnabled:(val?YES:NO)];
		break;
	case AP_Dialog_MergeCells::radio_below:
		[_mergeBelowBtn setEnabled:(val?YES:NO)];
		break;
	default:
		break;
	}
}


@end
