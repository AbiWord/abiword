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

#include <zlib.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_types.h"

#include "ie_impexp_AbiWord.h"
#include "ie_imp_GZipAbiWord.h"
#include "ie_FileInfo.h"

/*****************************************************************/
/*****************************************************************/

IE_Imp_GZipAbiWord_Sniffer::IE_Imp_GZipAbiWord_Sniffer ()
	: IE_ImpSniffer(IE_IMPEXPNAME_AWML11GZ)
{
	// 
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_GZipAbiWord_Sniffer__SuffixConfidence[] = {
	{ "zabw", 	UT_CONFIDENCE_PERFECT 	},
	{ "abw.gz", UT_CONFIDENCE_PERFECT 	},
	{ NULL, 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_GZipAbiWord_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_GZipAbiWord_Sniffer__SuffixConfidence;
}

// supported mimetypes
static IE_MimeConfidence IE_Imp_GZipAbiWord_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 		IE_MIMETYPE_AbiWord, 	UT_CONFIDENCE_GOOD 		}, 
	{ IE_MIME_MATCH_FULL, 		"application/xml", 		UT_CONFIDENCE_POOR / 2  }, /* i.e., VERYPOOR ?? */
	{ IE_MIME_MATCH_FULL, 		"text/xml", 			UT_CONFIDENCE_POOR / 2  }, /* i.e., VERYPOOR ?? */
	{ IE_MIME_MATCH_BOGUS, 		NULL, 					UT_CONFIDENCE_ZILCH 	}
};

const IE_MimeConfidence * IE_Imp_GZipAbiWord_Sniffer::getMimeConfidence ()
{
	return IE_Imp_GZipAbiWord_Sniffer__MimeConfidence;
}

UT_Confidence_t IE_Imp_GZipAbiWord_Sniffer::recognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// TODO: This is a hack.  Since we're just passed in some
	// TODO: some data, and not the actual filename, there isn't
	// TODO: much we can do other than verify that it is gzip'ed
	// TODO: data.  For the time being, assume that if it is
	// TODO: gzip'ed, it's gzip'ed abiword.  This assumption will
	// TODO: be false if and when we support any other compressed
	// TODO: formats.
	if ( iNumbytes < 2 ) return(false);
	if ( ( szBuf[0] == static_cast<char>(0x1f) ) && ( szBuf[1] == static_cast<char>(0x8b) ) )
	{
		return(UT_CONFIDENCE_SOSO);
	}
	return(UT_CONFIDENCE_ZILCH);
}

UT_Error IE_Imp_GZipAbiWord_Sniffer::constructImporter(PD_Document * pDocument,
													   IE_Imp ** ppie)
{
    *ppie = new IE_Imp_AbiWord_1(pDocument);;
    return UT_OK;
}

bool IE_Imp_GZipAbiWord_Sniffer::getDlgLabels(const char ** pszDesc,
											  const char ** pszSuffixList,
											  IEFileType * ft)
{
    *pszDesc = "GZipped AbiWord (.zabw)";
#ifdef WIN32  
	*pszSuffixList = "*.zabw" ;  
#else  
	*pszSuffixList = "*.abw.gz; *.zabw";
#endif
    *ft = getFileType();
    return true;
}
