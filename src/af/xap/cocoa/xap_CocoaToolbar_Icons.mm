/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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
 
#include <stdio.h>
#include <string.h>

#include "ut_misc.h"
#include "ut_hash.h"
#include "ut_assert.h"
#include "ut_string.h"

#include "xap_CocoaApp.h"
#include "xap_CocoaToolbar_Icons.h"

static NSPoint s_ButtonOnPoint[12] = {
	{	 0.0f,	 7.0f	},
	{	 0.0f,	 0.0f	},
	{	 7.0f,	 0.0f	},
	{	19.0f,	 0.0f	},
	{	26.0f,	 0.0f	},
	{	26.0f,	 7.0f	},
	{	26.0f,	19.0f	},
	{	26.0f,	26.0f	},
	{	19.0f,	26.0f	},
	{	 7.0f,	26.0f	},
	{	 0.0f,	26.0f	},
	{	 0.0f,	19.0f	}
};

static NSPoint s_ButtonMenuPoint[3] = {
	{	26.0f,	21.0f	},
	{	20.0f,	21.0f	},
	{	23.0f,	26.0f	}
};

@implementation XAP_CocoaToolbarButton

- (id)initWithFrame:(NSRect)frameRect
{
	if (self = [super initWithFrame:frameRect])
		{
			m_menu = 0;
			m_controller = 0;
		}
	return self;
}

- (void)setMenu:(NSMenu *)menu withController:(id <XAP_CocoaButtonController>)controller
{
	m_menu = menu;
	m_controller = controller;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	if (m_menu && [self isEnabled])
		{
			if (m_controller)
				{
					[m_controller menuWillActivate:m_menu forButton:self];
				}
			[NSMenu popUpContextMenu:m_menu withEvent:theEvent forView:self];
		}
	else
		{
			[super mouseDown:theEvent];
		}
}

- (void)drawRect:(NSRect)aRect
{
	if ([self state] == NSOnState)
		{
			[[NSColor colorWithCalibratedWhite:0.0f alpha:0.25] set];

			NSBezierPath * path = [NSBezierPath bezierPath];

			[path  moveToPoint:s_ButtonOnPoint[ 0]];
			[path curveToPoint:s_ButtonOnPoint[ 2] controlPoint1:s_ButtonOnPoint[ 1] controlPoint2:s_ButtonOnPoint[ 1]];
			[path  lineToPoint:s_ButtonOnPoint[ 3]];
			[path curveToPoint:s_ButtonOnPoint[ 5] controlPoint1:s_ButtonOnPoint[ 4] controlPoint2:s_ButtonOnPoint[ 4]];
			[path  lineToPoint:s_ButtonOnPoint[ 6]];
			[path curveToPoint:s_ButtonOnPoint[ 8] controlPoint1:s_ButtonOnPoint[ 7] controlPoint2:s_ButtonOnPoint[ 7]];
			[path  lineToPoint:s_ButtonOnPoint[ 9]];
			[path curveToPoint:s_ButtonOnPoint[11] controlPoint1:s_ButtonOnPoint[10] controlPoint2:s_ButtonOnPoint[10]];
			[path closePath];
			[path fill];
		}
	[super drawRect:aRect];

	if (m_menu)
		{
			[[NSColor blackColor] set];

			NSBezierPath * path = [NSBezierPath bezierPath];

			[path moveToPoint:s_ButtonMenuPoint[0]];
			[path lineToPoint:s_ButtonMenuPoint[1]];
			[path lineToPoint:s_ButtonMenuPoint[2]];
			[path closePath];
			[path fill];
		}
}

@end

AP_CocoaToolbar_Icons::AP_CocoaToolbar_Icons(void)
{
}

AP_CocoaToolbar_Icons::~AP_CocoaToolbar_Icons(void)
{
	// TODO do we need to keep some kind of list
	// TODO of the things we have created and
	// TODO handed out, so that we can delete them ??
}

NSString * AP_CocoaToolbar_Icons::getPNGNameForIcon(const char * szIconID)
{
	const char * szIconName = NULL;

	if (AP_Toolbar_Icons::_findIconNameForID(szIconID, &szIconName))
	{
		UT_UTF8String name(szIconName);

		if (char * suffix = strstr(szIconName, "_xpm"))
		{
			name.assign(szIconName, suffix - szIconName);
		}
		name += ".png";

		return [NSString stringWithUTF8String:(name.utf8_str())];
	}

	UT_ASSERT_NOT_REACHED();
	return nil;
}

NSString * AP_CocoaToolbar_Icons::getFilenameForIcon(NSString * iconName)
{
	NSString * filename = iconName;

	if (iconName)
	{
		XAP_CocoaApp * pApp = static_cast<XAP_CocoaApp *>(XAP_App::getApp());

		UT_String path;

		if (pApp->findAbiSuiteLibFile(path, [iconName UTF8String], "ToolbarIcons")) // I'd love to do this inside the bundle but Cocoa gets confused if I try
		{
			filename = [NSString stringWithUTF8String:(path.c_str())];
		}
		else if (pApp->findAbiSuiteBundleFile(path, [iconName UTF8String]))
		{
			filename = [NSString stringWithUTF8String:(path.c_str())];
		}
	}
	return filename;
}

typedef struct _my_argb {
	UT_RGBColor rgb;
	unsigned char alpha;
} my_argb;


