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
#include "ie_imp_Text.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "xap_EncodingManager.h"

/*****************************************************************/
/*****************************************************************/

/*
  Import US-ASCII (actually Latin-1) data from a plain
  text file.  We allow either LF or CR or CRLF line
  termination.  Each line terminator is taken to be a
  paragraph break.
*/

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

UT_Error IE_Imp_Text::importFile(const char * szFilename)
{
	FILE *fp = fopen(szFilename, "r");
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

IE_Imp_Text::~IE_Imp_Text()
{
}

IE_Imp_Text::IE_Imp_Text(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
}

/*****************************************************************/
/*****************************************************************/

#define X_ReturnIfFail(exp,error)		do { bool b = (exp); if (!b) return (error); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

UT_Error IE_Imp_Text::_writeHeader(FILE * /* fp */)
{
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));

	return UT_OK;
}

UT_Error IE_Imp_Text::_parseFile(FILE * fp)
{
	UT_GrowBuf gbBlock(1024);
	bool bEatLF = false;
	bool bEmptyFile = true;
	unsigned char b;
	UT_UCSChar c;
	wchar_t wc;

	while (fread(&b, 1, sizeof(b), fp) > 0)
	{
		if(!m_Mbtowc.mbtowc(wc,b))
		    continue;
		c = (UT_UCSChar)wc;
		switch (c)
		{
		case (UT_UCSChar)'\r':
		case (UT_UCSChar)'\n':
			
			if ((c == (UT_UCSChar)'\n') && bEatLF)
			{
				bEatLF = false;
				break;
			}

			if (c == (UT_UCSChar)'\r')
			{
				bEatLF = true;
			}
			
			// we interprete either CRLF, CR, or LF as a paragraph break.
			
			// start a paragraph and emit any text that we
			// have accumulated.
			X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
			bEmptyFile = false;
			if (gbBlock.getLength() > 0)
			{
				X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
				gbBlock.truncate(0);
			}
			break;

		default:
			bEatLF = false;
			X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&c,1));
			break;
		}
	} 

	if (gbBlock.getLength() > 0 || bEmptyFile)
	{
		// if we have text left over (without final CR/LF),
		// or if we read an empty file,
		// create a paragraph and emit the text now.
		X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
		if (gbBlock.getLength() > 0)
			X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
	}

	return UT_OK;
}

#undef X_ReturnNoMemIfError
#undef X_ReturnIfFail

/*****************************************************************/
/*****************************************************************/

void IE_Imp_Text::pasteFromBuffer(PD_DocumentRange * pDocRange,
								  unsigned char * pData, UT_uint32 lenData)
{
	UT_ASSERT(m_pDocument == pDocRange->m_pDoc);
	UT_ASSERT(pDocRange->m_pos1 == pDocRange->m_pos2);

	UT_GrowBuf gbBlock(1024);
	bool bEatLF = false;
	bool bSuppressLeadingParagraph = true;
	bool bInColumn1 = true;
	unsigned char * pc;

	PT_DocPosition dpos = pDocRange->m_pos1;
	
	for (pc=pData; (pc<pData+lenData); pc++)
	{
		unsigned char b = *pc;
		UT_UCSChar c;
		wchar_t wc;
		if(!m_Mbtowc.mbtowc(wc,b))
		    continue;
		c = (UT_UCSChar)wc;
		
		switch (c)
		{
		case (UT_UCSChar)'\r':
		case (UT_UCSChar)'\n':
			if ((c == (UT_UCSChar)'\n') && bEatLF)
			{
				bEatLF = false;
				break;
			}

			if (c == (UT_UCSChar)'\r')
			{
				bEatLF = true;
			}
			
			// we interprete either CRLF, CR, or LF as a paragraph break.
			
			if (gbBlock.getLength() > 0)
			{
				// flush out what we have
				m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
				dpos += gbBlock.getLength();
				gbBlock.truncate(0);
			}
			bInColumn1 = true;
			break;

		default:
			bEatLF = false;
			if (bInColumn1 && !bSuppressLeadingParagraph)
			{
				m_pDocument->insertStrux(dpos,PTX_Block);
				dpos++;
			}
			
			gbBlock.ins(gbBlock.getLength(),&c,1);

			bInColumn1 = false;
			bSuppressLeadingParagraph = false;
			break;
		}
	} 

	if (gbBlock.getLength() > 0)
	{
		// if we have text left over (without final CR/LF),
		m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
		dpos += gbBlock.getLength();
	}

	return;
}

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_Text::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// We give the other guys a chance, since this
	// importer is so generic.  
	return false;
}

bool IE_Imp_Text::RecognizeSuffix(const char * szSuffix)
{
	// We give the other guys a chance, since this
	// importer is so generic.  
	return false;
}

UT_Error IE_Imp_Text::StaticConstructor(PD_Document * pDocument,
										IE_Imp ** ppie)
{
	IE_Imp_Text * p = new IE_Imp_Text(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_Text::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "Text (.txt)";
	*pszSuffixList = "*.txt";
	*ft = IEFT_Text;
	return true;
}

bool IE_Imp_Text::SupportsFileType(IEFileType ft)
{
	return (IEFT_Text == ft);
}

