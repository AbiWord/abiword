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

#define X_CleanupIfError(ies,exp)	do { if (((ies)=(exp)) != IES_OK) goto Cleanup; } while (0)

IEStatus IE_Imp_Text::importFile(const char * szFilename)
{
	FILE *fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return IES_FileNotFound;
	}
	
	IEStatus iestatus;

	X_CleanupIfError(iestatus,_writeHeader(fp));
	X_CleanupIfError(iestatus,_parseFile(fp));

	iestatus = IES_OK;

Cleanup:
	fclose(fp);
	return iestatus;
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

#define X_ReturnIfFail(exp,ies)		do { UT_Bool b = (exp); if (!b) return (ies); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,IES_NoMemory)

IEStatus IE_Imp_Text::_writeHeader(FILE * /* fp */)
{
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));

	return IES_OK;
}

IEStatus IE_Imp_Text::_parseFile(FILE * fp)
{
	UT_GrowBuf gbBlock(1024);
	UT_Bool bEatLF = UT_FALSE;
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
			if (gbBlock.getLength() > 0)
			{
				X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
				gbBlock.truncate(0);
			}
			break;

		default:
			bEatLF = UT_FALSE;

			// deal with plain character.
			// this cast is OK.  we have US-ASCII (actually Latin-1) character
			// data, so we can do this.
			
			UT_UCSChar uc = (UT_UCSChar) c;
			X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&uc,1));
			break;
		}
	} 

	if (gbBlock.getLength() > 0)
	{
		// if we have text left over (without final CR/LF),
		// create a paragraph and emit the text now.
		X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
		X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
	}

	return IES_OK;
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
	UT_Bool bEatLF = UT_FALSE;
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
			
			// start a paragraph and emit any text that we
			// have accumulated.
			m_pDocument->insertStrux(dpos,PTX_Block);
			dpos += 1;
			
			if (gbBlock.getLength() > 0)
			{
				m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
				dpos += gbBlock.getLength();
				gbBlock.truncate(0);
			}
			break;

		default:
			bEatLF = UT_FALSE;

			// deal with plain character.
			// this cast is OK.  we have US-ASCII (actually Latin-1) character
			// data, so we can do this.
			
			UT_UCSChar uc = (UT_UCSChar) c;
			gbBlock.ins(gbBlock.getLength(),&uc,1);
			break;
		}
	} 

	if (gbBlock.getLength() > 0)
	{
		// if we have text left over (without final CR/LF),
		// create a paragraph and emit the text now.
		m_pDocument->insertStrux(dpos, PTX_Block);
		dpos += 1;
		m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
		dpos += gbBlock.getLength();
	}

	return;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_Text::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".txt") == 0);
}

IEStatus IE_Imp_Text::StaticConstructor(PD_Document * pDocument,
										IE_Imp ** ppie)
{
	IE_Imp_Text * p = new IE_Imp_Text(pDocument);
	*ppie = p;
	return IES_OK;
}

UT_Bool	IE_Imp_Text::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "Text (.txt)";
	*pszSuffixList = "*.txt";
	*ft = IEFT_Text;
	return UT_TRUE;
}

UT_Bool IE_Imp_Text::SupportsFileType(IEFileType ft)
{
	return (IEFT_Text == ft);
}

