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

#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "ut_exception.h"

#import "xap_CocoaAbiConversions.h"

#include "gr_CocoaImage.h"


GR_CocoaImage::GR_CocoaImage(const char* szName)
  : m_image(nil),
	m_pngData(nil),
    m_grtype(GRT_Raster), // Probably the safest default.
    m_iDisplayWidth(0),
    m_iDisplayHeight(0)
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

UT_sint32	GR_CocoaImage::getDisplayWidth(void) const
{
	return m_iDisplayWidth;
}

UT_sint32	GR_CocoaImage::getDisplayHeight(void) const
{
	return m_iDisplayHeight;
}

bool		GR_CocoaImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
	UT_ByteBuf* pBB = new UT_ByteBuf();

	[m_pngData convertToAbiByteBuf:pBB];
	
	*ppBB = pBB;
	
	return true;
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

	m_iDisplayWidth  = iDisplayWidth;
	m_iDisplayHeight = iDisplayHeight;

	return true;
}


bool GR_CocoaImage::_convertPNGFromBuffer(const NSData* data, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	if (m_image) {
		[m_image release];
	}
	m_image = [[NSImage alloc] initWithData:data];
	UT_ASSERT (m_image);
	m_iDisplayWidth = iDisplayWidth;
	m_iDisplayHeight = iDisplayHeight;
	return (m_image != nil);
}


bool GR_CocoaImage::render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	UT_DEBUGMSG(("Choosing not to render what can't be a raster image!\n"));
	return false;
}


NSImage * GR_CocoaImage::imageFromPNG (NSData * data, UT_uint32 & image_width, UT_uint32 & image_height)
{
	NSImage* image = [[NSImage alloc] initWithData:data];
	UT_ASSERT(image);
	NSSize size = [image size];
	image_width = size.width;
	image_height = size.height;
	
	return [image autorelease];
}
