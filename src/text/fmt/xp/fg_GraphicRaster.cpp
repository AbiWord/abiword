/* AbiWord -- Embedded graphics for layout
 * Copyright (C) 1999 Matt Kimball
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


#include "ut_assert.h"
#include "ut_png.h"

#include "fg_GraphicRaster.h"

FG_GraphicRaster::FG_GraphicRaster()
{
	m_pbbPNG = NULL;
}

FG_GraphicRaster::~FG_GraphicRaster()
{
	DELETEP(m_pbbPNG);
}

FGType FG_GraphicRaster::getType(void)
{
	return FGT_Raster;
}

double FG_GraphicRaster::getWidth(void)
{
	UT_ASSERT(m_pbbPNG);

	return m_iWidth / 72.0;
}

double FG_GraphicRaster::getHeight(void)
{
	UT_ASSERT(m_pbbPNG);

	return m_iHeight / 72.0;
}

UT_Bool FG_GraphicRaster::setRaster_PNG(UT_ByteBuf* pBB)
{
	DELETEP(m_pbbPNG);

	m_pbbPNG = pBB;
	//  We want to calculate the dimensions of the image here.
	UT_PNG_getDimensions(pBB, m_iWidth, m_iHeight);

	return UT_TRUE;
}

UT_ByteBuf* FG_GraphicRaster::getRaster_PNG(void)
{
	UT_ASSERT(m_pbbPNG);

	return m_pbbPNG;
}

