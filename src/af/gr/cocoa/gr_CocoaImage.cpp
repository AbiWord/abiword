/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001-2002 Hubert Figuiere
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "ut_exception.h"

#import "xap_CocoaAbiConversions.h"

#include "gr_Graphics.h"
#include "gr_CocoaImage.h"


GR_CocoaImage::GR_CocoaImage(const char* szName)
  : m_image(nil),
	m_pngData(nil),
    m_grtype(GRT_Raster) // Probably the safest default.
{
	if (szName)
	{
		setName (szName);
	}
	else
	{
		setName("CocoaImage");
	}
}

GR_CocoaImage::~GR_CocoaImage()
{
	[m_image release];
	[m_pngData release];
}


bool		GR_CocoaImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
	UT_ByteBuf* pBB = new UT_ByteBuf();

	[m_pngData convertToAbiByteBuf:pBB];
	
	*ppBB = pBB;
	
	return true;
}

 

/*! 
 * Returns true if pixel at point (x,y) in device units is transparent.
 * See gr_UnixImage.cpp for how it's done in GTK.
 */
bool	GR_CocoaImage::isTransparentAt(UT_sint32 x, UT_sint32 y)
{
	UT_ASSERT(0);
	return false;
}


/*!
 * Returns true if there is any transparency in the image.
 */ 
bool	GR_CocoaImage::hasAlpha(void) const
{
	UT_ASSERT(0);
	return false;
}


bool	GR_CocoaImage::convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	const char *buffer = (const char *) pBB->getPointer(0);
	UT_uint32 buflen = pBB->getLength();

	if (buflen < 6) return false;

	char str1[10] = "\211PNG";
	char str2[10] = "<89>PNG";

	if ( !(strncmp(buffer, str1, 4)) || !(strncmp(buffer, str2, 6)) )
	{
		m_grtype = GRT_Raster;
		if (m_pngData) {
			[m_pngData release];
		}
		m_pngData = [[NSData alloc] initWithAbiByteBuffer:pBB];
		bool ret = _convertPNGFromBuffer(m_pngData, iDisplayWidth, iDisplayHeight);
		return ret;
	}

	// Otherwise, assume SVG. Do scaling when drawing; save size for then:
	m_grtype = GRT_Vector;

	setDisplaySize(iDisplayWidth, iDisplayHeight);

	return true;
}

GR_Image * 
GR_CocoaImage::createImageSegment(GR_Graphics * pG, const UT_Rect & rec)
{
	UT_sint32 x = pG->tdu(rec.left);
	UT_sint32 y = pG->tdu(rec.top);
	if(x < 0)
	{
		x = 0;
	}
	if(y < 0)
	{
		y = 0;
	}
	UT_sint32 width = pG->tdu(rec.width);
	UT_sint32 height = pG->tdu(rec.height);
	UT_sint32 dH = getDisplayHeight();
	UT_sint32 dW = getDisplayWidth();
	if(height > dH)
	{
		height = dH;
	}
	if(width > dW)
	{
		width = dW;
	}
	if(x + width > dW)
	{
		width = dW - x;
	}
	if(y + height > dH)
	{
		height = dH - y;
	}
	if(width < 0)
	{
		x = dW -1;
		width = 1;
	}
	if(height < 0)
	{
		y = dH -1;
		height = 1;
	}
	UT_String sName("");
	getName(sName);
    UT_String sSub("");
	UT_String_sprintf(sSub,"_segemnt_%d_%d_%d_%d",x,y,width,height);
	sName += sSub;

	GR_CocoaImage * image = new GR_CocoaImage(sName.c_str());
	NSImage * realImage = image->getNSImage();
	[realImage setFlipped:YES];
	[realImage lockFocus];
	[m_image compositeToPoint:NSMakePoint(0,0) fromRect:NSMakeRect(x,y,width,height) operation:NSCompositeCopy];
	[realImage unlockFocus];
	
	return image;
}



bool GR_CocoaImage::_convertPNGFromBuffer(NSData* data, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	if (m_image) {
		[m_image release];
	}
	m_image = [[NSImage alloc] initWithData:data];
	UT_ASSERT (m_image);
	[m_image setFlipped:YES];
	setDisplaySize(iDisplayWidth, iDisplayHeight);
	return (m_image != nil);
}


bool GR_CocoaImage::render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	UT_DEBUGMSG(("Choosing not to render what can't be a raster image!\n"));
	
	return false;
}

void GR_CocoaImage::setFromImageRep(NSImageRep *imageRep)
{ 
	[m_image release]; 
	NSSize size = [imageRep size];
	m_image = [[NSImage alloc] initWithSize:size];
	[m_image setFlipped:YES];
	[m_image addRepresentation:imageRep];
	setDisplaySize(lrintf(size.width), lrintf(size.height));
}

NSImage * GR_CocoaImage::imageFromPNG (NSData * data, UT_uint32 & image_width, UT_uint32 & image_height)
{
	NSImage* image = [[NSImage alloc] initWithData:data];
	UT_ASSERT(image);
	NSSize size = [image size];
	image_width = lrintf(size.width);
	image_height = lrintf(size.height);
	
	return [image autorelease];
}
