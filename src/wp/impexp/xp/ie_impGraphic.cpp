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

#include "ut_assert.h"
#include "ie_impGraphic.h"

#include "ie_impGraphic_PNG.h"
// #include "ie_impGraphic_SVG.h"

/*****************************************************************/
/*****************************************************************/

struct _impGraphic
{
	UT_Bool			(*fpRecognizeSuffix)(const char * szSuffix);
	UT_Bool			(*fpGetDlgLabels)(const char ** szDesc,
									  const char ** szSuffixList,
									  IEGraphicFileType * ft);
	UT_Bool			(*fpSupportsFileType)(IEGraphicFileType ft);
};

#define DeclareImporter(n)	{ n::RecognizeSuffix, n::GetDlgLabels, n::SupportsFileType }

static struct _impGraphic s_impGraphicTable[] =
{
	DeclareImporter(IE_ImpGraphic_PNG),
	//	DeclareImporter(IE_ImpGraphic_SVG),
};

/*****************************************************************/
/*****************************************************************/

IEGraphicFileType IE_ImpGraphic::fileTypeForSuffix(const char * szSuffix)
{
	if (!szSuffix)
		return IEGFT_Unknown;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	for (UT_uint32 k=0; (k < NrElements(s_impGraphicTable)); k++)
	{
		struct _impGraphic * s = &s_impGraphicTable[k];
		if (s->fpRecognizeSuffix(szSuffix))
		{
			for (UT_uint32 a = 0; a < (int) IEFT_LAST_BOGUS; a++)
			{
				if (s->fpSupportsFileType((IEGraphicFileType) a))
					return (IEGraphicFileType) a;
			}

			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// Hm... an importer has registered for the given suffix,
			// bug refuses to support any file type we request.
			return IEGFT_Unknown;
		}
	}

	// No filter is registered for that extension, try Text for import
	return IEGFT_Unknown;
}
	
UT_Bool IE_ImpGraphic::enumerateDlgLabels(UT_uint32 ndx,
										  const char ** pszDesc,
										  const char ** pszSuffixList,
										  IEGraphicFileType * ft)
{
	if (ndx < NrElements(s_impGraphicTable))
		return s_impGraphicTable[ndx].fpGetDlgLabels(pszDesc,pszSuffixList,ft);

	return UT_FALSE;
}

UT_uint32 IE_ImpGraphic::getImporterCount(void)
{
	return NrElements(s_impGraphicTable);
}
