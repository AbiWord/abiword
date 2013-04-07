/* AbiWord
 * Copyright (C) 2002-2003 Hubert Figuiere
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

#include "ut_types.h"

#import "xap_GenericListChooser_Controller.h"


@implementation XAP_GenericListChooser_Controller

+ (XAP_GenericListChooser_Controller *)loadFromNib
{
	XAP_GenericListChooser_Controller * dlg = [[XAP_GenericListChooser_Controller alloc] initWithWindowNibName:@"xap_GenericListChooser_Controller"];
	return [dlg autorelease];
}

- (void)windowDidLoad
{
	[m_listTable setDoubleAction:@selector(listClicked:)];
}


- (IBAction)cancelClicked:(id)sender
{
	UT_UNUSED(sender);
	if (_proxy) {
		_proxy->cancelAction();
	}
}

- (IBAction)listClicked:(id)sender
{
	UT_UNUSED(sender);
	if (_proxy) {
		_proxy->selectAction();
	}
}

- (IBAction)okClicked:(id)sender
{
	UT_UNUSED(sender);
	if (_proxy) {
		_proxy->okAction();
	}
}

- (void)setTitle:(NSString*)title
{
	[[self window] setTitle:title];
}


- (void)setLabel:(NSString*)label
{
	[m_enclosingBox setTitle:label];
}


- (void)setXAPProxy:(XAP_GenericListChooser_Proxy*)proxy
{
	_proxy = proxy;
}

- (void)setDataSource:(id)obj
{
	[m_listTable setDataSource:obj];
}

- (void)setSelected:(int)idx
{
	[m_listTable selectRowIndexes:[NSIndexSet indexSetWithIndex:idx] byExtendingSelection:NO];
	[m_listTable scrollRowToVisible:(int)idx];
}

- (int)selected
{
	return [m_listTable selectedRow];
}


@end
