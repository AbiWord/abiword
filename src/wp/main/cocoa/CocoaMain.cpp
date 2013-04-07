/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#import <Cocoa/Cocoa.h>
#ifdef DEBUG
#import <Foundation/NSDebug.h>
#endif

#include "ut_debugmsg.h"
#include "ap_CocoaApp.h"

int main (int argc, char **argv)
{
#ifdef DEBUG
	UT_DEBUGMSG(("activating NSDebug\n"));
	NSDebugEnabled = NSZombieEnabled = NO;
#endif
	int ret = AP_CocoaApp::main(PACKAGE_NAME, argc, argv);
	
	return ret;
}
