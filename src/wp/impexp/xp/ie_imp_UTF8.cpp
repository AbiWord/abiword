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
#include "ie_imp_UTF8.h"
#include "pd_Document.h"
#include "ut_growbuf.h"

/*****************************************************************/
/*****************************************************************/

/*
  Import a plain text file formatted in UTF8.
  We allow either LF or CR or CRLF line termination.
  Each line terminator is taken to be a paragraph break.
*/

/*****************************************************************/
/*****************************************************************/

static void _smashUTF8(UT_GrowBuf * pgb)
{
	// smash any utf8 sequences into a single ucs char

	// since we change the GrowBuf in-place, we
	// recompute the length and buffer pointer
	// to avoid accidents....

	for (UT_uint32 k=0; (k < pgb->getLength()); k++)
	{
		UT_uint16 * p = pgb->getPointer(k);
		UT_uint16 ck = *p;
		
		if (ck < 0x0080)						// latin-1
			continue;
		else if ((ck & 0x00f0) == 0x00f0)		// lead byte in 4-byte surrogate pair, ik
		{
			UT_ASSERT(UT_NOT_IMPLEMENTED);
			continue;
		}
		else if ((ck & 0x00e0) == 0x00e0)		// lead byte in 3-byte sequence
		{
			UT_ASSERT(k+2 < pgb->getLength());
			XML_Char buf[4];
			buf[0] = (XML_Char)p[0];
			buf[1] = (XML_Char)p[1];
			buf[2] = (XML_Char)p[2];
			buf[3] = 0;
			UT_UCSChar ucs = UT_decodeUTF8char(buf,3);
			pgb->overwrite(k,&ucs,1);
			pgb->del(k+1,2);
			continue;
		}
		else if ((ck & 0x00c0) == 0x00c0)		// lead byte in 2-byte sequence
		{
			UT_ASSERT(k+1 < pgb->getLength());
			XML_Char buf[3];
			buf[0] = (XML_Char)p[0];
			buf[1] = (XML_Char)p[1];
			buf[2] = 0;
			UT_UCSChar ucs = UT_decodeUTF8char(buf,2);
			pgb->overwrite(k,&ucs,1);
			pgb->del(k+1,1);
			continue;
		}
		else // ((ck & 0x0080) == 0x0080)		// trailing byte in multi-byte sequence
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// let it remain as is...
			continue;
		}
	}
}

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(error,exp)	do { if (((error)=(exp)) != UT_OK) goto Cleanup; } while (0)

UT_Error IE_Imp_UTF8::importFile(const char * szFilename)
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

IE_Imp_UTF8::~IE_Imp_UTF8()
{
}

IE_Imp_UTF8::IE_Imp_UTF8(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
}

/*****************************************************************/
/*****************************************************************/

#define X_ReturnIfFail(exp,error)		do { UT_Bool b = (exp); if (!b) return (error); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

UT_Error IE_Imp_UTF8::_writeHeader(FILE * /* fp */)
{
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));

	return UT_OK;
}

UT_Error IE_Imp_UTF8::_parseFile(FILE * fp)
{
	UT_GrowBuf gbBlock(1024);
	UT_Bool bEatLF = UT_FALSE;
	UT_Bool bEmptyFile = UT_TRUE;
	UT_Bool bSmashUTF8 = UT_FALSE;
	unsigned char c;

	while (fread(&c, 1, sizeof(c), fp) > 0)
	{
		switch (c)
		{
		case '\r':
		case '\n':
			if ((c == '\n') && bEatLF)
			{
				bEatLF = UT_FALSE;
				break;
			}

			if (c == '\r')
			{
				bEatLF = UT_TRUE;
			}
			
			// we interprete either CRLF, CR, or LF as a paragraph break.
			
			// start a paragraph and emit any text that we
			// have accumulated.
			X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
			bEmptyFile = UT_FALSE;
			if (gbBlock.getLength() > 0)
			{
				if (bSmashUTF8)
					_smashUTF8(&gbBlock);
				X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
				gbBlock.truncate(0);
				bSmashUTF8 = UT_FALSE;
			}
			break;

		default:
			bEatLF = UT_FALSE;

			// deal with plain character.  to simplify parsing logic,
			// we just stuff all text chars (latin-1 and utf8 escape
			// sequences) into the GrowBuf and will smash the utf8
			// sequences into unicode in a moment.

			if (c > 0x7f)
				bSmashUTF8 = UT_TRUE;
			UT_UCSChar uc = (UT_UCSChar) c;
			X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&uc,1));
			break;
		}
	} 

	if (gbBlock.getLength() > 0 || bEmptyFile)
	{
		// if we have text left over (without final CR/LF),
		// or if we read an empty file,
		// create a paragraph and emit the text now.
		if (bSmashUTF8)
			_smashUTF8(&gbBlock);

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

void IE_Imp_UTF8::pasteFromBuffer(PD_DocumentRange * pDocRange,
								  unsigned char * pData, UT_uint32 lenData)
{
	UT_ASSERT(m_pDocument == pDocRange->m_pDoc);
	UT_ASSERT(pDocRange->m_pos1 == pDocRange->m_pos2);

	UT_GrowBuf gbBlock(1024);
	UT_Bool bEatLF = UT_FALSE;
	UT_Bool bSuppressLeadingParagraph = UT_TRUE;
	UT_Bool bInColumn1 = UT_TRUE;
	unsigned char * pc;

	PT_DocPosition dpos = pDocRange->m_pos1;
	
	for (pc=pData; (pc<pData+lenData); pc++)
	{
		unsigned char c = *pc;
		
		switch (c)
		{
		case '\r':
		case '\n':
			if ((c == '\n') && bEatLF)
			{
				bEatLF = UT_FALSE;
				break;
			}

			if (c == '\r')
			{
				bEatLF = UT_TRUE;
			}
			
			// we interprete either CRLF, CR, or LF as a paragraph break.
			
			if (gbBlock.getLength() > 0)
			{
				// flush out what we have
				m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
				dpos += gbBlock.getLength();
				gbBlock.truncate(0);
			}
			bInColumn1 = UT_TRUE;
			break;

		default:
			bEatLF = UT_FALSE;
			if (bInColumn1 && !bSuppressLeadingParagraph)
			{
				m_pDocument->insertStrux(dpos,PTX_Block);
				dpos++;
			}
			
			// deal with plain character.  to simplify parsing logic,
			// we just stuff all text chars (latin-1 and utf8 escape
			// sequences) into the GrowBuf and will smash the utf8
			// sequences into unicode in a moment.
			
			UT_UCSChar uc = (UT_UCSChar) c;
			gbBlock.ins(gbBlock.getLength(),&uc,1);

			bInColumn1 = UT_FALSE;
			bSuppressLeadingParagraph = UT_FALSE;
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

UT_Bool IE_Imp_UTF8::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// TODO: Not yet written
	return(UT_FALSE);
}

UT_Bool IE_Imp_UTF8::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".utf8") == 0);
}

UT_Error IE_Imp_UTF8::StaticConstructor(PD_Document * pDocument,
										IE_Imp ** ppie)
{
	IE_Imp_UTF8 * p = new IE_Imp_UTF8(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Imp_UTF8::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "UTF8 (.utf8)";
	*pszSuffixList = "*.utf8";
	*ft = IEFT_UTF8;
	return UT_TRUE;
}

UT_Bool IE_Imp_UTF8::SupportsFileType(IEFileType ft)
{
	return (IEFT_UTF8 == ft);
}

