/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#include "ut_go_file.h"
#include <gsf/gsf-input.h>

/*****************************************************************/
/*****************************************************************/

static UT_GenericVector<IE_ImpGraphicSniffer*> 	IE_IMP_GraphicSniffers (6);
static std::vector<const std::string *> 		IE_IMP_GraphicMimeTypes;
static std::vector<const std::string *> 		IE_IMP_GraphicMimeClasses;

void IE_ImpGraphic::registerImporter (IE_ImpGraphicSniffer * s)
{
	UT_uint32 ndx = 0;
	UT_Error err = IE_IMP_GraphicSniffers.addItem (s, &ndx);

	UT_return_if_fail(err == UT_OK);
	UT_return_if_fail(ndx >= 0);

	s->setType(ndx+1);
}

void IE_ImpGraphic::unregisterImporter (IE_ImpGraphicSniffer * s)
{
	UT_uint32 ndx = s->getType(); // 1:1 mapping

	UT_return_if_fail(ndx >= 0);

	IE_IMP_GraphicSniffers.deleteNthItem (ndx-1);

	// Refactor the indexes
	IE_ImpGraphicSniffer * pSniffer = 0;
	UT_uint32 size  = IE_IMP_GraphicSniffers.size();
	UT_uint32 i     = 0;
	for( i = ndx-1; i < size; i++)
	{
		pSniffer = IE_IMP_GraphicSniffers.getNthItem(i);
		if (pSniffer)
        	pSniffer->setType(i+1);
	}
}

void IE_ImpGraphic::unregisterAllImporters ()
{
	IE_ImpGraphicSniffer * pSniffer = 0;
	UT_uint32 size = IE_IMP_GraphicSniffers.size();

	for (UT_uint32 i = 0; i < size; i++)
	{
		pSniffer = IE_IMP_GraphicSniffers.getNthItem(i);
		if (pSniffer)
			pSniffer->unref();
	}
}

/*!
 * Get supported mimetypes by builtin- and plugin-filters.
 */
std::vector<const std::string *> IE_ImpGraphic::getSupportedMimeTypes ()
{
	if (IE_IMP_GraphicMimeTypes.size() > 0) {
		return IE_IMP_GraphicMimeTypes;
	}

	const IE_MimeConfidence *mc;
	for (guint i = 0; i < IE_IMP_GraphicSniffers.size(); i++) {
		mc = IE_IMP_GraphicSniffers.getNthItem(i)->getMimeConfidence();
		while (mc && mc->match) {
			if (mc->match == IE_MIME_MATCH_FULL) {
				IE_IMP_GraphicMimeTypes.push_back(new std::string(mc->mimetype));
			}
			mc++;
		}
	}

	/* TODO rob: unique */
	return IE_IMP_GraphicMimeTypes;
}

/*!
 * Get supported mime classes by builtin- and plugin-filters.
 */
