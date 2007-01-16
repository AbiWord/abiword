/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere 
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

#ifndef GR_COCOAIMAGE_H
#define GR_COCOAIMAGE_H

#import <Cocoa/Cocoa.h>

#include "gr_Image.h"

struct Fatmap
{
  Fatmap ();
	int width;
	int height;

	// Always 24-bit pixel data
	unsigned char * data;
};

class GR_CocoaImage : public GR_RasterImage
{
public:
	GR_CocoaImage(const char* pszName);
	virtual ~GR_CocoaImage();

	virtual bool		convertToBuffer(UT_ByteBuf** ppBB) const;
	virtual bool		convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

    virtual GR_Image *  createImageSegment(GR_Graphics * pG, const UT_Rect & rec);
	virtual bool hasAlpha (void) const;
	virtual bool isTransparentAt(UT_sint32 x, UT_sint32 y);

//	void			setData(Fatmap * image) { m_image = image; }

   	virtual GRType		getType() const { return m_grtype; }
   	virtual bool		render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
	
	void setFromImageRep(NSImageRep *imgRep);
	NSImage * getNSImage ()
		{ return m_image; };

	static NSImage * imageFromPNG (NSData * data, UT_uint32 & image_width, UT_uint32 & image_height);

private:
	NSImage*	m_image;
	NSData*		m_pngData;

	GRType m_grtype;

	bool _convertPNGFromBuffer(NSData* data, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

//	bool _convertPNGFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
};

#endif /* GR_COCOAIMAGE_H */
