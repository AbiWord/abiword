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

static UT_Vector s_impGraphicTable ( 6 );

void IE_ImpGraphic::registerImporter (IE_ImpGraphicSniffer * s)
{
	UT_uint32 ndx = 0;
	UT_Error err = s_impGraphicTable.addItem (s, &ndx);

	UT_return_if_fail(err == UT_OK);
	UT_return_if_fail(ndx >= 0);

	s->setType(ndx+1);
}

void IE_ImpGraphic::unregisterImporter (IE_ImpGraphicSniffer * s)
{
	UT_uint32 ndx = s->getType(); // 1:1 mapping

	UT_return_if_fail(ndx >= 0);

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
	if (!szSuffix || !strlen(szSuffix))
		return IEGFT_Unknown;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getImporterCount();

	IEGraphicFileType best = IEGFT_Unknown;
	UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpGraphicSniffer * s = static_cast<IE_ImpGraphicSniffer *>(s_impGraphicTable.getNthItem(k));
		UT_Confidence_t confidence = s->recognizeSuffix(szSuffix);
		if ((confidence > 0) && ((IEGFT_Unknown == best) || (confidence >= best_confidence)))
		{
		        best_confidence = confidence;
			for (UT_sint32 a = 0; a < (int) nrElements; a++)
			{
				if (s->supportsType(static_cast<IEGraphicFileType>(a+1)))
				  {
				    best = static_cast<IEGraphicFileType>(a+1);
				    
				    // short-circuit if we're 100% sure
				    if ( UT_CONFIDENCE_PERFECT == best_confidence )
				      return best;
				    break;
				  }
			}
		}
	}

	return best;	
}
	
IEGraphicFileType IE_ImpGraphic::fileTypeForContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a match for all file types
	UT_uint32 nrElements = getImporterCount();

	IEGraphicFileType best = IEGFT_Unknown;
	UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpGraphicSniffer * s = (IE_ImpGraphicSniffer *)s_impGraphicTable.getNthItem (k);
		UT_Confidence_t confidence = s->recognizeContents(szBuf, iNumbytes);
		if ((confidence > 0) && ((IEGFT_Unknown == best) || (confidence >= best_confidence)))
		{
		        best_confidence = confidence;
			for (UT_sint32 a = 0; a < (int) nrElements; a++)
			{
				if (s->supportsType((IEGraphicFileType) (a+1)))
				  {
				    best = static_cast<IEGraphicFileType>(a+1);
				    
				    // short-circuit if we're 100% sure
				    if ( UT_CONFIDENCE_PERFECT == best_confidence )
				      return best;
				    break;
				  }
			}
		}
	}

	return best;
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
	UT_return_val_if_fail(ppieg, UT_ERROR);
	UT_return_val_if_fail(bytes, UT_ERROR);

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

static UT_Confidence_t s_condfidence_heuristic ( UT_Confidence_t content_confidence, 
						 UT_Confidence_t suffix_confidence )
{
  return (UT_Confidence_t) ( ((double)content_confidence * 0.85) + ((double)suffix_confidence * 0.15) ) ;
}

UT_Error IE_ImpGraphic::constructImporter(const char * szFilename,
					  IEGraphicFileType ft,
					  IE_ImpGraphic **ppieg)
{
  // construct an importer of the right type.
  // caller is responsible for deleting the importer object
  // when finished with it.
  UT_return_val_if_fail(ppieg, UT_ERROR);
  
  UT_uint32 nrElements = s_impGraphicTable.size();
  
  // no filter will support IEGFT_Unknown, so we detect from the
  // suffix of the filename and the contents of the file, the real 
  // importer to use and assign that back to ft.
  if (ft == IEGFT_Unknown)
    {
      UT_ASSERT(szFilename && *szFilename);
      char szBuf[4096] = "";
      UT_uint32 iNumbytes = 0;
      FILE *f = NULL;
      if ( ( f= fopen( szFilename, "rb" ) ) != (FILE *)0 )
	{
	  iNumbytes = fread(szBuf, 1, sizeof(szBuf), f);
	  fclose(f);
	}
      
      UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;
      
      for (UT_uint32 k=0; k < nrElements; k++)
	{
	  IE_ImpGraphicSniffer * s = (IE_ImpGraphicSniffer*)s_impGraphicTable[k];
	  
	  UT_Confidence_t content_confidence = UT_CONFIDENCE_ZILCH;
	  UT_Confidence_t suffix_confidence = UT_CONFIDENCE_ZILCH;
	  
	  if ( iNumbytes > 0 )
	    content_confidence = s->recognizeContents(szBuf, iNumbytes);
	  
	  const char * suffix = UT_pathSuffix(szFilename) ;
	  if ( suffix != NULL )
	    suffix_confidence = s->recognizeSuffix(UT_pathSuffix(szFilename));
	  
	  UT_Confidence_t confidence = s_condfidence_heuristic ( content_confidence, 
								 suffix_confidence ) ;
	  
	  if ( confidence != 0 && confidence >= best_confidence )
	    {
	      best_confidence = confidence;
	      ft = (IEGraphicFileType)(k+1);
	    }
	}
    }
   
  // use the importer for the specified file type
  for (UT_uint32 k=0; (k < nrElements); k++)
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