std::vector<const std::string *> IE_ImpGraphic::getSupportedMimeClasses ()
{
	if (IE_IMP_GraphicMimeClasses.size() > 0) {
		return IE_IMP_GraphicMimeClasses;
	}

	const IE_MimeConfidence *mc;
	for (guint i = 0; i < IE_IMP_GraphicSniffers.size(); i++) {
		mc = IE_IMP_GraphicSniffers.getNthItem(i)->getMimeConfidence();
		while (mc && mc->match) {
			if (mc->match == IE_MIME_MATCH_CLASS) {
				IE_IMP_GraphicMimeClasses.push_back(new std::string(mc->mimetype));
			}
			mc++;
		}
	}

	/* TODO rob: unique */
	return IE_IMP_GraphicMimeClasses;
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
		IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers.getNthItem(k);

		const IE_SuffixConfidence * sc = s->getSuffixConfidence();
		UT_Confidence_t confidence = UT_CONFIDENCE_ZILCH;
		while (sc && sc->suffix) {
			/* suffixes do not have a leading '.' */
			if (0 == UT_stricmp(sc->suffix, szSuffix+1) && 
				sc->confidence > confidence) {
				confidence = sc->confidence;
			}
			sc++;
		}

		if ((confidence > 0) && ((IEGFT_Unknown == best) || (confidence >= best_confidence)))
		{
		        best_confidence = confidence;
			for (UT_sint32 a = 0; a < static_cast<int>(nrElements); a++)
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
		IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers.getNthItem (k);
		UT_Confidence_t confidence = s->recognizeContents(szBuf, iNumbytes);
		if ((confidence > 0) && ((IEGFT_Unknown == best) || (confidence >= best_confidence)))
		{
		        best_confidence = confidence;
			for (UT_sint32 a = 0; a < static_cast<int>(nrElements); a++)
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
		IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers.getNthItem (ndx);
		return s->getDlgLabels(pszDesc,pszSuffixList,ft);
	}

	return false;
}

UT_uint32 IE_ImpGraphic::getImporterCount(void)
{
	return IE_IMP_GraphicSniffers.size ();
}

UT_Error IE_ImpGraphic::constructImporterWithDescription(const char * szDesc, IE_ImpGraphic ** ppieg)
{
	UT_return_val_if_fail(ppieg,  UT_ERROR);
	UT_return_val_if_fail(szDesc, UT_ERROR);

	UT_Error err = UT_ERROR;

	UT_uint32 count = IE_IMP_GraphicSniffers.size();

	for (UT_uint32 i = 0; i < count; i++)
	{
		const char * szDescription = 0;
		const char * szSuffixList  = 0;

		IEGraphicFileType ft = 0;

		IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers.getNthItem(i);

		if (s->getDlgLabels(&szDescription, &szSuffixList, &ft))
			if (szDescription)
				if (UT_strcmp (szDescription, szDesc) == 0)
				{
					err = s->constructImporter(ppieg);
					break;
				}
	}
	return err;
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
	  ft = IE_ImpGraphic::fileTypeForContents( reinterpret_cast<const char *>(bytes->getPointer(0)),
						   bytes->getLength() );
	}

	// use the importer for the specified file type
	for (UT_uint32 k=0; (k < IE_IMP_GraphicSniffers.size()); k++)
	{
		IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers[k];
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
  return (UT_Confidence_t) ( (static_cast<double>(content_confidence) * 0.85) + (static_cast<double>(suffix_confidence) * 0.15) ) ;
}

UT_Error IE_ImpGraphic::constructImporter(const char * szFilename,
					  IEGraphicFileType ft,
					  IE_ImpGraphic **ppieg)
{
  // construct an importer of the right type.
  // caller is responsible for deleting the importer object
  // when finished with it.
  UT_return_val_if_fail(ppieg, UT_ERROR);
  
  UT_uint32 nrElements = IE_IMP_GraphicSniffers.size();
  
  // no filter will support IEGFT_Unknown, so we detect from the
  // suffix of the filename and the contents of the file, the real 
  // importer to use and assign that back to ft.
  if (ft == IEGFT_Unknown)
    {
      UT_return_val_if_fail(szFilename && *szFilename, UT_ERROR);
      char szBuf[4096] = "";
      UT_uint32 iNumbytes = 0;
      GsfInput *f = NULL;
      if ( ( f= UT_go_file_open( szFilename, NULL ) ) != NULL )
	{
	  iNumbytes = UT_MIN(sizeof(szBuf), gsf_input_size(f));
	  gsf_input_read(f, iNumbytes, (guint8*)szBuf);
	  g_object_unref(G_OBJECT(f));
	}
      
      UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;
      
      for (UT_uint32 k=0; k < nrElements; k++)
	{
	  IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers[k];
	  
	  UT_Confidence_t content_confidence = UT_CONFIDENCE_ZILCH;
	  UT_Confidence_t suffix_confidence = UT_CONFIDENCE_ZILCH;
	  
	  if ( iNumbytes > 0 )
	    content_confidence = s->recognizeContents(szBuf, iNumbytes);
	  
	  const char * suffix = UT_pathSuffix(szFilename);
	    if (suffix) {
			const IE_SuffixConfidence * sc = s->getSuffixConfidence();
			while (sc && sc->suffix) {
				/* suffixes do not have a leading '.' */
				if (0 == UT_stricmp(sc->suffix, suffix+1) && 
					sc->confidence > suffix_confidence) {
					suffix_confidence = sc->confidence;
				}
				sc++;
			}
		}

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
      IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers[k];
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

	if (!pBB->insertFromURI(0, szFilename))
	{
		DELETEP(pBB);
		return UT_IE_FILENOTFOUND;
	}

	//  The ownership of pBB changes here.  The subclass of IE_ImpGraphic
	//  should either delete pBB when it is done importing, or give it
	//  to the FG_Graphic object which is eventually constructed.
	return importGraphic(pBB, ppfg);
}
