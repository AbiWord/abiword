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
#include "ut_svg.h"

#include "ie_impGraphic_SVG.h"
#include "fg_GraphicVector.h"

UT_Confidence_t IE_ImpGraphicSVG_Sniffer::recognizeSuffix(const char * szSuffix)
{
	if (UT_stricmp(szSuffix,".svg") == 0)
	  return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

UT_Confidence_t IE_ImpGraphicSVG_Sniffer::recognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
  if ( UT_SVG_recognizeContent(szBuf,iNumbytes) )
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}

bool IE_ImpGraphicSVG_Sniffer::getDlgLabels(const char ** pszDesc,
					    const char ** pszSuffixList,
					    IEGraphicFileType * ft)
{
	*pszDesc = "Scalable Vector Graphics (.svg)";
	*pszSuffixList = "*.svg";
	*ft = getType ();
	return true;
}

UT_Error IE_ImpGraphicSVG_Sniffer::constructImporter(IE_ImpGraphic **ppieg)
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
		
		return UT_IE_FAKETYPE;
	}

	*ppfg = static_cast<FG_Graphic *>(pFGR);
	return UT_OK;
}

UT_Error IE_ImpGraphic_SVG::convertGraphic(UT_ByteBuf* pBB,
					   UT_ByteBuf** ppBB)
{
   	if (!ppBB) return UT_ERROR;
   	*ppBB = pBB;
   	return UT_OK;
}
