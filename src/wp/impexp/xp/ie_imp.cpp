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


#include <string.h>
#include <ctype.h>

#include "ut_string.h"
#include "ut_vector.h"
#include "ut_assert.h"
#include "ut_misc.h"

#include "ie_imp.h"
#include "ie_imp_AbiWord_1.h"
#include "ie_imp_GraphicAsDocument.h"
#include "pd_Document.h"
#include "pf_Frag_Strux.h"

#include "ut_debugmsg.h"
#include "ut_string_class.h"

static const UT_uint32 importer_size_guess = 20;
static UT_GenericVector<IE_ImpSniffer *> m_sniffers (importer_size_guess);

#include "ie_imp_XML.h"
IE_Imp_XML * abi_ie_imp_xml_instance = 0;

/*****************************************************************/
/*****************************************************************/

IE_Imp::IE_Imp(PD_Document * pDocument, UT_Confidence_t fidelity)
	: m_pDocument(pDocument),
	  m_isPaste (false),
	  m_dpos(0),
	  m_bStylesOnly(false),
	  m_bDocProps(false), 
	  m_fidelity(fidelity)
{
  if (abi_ie_imp_xml_instance)
    {
      delete abi_ie_imp_xml_instance;
      abi_ie_imp_xml_instance = new IE_Imp_XML(pDocument,false);
    }
  m_pDocument->invalidateCache();
}

IE_Imp::~IE_Imp()
{
}

void IE_Imp::setProps (const char * props)
{
	m_props_map.clear ();
	m_props_map.parse_properties (props);
}

PT_DocPosition IE_Imp::getDocPos() const
{
	return m_dpos;
}

void IE_Imp::setClipboard (PT_DocPosition dpos)
{
	m_isPaste = true;
	m_dpos = dpos;
}

bool IE_Imp::isClipboard () const
{
	return m_isPaste;
}

bool IE_Imp::appendStrux (PTStruxType pts, const XML_Char ** attributes)
{
	if (!m_isPaste)
		return m_pDocument->appendStrux (pts, attributes);
	else
		{
			bool bRes = m_pDocument->insertStrux (m_dpos, pts, attributes, NULL);
			m_dpos++;
			return bRes;
		}
}

bool IE_Imp::appendStruxFmt(pf_Frag_Strux * pfs, const XML_Char ** attributes)
{
	if (!m_isPaste)
		return m_pDocument->appendStruxFmt(pfs, attributes);
	else 
	{
		bool bRes;

		if (pfs == NULL) 
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			bRes = false;
		} 
		else 
		{
			bRes = m_pDocument->changeStruxFmt(PTC_AddFmt,
											   m_dpos, m_dpos,
											   attributes, NULL,
											   PTX_Block);
		}
		return bRes;
	}
}

bool IE_Imp::appendSpan (const UT_UCSChar * p, UT_uint32 length)
{
	if (!m_isPaste)
		return m_pDocument->appendSpan(p, length);
	else
		{
			bool bRes = m_pDocument->insertSpan (m_dpos, p, length);
			m_dpos += length;
			return bRes;
		}
}

bool IE_Imp::appendObject (PTObjectType pto, const XML_Char ** attribs,
						   const XML_Char ** props)
{
	if (!m_isPaste)
		return m_pDocument->appendObject (pto, attribs);
	else
		{
			bool bRes = m_pDocument->insertObject (m_dpos, pto, attribs, props);
			m_dpos++;
			return bRes;
		}
}

bool IE_Imp::appendFmt(const XML_Char ** attributes)
{
	bool bRes;

	if (!m_isPaste) {
		bRes = m_pDocument->appendFmt (attributes);
		// m_pDocument->appendFmtMark();
	}
	else {
		bRes = m_pDocument->changeSpanFmt(PTC_AddFmt,
										  m_dpos, m_dpos,
										  attributes, NULL);
	}

	return bRes;
}

bool IE_Imp::appendFmt(const UT_GenericVector<XML_Char*> * pVecAttributes)
{
	bool bRes;

	if (!m_isPaste) {
		bRes = m_pDocument->appendFmt (pVecAttributes);
		// m_pDocument->appendFmtMark();
	}
	else {
		const XML_Char ** attributes;

		attributes = (const XML_Char **)pVecAttributes->getNthItem(0);

		bRes = m_pDocument->changeSpanFmt(PTC_AddFmt,
										  m_dpos, m_dpos,
										  attributes, NULL);
	}

	return bRes;
}

