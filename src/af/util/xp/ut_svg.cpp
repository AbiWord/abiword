/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "ut_units.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"

bool UT_SVG_getDimensions(const UT_ByteBuf* pBB,
			     UT_Byte ** ppszWidth, 
			     UT_Byte ** ppszHeight)
{
   	const UT_Byte* pData = pBB->getPointer(0);
   
   	UT_sint32 iOffset = 0;
   	UT_sint32 iLength = pBB->getLength();
   
   	while (isspace(pData[iOffset]) && iOffset < iLength) iOffset++;
   	// the first thing we see should be <svg followed by a character of whitespace...
   	if (iOffset >= iLength) return false;
   	if (UT_strnicmp((const char*)(pData+iOffset), "<svg", 4) != 0) return false;
   	if (!isspace(pData[iOffset+4])) return false;
   	iOffset+=5;
   
   	UT_Byte * width = NULL, * height = NULL;
   	while (!width || !height) {
	   while (isspace(pData[iOffset]) && iOffset < iLength) iOffset++;
	   if (iOffset >= iLength || pData[iOffset] == '>')  return false;

	   if (UT_strnicmp((const char*)(pData+iOffset), "width", 5) == 0 ||
	       UT_strnicmp((const char*)(pData+iOffset), "height", 6) == 0) {
	      bool bWidth = (pData[iOffset] == 'w' || pData[iOffset] == 'W');

	      if (bWidth) iOffset += 5;
	      else iOffset += 6;
	      
	      while (isspace(pData[iOffset]) && iOffset < iLength) iOffset++;
	      if (pData[iOffset] != '=') return false;
	      iOffset++;
	      
	      while (isspace(pData[iOffset]) && iOffset < iLength) iOffset++;
	      if (pData[iOffset] != '"') return false;
	      iOffset++;
	      
	      int iStart = iOffset;
	      
	      while (pData[iOffset] != '"' && iOffset < iLength) iOffset++;
	      if (pData[iOffset] != '"') return false;

	      UT_Byte *pDim = (UT_Byte*)malloc((iOffset-iStart+1)*sizeof(UT_Byte));
	      UT_ASSERT(pDim != NULL);

	      strncpy((char*)pDim, (const char*)(pData+iStart), iOffset-iStart);
	      pDim[iOffset-iStart] = 0;
	      iOffset++;
	      
	      if (bWidth)
		width = pDim;
	      else
		height = pDim;

	   } else {
	      // skips two quotes
	      while (pData[iOffset] != '"' && iOffset < iLength) iOffset++;
	      iOffset++;
	      while (pData[iOffset] != '"' && iOffset < iLength) iOffset++;
	      iOffset++;
	   }
	}		  

   	if (ppszWidth) *ppszWidth = width;
	if (ppszHeight) *ppszHeight = height;

	return true;
}

