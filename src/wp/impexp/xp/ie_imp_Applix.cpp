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


#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_Applix.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "xap_EncodingManager.h"

/*****************************************************************/
/*****************************************************************/

/*
  Import Applix Word documents
*/

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

UT_Error IE_Imp_Applix::importFile(const char * szFilename)
{
	FILE *fp = fopen(szFilename, "rb");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}
	
	UT_Error error;

	X_CleanupIfError(error,_writeHeader(fp));
	X_CleanupIfError(error,_parseFile(fp));

	error = UT_OK;

Cleanup:
	fclose(fp);
	return error;
}

#undef X_CleanupIfError

/*****************************************************************/
/*****************************************************************/

IE_Imp_Applix::~IE_Imp_Applix()
{
}

IE_Imp_Applix::IE_Imp_Applix(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
}

/*****************************************************************/
/*****************************************************************/

#define X_ReturnIfFail(exp,error)		do { bool b = (exp); if (!b) return (error); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

UT_Error IE_Imp_Applix::_writeHeader(FILE * /* fp */)
{
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));

	return UT_OK;
}

UT_Error IE_Imp_Applix::_parseFile(FILE * fp)
{
	return UT_OK;
}

#undef X_ReturnNoMemIfError
#undef X_ReturnIfFail

/*****************************************************************/
/*****************************************************************/

void IE_Imp_Applix::pasteFromBuffer(PD_DocumentRange * pDocRange,
								  unsigned char * pData, UT_uint32 lenData)
{
	return;
}

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_Applix::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
  // TODO: try to sensibly recognize the contents of the buffer
	return(false);
}

bool IE_Imp_Applix::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".apx") == 0);
}

UT_Error IE_Imp_Applix::StaticConstructor(PD_Document * pDocument,
										IE_Imp ** ppie)
{
	IE_Imp_Applix * p = new IE_Imp_Applix(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_Applix::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
  // TOOD: get the real filename extension used
	*pszDesc = "Applix Word (.apx)";
	*pszSuffixList = "*.apx";
	*ft = IEFT_APPLIX;
	return true;
}

bool IE_Imp_Applix::SupportsFileType(IEFileType ft)
{
	return (IEFT_APPLIX == ft);
}

