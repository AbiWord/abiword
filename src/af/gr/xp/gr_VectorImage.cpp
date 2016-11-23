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
#include <memory>

#include "gr_VectorImage.h"
#include "ut_bytebuf.h"
#include "gr_Graphics.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

GR_VectorImage::GR_VectorImage()
{
}

GR_VectorImage::GR_VectorImage(const char* szName)
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
}

bool GR_VectorImage::convertToBuffer(UT_ConstByteBufPtr & pBB) const
{
  UT_ByteBufPtr bb(new UT_ByteBuf);

  bool bCopied = bb->append(m_pBB_Image->getPointer(0), m_pBB_Image->getLength());
  if (bCopied) {
    pBB = std::move(bb);
  }

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

bool GR_VectorImage::convertFromBuffer(const UT_ConstByteBufPtr & pBB,
                                       const std::string& /*mimetype*/,
                                       UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
  setDisplaySize ( iDisplayWidth, iDisplayHeight );

  m_pBB_Image.reset();
  auto bb = UT_ByteBufPtr(new UT_ByteBuf);

  bool bCopied = bb->append(pBB->getPointer(0), pBB->getLength());

  if (bCopied) {
    m_pBB_Image = std::move(bb);
  }

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
