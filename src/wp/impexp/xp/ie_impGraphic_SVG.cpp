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

#include "ut_string.h"

#include "ie_impGraphic_SVG.h"
#include "fg_GraphicVector.h"

UT_Bool IE_ImpGraphic_SVG::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".svg") == 0);
}

UT_Bool IE_ImpGraphic_SVG::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	UT_uint32 off = 0;
   	for (;;) {
      		while (off < iNumbytes &&
		       (szBuf[off] == ' ' || szBuf[off] == '\t' ||
			szBuf[off] == '\n' || szBuf[off] == '\r')) off++;
      		if (szBuf[off] == '<') {
		   	if ((szBuf[off+1] == 's' || szBuf[off+1] == 'S') &&
			    (szBuf[off+2] == 'v' || szBuf[off+2] == 'V') &&
			    (szBuf[off+3] == 'g' || szBuf[off+3] == 'G') &&
			    (szBuf[off] == ' ' || szBuf[off] == '\t' ||
			     szBuf[off] == '\n' || szBuf[off] == '\r'))
		     		return UT_TRUE;
		   	else {
			   	off++;
			   	while (off < iNumbytes && szBuf[off] != '>') off++;
			}
		}
	   	else return UT_FALSE;
	}
   	return UT_FALSE;
}

UT_Bool IE_ImpGraphic_SVG::GetDlgLabels(const char ** pszDesc,
					const char ** pszSuffixList,
					IEGraphicFileType * ft)
{
	*pszDesc = "Scalable Vector Graphics (.svg)";
	*pszSuffixList = "*.svg";
	*ft = IEGFT_SVG;
	return UT_TRUE;
}

UT_Bool IE_ImpGraphic_SVG::SupportsFileType(IEGraphicFileType ft)
{
	return (IEGFT_SVG == ft);
}

UT_Error IE_ImpGraphic_SVG::StaticConstructor(IE_ImpGraphic **ppieg)
{
	*ppieg = new IE_ImpGraphic_SVG();
	if (*ppieg == NULL)
	  return UT_IE_NOMEMORY;

	return UT_OK;
}

//  This actually creates our FG_Graphic object for a SVG
UT_Error IE_ImpGraphic_SVG::importGraphic(UT_ByteBuf* pBB, 
					  FG_Graphic ** ppfg)
{
	FG_GraphicVector *pFGR;

	pFGR = new FG_GraphicVector();
	if(pFGR == NULL)
		return UT_IE_NOMEMORY;

	if(!pFGR->setVector_SVG(pBB)) {
		DELETEP(pFGR);
		
		return UT_IE_BOGUSDOCUMENT;
	}

	*ppfg = (FG_Graphic *) pFGR;
	return UT_OK;
}