/*!
	returns the pixmap for the named icon
	
	\param szIconName the name of the icon
	\return the newly allocated NSImage [autoreleased]
 */
NSImage * AP_CocoaToolbar_Icons::getPixmapForIcon(const char * szIconID)
{
#if 1
	UT_ASSERT(szIconID && *szIconID);

	NSImage * pixmap = nil;

	NSString * path = AP_CocoaToolbar_Icons::getFilenameForIcon(AP_CocoaToolbar_Icons::getPNGNameForIcon(szIconID));

	if (path)
	{
		pixmap = [[NSImage alloc] initWithContentsOfFile:path];

		if (pixmap)
		{
			[pixmap autorelease];
		}
		else
		{
			UT_ASSERT(pixmap);
			pixmap = [NSImage imageNamed:@"NSApplicationIcon"];
		}
	}
	else
	{
		UT_ASSERT_NOT_REACHED();
	}
	return pixmap;
#else
	UT_uint32 width, height, nrColors, charsPerPixel;
	
	const char ** pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array
	
	bool bFound = _findIconDataByName(szIconID, &pIconData, &sizeofIconData);
	if (!bFound)
		return nil;


	UT_uint32 n = sscanf(pIconData[0],"%ld %ld %ld %ld",
					&width,&height,&nrColors,&charsPerPixel);
	UT_ASSERT (n == 4);
	my_argb *pRGB = (my_argb*)g_try_malloc((nrColors + 1) * sizeof(my_argb));
	UT_ASSERT(pRGB);
	UT_StringPtrMap hash(61);
	UT_RGBColor color(0,0,0);
	
	// walk thru the palette
	const char ** pIconDataPalette = &pIconData[1];
	for (UT_uint32 k=0; (k < nrColors); k++)
	{
		char bufSymbol[10];
		char bufKey[10];
		char bufColorValue[100];

		// we expect something of the form: ".. c #000000"
		// but we allow a space as a character in the symbol, so we
		// get the first field the hard way.
		memset(bufSymbol,0,sizeof(bufSymbol));
		for (UT_uint32 kPx=0; (kPx < charsPerPixel); kPx++)
			bufSymbol[kPx] = pIconDataPalette[k][kPx];
		UT_ASSERT(strlen(bufSymbol) == charsPerPixel);
		
		UT_uint32 nf = sscanf(&pIconDataPalette[k][charsPerPixel+1],
						" %s %s",&bufKey,&bufColorValue);
		UT_ASSERT(nf == 2);
		UT_ASSERT(bufKey[0] = 'c');

		// make the ".." a hash key and store our color index as the data.
		// we add k+1 because the hash code does not like null pointers...
		hash.insert(bufSymbol,(void *)(k+1));
		
		// store the actual color value in the 
		// rgb quad array with our color index.
		if (g_ascii_strcasecmp(bufColorValue,"None")==0) {
			pRGB[k].rgb.m_red = 255;
			pRGB[k].rgb.m_grn = 255;
			pRGB[k].rgb.m_blu = 255;
			pRGB[k].alpha = 0;
		}
		else
		{
			// TODO fix this to also handle 
			// #ffffeeeedddd type color references
			UT_ASSERT((bufColorValue[0] == '#') && strlen(bufColorValue)==7);
			UT_parseColor(bufColorValue, color);
			pRGB[k].rgb = color;
			pRGB[k].alpha   = 255;
		}
	}

	long bytesPerRow = width;
	NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
							pixelsWide:width pixelsHigh:height bitsPerSample:8 samplesPerPixel:4
							hasAlpha:YES isPlanar:YES 
							colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 
							bitsPerPixel:0];

	int rgb_index;
	const char ** pIconDataImage = &pIconDataPalette[nrColors];
	unsigned char *planes [5];
	[bitmap getBitmapDataPlanes:planes];

	for (UT_uint32 kRow=0; (kRow < height); kRow++) {
		const char * p = pIconDataImage[kRow];
		
		for (UT_uint32 kCol=0; (kCol < width); kCol++) {
			char bufPixel[10];
			memset(bufPixel,0,sizeof(bufPixel));
			for (UT_uint32 kPx=0; (kPx < charsPerPixel); kPx++)
				bufPixel[kPx] = *p++;

			//printf("Looking for character %s \n", bufPixel);
			const void * pEntry = hash.pick(bufPixel);
			
			rgb_index = ((UT_Byte)(pEntry)) -1;
			//printf("Returned hash index %d \n", rgb_index); 

			*(planes[0] + kRow * bytesPerRow + kCol) 
							= pRGB[rgb_index].rgb.m_red;
			*(planes[1] + kRow * bytesPerRow + kCol) 
							= pRGB[rgb_index].rgb.m_grn;
			*(planes[2] + kRow * bytesPerRow + kCol) 
							= pRGB[rgb_index].rgb.m_blu; 
			*(planes[3] + kRow * bytesPerRow + kCol) 
							= pRGB[rgb_index].alpha;
		}
	}

	FREEP(pRGB);

	pixmap = [[NSImage alloc] initWithSize:NSMakeSize(0.0, 0.0)];
	[pixmap addRepresentation:bitmap];
	[bitmap release];

	UT_ASSERT (pixmap);	

	return [pixmap autorelease];
#endif
}

