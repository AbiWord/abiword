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


#include "string.h"

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_misc.h"

#include "ie_imp.h"
#include "ie_imp_AbiWord_1.h"
#include "ie_imp_GZipAbiWord.h"
#include "ie_imp_MsWord_97.h"
#include "ie_imp_RTF.h"
#include "ie_imp_Text.h"
#include "ie_imp_UTF8.h"

/*****************************************************************/
/*****************************************************************/

struct _imp
{
	UT_Bool			(*fpRecognizeSuffix)(const char * szSuffix);
	IEStatus		(*fpStaticConstructor)(PD_Document * pDocument,
										   IE_Imp ** ppie);
	UT_Bool			(*fpGetDlgLabels)(const char ** szDesc,
									  const char ** szSuffixList,
									  IEFileType * ft);
	UT_Bool			(*fpSupportsFileType)(IEFileType ft);
};

#define DeclareImporter(n)	{ n::RecognizeSuffix, n::StaticConstructor, n::GetDlgLabels, n::SupportsFileType }

static struct _imp s_impTable[] =
{
	DeclareImporter(IE_Imp_AbiWord_1),
      DeclareImporter(IE_Imp_GZipAbiWord),
	DeclareImporter(IE_Imp_Text),
	DeclareImporter(IE_Imp_RTF),
	DeclareImporter(IE_Imp_MsWord_97),
	DeclareImporter(IE_Imp_UTF8),
};

		
/*****************************************************************/
/*****************************************************************/

IE_Imp::IE_Imp(PD_Document * pDocument)
{
	m_pDocument = pDocument;
}

IE_Imp::~IE_Imp()
{
}
	
/*****************************************************************/
/*****************************************************************/

IEFileType IE_Imp::fileTypeForSuffix(const char * szSuffix)
{
	if (!szSuffix)
		return IEFT_Text;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	for (UT_uint32 k=0; (k < NrElements(s_impTable)); k++)
	{
		struct _imp * s = &s_impTable[k];
		if (s->fpRecognizeSuffix(szSuffix))
		{
			for (UT_uint32 a = 0; a < (int) IEFT_LAST_BOGUS; a++)
			{
				if (s->fpSupportsFileType((IEFileType) a))
					return (IEFileType) a;
			}

			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// Hm... an importer has registered for the given suffix,
			// bug refuses to support any file type we request.
			// Default to Text.
			return IEFT_Text;
		}
	}

	// No filter is registered for that extension, try Text for import
	return IEFT_Text;
	
}

IEStatus IE_Imp::constructImporter(PD_Document * pDocument,
								   const char * szFilename,
								   IEFileType ieft,
								   IE_Imp ** ppie)
{
	// construct an importer of the right type.
	// caller is responsible for deleting the importer object
	// when finished with it.
	
	UT_ASSERT(pDocument);
	UT_ASSERT(szFilename && *szFilename);
	UT_ASSERT(ppie);

	// no filter will support IEFT_Unknown, so we detect from the
	// suffix of the filename, the real importer to use and assign
	// that back to ieft.
	if (ieft == IEFT_Unknown)
	{
		ieft = IE_Imp::fileTypeForSuffix(UT_pathSuffix(szFilename));
	}

	UT_ASSERT(ieft != IEFT_Unknown);

	// use the importer for the specified file type
	for (UT_uint32 k=0; (k < NrElements(s_impTable)); k++)
	{
		struct _imp * s = &s_impTable[k];
		if (s->fpSupportsFileType(ieft))
			return s->fpStaticConstructor(pDocument,ppie);
	}

	// if we got here, no registered importer handles the
	// type of file we're supposed to be reading.
	// assume it is our format and try to read it.
	// if that fails, just give up.
	*ppie = new IE_Imp_AbiWord_1(pDocument);
	return ((*ppie) ? IES_OK : IES_NoMemory);
}

UT_Bool IE_Imp::enumerateDlgLabels(UT_uint32 ndx,
								   const char ** pszDesc,
								   const char ** pszSuffixList,
								   IEFileType * ft)
{
	if (ndx < NrElements(s_impTable))
		return s_impTable[ndx].fpGetDlgLabels(pszDesc,pszSuffixList,ft);

	return UT_FALSE;
}

UT_uint32 IE_Imp::getImporterCount(void)
{
	return NrElements(s_impTable);
}
