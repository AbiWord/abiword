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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <string.h>

#include "gr_Image.h"

#include "ut_assert.h"

GR_Image::GR_Image()
{
	m_szName[0] = 0;
}

GR_Image::~GR_Image()
{
}

void GR_Image::getName(char* p) const
{
	UT_ASSERT(p);
	
	strcpy(p, m_szName);
}

void GR_Image::setLayoutSize(UT_sint32 iLayoutWidth, UT_sint32 iLayoutHeight)
{
	m_iLayoutWidth = iLayoutWidth;
	m_iLayoutHeight = iLayoutHeight;
}

GR_StretchableImage::GR_StretchableImage()
{
	m_iDisplayWidth = 0;
	m_iDisplayHeight = 0;
}

GR_StretchableImage::~GR_StretchableImage()
{
}

void GR_StretchableImage::setDisplaySize(UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	m_iDisplayWidth = iDisplayWidth;
	m_iDisplayHeight = iDisplayHeight;
}

