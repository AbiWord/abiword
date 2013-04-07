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

#include <string.h>

#include "gr_VectorImage.h"
#include "ut_bytebuf.h"
#include "gr_Graphics.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

GR_VectorImage::GR_VectorImage()
	: m_pBB_Image(0)
{
}

GR_VectorImage::GR_VectorImage(const char* szName)
	: m_pBB_Image(0)
{
   if (szName)
     {
       setName (szName);
     }
   else
     {
       setName ( "VectorImage" );
     }
}

GR_VectorImage::~GR_VectorImage()
{
  DELETEP(m_pBB_Image);
}

bool GR_VectorImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
  UT_ByteBuf* pBB = new UT_ByteBuf;

  bool bCopied = pBB->append(m_pBB_Image->getPointer(0), m_pBB_Image->getLength());

  if (!bCopied) DELETEP(pBB);

  *ppBB = pBB;

  return bCopied;
}

bool   GR_VectorImage::hasAlpha(void) const
{
  UT_ASSERT(0);
  return false;
}

bool   GR_VectorImage::isTransparentAt(UT_sint32 /*x*/, UT_sint32 /*y*/)
{
  UT_ASSERT(0);
  return false;
}

bool GR_VectorImage::convertFromBuffer(const UT_ByteBuf* pBB, 
                                       const std::string& /*mimetype*/,
									   UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
  setDisplaySize ( iDisplayWidth, iDisplayHeight );

  DELETEP(m_pBB_Image);

  m_pBB_Image = new UT_ByteBuf;

  bool bCopied = m_pBB_Image->append(pBB->getPointer(0), pBB->getLength());

  if (!bCopied) DELETEP(m_pBB_Image);

  return bCopied;
}

bool GR_VectorImage::render(GR_Graphics* pGR, UT_sint32 xDest, UT_sint32 yDest)
{
  UT_UNUSED(pGR);
  UT_UNUSED(xDest);
  UT_UNUSED(yDest);
  UT_ASSERT_NOT_REACHED();
  return false;
}
