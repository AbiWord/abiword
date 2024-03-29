/* AbiWord
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

#ifndef GR_VECTORIMAGE_H
#define GR_VECTORIMAGE_H

#include "ut_types.h"
#include "gr_Image.h"

class GR_Graphics;

class ABI_EXPORT GR_VectorImage : public GR_Image
{
public:
	GR_VectorImage(const char* szName);
	GR_VectorImage();
	virtual ~GR_VectorImage();

	virtual bool		convertToBuffer(UT_ConstByteBufPtr & ppBB) const override;
	virtual bool		convertFromBuffer(const UT_ConstByteBufPtr & pBB, const std::string& mimetype,
										  UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) override;

    virtual GR_Image *  createImageSegment(GR_Graphics * /*pG*/,
										   const UT_Rect & /*rec*/) override
		// TODO: we need createImageSegment for converting inline images to positioned images
		// via the context menu
		{ UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED) ; return nullptr; }

	virtual GRType		getType() const override { return GRT_Vector; }
	virtual bool		render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) override;
	virtual bool            hasAlpha(void) const override;
	virtual bool            isTransparentAt(UT_sint32 x, UT_sint32 y) override;

private:
	UT_ConstByteBufPtr m_pBB_Image;
};

#endif /* GR_VECTORIMAGE */
