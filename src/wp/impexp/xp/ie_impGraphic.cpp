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

#include "ie_impGraphic.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_bytebuf.h"
#include "ut_vector.h"

/*****************************************************************/
/*****************************************************************/

static UT_Vector s_impGraphicTable ( 5 );

void IE_ImpGraphic::registerImporter (IE_ImpGraphicSniffer * s)
{
	UT_uint32 ndx = 0;
	UT_Error err = s_impGraphicTable.addItem (s, &ndx);

	UT_ASSERT(err == UT_OK);
	UT_ASSERT(ndx >= 0);

	s->setType(ndx+1);
}

void IE_ImpGraphic::unregisterImporter (IE_ImpGraphicSniffer * s)
{
	UT_uint32 ndx = s->getType(); // 1:1 mapping

	UT_ASSERT(ndx >= 0);

	s_impGraphicTable.deleteNthItem (ndx-1);

	// Refactor the indexes
	IE_ImpGraphicSniffer * pSniffer = 0;
	UT_uint32 size  = s_impGraphicTable.size();
	UT_uint32 i     = 0;
	for( i = ndx-1; i < size; i++)
	{
		pSniffer = static_cast <IE_ImpGraphicSniffer *>(s_impGraphicTable.getNthItem(i));
		if (pSniffer)
        	pSniffer->setType(i+1);
	}
}

void IE_ImpGraphic::unregisterAllImporters ()
{
	IE_ImpGraphicSniffer * pSniffer = 0;
	UT_uint32 size = s_impGraphicTable.size();

	for (UT_uint32 i = 0; i < size; i++)
	{
		pSniffer = static_cast <IE_ImpGraphicSniffer *>(s_impGraphicTable.getNthItem(i));
		if (pSniffer)
			pSniffer->unref();
	}
}

/*****************************************************************/
/*****************************************************************/

IEGraphicFileType IE_ImpGraphic::fileTypeForSuffix(const char * szSuffix)
{
	if (!szSuffix)
		return IEGFT_Unknown;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getImporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpGraphicSniffer * s = static_cast<IE_ImpGraphicSniffer *>(s_impGraphicTable.getNthItem(k));
		if (s->recognizeSuffix(szSuffix))
		{
			for (UT_sint32 a = 0; a < (int) nrElements; a++)
			{
				if (s->supportsType(static_cast<IEGraphicFileType>(a+1)))
					return static_cast<IEGraphicFileType>(a+1);
			}

			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// Hm... an importer has registered for the given suffix,
			// but refuses to support any file type we request.
			return IEGFT_Unknown;
		}
	}

	// No filter is registered for that extension
	return IEGFT_Unknown;
	
}
	
IEGraphicFileType IE_ImpGraphic::fileTypeForContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a match for all file types
	UT_uint32 nrElements = getImporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpGraphicSniffer * s = (IE_ImpGraphicSniffer *)s_impGraphicTable.getNthItem (k);
		if (s->recognizeContents(szBuf, iNumbytes))
		{
			for (UT_sint32 a = 0; a < (int) nrElements; a++)
			{
				if (s->supportsType((IEGraphicFileType) (a+1)))
					return (IEGraphicFileType) (a+1);
			}

			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// Hm... an importer recognizes the given data
			// but refuses to support any file type we request.
			return IEGFT_Unknown;
		}
	}

	// No filter recognizes this data
	return IEGFT_Unknown;	
}
	
bool IE_ImpGraphic::enumerateDlgLabels(UT_uint32 ndx,
					  const char ** pszDesc,
					  const char ** pszSuffixList,
					  IEGraphicFileType * ft)
{
	UT_uint32 nrElements = getImporterCount();
	if (ndx < nrElements)
	{
		IE_ImpGraphicSniffer * s = (IE_ImpGraphicSniffer *) s_impGraphicTable.getNthItem (ndx);
		return s->getDlgLabels(pszDesc,pszSuffixList,ft);
	}

	return false;
}

UT_uint32 IE_ImpGraphic::getImporterCount(void)
{
	return s_impGraphicTable.size ();
}

UT_Error IE_ImpGraphic:: constructImporter(const UT_ByteBuf * bytes,
					   IEGraphicFileType ft,
					   IE_ImpGraphic **ppieg)
{
	// construct an importer of the right type.
	// caller is responsible for deleting the importer object
	// when finished with it.
	UT_ASSERT(ppieg);
	UT_ASSERT(bytes);

	// no filter will support IEGFT_Unknown, so we detect from the
	// suffix of the filename and the contents of the file, the real 
        // importer to use and assign that back to ieft.
	if (ft == IEGFT_Unknown)
	{
	  ft = IE_ImpGraphic::fileTypeForContents( (const char *)bytes->getPointer(0), 
						   bytes->getLength() );
	}

	// use the importer for the specified file type
	for (UT_uint32 k=0; (k < s_impGraphicTable.size()); k++)
	{
		IE_ImpGraphicSniffer * s = (IE_ImpGraphicSniffer*)s_impGraphicTable[k];
		if (s->supportsType(ft))
			return s->constructImporter(ppieg);
	}

	// if we got here, no registered importer handles the
	// type of file we're supposed to be reading.
	return UT_IE_UNKNOWNTYPE;
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
	for (UT_uint32 k=0; (k < s_impGraphicTable.size()); k++)
	{
		IE_ImpGraphicSniffer * s = (IE_ImpGraphicSniffer*)s_impGraphicTable[k];
		if (s->supportsType(ft))
			return s->constructImporter(ppieg);
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
