/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ie_impGraphic.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_bytebuf.h"
#include "ut_vector.h"

#include "ut_go_file.h"
#include <gsf/gsf-input.h>
#include <gsf/gsf-input-memory.h>

#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"

/*****************************************************************/
/*****************************************************************/

static UT_GenericVector<IE_ImpGraphicSniffer*> 	IE_IMP_GraphicSniffers (6);
static std::vector<std::string> 		IE_IMP_GraphicMimeTypes;
static std::vector<std::string> 		IE_IMP_GraphicMimeClasses;
static std::vector<std::string> 		IE_IMP_GraphicSuffixes;

void IE_ImpGraphic::registerImporter (IE_ImpGraphicSniffer * s)
{
	UT_sint32 ndx = 0;
	UT_Error err = IE_IMP_GraphicSniffers.addItem (s, &ndx);

	UT_return_if_fail(err == UT_OK);

	s->setType(ndx+1);
}

void IE_ImpGraphic::unregisterImporter (IE_ImpGraphicSniffer * s)
{
	UT_uint32 ndx = s->getType(); // 1:1 mapping

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
	// Delete the supported types lists
	IE_IMP_GraphicMimeTypes.clear();
	IE_IMP_GraphicMimeClasses.clear();
	IE_IMP_GraphicSuffixes.clear();
}

void IE_ImpGraphic::unregisterAllImporters ()
{
	IE_ImpGraphicSniffer * pSniffer = 0;
	UT_uint32 size = IE_IMP_GraphicSniffers.size();

	for (UT_uint32 i = 0; i < size; i++)
	{
		pSniffer = IE_IMP_GraphicSniffers.getNthItem(i);
		DELETEP(pSniffer);
	}

	IE_IMP_GraphicSniffers.clear();
}

/*!
 * Get supported mimetypes by builtin- and plugin-filters.
 */
