/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
 * Copyright (C) 2002 Hubert Figuiere
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

#import "ut_assert.h"
#import "ut_debugmsg.h"
#import "ut_exception.h"

#import "ap_CocoaSplash.h"

#import "ap_Cocoa_ResourceIDs.h"

static AP_CocoaSplash * s_Splash = 0;

@interface Wave : NSObject
{
    int ripple;
}
- GoodBye:(NSTimer *)timer;
@end

@implementation Wave
- GoodBye:(NSTimer *)timer
{
	AP_CocoaSplash * splash = AP_CocoaSplash::instance ();
	if (splash) 
		delete splash;
	return self;
}
@end

static Wave * wave = 0;

AP_CocoaSplash * AP_CocoaSplash::instance (bool instantiate)
{
	if (s_Splash) 
		return s_Splash;

	if (!instantiate) 
		return NULL; // the default is to instantiate the splash

	if (wave) 
		return NULL; // hmm, must have done the splash thing already
	wave = [[Wave alloc] init];
	UT_ASSERT(wave);

	NSImage*	image = [NSImage imageNamed:AP_COCOA_SPLASH_RESOURCE_NAME];
	UT_ASSERT (image);
	NSSize	size = [image size];

	UT_TRY
	{
		s_Splash = new AP_CocoaSplash(size, image);
	}
	UT_CATCH (...)
	{
		s_Splash = NULL;
	}
	if (s_Splash)
	{
		NSTimeInterval s = 2;
		[NSTimer scheduledTimerWithTimeInterval:s target:wave selector:@selector(GoodBye:) userInfo:nil repeats:NO];
	}

	return s_Splash;
}

AP_CocoaSplash::AP_CocoaSplash (const NSSize & size, NSImage * image) :
	XAP_CocoaWindow(ws_Raw,NSMakeRect(100.0,100.0,size.width,size.height)), // fix position later
	m_statusbar(0)
{	
    [m_window setHidesOnDeactivate:NO];
    [m_window setExcludedFromWindowsMenu:YES];
    [m_window setAlphaValue:0.8];
#ifdef NDEBUG
    [m_window setLevel:NSFloatingWindowLevel];
#endif

	NSRect box;
	
	box = NSMakeRect(0.0, 0.0, size.width, size.height);
	NSImageView * iview = [[NSImageView alloc] initWithFrame:box];
	UT_ASSERT (iview);
	[iview setImage:image];
	[[m_window contentView] addSubview:iview];
	[iview release];
	
	
	_show ();

	box = NSMakeRect(4.0, 8.0, size.width - 8.0, 12.0);
	m_statusbar = [[NSText alloc] initWithFrame:box];
	UT_ASSERT(m_statusbar);
	
	[[m_window contentView] addSubview:m_statusbar];	/* m_statusbar will be released in destructor of this */
										/* that allow the view to be detached safely */
	XAP_StatusBar::setStatusBar (this);
}

AP_CocoaSplash::~AP_CocoaSplash ()
{
	XAP_StatusBar::unsetStatusBar (this);
	s_Splash = 0;
}

void AP_CocoaSplash::statusMessage (const char * utf8str, bool urgent)
{
	if (m_statusbar == 0) return;

	if (urgent) {
		[m_statusbar setTextColor:[NSColor redColor]];
	}
	else {
		[m_statusbar setTextColor:[NSColor blackColor]];
	}

	NSString * str = [NSString stringWithUTF8String:utf8str];
	if (str)
	{
		[m_statusbar setString:str];
		[m_statusbar display];
	}
}
