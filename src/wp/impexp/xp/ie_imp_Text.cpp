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
#include <malloc.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_Text.h"
#include "pd_Document.h"
#include "ut_growbuf.h"

/*****************************************************************/
/*****************************************************************/

// Import US-ASCII (actually Latin-1) data from a plain
// text file.  We allow either LF or CRLF line termination.
// We consider adjacent lines to be in the same paragraph.
// We consider blank lines to be paragraph break.

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

IEStatus IE_Imp_Text::_writeHeader(FILE * fp)
{
	const char *attr[] = {"type", "Box", "left", "0pt", "top", "0pt", "width", "*", "height", "*", NULL};

	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_ColumnSet, NULL));
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Column, attr));
	X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));

	return IES_OK;
}

IEStatus IE_Imp_Text::_parseFile(FILE * fp)
{
	// TODO do we also want to treat a line with just whitespace as a
	// TODO paragraph separator -- in addition to just a blank line.
	
	UT_GrowBuf gbBlock(1024);
	UT_Bool bSeenLF = UT_TRUE;
	unsigned char c;

	while (fread(&c, 1, sizeof(c), fp) > 0)
	{
		if (c == '\r')					// we just ignore CR's
			continue;
		
		if (c == '\n')
		{
			if (bSeenLF)
			{
				// the previous character was a LF, this LF is interpreted
				// as a paragraph break.  output any text that we have
				// accumulated so far and begin a new paragraph.

				if (gbBlock.getLength() > 0)
				{
					X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
					gbBlock.truncate(0);
				}

				X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));

				// keep bSeenLF true so that we handle multiple blank lines.
			}
			else
			{
				// remember that we saw the LF, but don't do
				// anything about it yet.

				bSeenLF = UT_TRUE;
			}
		}
		else
		{
			// deal with plain character.
			// if we already have one LF in the middle of the paragraph,
			// we convert it into whitespace first.

			if (bSeenLF)
			{
				if (gbBlock.getLength() > 0)
				{
					UT_UCSChar ucSpace = 0x0020;
					X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&ucSpace,1));
				}
				bSeenLF = UT_FALSE;
			}

			// this cast is OK.  we have US-ASCII (actually Latin-1) character
			// data, so we can do this.
			
			UT_UCSChar uc = (UT_UCSChar) c;
			X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&uc,1));
		}
	} 

	if (gbBlock.getLength() > 0)
	{
		X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength()));
	}

	return IES_OK;
}

#undef X_ReturnNoMemIfError
#undef X_ReturnIfFail
