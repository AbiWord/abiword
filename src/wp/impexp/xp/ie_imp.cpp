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
#include "ie_imp.h"
#include "ie_imp_AbiWord_1.h"
#include "ie_imp_MsWord_97.h"
#include "ie_imp_RTF.h"
#include "ie_imp_Text.h"
// #include "ie_types.h"

/*****************************************************************/
/*****************************************************************/

struct _imp
{
	UT_Bool			(*fpRecognizeSuffix)(const char * szSuffix);
	IEStatus		(*fpStaticConstructor)(PD_Document * pDocument,
										   IE_Imp ** ppie);
	UT_Bool			(*fpGetDlgLabels)(const char ** szDesc,
									  const char ** szSuffixList);
	UT_Bool			(*fpSupportsFileType)(IEFileType ft);
};

#define DeclareImporter(n)	{ n::RecognizeSuffix, n::StaticConstructor, n::GetDlgLabels, n::SupportsFileType }

static struct _imp s_impTable[] =
{
	DeclareImporter(IE_Imp_AbiWord_1),
	DeclareImporter(IE_Imp_Text),
	DeclareImporter(IE_Imp_RTF),
	DeclareImporter(IE_Imp_MsWord_97),
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

	// if a file type is specified, use that importer
	if (ieft != IEFT_Unknown)
	{
		for (UT_uint32 k=0; (k < NrElements(s_impTable)); k++)
		{
			struct _imp * s = &s_impTable[k];
			if (s->fpSupportsFileType(ieft))
				return s->fpStaticConstructor(pDocument,ppie);
		}
	}

	// if we got here, the user wants auto-detect
	
	/*
	  This might be somewhat broken if one has filenames with
	  multiple periods in it, or if one has specified the
	  directories "." or ".." in the filename, etc.
	  
	  Suffix matching is just pretty broken.
	*/
	const char * pExt = strrchr(szFilename,'.');
	
	if (!pExt)
	{
		// no suffix -- what to do ??
		// assume it is our format and try to read it.
		// if that fails, just give up.
		
		*ppie = new IE_Imp_AbiWord_1(pDocument);
		return ((*ppie) ? IES_OK : IES_NoMemory);
	}

	for (UT_uint32 k=0; (k < NrElements(s_impTable)); k++)
	{
		struct _imp * s = &s_impTable[k];
		if (s->fpRecognizeSuffix(pExt))
			return s->fpStaticConstructor(pDocument,ppie);
	}
	
	return IES_UnknownType;
}

UT_Bool IE_Imp::enumerateDlgLabels(UT_uint32 ndx,
								   const char ** pszDesc,
								   const char ** pszSuffixList)
{
	if (ndx < NrElements(s_impTable))
		return s_impTable[ndx].fpGetDlgLabels(pszDesc,pszSuffixList);

	return UT_FALSE;
}
