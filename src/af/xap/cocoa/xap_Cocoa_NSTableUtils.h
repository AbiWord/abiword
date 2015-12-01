/* AbiWord
 * Copyright (C) 2002-2003, 2009 Hubert Figuiere
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


@interface XAP_StringListDataSource
	: NSObject <NSTableViewDataSource, NSComboBoxDataSource>
{
	NSMutableArray*		_array;
}

- (id)init;

- (oneway void)dealloc;

- (void)insertString:(NSString*)string atIndex:(int)index;
- (void)addString:(NSString*)string;
- (void)addCString:(const char * )string;
- (int)rowWithCString:(const char *)cString;
- (void)removeAllStrings;
- (NSArray*)array;

- (void)loadFontList;

/* NSTableDataSource */
- (int)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row;
@end
