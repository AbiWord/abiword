/* AbiSource Application Framework
 * Copyright (C) 2004 Hubert Figuiere
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

#include "xap_CocoaAppImpl.h"

bool XAP_CocoaAppImpl::openURL(const char * url)
{
	NSString* str = [[NSString alloc] initWithUTF8String:url];
	NSURL *URL = [[NSURL alloc] initWithString:str];		
	
	NSWorkspace * space = [NSWorkspace sharedWorkspace];
	[space openURL:URL];

	[URL release];
	[str release];
	
	return true;
}

