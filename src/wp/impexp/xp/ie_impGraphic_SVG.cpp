/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ut_string.h"
#include "ut_svg.h"

#include "ie_impGraphic_SVG.h"
#include "fg_GraphicVector.h"

// supported suffixes
static IE_SuffixConfidence IE_ImpGraphicSVG_Sniffer__SuffixConfidence[] = {
	{ "svg", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_ImpGraphicSVG_Sniffer::getSuffixConfidence ()
{
	return IE_ImpGraphicSVG_Sniffer__SuffixConfidence;
}

// supported mimetypes
static IE_MimeConfidence IE_ImpGraphicSVG_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	"image/svg+xml",	UT_CONFIDENCE_PERFECT 	},
	{ IE_MIME_MATCH_FULL, 	"image/svg",	UT_CONFIDENCE_PERFECT 	},
	{ IE_MIME_MATCH_FULL, 	"image/svg-xml",	UT_CONFIDENCE_PERFECT 	},
	{ IE_MIME_MATCH_FULL, 	"image/xml-svg",	UT_CONFIDENCE_PERFECT 	},
	{ IE_MIME_MATCH_FULL, 	"image/vnd.adobe.svg+xml",	UT_CONFIDENCE_PERFECT 	},
	{ IE_MIME_MATCH_FULL, 	"image/svg+xml-compressed",	UT_CONFIDENCE_PERFECT 	},
	{ IE_MIME_MATCH_BOGUS, 	"", 			UT_CONFIDENCE_ZILCH }
};

const IE_MimeConfidence * IE_ImpGraphicSVG_Sniffer::getMimeConfidence ()
{
	return IE_ImpGraphicSVG_Sniffer__MimeConfidence;
}

UT_Confidence_t IE_ImpGraphicSVG_Sniffer::recognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
  UT_DEBUGMSG(("SVG SNIFF happenning \n"));
  if ( UT_SVG_recognizeContent(szBuf,iNumbytes) )
  {
    UT_DEBUGMSG(("NOT SVG \n"));
    return UT_CONFIDENCE_PERFECT;
  }
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
                                          FG_ConstGraphicPtr& pfg)
{
	FG_GraphicVectorPtr pFGR(new FG_GraphicVector);
	if(pFGR == NULL)
		return UT_IE_NOMEMORY;

	if(!pFGR->setVector_SVG(pBB)) {

		return UT_IE_FAKETYPE;
	}

	pfg = std::move(pFGR);
	return UT_OK;
}

