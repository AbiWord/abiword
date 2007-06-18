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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#import "xap_Cocoa_NSTableUtils.h"


@implementation XAP_StringListDataSource



- (id)init
{
	self = [super init];
	if (self) {
		_array = [[NSMutableArray alloc] init];
	}
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
	return [_array count];
}


- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
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

- (void)addUT_UTF8String:(const UT_UTF8String &)string
{
	NSString *str = [[NSString alloc] initWithUTF8String:string.utf8_str()];
	[_array addObject:str];		
	[str release];
}

- (int)rowWithCString:(const char *)cString
{
	NSString *string = [[NSString alloc] initWithUTF8String:cString];
	unsigned int idx = [_array indexOfObject:string];
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
