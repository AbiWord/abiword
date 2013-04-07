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

#include "ut_types.h"
#import "xap_CocoaCompat.h"
#import "xap_Cocoa_NSTableUtils.h"


@implementation XAP_StringListDataSource



- (id)init
{
	if (![super init]) {
		return nil;
	}
	_array = [[NSMutableArray alloc] init];
	return self;
}

- (oneway void)dealloc
{
	[_array release];
	[super dealloc];
}


/* NSTableDataSource */
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	UT_UNUSED(tableView);
	return [_array count];
}


- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	UT_UNUSED(tableView);
	UT_UNUSED(tableColumn);
	return [_array objectAtIndex:row];
}

- (void)insertString:(NSString*)string atIndex:(int)index
{
	[_array insertObject:string atIndex:index];
}

- (void)addString:(NSString*)string
{
	[_array addObject:string];
}

- (void)addCString:(const char *)string
{
	NSString *str = [[NSString alloc] initWithUTF8String:string];
	[_array addObject:str];		
	[str release];
}

- (int)rowWithCString:(const char *)cString
{
	NSString *string = [[NSString alloc] initWithUTF8String:cString];
	NSUInteger idx = [_array indexOfObject:string];
	[string release];
	if (idx == NSNotFound) { 
		return -1;
	}
	return idx;
}

- (void)removeAllStrings
{
	[_array removeAllObjects];
}

- (NSArray*)array
{
	return _array;
}


- (void)loadFontList
{
	[_array removeAllObjects];
	[_array addObjectsFromArray:
		[[[NSFontManager sharedFontManager] availableFontFamilies] 
			sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)]];
}


@end