const std::vector<std::string> & IE_ImpGraphic::getSupportedMimeTypes()
{
	if (IE_IMP_GraphicMimeTypes.size() > 0) {
		return IE_IMP_GraphicMimeTypes;
	}

	const IE_MimeConfidence *mc;
	for (UT_sint32 i = 0; i < IE_IMP_GraphicSniffers.size(); i++) {
		mc = IE_IMP_GraphicSniffers.getNthItem(i)->getMimeConfidence();
		while (mc && mc->match) {
			if (mc->match == IE_MIME_MATCH_FULL) {
				IE_IMP_GraphicMimeTypes.push_back(mc->mimetype);
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
const std::vector<std::string> & IE_ImpGraphic::getSupportedMimeClasses()
{
	if (IE_IMP_GraphicMimeClasses.size() > 0) {
		return IE_IMP_GraphicMimeClasses;
	}

	const IE_MimeConfidence *mc;
	for (UT_sint32 i = 0; i < IE_IMP_GraphicSniffers.size(); i++) {
		mc = IE_IMP_GraphicSniffers.getNthItem(i)->getMimeConfidence();
		while (mc && mc->match) {
			if (mc->match == IE_MIME_MATCH_CLASS) {
				IE_IMP_GraphicMimeClasses.push_back(mc->mimetype);
			}
			mc++;
		}
	}

	/* TODO rob: unique */
	return IE_IMP_GraphicMimeClasses;
}

/*!
 * Get supported suffixes by builtin- and plugin-filters.
 */
const std::vector<std::string> & IE_ImpGraphic::getSupportedSuffixes()
{
	if (IE_IMP_GraphicSuffixes.size() > 0) {
		return IE_IMP_GraphicSuffixes;
	}

	const IE_SuffixConfidence *sc;
	for (UT_sint32 i = 0; i < IE_IMP_GraphicSniffers.size(); i++) {
		sc = IE_IMP_GraphicSniffers.getNthItem(i)->getSuffixConfidence();
		while (sc && !sc->suffix.empty()) {
			IE_IMP_GraphicSuffixes.push_back(sc->suffix);
			sc++;
		}
	}
	
	/* TODO rob: unique */
	return IE_IMP_GraphicSuffixes;
}

/*!
 * Map mime type to a suffix. Returns NULL if not found.
 */
const char * IE_ImpGraphic::getMimeTypeForSuffix(const char * suffix)
{
	if (!suffix || !(*suffix))
		return NULL;
		
	if (suffix[0] == '.') {
		suffix++;
	}

	const IE_SuffixConfidence *sc;
	for (UT_sint32 i = 0; i < IE_IMP_GraphicSniffers.size(); i++) {
		IE_ImpGraphicSniffer *sniffer = IE_IMP_GraphicSniffers.getNthItem(i);
		sc = sniffer->getSuffixConfidence();
		while (sc && !sc->suffix.empty()) {
			if (0 == g_ascii_strcasecmp(suffix, sc->suffix.c_str())) {
				const IE_MimeConfidence *mc = sniffer->getMimeConfidence();
				if (mc) {
					return mc->mimetype.c_str();
				}
				else {
					return NULL;
				}
			}
			sc++;
		}
	}

	return NULL;
}

/*****************************************************************/
/*****************************************************************/

IEGraphicFileType IE_ImpGraphic::fileTypeForMimetype(const char * szMimetype)
{
	if (!szMimetype || !strlen(szMimetype))
		return IEGFT_Unknown;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a mimetype match for all file types
	UT_uint32 nrElements = getImporterCount();

	IEGraphicFileType best = IEGFT_Unknown;
	UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers.getNthItem(k);

		const IE_MimeConfidence * mc = s->getMimeConfidence();
		UT_Confidence_t confidence = UT_CONFIDENCE_ZILCH;
		while (mc && mc->match) {
			if (mc->match == IE_MIME_MATCH_FULL) {
				if (0 == g_ascii_strcasecmp(mc->mimetype.c_str(), szMimetype) && 
					mc->confidence > confidence) {
					confidence = mc->confidence;
				}
			}
			mc++;
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
		while (sc && !sc->suffix.empty()) {
			/* suffixes do not have a leading '.' */
			if (0 == g_ascii_strcasecmp(sc->suffix.c_str(), szSuffix+1) && 
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
	GsfInput * input = gsf_input_memory_new ((guint8 *)szBuf, (gsf_off_t)iNumbytes, FALSE);
	if (!input)
		return IEGFT_Unknown;

	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a match for all file types
	UT_uint32 nrElements = getImporterCount();

	IEGraphicFileType best = IEGFT_Unknown;
	UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers.getNthItem (k);
		UT_Confidence_t confidence = s->recognizeContents(input);
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

	g_object_unref (G_OBJECT (input));

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
				if (strcmp (szDescription, szDesc) == 0)
				{
					err = s->constructImporter(ppieg);
					break;
				}
	}
	return err;
}

UT_Error IE_ImpGraphic:: constructImporter(const UT_ByteBuf & bytes,
					   IEGraphicFileType ft,
					   IE_ImpGraphic **ppieg)
{
	// construct an importer of the right type.
	// caller is responsible for deleting the importer object
	// when finished with it.
	UT_return_val_if_fail(ppieg, UT_ERROR);

	// no filter will support IEGFT_Unknown, so we detect from the
	// suffix of the filename and the contents of the file, the real 
        // importer to use and assign that back to ieft.
	if (ft == IEGFT_Unknown)
	{
	  ft = IE_ImpGraphic::fileTypeForContents( reinterpret_cast<const char *>(bytes.getPointer(0)),
						   bytes.getLength() );
	}

	// use the importer for the specified file type
	for (UT_sint32 k=0; (k < IE_IMP_GraphicSniffers.size()); k++)
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
	GsfInput * input;

	input = UT_go_file_open (szFilename, NULL);
	if (!input)
		return UT_IE_FILENOTFOUND;

	UT_Error result = constructImporter (input, ft, ppieg);

	g_object_unref (G_OBJECT (input));

	return result;
}

#define CONFIDENCE_THRESHOLD 72

UT_Error IE_ImpGraphic::constructImporter(GsfInput * input,
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
		UT_return_val_if_fail (input != NULL, UT_IE_FILENOTFOUND);

		UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;
		
		for (UT_uint32 k=0; k < nrElements; k++)
			{
				IE_ImpGraphicSniffer * s = IE_IMP_GraphicSniffers[k];
				
				UT_Confidence_t content_confidence = UT_CONFIDENCE_ZILCH;
				UT_Confidence_t suffix_confidence = UT_CONFIDENCE_ZILCH;
				
				{
					GsfInputMarker marker(input);
					content_confidence = s->recognizeContents(input);
				}

				const char * name = gsf_input_name (input);
				// we can have an empty name (NULL) because we can have a memory stream.
				if(name) {
					const IE_SuffixConfidence * sc = s->getSuffixConfidence();
					while (sc && !sc->suffix.empty() && suffix_confidence != UT_CONFIDENCE_PERFECT) {
						/* suffixes do not have a leading '.' */
						// we use g_str_has_suffix like this to make sure we properly autodetect the extensions
						// of files that have dots in their names, like foo.bar.png
						std::string suffix = std::string(".") + sc->suffix;
						if (g_str_has_suffix(name, suffix.c_str()) && 
							sc->confidence > suffix_confidence) {
							suffix_confidence = sc->confidence;
						}
						sc++;
					}
				}
				UT_Confidence_t confidence = s_condfidence_heuristic ( content_confidence, 
																	   suffix_confidence ) ;
				
				if ( confidence > CONFIDENCE_THRESHOLD && confidence >= best_confidence )
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

UT_Error IE_ImpGraphic::importGraphic(UT_ByteBuf * byteBuf,
									  FG_ConstGraphicPtr& pfg)
{
	UT_return_val_if_fail (byteBuf != NULL, UT_IE_FILENOTFOUND);

	GsfInput * input = gsf_input_memory_new_clone (byteBuf->getPointer(0), byteBuf->getLength());

	// method assumes that we take ownership of the byteBuf
	DELETEP(byteBuf);

	if (!input)
		return UT_IE_NOMEMORY;

	UT_Error result = importGraphic(input, pfg);

	g_object_unref (G_OBJECT (input));

	return result;
}

UT_Error IE_ImpGraphic::importGraphic(GsfInput * input,
									  FG_ConstGraphicPtr& pfg)
{
	UT_return_val_if_fail (input != NULL, UT_IE_FILENOTFOUND);

	UT_ByteBuf* pBB = new UT_ByteBuf;

	if (pBB == NULL)
		return UT_IE_NOMEMORY;

	if (!pBB->insertFromInput(0, input))
		{
			DELETEP(pBB);
			return UT_IE_FILENOTFOUND;
		}

	//  The ownership of pBB changes here.  The subclass of IE_ImpGraphic
	//  should either delete pBB when it is done importing, or give it
	//  to the FG_Graphic object which is eventually constructed.
	return importGraphic(pBB, pfg);
}

UT_Error IE_ImpGraphic::importGraphic(const char * szFilename,
									  FG_ConstGraphicPtr& pfg)
{
	GsfInput * input;

	input = UT_go_file_open (szFilename, NULL);
	if (!input)
		return UT_IE_FILENOTFOUND;

	UT_Error res = importGraphic(input, pfg);

	g_object_unref (G_OBJECT (input));
	return res;
}


UT_Error IE_ImpGraphic::loadGraphic(const char * szFilename,
									IEGraphicFileType iegft,
									FG_ConstGraphicPtr& pfg)
{
	GsfInput *input;
	
	input = UT_go_file_open (szFilename, NULL);
	if (!input)
		return UT_IE_FILENOTFOUND;

	UT_Error result = loadGraphic (input, iegft, pfg);

	g_object_unref (G_OBJECT (input));

	return result;
}

UT_Error IE_ImpGraphic::loadGraphic(GsfInput * input,
									IEGraphicFileType iegft,
									FG_ConstGraphicPtr& pfg)
{
	UT_return_val_if_fail (input != NULL, UT_IE_FILENOTFOUND);

	IE_ImpGraphic *importer;
	
	UT_Error result = constructImporter(input, iegft, &importer);
	if (result != UT_OK || !importer)
		return UT_ERROR;

	result = importer->importGraphic (input, pfg);

	delete importer;

	return result;
}

UT_Error IE_ImpGraphic::loadGraphic(const UT_ByteBuf &pBB,
									IEGraphicFileType iegft,
									FG_ConstGraphicPtr& pfg)
{
	GsfInput * input;

	input = gsf_input_memory_new (pBB.getPointer (0), pBB.getLength(), FALSE);
	if (!input)
		return UT_IE_NOMEMORY;

	UT_Error result = loadGraphic (input, iegft, pfg);

	g_object_unref (G_OBJECT (input));

	return result;
}

UT_Confidence_t IE_ImpGraphicSniffer::recognizeContents (GsfInput * input)
{
	char szBuf[4097] = "";  // 4096+nul ought to be enough
	UT_uint32 iNumbytes = UT_MIN(4096, gsf_input_size(input));
	gsf_input_read(input, iNumbytes, (guint8 *)(szBuf));
	szBuf[iNumbytes] = '\0';

	return recognizeContents(szBuf, iNumbytes);
}

UT_Confidence_t IE_ImpGraphicSniffer::recognizeContents (const char * /*szBuf*/, 
														 UT_uint32 /*iNumbytes*/)
{
	// should be explicitly overriden, or not return anything
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	return UT_CONFIDENCE_ZILCH;
}
