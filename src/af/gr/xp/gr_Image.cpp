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

#include "ut_assert.h"

GR_Image::GR_Image()
{
	m_szName[0] = 0;
	m_iLayoutWidth = 0;
	m_iLayoutHeight = 0;
	m_iDisplayWidth = 0;
	m_iDisplayHeight = 0;
}

GR_Image::~GR_Image()
{
}

void GR_Image::getName(char* p) const
{
	UT_ASSERT(p);
	
	strcpy(p, m_szName);
}

GR_Image::GRType GR_Image::getBufferType(const UT_ByteBuf * pBB)
{
   const char * buf = (const char*)pBB->getPointer(0);
   char * comp1 = "\211PNG";
   char * comp2 = "<89>PNG";
   if (!(strncmp((const char*)buf, comp1, 4)) || !(strncmp((const char*)buf, comp2, 6)))
     return GR_Image::GRT_Raster;
   else {
      UT_uint32 len = pBB->getLength();
      if (len > 1000) len = 1000; // only scan first 1000 bytes
      UT_uint32 off = 0;
      for (;;) {
	 while (off < len &&
		(buf[off] == ' ' || buf[off] == '\t' || 
		 buf[off] == '\n' || buf[off] == '\r')) off++;
	 if (buf[off] == '<') {
	    if ((buf[off+1] == 's' || buf[off+1] == 'S') &&
		(buf[off+2] == 'v' || buf[off+2] == 'V') &&
		(buf[off+3] == 'g' || buf[off+3] == 'G') &&
		(buf[off] == ' ' || buf[off] == '\t' ||
		 buf[off] == '\n' || buf[off] == '\r'))
	      return GR_Image::GRT_Vector;
	    else {
	       off++;
	       while (off < len && buf[off] != '>') off++;
	    }
	 }
	 else return GR_Image::GRT_Unknown;
      }
   }
   return GR_Image::GRT_Unknown;
}
