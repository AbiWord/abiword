/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:nil; -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002, 2009-2021 Hubert Figuière
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

#pragma once

#include <Cocoa/Cocoa.h>

#include "gr_CocoaGraphics.h"
#include "gr_Image.h"

class GR_CocoaImage
    : public GR_RasterImage
{
public:
    GR_CocoaImage(const char* pszName);
    virtual ~GR_CocoaImage();

    virtual bool convertToBuffer(UT_ConstByteBufPtr& ppBB) const override;
    virtual bool convertFromBuffer(const UT_ConstByteBufPtr& pBB, const std::string& mimetype, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) override;

    virtual bool hasAlpha(void) const override;
    virtual bool isTransparentAt(UT_sint32 x, UT_sint32 y) override;

    virtual GRType getType() const override
    {
        return m_grtype;
    }
    virtual bool render(GR_Graphics* pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) override;

    void setFromImageRep(NSImageRep* imgRep);

    virtual GR_Image* createImageSegment(GR_Graphics* pG, const UT_Rect& rec) override;

    NSImage* getNSImage() const
    {
        return m_image;
    }

private:
    GRType m_grtype;
    NSImage* m_image;
};
