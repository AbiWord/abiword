/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:nil; -*- */
/* AbiWord
 * Copyright (C) 2001-2002, 2009-2021 Hubert Figuière
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "ut_std_string.h"

#import "xap_CocoaAbiConversions.h"

#include "gr_CocoaImage.h"
#include "gr_Graphics.h"

GR_CocoaImage::GR_CocoaImage(const char* szName)
    : m_grtype(GRT_Raster) // Probably the safest default.
    , m_image(nil)
{
    setName(szName ? szName : "CocoaImage");
}

GR_CocoaImage::~GR_CocoaImage()
{
    [m_image release];
}

bool GR_CocoaImage::convertToBuffer(UT_ConstByteBufPtr& pBB) const
{
    UT_ByteBufPtr bb(new UT_ByteBuf);

    UT_ASSERT(UT_NOT_IMPLEMENTED);
    //	[m_pngData convertToAbiByteBuf:pBB];

    pBB = bb;

    return true;
}

void GR_CocoaImage::setFromImageRep(NSImageRep* imageRep)
{
    NSSize size = imageRep.size;
    [m_image release];
    m_image = [[NSImage alloc] initWithSize:size];
    [m_image addRepresentation:imageRep];
    setDisplaySize(lrintf(size.width), lrintf(size.height));
}

GR_Image* GR_CocoaImage::createImageSegment(GR_Graphics* pG, const UT_Rect& rec)
{
    UT_sint32 x = pG->tdu(rec.left);
    UT_sint32 y = pG->tdu(rec.top);
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    UT_sint32 width = pG->tdu(rec.width);
    UT_sint32 height = pG->tdu(rec.height);
    UT_sint32 dH = getDisplayHeight();
    UT_sint32 dW = getDisplayWidth();
    if (height > dH) {
        height = dH;
    }
    if (width > dW) {
        width = dW;
    }
    if (x + width > dW) {
        width = dW - x;
    }
    if (y + height > dH) {
        height = dH - y;
    }
    if (width < 0) {
        x = dW - 1;
        width = 1;
    }
    if (height < 0) {
        y = dH - 1;
        height = 1;
    }
    std::string sName;
    getName(sName);
    std::string sSub = UT_std_string_sprintf("_segemnt_%d_%d_%d_%d", x, y, width, height);
    sName += sSub;

    GR_CocoaImage* image = new GR_CocoaImage(sName.c_str());
    NSImage* realImage = image->getNSImage();
    [realImage lockFocusFlipped:YES];
    [m_image drawAtPoint:NSMakePoint(0, 0) fromRect:NSMakeRect(x, y, width, height) operation:NSCompositingOperationCopy fraction:1.0];
    [realImage unlockFocus];
    return image;
}

/*! 
 * Returns true if pixel at point (x,y) in device units is transparent.
 * See gr_UnixImage.cpp for how it's done in GTK.
 */
bool GR_CocoaImage::isTransparentAt(UT_sint32 /*x*/, UT_sint32 /*y*/)
{
    UT_ASSERT(UT_NOT_IMPLEMENTED);
    return false;
}

/*!
 * Returns true if there is any transparency in the image.
 */
bool GR_CocoaImage::hasAlpha(void) const
{
    UT_ASSERT(UT_NOT_IMPLEMENTED);
    return false;
}

bool GR_CocoaImage::convertFromBuffer(const UT_ConstByteBufPtr& pBB, const std::string& mimetype,
    UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
    const char* buffer = (const char*)pBB->getPointer(0);
    UT_uint32 buflen = pBB->getLength();

    // We explicitely do not scale the image on load, since cairo can do
    // that during rendering. Scaling during loading will result in lost
    // image information, which in turns results in bugs like
    // in bugs like #12183.
    // So simply remember the display size for drawing later.
    setDisplaySize(iDisplayWidth, iDisplayHeight);

    if (mimetype == "image/png") {

        if (buflen < 6) {
            return false;
        }

        char str1[10] = "\211PNG";
        char str2[10] = "<89>PNG";

        if (!(strncmp(buffer, str1, 4)) || !(strncmp(buffer, str2, 6))) {
            m_grtype = GRT_Raster;
            [m_image release];

            NSData* data = [NSData dataWithBytesNoCopy:(void*)pBB->getPointer(0) length:pBB->getLength() freeWhenDone:NO];
            m_image = [[NSImage alloc] initWithData:data];
        }
    } else {
        m_grtype = GRT_Vector;
    }

    return true;
}

bool GR_CocoaImage::render(GR_Graphics* /*pGR*/, UT_sint32 /*iDisplayWidth*/, UT_sint32 /*iDisplayHeight*/)
{
    UT_DEBUGMSG(("Choosing not to render what can't be a raster image!\n"));

    return false;
}
