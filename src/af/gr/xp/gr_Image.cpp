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
#include "ut_bytebuf.h"
#include "ut_svg.h"
#include "ut_assert.h"

GR_Image::GR_Image()
  : m_szName(""), m_iDisplayWidth(0), m_iDisplayHeight(0)
{
}

GR_Image::~GR_Image()
{
}

void GR_Image::getName(char* p) const
{
	UT_ASSERT(p);
	
	strcpy(p, m_szName.c_str());
}

void GR_Image::getName ( UT_String & copy ) const
{
  // assign
  copy = m_szName;
}

void GR_Image::setName ( const char * name )
{
  m_szName = name;
}

void GR_Image::setName ( const UT_String & name )
{
  m_szName = name;
}

GR_Image::GRType GR_Image::getBufferType(const UT_ByteBuf * pBB)
{
   const char * buf = reinterpret_cast<const char*>(pBB->getPointer(0));
   UT_uint32 len = pBB->getLength();

   if (len < 6) return GR_Image::GRT_Unknown;

   char * comp1 = "\211PNG";
   char * comp2 = "<89>PNG";
   if (!(strncmp(static_cast<const char*>(buf), comp1, 4)) || !(strncmp(static_cast<const char*>(buf), comp2, 6)))
     return GR_Image::GRT_Raster;

   if (UT_SVG_recognizeContent (buf,len))
     return GR_Image::GRT_Vector;

   return GR_Image::GRT_Unknown;
}

void GR_Image::setDisplaySize(UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) 
{ 
  m_iDisplayWidth = iDisplayWidth; 
  m_iDisplayHeight = iDisplayHeight; 
}
	
UT_sint32 GR_Image::getDisplayWidth(void) const 
{ 
  return m_iDisplayWidth; 
}

UT_sint32 GR_Image::getDisplayHeight(void) const 
{ 
  return m_iDisplayHeight; 
}

bool GR_Image::convertToBuffer(UT_ByteBuf** ppBB) const 
{ 
  // default no impl
  return false; 
}

bool GR_Image::convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iWidth, UT_sint32 iHeight) 
{ 
  // default no impl
  UT_ASSERT_NOT_REACHED ();
  return false; 
}

GR_Image::GRType GR_Image::getType() const
{ 
//
// While this is technically the right thing to do it screws up printing on Windows and Gnome
//
//  return GRT_Unknown;
//
// FIXME: Subclasses should ensure this works.
	return GRT_Raster;
}

bool GR_Image::render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{ 
  UT_ASSERT_NOT_REACHED ();
  return false; 
}
