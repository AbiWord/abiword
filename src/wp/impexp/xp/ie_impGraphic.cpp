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
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_bytebuf.h"
#include "ie_impGraphic.h"

#include "ie_impGraphic_PNG.h"
#include "ie_impGraphic_BMP.h"
#include "ie_impGraphic_SVG.h"

/*****************************************************************/
/*****************************************************************/

struct _impGraphic
{
	bool			(*fpRecognizeSuffix)(const char * szSuffix);
	bool			(*fpRecognizeContents)(const char * szBuf, UT_uint32 iNumbytes);
	bool			(*fpGetDlgLabels)(const char ** szDesc,
									  const char ** szSuffixList,
									  IEGraphicFileType * ft);
	bool			(*fpSupportsFileType)(IEGraphicFileType ft);
        UT_Error		(*fpStaticConstructor)(IE_ImpGraphic **ppieg);
};

#define DeclareImporter(n)	{ n::RecognizeSuffix, n::RecognizeContents, n::GetDlgLabels, n::SupportsFileType, n::StaticConstructor }

static struct _impGraphic s_impGraphicTable[] =
{
	DeclareImporter(IE_ImpGraphic_PNG),
	DeclareImporter(IE_ImpGraphic_BMP),
	DeclareImporter(IE_ImpGraphic_SVG),
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
			// but refuses to support any file type we request.
			return IEGFT_Unknown;
		}
	}

	// No filter is registered for that extension, try Text for import
	return IEGFT_Unknown;
}
	
IEGraphicFileType IE_ImpGraphic::fileTypeForContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	for (UT_uint32 k=0; (k < NrElements(s_impGraphicTable)); k++)
	{
		struct _impGraphic * s = &s_impGraphicTable[k];
		if (s->fpRecognizeContents(szBuf, iNumbytes))
		{
			for (UT_uint32 a = 0; a < (int) IEFT_LAST_BOGUS; a++)
			{
				if (s->fpSupportsFileType((IEGraphicFileType) a))
					return (IEGraphicFileType) a;
			}

			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// Hm... an importer recognizes the given data
			// but refuses to support any file type we request.
			return IEGFT_Unknown;
		}
	}

	// No filter is registered for that extension, try Text for import
	return IEGFT_Unknown;
}
	
bool IE_ImpGraphic::enumerateDlgLabels(UT_uint32 ndx,
					  const char ** pszDesc,
					  const char ** pszSuffixList,
					  IEGraphicFileType * ft)
{
	if (ndx < NrElements(s_impGraphicTable))
		return s_impGraphicTable[ndx].fpGetDlgLabels(pszDesc,pszSuffixList,ft);

	return false;
}

UT_uint32 IE_ImpGraphic::getImporterCount(void)
{
	return NrElements(s_impGraphicTable);
}

UT_Error IE_ImpGraphic::constructImporter(const char * szFilename,
					  IEGraphicFileType ft,
					  IE_ImpGraphic **ppieg)
{
	// construct an importer of the right type.
	// caller is responsible for deleting the importer object
	// when finished with it.
	UT_ASSERT(ppieg);

	// no filter will support IEGFT_Unknown, so we detect from the
	// suffix of the filename and the contents of the file, the real 
        // importer to use and assign that back to ieft.
	if (ft == IEGFT_Unknown)
	{
		UT_ASSERT(szFilename && *szFilename);
		char szBuf[4096];
	   	UT_uint32 iNumbytes;
	   	FILE *f;
	   	if ( ( f= fopen( szFilename, "rb" ) ) != (FILE *)0 )
	     	{
		   	iNumbytes = fread(szBuf, 1, sizeof(szBuf), f);
		   	fclose(f);
	   		ft = IE_ImpGraphic::fileTypeForContents(szBuf, iNumbytes);
		}
	}
	if (ft == IEGFT_Unknown)
     	{
	   	ft = IE_ImpGraphic::fileTypeForSuffix(UT_pathSuffix(szFilename));
	}
   
	// use the importer for the specified file type
	for (UT_uint32 k=0; (k < NrElements(s_impGraphicTable)); k++)
	{
		struct _impGraphic * s = &s_impGraphicTable[k];
		if (s->fpSupportsFileType(ft))
			return s->fpStaticConstructor(ppieg);
	}

	// if we got here, no registered importer handles the
	// type of file we're supposed to be reading.
	return UT_IE_UNKNOWNTYPE;
}


//  Load the contents of the file into a ByteBuffer, and pass it to
//  the other importGraphic function.  Used as a convenience for importing
//  graphics from a file on disk.

UT_Error	IE_ImpGraphic::importGraphic(const char * szFilename,
										 FG_Graphic ** ppfg)
{
	UT_ByteBuf* pBB = new UT_ByteBuf();

	if (pBB == NULL)
		return UT_IE_NOMEMORY;

	if (!pBB->insertFromFile(0, szFilename))
	{
		DELETEP(pBB);
		return UT_IE_FILENOTFOUND;
	}

	//  The ownership of pBB changes here.  The subclass of IE_ImpGraphic
	//  should either delete pBB when it is done importing, or give it
	//  to the FG_Graphic object which is eventually constructed.
	return importGraphic(pBB, ppfg);
}
