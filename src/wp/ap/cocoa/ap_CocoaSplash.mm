/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
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

#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "ut_exception.h"

#include "gr_CocoaImage.h"

#include "ap_CocoaSplash.h"

/* build system converts PNG to static array;
 * see build-generated file ap_wp_splash.cpp
 */
extern unsigned char g_pngSplash[];
extern unsigned long g_pngSplash_sizeof;

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
	if (splash) delete splash;
	return self;
}
@end

static Wave * wave = 0;

AP_CocoaSplash * AP_CocoaSplash::instance (bool instantiate)
{
	if (s_Splash) return s_Splash;

	if (!instantiate) return 0; // the default is to instantiate the splash

	if (wave) return 0; // hmm, must have done the splash thing already
	wave = [Wave alloc];
	if (wave == 0) return 0;
	[wave init];

	UT_ByteBuf * pBB = 0;
	UT_TRY
		{
			pBB = new UT_ByteBuf();
		}
	UT_CATCH (...)
		{
			pBB = 0;
		}
	if (pBB == 0) return 0;

	if (!pBB->ins (0, g_pngSplash, g_pngSplash_sizeof))
		{
			DELETEP (pBB);
			return 0;
		}

	UT_uint32 width = 0;
	UT_uint32 height = 0;

	NSImage * image = GR_CocoaImage::imageFromPNG (pBB, width, height);
	DELETEP (pBB);
	if (image == 0) return 0;

	UT_TRY
		{
			s_Splash = new AP_CocoaSplash(width,height,image);
		}
	UT_CATCH (...)
		{
			s_Splash = 0;
		}
	if (s_Splash)
		{
			NSTimeInterval s = 2;
			[NSTimer scheduledTimerWithTimeInterval:s target:wave selector:@selector(GoodBye:) userInfo:nil repeats:NO];
		}

	return s_Splash;
}

AP_CocoaSplash::AP_CocoaSplash (UT_uint32 width, UT_uint32 height, NSImage * image) :
	XAP_CocoaWindow(ws_Raw,100,100,width,height), // fix position later
	m_statusbar(0)
{
    [m_window setHidesOnDeactivate:NO];
    [m_window setExcludedFromWindowsMenu:YES];
    [m_window setAlphaValue:0.8];
#ifdef NDEBUG
    [m_window setLevel:NSFloatingWindowLevel];
#endif

	NSRect box;

	NSImageView * iview = [NSImageView alloc];
	if (iview)
		{
			box.origin.x = static_cast<float>(0);
			box.origin.y = static_cast<float>(0);
			box.size.width  = static_cast<float>(width);
			box.size.height = static_cast<float>(height);
			[iview initWithFrame:box];

			[iview setImage:image];

			[m_view addSubview:iview];
		}

	_show ();

	m_statusbar = (NSText *) [NSText alloc];
	if (m_statusbar == 0) return;

	box.origin.x = static_cast<float>(4);
	box.origin.y = static_cast<float>(8);
	box.size.width  = static_cast<float>(width - 8);
	box.size.height = static_cast<float>(12);
	[m_statusbar initWithFrame:box];

	[m_view addSubview:m_statusbar];

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

	if (urgent)
		[m_statusbar setTextColor:[NSColor redColor]];
	else
		[m_statusbar setTextColor:[NSColor blackColor]];

	NSString * str = [NSString stringWithUTF8String:utf8str];
	if (str)
		{
			[m_statusbar setString:str];
			[m_statusbar display];
		}
}
