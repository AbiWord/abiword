/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net> 
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

#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_iconv.h"
#include "ie_imp_MathML.h"
#include "pd_Document.h"

#include "xap_EncodingManager.h"


#include "ap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Encoding.h"
#include "ap_Prefs.h"
#include "ie_imp_Text.h"

/*****************************************************************/
/*****************************************************************/

IE_Imp_MathML_Sniffer::IE_Imp_MathML_Sniffer ()
	: IE_ImpSniffer(IE_IMPEXPNAME_MATHML, true)
{
	// 
}

UT_Confidence_t IE_Imp_MathML_Sniffer::supportsMIME (const char * szMIME)
{
	if (UT_strcmp (IE_FileInfo::mapAlias (szMIME), IE_MIME_MathML) == 0)
		{
			return UT_CONFIDENCE_GOOD;
		}
	if (strncmp (szMIME, "text/", 5) == 0)
		{
			return UT_CONFIDENCE_SOSO;
		}
	return UT_CONFIDENCE_ZILCH;
}

/*!
  Check if buffer contains data meant for this importer.

 We don't attmpt to recognize since other filetypes (HTML) can
 use the same encodings a text file can.
 We also don't want to steal recognition when user wants to use
 the Encoded Text importer.
 */
UT_Confidence_t IE_Imp_MathML_Sniffer::recognizeContents(const char * szBuf,
													   UT_uint32 iNumbytes)
{
	char * magic = "<math";
	if(strncmp(szBuf,magic,strlen(magic) == 0))
	   return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
 

}


/*!
  Check filename extension for filetypes we support
 \param szSuffix Filename extension
 */
UT_Confidence_t IE_Imp_MathML_Sniffer::recognizeSuffix(const char * szSuffix)
{
  if (!UT_stricmp (szSuffix, ".xml"))
    return UT_CONFIDENCE_GOOD;
  if (!UT_stricmp (szSuffix, ".mathml"))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_MathML_Sniffer::constructImporter(PD_Document * pDocument,
												IE_Imp ** ppie)
{
	IE_Imp_MathML * p = new IE_Imp_MathML(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Imp_MathML_Sniffer::getDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEFileType * ft)
{
	*pszDesc = "MathML (.xml, .mathml)";
	*pszSuffixList = "*.xml; *.mathml";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

/*
  Import MathML data from a plain text file
 \param szFilename Name of file to import
 Simply fills a UT_byteBuf with the contents of the MathML
*/
UT_Error IE_Imp_MathML::importFile(const char * szFilename)
{
	// We must open in binary mode for UCS-2 compatibility.
	FILE *fp = fopen(szFilename, "rb");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}

	ImportStreamFile * pStream = new ImportStreamFile(fp);
	UT_Error error;


	pStream->init(NULL);
	X_CleanupIfError(error,_parseStream(pStream));
	error = UT_OK;


Cleanup:
	delete pStream;
	fclose(fp);
	return error;
}

#undef X_CleanupIfError

/*****************************************************************/
/*****************************************************************/

/*
  Construct text importer
 \param pDocument Document to import MathML into

 Uses current document's encoding if it is set
*/
IE_Imp_MathML::IE_Imp_MathML(PD_Document * pDocument)
	: IE_Imp(pDocument),m_pByteBuf(NULL)
{
	m_pByteBuf = new UT_ByteBuf;

}

IE_Imp_MathML::~IE_Imp_MathML(void)
{
	DELETEP(m_pByteBuf);
}

/*****************************************************************/

#define X_ReturnIfFail(exp,error)		do { bool b = (exp); if (!b) return (error); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)


/*!
  Parse stream contents into the document
 \param stream Stream to import from

 This code is used for both files and the clipboard
 */
UT_Error IE_Imp_MathML::_parseStream(ImportStream * pStream)
{
	UT_return_val_if_fail(pStream, UT_ERROR);

	bool bFirstChar = true;
	UT_UCSChar c;
	unsigned char uc;
	while (pStream->getChar(c))
	{
		uc = static_cast<unsigned char>(c);
		m_pByteBuf->append(&uc,1);
	}
	return UT_OK;
}

bool IE_Imp_MathML::pasteFromBuffer(PD_DocumentRange * pDocRange,
								  const unsigned char * pData, UT_uint32 lenData,
								  const char * /* encoding */)
{
	UT_return_val_if_fail(getDoc() == pDocRange->m_pDoc,false);
	UT_return_val_if_fail(pDocRange->m_pos1 == pDocRange->m_pos2,false);

	ImportStreamClipboard stream(pData, lenData);
	setClipboard (pDocRange->m_pos1);
	stream.init(NULL);
	_parseStream(&stream);
	return true;
}