/*****************************************************************/
/*****************************************************************/

// default impl, meant to abort

bool	  IE_Imp::pasteFromBuffer(PD_DocumentRange * pDocRange,
				  const unsigned char * pData, 
				  UT_uint32 lenData, 
				  const char * szEncoding)
{
  UT_ASSERT_NOT_REACHED();
  return false;
}

PD_Document * IE_Imp::getDoc () const
{
  return m_pDocument;
}
	
/*****************************************************************/
/*****************************************************************/

IE_ImpSniffer::IE_ImpSniffer(const char * name, bool canPaste)
	: m_name(name),
	  m_type(IEFT_Bogus),
	  m_bCanPaste(canPaste)
{
}

IE_ImpSniffer::~IE_ImpSniffer()
{
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp::registerImporter (IE_ImpSniffer * s)
{
	UT_uint32 ndx = 0;
	UT_Error err = m_sniffers.addItem (s, &ndx);

	UT_return_if_fail(err == UT_OK);
	UT_return_if_fail(ndx >= 0);

	s->setFileType(ndx+1);
}

void IE_Imp::unregisterImporter (IE_ImpSniffer * s)
{
	UT_uint32 ndx = s->getFileType(); // 1:1 mapping

	UT_return_if_fail(ndx >= 0);

	m_sniffers.deleteNthItem (ndx-1);

	// Refactor the indexes
	IE_ImpSniffer * pSniffer = 0;
	UT_uint32 size  = m_sniffers.size();
	UT_uint32 i     = 0;
	for( i = ndx-1; i < size; i++)
	{
		pSniffer = m_sniffers.getNthItem(i);
		if (pSniffer)
        	pSniffer->setFileType(i+1);
	}
}

void IE_Imp::unregisterAllImporters ()
{
	IE_ImpSniffer * pSniffer = 0;
	UT_uint32 size = m_sniffers.size();

	for (UT_uint32 i = 0; i < size; i++)
	{
		pSniffer = m_sniffers.getNthItem(i);
		if (pSniffer)
			pSniffer->unref();
	}
}

/*****************************************************************/
/*****************************************************************/

IEFileType IE_Imp::fileTypeForContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a match for all file types
	UT_uint32 nrElements = getImporterCount();

	IEFileType best = IEFT_Unknown;
	UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpSniffer * s = m_sniffers.getNthItem (k);
		UT_Confidence_t confidence = s->recognizeContents(szBuf, iNumbytes);
		if ((confidence > 0) && ((IEFT_Unknown == best) || (confidence >= best_confidence)))
		{
		  best_confidence = confidence;
		  for (UT_sint32 a = 0; a < static_cast<int>(nrElements); a++)
		    {
		      if (s->supportsFileType((IEFileType) (a+1)))
			{
			  best = (IEFileType) (a+1);

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

/*! 
  Find the filetype for the given suffix.
 \param szSuffix File suffix

 Returns IEFT_Unknown if no importer knows this suffix.
 Note that more than one importer may support a suffix.
 We return the first one we find.
 This function should closely resemble IE_Exp::fileTypeForSuffix()
*/
IEFileType IE_Imp::fileTypeForSuffix(const char * szSuffix)
{
	if (!szSuffix)
		return IEFT_Unknown;

	// this assert alerts us to bugs like 9571
	UT_ASSERT_HARMLESS(*szSuffix == '.');
	
	IEFileType best = IEFT_Unknown;
	UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;

	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getImporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpSniffer * s = m_sniffers.getNthItem(k);

		UT_Confidence_t confidence = s->recognizeSuffix(szSuffix);
		if ((confidence > 0) && ((IEFT_Unknown == best) || (confidence >= best_confidence)))
		  {
		        best_confidence = confidence;
			for (UT_sint32 a = 0; a < static_cast<int>(nrElements); a++)
			{
				if (s->supportsFileType(static_cast<IEFileType>(a+1)))
				{
				  best = static_cast<IEFileType>(a+1);
				  
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

/*! 
  Find the filetype for the given filetype description.
 \param szDescription Filetype description

 Returns IEFT_Unknown if no importer has this description.
 This function should closely resemble IE_Exp::fileTypeForDescription()
*/
IEFileType IE_Imp::fileTypeForDescription(const char * szDescription)
{
	IEFileType ieft = IEFT_Unknown;

	if (!szDescription)
		return ieft;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getImporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpSniffer * pSniffer = static_cast<IE_ImpSniffer *>(m_sniffers.getNthItem(k));

		const char * szDummy;
		const char * szDescription2 = 0;

		if (pSniffer->getDlgLabels(&szDescription2,&szDummy,&ieft))
		{
			if (!UT_strcmp(szDescription,szDescription2))
				return ieft;
		}
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	return ieft;
}

IEFileType IE_Imp::fileTypeForSuffixes(const char * suffixList)
{
	IEFileType ieft = IEFT_Unknown;
	if (!suffixList)
		return ieft;

	UT_String utSuffix (suffixList);
	const size_t len = strlen(suffixList);
	size_t i = 0;

	while (true)
		{
			while (i < len && suffixList[i] != '.')
				i++;

			// will never have all-space extension

			const size_t start = i;
			while (i < len && suffixList[i] != ';')
				i++;

			if (i <= len) {
				UT_String suffix (utSuffix.substr(start, i-start).c_str());
				UT_DEBUGMSG(("DOM: suffix: %s\n", suffix.c_str()));
				
				ieft = fileTypeForSuffix (suffix.c_str());
				if (ieft != IEFT_Unknown || i == len)
					return ieft;
				
				i++;
			}
		}
	return ieft;
}

/*! 
  Find the filetype sniffer for the given filetype.
 \param ieft Filetype

 Returns 0 if no exporter knows this filetype.
 This function should closely resemble IE_Imp::snifferForFileType()
*/
IE_ImpSniffer * IE_Imp::snifferForFileType(IEFileType ieft)
{
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getImporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpSniffer * s = m_sniffers.getNthItem(k);
		if (s->supportsFileType(ieft))
			return s;
	}

	// The passed in filetype is invalid.
	return 0;
}

/*! 
  Find the suffixes for the given filetype.
 \param szSuffix File suffix

 Returns 0 if no exporter knows this filetype.
 This function should closely resemble IE_Imp::suffixesForFileType()
*/
const char * IE_Imp::suffixesForFileType(IEFileType ieft)
{
	const char * szDummy;
	const char * szSuffixes = 0;
	IEFileType ieftDummy;

	IE_ImpSniffer * pSniffer = snifferForFileType(ieft);

	if (pSniffer->getDlgLabels(&szDummy,&szSuffixes,&ieftDummy))
		return szSuffixes;
	else
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	// The passed in filetype is invalid.
	return 0;
}

/*! 
  Find the description for the given filetype.
 \param ieft Numerical "import filetype" ID

 Returns 0 if filetype doesn't exist.
 This function should closely resemble IE_Imp::descriptionForFileType()
*/
const char * IE_Imp::descriptionForFileType(IEFileType ieft)
{
	const char * szDummy;
	const char * szDescription = 0;
	IEFileType ieftDummy;

	IE_ImpSniffer * pSniffer = snifferForFileType(ieft);

	if (pSniffer->getDlgLabels(&szDescription,&szDummy,&ieftDummy))
		return szDescription;
	else
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	// The passed in filetype is invalid.
	return 0;
}

static UT_Confidence_t s_confidence_heuristic ( UT_Confidence_t content_confidence, 
						 UT_Confidence_t suffix_confidence )
{
  return (UT_Confidence_t) ( (static_cast<double>(content_confidence) * 0.85) + (static_cast<double>(suffix_confidence) * 0.15) ) ;
}

/*! 
  Construct an importer of the right type.
 \param pDocument Document
 \param szFilename Name of file - optional
 \param ieft Desired filetype - pass IEFT_Unknown for best guess
 \param ppie Pointer to return importer in
 \param pieft Pointer to fill in actual filetype

 Caller is responsible for deleting the importer object
 when finished with it.
 This function should closely match IE_Exp::contructExporter()
*/
UT_Error IE_Imp::constructImporter(PD_Document * pDocument,
				   const char * szFilename,
				   IEFileType ieft,
				   IE_Imp ** ppie,
				   IEFileType * pieft)
{
	bool bUseGuesswork = (ieft != IEFT_Unknown);
	
	UT_return_val_if_fail(pDocument, UT_ERROR);
	UT_return_val_if_fail(ieft != IEFT_Unknown || (szFilename && *szFilename), UT_ERROR);
	UT_return_val_if_fail(ppie, UT_ERROR);

	UT_uint32 nrElements = getImporterCount();

	// no filter will support IEFT_Unknown, so we try to detect
	// from the contents of the file or the filename suffix
	// the importer to use and assign that back to ieft.
	// Give precedence to the file contents
	if (ieft == IEFT_Unknown && szFilename && *szFilename)
	{
		char szBuf[4097] = "";  // 4096+nul ought to be enough
		UT_uint32 iNumbytes = 0;
		FILE *f = NULL;

		// we must open in binary mode for UCS-2 compatibility
		if ( ( f = fopen( szFilename, "rb" ) ) != static_cast<FILE *>(0) )
		{
			iNumbytes = fread(szBuf, 1, sizeof(szBuf)-1, f);
			szBuf[iNumbytes] = '\0';
			fclose(f);
		}

		UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;
		IE_ImpSniffer * best_sniffer = 0;

		for (UT_uint32 k=0; k < nrElements; k++)
		  {
		    IE_ImpSniffer * s = m_sniffers.getNthItem (k);

		    UT_Confidence_t content_confidence = UT_CONFIDENCE_ZILCH;
		    UT_Confidence_t suffix_confidence = UT_CONFIDENCE_ZILCH;

		    if ( iNumbytes > 0 )
		      content_confidence = s->recognizeContents(szBuf, iNumbytes);
		    
		    const char * suffix = UT_pathSuffix(szFilename) ;
		    if ( suffix != NULL )
				{
					suffix_confidence = s->recognizeSuffix(UT_pathSuffix(szFilename));
				}
		    
		    UT_Confidence_t confidence = s_confidence_heuristic ( content_confidence, 
																  suffix_confidence ) ;
		    
		    if ( confidence != 0 && confidence >= best_confidence )
				{
					best_sniffer = s;
					best_confidence = confidence;
					ieft = (IEFileType) (k+1);
				}
		  }
		if (best_sniffer)
			{
				if (pieft != NULL) *pieft = ieft;
				return best_sniffer->constructImporter (pDocument, ppie);
			}
	}

	if (ieft == IEFT_Unknown)
	{
	   	// maybe they're trying to open an image directly?
	   	IE_ImpGraphic *pIEG;
 		UT_Error errorCode = IE_ImpGraphic::constructImporter(szFilename, IEGFT_Unknown, &pIEG);
		if (!errorCode && pIEG) 
 		{
			// tell the caller the type of importer they got
		   	if (pieft != NULL) *pieft = IEFT_Unknown; // to force a save-as

		   	// create the importer 
			*ppie = new IE_Imp_GraphicAsDocument(pDocument);
		   	if (*ppie) {
			   	// tell the importer where to get the graphic
			   	((IE_Imp_GraphicAsDocument*)(*ppie))->setGraphicImporter(pIEG);
			   	return UT_OK;
			} else {
			   	delete pIEG;
				return UT_IE_NOMEMORY;
			}
		}
		else
 		{
	   		// as a last resort, just try importing it as text  :(
			ieft = IE_Imp::fileTypeForSuffix(".txt");
		}
	}

	UT_ASSERT_HARMLESS(ieft != IEFT_Unknown);

	// tell the caller the type of importer they got
	if (pieft != NULL) 
		*pieft = ieft;

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ImpSniffer * s = m_sniffers.getNthItem (k);
		if (s->supportsFileType(ieft))
			return s->constructImporter(pDocument,ppie);
	}

	// if we got here, no registered importer handles the
	// type of file we're supposed to be reading.
	// assume it is our format and try to read it.
	// if that fails, just give up.
	if (bUseGuesswork)
	{
		*ppie = new IE_Imp_AbiWord_1(pDocument);
		return ((*ppie) ? UT_OK : UT_IE_NOMEMORY);
	}
	else
		return UT_ERROR;
}

bool IE_Imp::enumerateDlgLabels(UT_uint32 ndx,
				const char ** pszDesc,
				const char ** pszSuffixList,
				IEFileType * ft)
{
	UT_uint32 nrElements = getImporterCount();
	if (ndx < nrElements)
	{
		IE_ImpSniffer * s = m_sniffers.getNthItem (ndx);
		return s->getDlgLabels(pszDesc,pszSuffixList,ft);
	}

	return false;
}

UT_uint32 IE_Imp::getImporterCount(void)
{
	return m_sniffers.size();
}
