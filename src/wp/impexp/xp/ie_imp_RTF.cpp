/* AbiWord
 * Copyright (C) 1999 AbiSource, Inc.
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


/* RTF importer by Peter Arnold <petera@intrinsica.co.uk> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <memory.h>
#include <math.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ie_types.h"
#include "ie_imp_RTF.h"
#include "pd_Document.h"


// Font table items
RTFFontTableItem::RTFFontTableItem(FontFamilyEnum fontFamily, int charSet, int codepage, FontPitch pitch,
									unsigned char* panose, char* pFontName, char* pAlternativeFontName)
{
	m_family = fontFamily;
	m_charSet = charSet;
	m_codepage = codepage;
	m_pitch = pitch;
	memcpy(m_panose, panose, 10*sizeof(unsigned char));
	m_pFontName = pFontName;
	m_pAlternativeFontName = pAlternativeFontName;
}


RTFFontTableItem::~RTFFontTableItem()
{
	free(m_pFontName);
	free(m_pAlternativeFontName);
}




// Character properties
RTFProps_CharProps::RTFProps_CharProps()
{
	m_deleted = UT_FALSE;
	m_bold = UT_FALSE;
	m_italic = UT_FALSE;
	m_underline = UT_FALSE;
	m_overline = UT_FALSE;
	m_strikeout = UT_FALSE;
	m_superscript = UT_FALSE;
	m_subscript = UT_FALSE;
	m_fontSize = 12.0;
	m_fontNumber = 0;
	m_colourNumber = 0;
}                


// Paragraph properties
RTFProps_ParaProps::RTFProps_ParaProps()
{
	m_justification = pjLeft;
	m_spaceBefore = 0;
	m_spaceAfter = 0;
    m_indentLeft = 0;
    m_indentRight = 0;
    m_indentFirst = 0;
	m_lineSpaceExact = UT_FALSE;
	m_lineSpaceVal = 240;
};                  


RTFProps_ParaProps& RTFProps_ParaProps::operator=(const RTFProps_ParaProps& other)
{
	if (this != &other)
	{
		m_tabStops.clear();
		m_tabTypes.clear();

		m_justification = other.m_justification;
		m_spaceBefore = other.m_spaceBefore;
		m_spaceAfter = other.m_spaceAfter;
		m_indentLeft = other.m_indentLeft;
		m_indentRight = other.m_indentRight;
		m_indentFirst = other.m_indentFirst;
		m_lineSpaceVal = other.m_lineSpaceVal;
		m_lineSpaceExact = other.m_lineSpaceExact;

		if (other.m_tabStops.getItemCount() > 0)
		{
			for (UT_uint32 i = 0; i < other.m_tabStops.getItemCount(); i++)
			{
				m_tabStops.addItem(other.m_tabStops.getNthItem(i));
			}
		}
		if (other.m_tabTypes.getItemCount() > 0)
		{
			for (UT_uint32 i = 0; i < other.m_tabTypes.getItemCount(); i++)
			{
				m_tabTypes.addItem(other.m_tabTypes.getNthItem(i));
			}
		}
	}

	return *this;
}


RTFProps_SectionProps::RTFProps_SectionProps()
{
	m_numCols = 1;
	m_breakType = sbkNone;
	m_pageNumFormat = pgDecimal;
};                  


RTFProps_SectionProps& RTFProps_SectionProps::operator=(const RTFProps_SectionProps& other)
{
	if (this != &other)
	{
		m_numCols = other.m_numCols;
		m_breakType = other.m_breakType;
		m_pageNumFormat = other.m_pageNumFormat;
	}

	return *this;
}






RTFStateStore::RTFStateStore()
{
	m_destinationState = rdsNorm;
	m_internalState = risNorm;
	m_unicodeAlternateSkipCount = 1;
	m_unicodeInAlternate = 0;
}





/*****************************************************************/
/*****************************************************************/

IE_Imp_RTF::IE_Imp_RTF(PD_Document * pDocument)
:	IE_Imp(pDocument),
	m_gbBlock(1024),
	m_groupCount(0),
	m_newParaFlagged(UT_FALSE),
	m_newSectionFlagged(UT_FALSE),
	m_cbBin(0),
	m_pImportFile(NULL)
{
	m_pPasteBuffer = NULL;
	m_lenPasteBuffer = 0;
	m_pCurrentCharInPasteBuffer = NULL;
}


IE_Imp_RTF::~IE_Imp_RTF()
{
	// Empty the state stack
	while (m_stateStack.getDepth() > 0)
	{
		RTFStateStore* pItem = NULL;
		m_stateStack.pop((void**)(&pItem));
		delete pItem;
	}

	// and the font table (can't use the macro as we allow NULLs in the vector
	const int size = m_fontTable.getItemCount();
	for (int i = size-1; i>=0; i--)
	{
		RTFFontTableItem* pItem = (RTFFontTableItem*) m_fontTable.getNthItem(i);
		delete pItem;
	}
}


UT_Error IE_Imp_RTF::importFile(const char * szFilename)
{
	m_newParaFlagged = UT_TRUE;
	m_newSectionFlagged = UT_TRUE;

	FILE *fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}
	
	UT_Error error = _writeHeader(fp);
	if (error)
	{
		error = _parseFile(fp);
	}

	fclose(fp);
	
	return error;
}


UT_Error IE_Imp_RTF::_writeHeader(FILE * /*fp*/)
{
		return UT_OK;
}


UT_Error IE_Imp_RTF::_parseFile(FILE* fp)
{
	m_pImportFile = fp;

	m_currentRTFState.m_internalState = RTFStateStore::risNorm;
	m_currentRTFState.m_destinationState = RTFStateStore::rdsNorm;

	UT_Bool ok = UT_TRUE;
    int cNibble = 2;
	int b = 0;
	unsigned char c;
	while (ok  &&  ReadCharFromFile(&c))
	{
		if (m_currentRTFState.m_internalState == RTFStateStore::risBin)
		{
			// if we're parsing binary data, handle it directly
			ok = ParseChar(c);
		}
		else
		{
			switch (c)
			{
			case '{':
				ok = PushRTFState();
				break;
			case '}':
				ok = PopRTFState();
				break;
			case '\\':
				ok = ParseRTFKeyword();
				break;
			default:
				if (m_currentRTFState.m_internalState == RTFStateStore::risNorm)
				{
					ok = ParseChar(c);
				}
				else
				{
					UT_ASSERT(m_currentRTFState.m_internalState == RTFStateStore::risHex);

					b = b << 4;
					if (isdigit(c))
						b += (char) c - '0';
					else
					{
						if (islower(c))
						{
							ok = (c >= 'a' && c <= 'f');
							b += (char) c - 'a' + 10;
						}
						else
						{
							ok = (c >= 'A' && c <= 'F');
							b += (char) c - 'A' + 10;
						}
					}
					cNibble--;
					if (!cNibble  &&  ok)
					{
						ok = ParseChar(b);
						cNibble = 2;
						b = 0;
						m_currentRTFState.m_internalState = RTFStateStore::risNorm;
						if (ok)  // skip following space if applicable
						{   
							unsigned char ch;
							if (!ReadCharFromFile(&ch))
								ok = UT_FALSE; 
							else if (ch != ' ') 
								SkipBackChar(ch);
						}   
					}
				}
			}
		}
	}

	if (ok)
	{
		ok = FlushStoredChars();
	}
	return ok ? UT_OK : UT_ERROR;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_RTF::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".rtf") == 0);
}

UT_Error IE_Imp_RTF::StaticConstructor(PD_Document * pDocument,
										IE_Imp ** ppie)
{
	IE_Imp_RTF * p = new IE_Imp_RTF(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Imp_RTF::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "Rich Text Format (.rtf)";
	*pszSuffixList = "*.rtf";
	*ft = IEFT_RTF;
	return UT_TRUE;
}

UT_Bool IE_Imp_RTF::SupportsFileType(IEFileType ft)
{
	return (IEFT_RTF == ft);
}


// flush any remaining text in the previous para and flag
// a new para to be started.  Don't actually start a new
// para as it may turn out to be empty
// 
UT_Bool IE_Imp_RTF::StartNewPara()
{
	UT_Bool ok = FlushStoredChars(UT_TRUE);
	
	m_newParaFlagged = UT_TRUE;

	return ok;
}


// flush any remaining text in the previous sction and
// flag a new section to be started.  Don't actually 
// start a new section as it may turn out to be empty
// 
UT_Bool IE_Imp_RTF::StartNewSection()
{
	UT_Bool ok = FlushStoredChars(UT_TRUE);
	
	m_newParaFlagged = UT_TRUE;
	m_newSectionFlagged = UT_TRUE;

	return ok;
}


// add a new character.  Characters are actually cached and 
// inserted into the document in batches - see FlushStoredChars
// 
UT_Bool IE_Imp_RTF::AddChar(UT_UCSChar ch)
{
	return m_gbBlock.ins(m_gbBlock.getLength(), &ch, 1);
}


// flush any stored text into the document
// 
UT_Bool IE_Imp_RTF::FlushStoredChars(UT_Bool forceInsertPara)
{
	// start a new para if we have to
	UT_Bool ok = UT_TRUE;
        if (m_newSectionFlagged && (forceInsertPara || (m_gbBlock.getLength() > 0)) )
	{
		ok = ApplySectionAttributes();
		m_newSectionFlagged = UT_FALSE;
	}

	if (ok  &&  m_newParaFlagged  &&  (forceInsertPara  ||  (m_gbBlock.getLength() > 0)) )
	{
		ok = ApplyParagraphAttributes();
		m_newParaFlagged = UT_FALSE;
	}

	if (ok  &&  (m_gbBlock.getLength() > 0))
		ok = ApplyCharacterAttributes();

	return ok;
}


// Get a font out of the font table, making sure we dont attempt to access off the end
RTFFontTableItem* IE_Imp_RTF::GetNthTableFont(UT_uint32 fontNum)
{
	if (fontNum < m_fontTable.getItemCount())
	{
		return (RTFFontTableItem*)m_fontTable.getNthItem(fontNum);
	}
	else
	{
		return NULL;
	}
}


// Get a colour out of the colour table, making sure we dont attempt to access off the end
UT_uint32 IE_Imp_RTF::GetNthTableColour(UT_uint32 colNum)
{
	if (colNum < m_colourTable.getItemCount())
	{
		return (UT_uint32)m_colourTable.getNthItem(colNum);
	}
	else
	{
		return 0;	// black
	}
}


// Pushes the current state RTF state onto the state stack
//
UT_Bool IE_Imp_RTF::PushRTFState(void)
{
	// Create a new object to store the state in
	RTFStateStore* pState = new RTFStateStore;
	if (pState == NULL)
	{
	    return UT_FALSE;
	}
	*pState = m_currentRTFState;
	m_stateStack.push(pState);

	// Reset the current state
	m_currentRTFState.m_internalState = RTFStateStore::risNorm;

	return UT_TRUE;
}



// Pops the state off the top of the RTF stack and set the current state to it.
//
UT_Bool IE_Imp_RTF::PopRTFState(void)
{
	RTFStateStore* pState = NULL;
	m_stateStack.pop((void**)&pState);

	if (pState != NULL)
	{
		UT_Bool ok = FlushStoredChars();
		m_currentRTFState = *pState;
		delete pState;

		m_currentRTFState.m_unicodeInAlternate = 0;
		
		return ok;
	}
	else
	{
		UT_ASSERT(pState != NULL);	// state stack should not be empty
		return UT_FALSE;
	}
}


// Process a single character from the RTF stream
//
UT_Bool IE_Imp_RTF::ParseChar(UT_UCSChar ch)
{
    // Have we reached the end of the binary skip?
	if (m_currentRTFState.m_internalState == RTFStateStore::risBin  && --m_cbBin <= 0)
	{
		m_currentRTFState.m_internalState = RTFStateStore::risNorm;
	}

	switch (m_currentRTFState.m_destinationState)
	{
		case RTFStateStore::rdsSkip:
			// Toss this character.
			return UT_TRUE;
		case RTFStateStore::rdsNorm:
			if (m_currentRTFState.m_unicodeInAlternate > 0)
			{
				m_currentRTFState.m_unicodeInAlternate--;
				return UT_TRUE;
			}
			// Insert a character into the story
			if ((ch >= 32  ||  ch == 9)  &&  !m_currentRTFState.m_charProps.m_deleted)
			{
				return AddChar(ch);
			}
		default:
			// handle other destinations....
			return UT_TRUE;
	}
}


// Reads and proccesses a RTF control word and its parameter
//
UT_Bool IE_Imp_RTF::ParseRTFKeyword()
{
	unsigned char keyword[256];
	long parameter = 0;
	UT_Bool parameterUsed = UT_FALSE;

	if (ReadKeyword(keyword, &parameter, &parameterUsed))
		return TranslateKeyword(keyword, parameter, parameterUsed);
	else
		return UT_FALSE;
}


UT_Bool IE_Imp_RTF::ReadKeyword(unsigned char* pKeyword, long* pParam, UT_Bool* pParamUsed)
{
	UT_Bool fNegative = UT_FALSE;
	*pParam = 0;
	*pParamUsed = UT_FALSE;
	*pKeyword = 0;
	unsigned char parameter[256];
	int count = 0;

	// Read the first character of the control word
	unsigned char ch;
	if (!ReadCharFromFile(&ch))
		return UT_FALSE;

	// If it's a control symbol there is no delimiter, its just one character
	if (!isalpha(ch))
	{
		pKeyword[0] = ch;
		pKeyword[1] = 0;
		return UT_TRUE;
	}

	// Read in the rest of the control word
	while (isalpha(ch))
	{
		*pKeyword = ch;
		pKeyword++;
		if (!ReadCharFromFile(&ch))
			return UT_FALSE;
	}
	*pKeyword = 0;

    // If the delimeter was '-' then the following parameter is negative
    if (ch == '-')
    {
        fNegative = UT_TRUE;
		if (!ReadCharFromFile(&ch))
			return UT_FALSE;
    }

    // Read the numeric parameter (if there is one)
	if (isdigit(ch))
	{
		*pParamUsed = UT_TRUE;
		while (isdigit(ch))
		{
			parameter[count++] = ch;
			if (!ReadCharFromFile(&ch))
				return UT_FALSE;
		}
		parameter[count] = 0;

		*pParam = atol((char*)parameter);
		if (fNegative)
			*pParam = -*pParam;
	}

	// If the delimeter was non-space then this character is part of the following text!
	if (ch != ' ')
	{
		SkipBackChar(ch);
	}

	return UT_TRUE;
}


UT_Bool IE_Imp_RTF::ReadCharFromFile(unsigned char* pCh)
{
	if (m_pImportFile)					// if we are reading a file
	{
		while (fread(pCh, 1, sizeof(unsigned char), m_pImportFile) > 0)
		{
			// line feed and cr should be ignored in RTF files
			if (*pCh != 10  &&  *pCh != 13)
			{
				return UT_TRUE;
			}
		}

		return UT_FALSE;
	}
	else								// else we are pasting from a buffer
	{
		while (m_pCurrentCharInPasteBuffer < m_pPasteBuffer+m_lenPasteBuffer)
		{
			*pCh = *m_pCurrentCharInPasteBuffer++;
			
			// line feed and cr should be ignored in RTF files
			if (*pCh != 10  &&  *pCh != 13)
			{
				return UT_TRUE;
			}
		}

		return UT_FALSE;
	}
}


UT_Bool IE_Imp_RTF::SkipBackChar(unsigned char ch)
{
	if (m_pImportFile)					// if we are reading a file
	{
		// TODO - I've got a sneaking suspicion that this doesn't work on the Macintosh
		return (ungetc(ch, m_pImportFile) != EOF);
	}
	else								// else we are pasting from a buffer
	{
		UT_Bool bStatus = (m_pCurrentCharInPasteBuffer > m_pPasteBuffer);
		if (bStatus)
			m_pCurrentCharInPasteBuffer--;
		return bStatus;
	}
}


// Test the keyword against all the known handlers
UT_Bool IE_Imp_RTF::TranslateKeyword(unsigned char* pKeyword, long param, UT_Bool fParam)
{
	// switch on the first char to reduce the number of string comparisons
	// NB. all RTF keywords are lowercase.
	switch (*pKeyword)
	{
	case 'b':
		if (strcmp((char*)pKeyword, "b") == 0)
		{
			// bold - either on or off depending on the parameter
			return HandleBold(fParam ? UT_FALSE : UT_TRUE);
		}
		else if (strcmp((char*)pKeyword, "bullet") == 0)
		{
			return ParseChar(UCS_BULLET);
		}
		break;

	case 'c':
		if (strcmp((char*)pKeyword, "colortbl") == 0)
		{
			return ReadColourTable();
		}
		else if (strcmp((char*)pKeyword, "cf") == 0)
		{
			return HandleColour(fParam ? param : 0);
		}
		else if (strcmp((char*)pKeyword, "cols") == 0)
		{
			m_currentRTFState.m_sectionProps.m_numCols = (UT_uint32)param;
		}
		break;

	case 'd':
		if (strcmp((char*)pKeyword, "deleted") == 0)
		{
			// bold - either on or off depending on the parameter
			return HandleDeleted(fParam ? UT_FALSE : UT_TRUE);
		}

	case 'e':
		if (strcmp((char*)pKeyword, "emdash") == 0)
		{
			return ParseChar(UCS_EM_DASH);
		}
		else if (strcmp((char*)pKeyword, "endash") == 0)
		{
			return ParseChar(UCS_EN_DASH);
		}
		else if (strcmp((char*)pKeyword, "emspace") == 0)
		{
			return ParseChar(UCS_EM_SPACE);
		}
		else if (strcmp((char*)pKeyword, "enspace") == 0)
		{
			return ParseChar(UCS_EN_SPACE);
		}
		break;

	case 'f':
		if (strcmp((char*)pKeyword, "fonttbl") == 0)
		{
			// read in the font table
			return ReadFontTable();
		}
		else if (strcmp((char*)pKeyword, "fs") == 0)
		{
			return HandleFontSize(fParam ? param : 24);
		}
		else if (strcmp((char*)pKeyword, "f") == 0)
		{
			return HandleFace(fParam ? param : 0); // TODO read the deff prop and use that instead of 0
		}
		else if (strcmp((char*)pKeyword, "fi") == 0)
		{
			m_currentRTFState.m_paraProps.m_indentFirst = param;
		}

		break;

	case 'i':
		if (strcmp((char*)pKeyword, "i") == 0)
		{
			// italic - either on or off depending on the parameter
			return HandleItalic(fParam ? UT_FALSE : UT_TRUE);
		}
		else if (strcmp((char*)pKeyword, "info") == 0)
		{
			// TODO Ignore document info for the moment
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
		}
		break;

	case 'l':
		if (strcmp((char*)pKeyword, "lquote") == 0)
		{
			return ParseChar(UCS_LQUOTE);
		}
		else if (strcmp((char*)pKeyword, "ldblquote") == 0)
		{
			return ParseChar(UCS_LDBLQUOTE);
		}
		else if (strcmp((char*)pKeyword, "li") == 0)
		{
			m_currentRTFState.m_paraProps.m_indentLeft = param;
		}
		break;
	case 'o': 
	        if (strcmp((char*)pKeyword,"ol") == 0)
	        {
		         return HandleOverline(fParam ? param : 1);
		}
		break;
	case 'p':
		if (strcmp((char*)pKeyword, "par") == 0)
		{
			// start new paragraph, continue current attributes
			return StartNewPara();
		}
		else if (strcmp((char*)pKeyword, "plain") == 0)
		{
			// reset character attributes
			return ResetCharacterAttributes();
		}
		else if (strcmp((char*)pKeyword, "pard") == 0)
		{
			// reset paragraph attributes
			return ResetParagraphAttributes();
		}
		break;

	case 'q':
		if (strcmp((char*)pKeyword, "ql") == 0)
		{
			return SetParaJustification(RTFProps_ParaProps::pjLeft);
		}
		else if (strcmp((char*)pKeyword, "qc") == 0)
		{
			return SetParaJustification(RTFProps_ParaProps::pjCentre);
		}
		else if (strcmp((char*)pKeyword, "qr") == 0)
		{
			return SetParaJustification(RTFProps_ParaProps::pjRight);
		}
		else if (strcmp((char*)pKeyword, "qj") == 0)
		{
			return SetParaJustification(RTFProps_ParaProps::pjFull);
		}
		break;

	case 'r':
		if (strcmp((char*)pKeyword, "rquote") == 0)
		{
			return ParseChar(UCS_RQUOTE);
		}
		else if (strcmp((char*)pKeyword, "rdblquote") == 0)
		{
			return ParseChar(UCS_RDBLQUOTE);
		}
		else if (strcmp((char*)pKeyword, "ri") == 0)
		{
			m_currentRTFState.m_paraProps.m_indentRight = param;
		}
		break;

	case 's':
		if (strcmp((char*)pKeyword, "stylesheet") == 0)
		{
			// TODO Ignore stylesheet as ABIWord doesn't do styles (at the moment)
			// In the text all applied styles also have their equivlent effects on too
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
		}
		else if (strcmp((char*)pKeyword, "strike") == 0  ||  strcmp((char*)pKeyword, "striked") == 0)
		{
			return HandleStrikeout(fParam ? param : 1);
		}
		else if (strcmp((char*)pKeyword, "sect") == 0 )
		{
			return StartNewSection();
		}
		else if (strcmp((char*)pKeyword, "sectd") == 0 )
		{
			return ResetSectionAttributes();
		}
		else if (strcmp((char*)pKeyword, "sa") == 0)
		{
			m_currentRTFState.m_paraProps.m_spaceAfter = param;
		}
		else if (strcmp((char*)pKeyword, "sb") == 0)
		{
			m_currentRTFState.m_paraProps.m_spaceBefore = param;
		}
		else if (strcmp((char*)pKeyword, "sl") == 0)
		{
			if (!fParam  ||  param == 0)
				m_currentRTFState.m_paraProps.m_lineSpaceVal = 360;
			else
				m_currentRTFState.m_paraProps.m_lineSpaceVal = param;
		}
		else if (strcmp((char*)pKeyword, "slmult") == 0)
		{
			m_currentRTFState.m_paraProps.m_lineSpaceExact = (!fParam  ||  param == 0);
		}
		else if (strcmp((char*)pKeyword, "super") == 0)
		{
			return HandleSuperscript(fParam ? UT_FALSE : UT_TRUE);
		}	
		else if (strcmp((char*)pKeyword, "sub") == 0)
		{
			return HandleSubscript(fParam ? UT_FALSE : UT_TRUE);
		}
		break;

	case 't':
		if (strcmp((char*)pKeyword, "tab") == 0)
		{
			return ParseChar('\t');
		}
		else if (strcmp((char*)pKeyword, "tx") == 0)
		{
			UT_ASSERT(fParam);	// tabstops should have parameters
			return AddTabstop(param);
		}
		break;

	case 'u':
		if (strcmp((char*)pKeyword, "ul") == 0        ||  strcmp((char*)pKeyword, "uld") == 0  ||
			strcmp((char*)pKeyword, "uldash") == 0    ||  strcmp((char*)pKeyword, "uldashd") == 0  ||
			strcmp((char*)pKeyword, "uldashdd") == 0  ||  strcmp((char*)pKeyword, "uldb") == 0  ||
			strcmp((char*)pKeyword, "ulth") == 0      ||  strcmp((char*)pKeyword, "ulw") == 0  ||
			strcmp((char*)pKeyword, "ulwave") == 0)
		{
			return HandleUnderline(fParam ? param : 1);
		}
		else if (strcmp((char*)pKeyword, "ulnone") == 0)
		{
			return HandleUnderline(0);
		}
		else if (strcmp((char*)pKeyword,"uc") == 0)
		{
			// "\uc<n>" defines the number of chars immediately following
			// any "\u<u>" unicode character that are needed to represent
			// a reasonable approximation for the unicode character.
			// generally, this is done by stripping off accents from latin-n
			// characters so that they fold into latin1.
			//
			// the spec says that we need to allow any arbitrary length
			// of chars for this and that we need to maintain a stack of
			// these lengths (as content is nested within {} groups) so
			// that different 'destinations' can have different approximations
			// or have a local diversion for a hard-to-represent character
			// or something like that.
			//
			// this is bullshit (IMHO) -- jeff
			
			m_currentRTFState.m_unicodeAlternateSkipCount = param;
			m_currentRTFState.m_unicodeInAlternate = 0;
		}
		else if (strcmp((char*)pKeyword,"u") == 0)
		{
			UT_Bool bResult = ParseChar((UT_UCSChar)param);
			m_currentRTFState.m_unicodeInAlternate = m_currentRTFState.m_unicodeAlternateSkipCount;
			return bResult;
		}
		break;

	case '*':
		if (strcmp((char*)pKeyword, "*") == 0)
		{
		  //
		  // Code to handle overline importing.
		  //
		  unsigned char keyword_star[256];
		  long parameter_star = 0;
		  UT_Bool parameterUsed_star = UT_FALSE;
		  //
		  // Look for \*\ol sequence. Ignore all others
		  // 
		  if (ReadKeyword(keyword_star, &parameter_star, &parameterUsed_star))
		    {
		      if( strcmp((char*)keyword_star, "\\")== 0)
                        {
		          if (ReadKeyword(keyword_star, &parameter_star, &parameterUsed_star))
		            { 		
      		               if( strcmp((char*)keyword_star,"ol") == 0)
			         { 
			            return HandleOverline(parameterUsed_star ? 
					      parameter_star : 1);
                                 }
                            }
			}
		    }

// Ignore all other \* tags
// TODO different destination (all unhandled at the moment, so enter skip mode)
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
		}
		break;
	case '\'':
		if (strcmp((char*)pKeyword, "\'") == 0)
		{
			m_currentRTFState.m_internalState = RTFStateStore::risHex;
		}
		break;
	case '{':
	case '}':
	case '\\':
		ParseChar(*pKeyword);
		break;
	case '~':
		ParseChar(UCS_NBSP);
		break;
	case '-':
	case '_':
		ParseChar('-');	// TODO - make these optional and nonbreaking
		break;
	}

	return UT_TRUE;
}


UT_Bool IE_Imp_RTF::ApplyCharacterAttributes()
{
	XML_Char* pProps = "PROPS";
	XML_Char propBuffer[1024];	//TODO is this big enough?  better to make it a member and stop running all over the stack
	XML_Char tempBuffer[128];

	propBuffer[0] = 0;

	// bold
	strcat(propBuffer, "font-weight:");
	strcat(propBuffer, m_currentRTFState.m_charProps.m_bold ? "bold" : "normal");
	// italic
	strcat(propBuffer, "; font-style:");
	strcat(propBuffer, m_currentRTFState.m_charProps.m_italic ? "italic" : "normal");
	// underline & overline & strike-out
	strcat(propBuffer, "; text-decoration:");
	if (m_currentRTFState.m_charProps.m_underline  &&  m_currentRTFState.m_charProps.m_strikeout && !m_currentRTFState.m_charProps.m_overline)
	{
		strcat(propBuffer, "underline line-through");
	}
	else if (m_currentRTFState.m_charProps.m_underline  &&  !m_currentRTFState.m_charProps.m_strikeout  && !m_currentRTFState.m_charProps.m_overline)
	{
		strcat(propBuffer, "underline");
	}
	else if (!m_currentRTFState.m_charProps.m_underline  &&  m_currentRTFState.m_charProps.m_strikeout  && !m_currentRTFState.m_charProps.m_overline)
	{
		strcat(propBuffer, "line-through");
	}
	else if(m_currentRTFState.m_charProps.m_underline  &&  m_currentRTFState.m_charProps.m_strikeout && m_currentRTFState.m_charProps.m_overline)
	{
		strcat(propBuffer, "underline overline line-through");
	}
	else if(!m_currentRTFState.m_charProps.m_underline  &&  m_currentRTFState.m_charProps.m_strikeout && m_currentRTFState.m_charProps.m_overline)
	{
		strcat(propBuffer, "overline line-through");
	}
	else if(m_currentRTFState.m_charProps.m_underline  &&  !m_currentRTFState.m_charProps.m_strikeout && m_currentRTFState.m_charProps.m_overline)
	{
		strcat(propBuffer, "underline overline");
	}
	else if(!m_currentRTFState.m_charProps.m_underline  &&  !m_currentRTFState.m_charProps.m_strikeout && m_currentRTFState.m_charProps.m_overline)
	{
		strcat(propBuffer, "overline");
	}
	else
	{
		strcat(propBuffer, "none");
	}
	//superscript and subscript
	strcat(propBuffer, "; text-position:");
	if (m_currentRTFState.m_charProps.m_superscript)
	{
		strcat(propBuffer, "superscript");
	}
	else if (m_currentRTFState.m_charProps.m_subscript)
	{
		strcat(propBuffer, "subscript");
	}
	else
	{
		strcat(propBuffer, "normal");
	}

	// font size
	sprintf(tempBuffer, "; font-size:%dpt", (int)(m_currentRTFState.m_charProps.m_fontSize+0.5));
	strcat(propBuffer, tempBuffer);
	// typeface
	RTFFontTableItem* pFont = GetNthTableFont(m_currentRTFState.m_charProps.m_fontNumber);
	if (pFont != NULL)
	{
		strcat(propBuffer, "; font-family:");
		strcat(propBuffer, pFont->m_pFontName);
	}
	// colour
	UT_uint32 colour = GetNthTableColour(m_currentRTFState.m_charProps.m_colourNumber);
	sprintf(tempBuffer, "; color:%06x", colour);
	tempBuffer[14] = 0;
	strcat(propBuffer, tempBuffer);

	const XML_Char* propsArray[3];
	propsArray[0] = pProps;
	propsArray[1] = propBuffer;
	propsArray[2] = NULL;

	UT_Bool ok;
	if (m_pImportFile)					// if we are reading from a file
	{
		ok = (   m_pDocument->appendFmt(propsArray)
			  && m_pDocument->appendSpan(m_gbBlock.getPointer(0), m_gbBlock.getLength()) );
	}
	else								// else we are pasting from a buffer
	{
		ok = (   m_pDocument->insertSpan(m_dposPaste,
										 m_gbBlock.getPointer(0),m_gbBlock.getLength())
				 && m_pDocument->changeSpanFmt(PTC_AddFmt,
											   m_dposPaste,m_dposPaste+m_gbBlock.getLength(),
											   propsArray,NULL));
		m_dposPaste += m_gbBlock.getLength();
	}

	m_gbBlock.truncate(0);
	return ok;
}


UT_Bool IE_Imp_RTF::ResetCharacterAttributes()
{
	UT_Bool ok = FlushStoredChars();

	m_currentRTFState.m_charProps = RTFProps_CharProps();

	return ok;
}


UT_Bool IE_Imp_RTF::ApplyParagraphAttributes()
{
	XML_Char* pProps = "PROPS";
	XML_Char propBuffer[1024];	//TODO is this big enough?  better to make it a member and stop running all over the stack // TODO consider using a UT_ByteBuf instead -- jeff
	XML_Char tempBuffer[128];

	propBuffer[0] = 0;

	// tabs
	if (m_currentRTFState.m_paraProps.m_tabStops.getItemCount() > 0)
	{
		UT_ASSERT(m_currentRTFState.m_paraProps.m_tabStops.getItemCount() ==
					m_currentRTFState.m_paraProps.m_tabTypes.getItemCount() );

		strcat(propBuffer, "tabstops:");
		for (UT_uint32 i = 0; i < m_currentRTFState.m_paraProps.m_tabStops.getItemCount(); i++)
		{
			if (i > 0)
				strcat(propBuffer, ",");

			UT_sint32 tabTwips = (UT_sint32)m_currentRTFState.m_paraProps.m_tabStops.getNthItem(i);
			double tabIn = tabTwips/(20.0*72.0);

			sprintf(tempBuffer, "%s/L", UT_convertToDimensionString(DIM_IN,tabIn,"04"));	// TODO - left tabs only
			strcat(propBuffer, tempBuffer);
		}

		strcat(propBuffer, "; ");
	}

	// justification
	strcat(propBuffer, "text-align:");
	switch (m_currentRTFState.m_paraProps.m_justification)
	{
		case RTFProps_ParaProps::pjCentre:
			strcat(propBuffer, "center");
			break;
		case RTFProps_ParaProps::pjRight:
			strcat(propBuffer, "right");
			break;
		case RTFProps_ParaProps::pjFull:
			strcat(propBuffer, "justify");
			break;
		default:
			UT_ASSERT(UT_FALSE);	// so what is it?
		case RTFProps_ParaProps::pjLeft:
			strcat(propBuffer, "left");
			break;
	}
	strcat(propBuffer, "; ");

	// indents - first, left and right, top and bottom
	sprintf(tempBuffer, "margin-top:%s; ",		UT_convertToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_spaceBefore/1440));
	strcat(propBuffer, tempBuffer);
	sprintf(tempBuffer, "margin-bottom:%s; ",	UT_convertToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_spaceAfter/1440));
	strcat(propBuffer, tempBuffer);
	sprintf(tempBuffer, "margin-left:%s; ",		UT_convertToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_indentLeft/1440));
	strcat(propBuffer, tempBuffer);
	sprintf(tempBuffer, "margin-right:%s; ",	UT_convertToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_indentRight/1440));
	strcat(propBuffer, tempBuffer);
	sprintf(tempBuffer, "text-indent:%s; ",		UT_convertToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_indentFirst/1440));
	strcat(propBuffer, tempBuffer);
	
	// line spacing
	if (m_currentRTFState.m_paraProps.m_lineSpaceExact)
	{
		// ABIWord doesn't (yet) support exact line spacing we'll just fall back to single
		sprintf(tempBuffer, "line-height:1.0");
	}
	else
	{
		sprintf(tempBuffer, "line-height:%s",	UT_convertToDimensionlessString(fabs(m_currentRTFState.m_paraProps.m_lineSpaceVal/240)));
	}

	strcat(propBuffer, tempBuffer);

	const XML_Char* propsArray[3];
	propsArray[0] = pProps;
	propsArray[1] = propBuffer;
	propsArray[2] = NULL;

	if (m_pImportFile)					// if we are reading a file
		return m_pDocument->appendStrux(PTX_Block, propsArray);
	else
	{
		UT_Bool bSuccess = m_pDocument->insertStrux(m_dposPaste,PTX_Block);
		m_dposPaste++;
		if (bSuccess)
			bSuccess = m_pDocument->changeStruxFmt(PTC_AddFmt,m_dposPaste,m_dposPaste,
												   propsArray,NULL,PTX_Block);
		return bSuccess;
	}
}


UT_Bool IE_Imp_RTF::ResetParagraphAttributes()
{
	UT_Bool ok = FlushStoredChars();

	m_currentRTFState.m_paraProps = RTFProps_ParaProps();

	return ok;
}


UT_Bool IE_Imp_RTF::ResetSectionAttributes()
{
	UT_Bool ok = FlushStoredChars();

	m_currentRTFState.m_sectionProps = RTFProps_SectionProps();

	return ok;
}


UT_Bool IE_Imp_RTF::ApplySectionAttributes()
{
	XML_Char* pProps = "PROPS";
	XML_Char propBuffer[1024];	//TODO is this big enough?  better to make it a member and stop running all over the stack
	XML_Char tempBuffer[128];

	propBuffer[0] = 0;

	// columns
	sprintf(tempBuffer, "columns:%d", m_currentRTFState.m_sectionProps.m_numCols);
	strcat(propBuffer, tempBuffer);

	const XML_Char* propsArray[3];
	propsArray[0] = pProps;
	propsArray[1] = propBuffer;
	propsArray[2] = NULL;

	if (m_pImportFile)					// if we are reading a file
		return m_pDocument->appendStrux(PTX_Section, propsArray);
	else
	{
		UT_Bool bSuccess = m_pDocument->insertStrux(m_dposPaste,PTX_Section);
		m_dposPaste++;
		if (bSuccess)
			bSuccess = m_pDocument->changeStruxFmt(PTC_AddFmt,m_dposPaste,m_dposPaste,
												   propsArray,NULL,PTX_Section);
		return bSuccess;
	}
}


//////////////////////////////////////////////////////////////////////////////
// Font table reader
//////////////////////////////////////////////////////////////////////////////

// Reads the RTF font table, storing it for future use
//
UT_Bool IE_Imp_RTF::ReadFontTable()
{
	// Ensure the font table is empty before we start
	UT_ASSERT(m_fontTable.getItemCount() == 0);
	UT_VECTOR_PURGEALL(RTFFontTableItem*, m_fontTable);	

	unsigned char ch;
	if (!ReadCharFromFile(&ch))
		return UT_FALSE;

	if (ch == '\\')
	{
		// one entry in the font table
		// TODO - Test one item font tables!
		if (!ReadOneFontFromTable())
			return UT_FALSE;
	}
	else
	{
		if (ch != '{')
			return UT_FALSE;

		// multiple entries in font table
		while (ch != '}')
		{
			if (ch == '{')
			{
				if (!ReadCharFromFile(&ch))
					return UT_FALSE;
			}
			
			if (!ReadOneFontFromTable())
				return UT_FALSE;

			// now eat whitespace until we hit either '}' (end of font group) or '{' (another font item)
			do
			{
				if (!ReadCharFromFile(&ch))
					return UT_FALSE;
			} while (ch != '}'  &&  ch != '{');
		}
	}

	// Put the close group symbol back into the input stream
	return SkipBackChar(ch);
}


// Reads one font item from the font table in the RTF font table.  When called
// the file must be at the f of the 'f' (fontnum) keyword.
// Our life is made easier as the order of items in the table is specified.
//
UT_Bool IE_Imp_RTF::ReadOneFontFromTable()
{
	unsigned char keyword[1024];
	unsigned char ch;
	long parameter = 0;
	UT_Bool paramUsed = UT_FALSE;

	// run though the item reading in these values
	RTFFontTableItem::FontFamilyEnum fontFamily = RTFFontTableItem::ffNone;
	RTFFontTableItem::FontPitch pitch = RTFFontTableItem::fpDefault;
	UT_uint32 fontIndex = 0;
	int charSet = 0;
	int codepage = 0;
	unsigned char panose[10];
	memset(panose, 0, sizeof(unsigned char));
	char* pFontName = NULL;
	char* pAlternativeFontName = NULL;

	// Read the font index (must be specified)
	if (!ReadKeyword(keyword, &parameter, &paramUsed)  ||
		 (strcmp((char*)keyword, "f") != 0)  ||
		!paramUsed)
	{
		return UT_FALSE;
	}
	else
	{
		fontIndex = parameter;
	}

	// Read the font family (must be specified)
	if (!ReadCharFromFile(&ch)  ||
		 ch != '\\'  ||
	    !ReadKeyword(keyword, &parameter, &paramUsed))
	{
		return UT_FALSE;
	}
	else
	{
		if (strcmp((char*)keyword, "fnil") == 0)
			fontFamily = RTFFontTableItem::ffNone;
		else if (strcmp((char*)keyword, "froman") == 0)
			fontFamily = RTFFontTableItem::ffRoman;
		else if (strcmp((char*)keyword, "fswiss") == 0)
			fontFamily = RTFFontTableItem::ffSwiss;
		else if (strcmp((char*)keyword, "fmodern") == 0)
			fontFamily = RTFFontTableItem::ffModern;
		else if (strcmp((char*)keyword, "fscript") == 0)
			fontFamily = RTFFontTableItem::ffScript;
		else if (strcmp((char*)keyword, "fdecor") == 0)
			fontFamily = RTFFontTableItem::ffDecorative;
		else if (strcmp((char*)keyword, "ftech") == 0)
			fontFamily = RTFFontTableItem::ffTechnical;
		else if (strcmp((char*)keyword, "fbidi") == 0)
			fontFamily = RTFFontTableItem::ffBiDirectional;
		else
		{
			fontFamily = RTFFontTableItem::ffNone;
		}
	}

	// Now (possibly) comes some optional keyword before the fontname
	if (!ReadCharFromFile(&ch))
		return UT_FALSE;
	while (ch == '\\'  ||  ch == '{')
	{
		if (ch == '{')
		{
			if (!ReadCharFromFile(&ch))
				return UT_FALSE;
		}

		if (!ReadKeyword(keyword, &parameter, &paramUsed))
			return UT_FALSE;

		if (strcmp((char*)keyword, "fcharset") == 0)
		{
			UT_ASSERT(paramUsed);	
			charSet = parameter;
		}
		else if (strcmp((char*)keyword, "cpg") == 0)
		{
			UT_ASSERT(paramUsed);	
			codepage = parameter;
		}
		else if (strcmp((char*)keyword, "panose") == 0)
		{
			if (!ReadCharFromFile(&ch))
				return UT_FALSE;
			for (int i = 0; i < 10; i++)
			{
				unsigned char buf[3];

				if ( !ReadCharFromFile(&(buf[0]))  ||  !ReadCharFromFile(&(buf[1])) )
				{
					return UT_FALSE;
				}
				unsigned char val = (unsigned char)(atoi((char*)buf));
				panose[i] = val;
			}
		}

		//TODO - handle the other keywords

		if (!ReadCharFromFile(&ch))
			return UT_FALSE;
	}

	// Now comes the font name, terminated by either a close brace or a slash or a semi-colon
	int count = 0;
	while ( ch != '}'  &&  ch != '\\'  &&  ch != ';' )
	{
		keyword[count++] = ch;
		if (!ReadCharFromFile(&ch))
			return UT_FALSE;
	}
	keyword[count] = 0;

	if (!UT_cloneString(pFontName, (char*)keyword))
	{
		// TODO outofmem
	}			

	// Munch the remaining control words down to the close brace
	while (ch != '}')
	{
		if (!ReadCharFromFile(&ch))
			return UT_FALSE;
	}

	// Create the font entry and put it into the font table
	RTFFontTableItem* pNewFont = new RTFFontTableItem(fontFamily, charSet, codepage, pitch,
						panose, pFontName, pAlternativeFontName);
	if (pNewFont == NULL)
		return UT_FALSE;

	// ensure that the font table is large enough for this index
	while (m_fontTable.getItemCount() <= fontIndex)
	{
		m_fontTable.addItem(NULL);
	}
	void* pOld = NULL;
	UT_sint32 res = m_fontTable.setNthItem(fontIndex, pNewFont, &pOld);
	UT_ASSERT(res == 0);
	UT_ASSERT(pOld == NULL);

	return UT_TRUE;
}




//////////////////////////////////////////////////////////////////////////////
// Colour table reader
//////////////////////////////////////////////////////////////////////////////

UT_Bool IE_Imp_RTF::ReadColourTable()
{
	// Ensure the table is empty before we start
	UT_ASSERT(m_colourTable.getItemCount() == 0);

	unsigned char keyword[1024];
	unsigned char ch;
	long parameter = 0;
	UT_Bool paramUsed = UT_FALSE;
	if (!ReadCharFromFile(&ch))
		return UT_FALSE;

	while (ch != '}')
	{
		UT_uint32 colour = 0;
		UT_Bool tableError = UT_FALSE;

		// Create a new entry for the colour table
 		if (ch == ';')
		{
			// Default colour required, black it is
			colour = 0;
		}
		else
		{
			if (ch == '\\')
			{
				// read colour definition
				long red = 0;
				long green = 0;
				long blue = 0;

				// read Red, Green and Blue values (will be in that order).
				if (!ReadKeyword(keyword, &parameter, &paramUsed))
					return UT_FALSE;
				if (strcmp((char*)keyword, "red") == 0  &&  paramUsed)
				{
					red = parameter;

					// Read slash at start of next keyword
					if (!ReadCharFromFile(&ch) ||  ch != '\\')
						tableError = UT_TRUE;
				}
				else
					tableError = UT_TRUE;


				if (!tableError)
				{
					if (!ReadKeyword(keyword, &parameter, &paramUsed))
						return UT_FALSE;
					if (strcmp((char*)keyword, "green") == 0  &&  paramUsed)
					{
						green = parameter;
						if (!ReadCharFromFile(&ch) ||  ch != '\\')
							tableError = UT_TRUE;
					}
					else
						tableError = UT_TRUE;
				}

				if (!tableError)
				{
					if (!ReadKeyword(keyword, &parameter, &paramUsed))
						return UT_FALSE;
					if (strcmp((char*)keyword, "blue") == 0  &&  paramUsed)
					{
						blue = parameter;
						if (!ReadCharFromFile(&ch) ||  ch != ';')
							tableError = UT_TRUE;
					}
					else
						tableError = UT_TRUE;
				}

				colour = (unsigned char)red << 16 | (unsigned char)green << 8 | (unsigned char)blue;
			}
			else
				tableError = UT_TRUE;
		}

		if (tableError)
		{
			return UT_FALSE;
		}
		else
		{
			m_colourTable.addItem((void*)colour);

			// Read in the next char
			if (!ReadCharFromFile(&ch))
				return UT_FALSE;
		}
	}

	// Put the '}' back into the input stream
	return SkipBackChar(ch);
}



//////////////////////////////////////////////////////////////////////////////
// Character Properties keyword handlers
//////////////////////////////////////////////////////////////////////////////

UT_Bool IE_Imp_RTF::HandleBoolCharacterProp(UT_Bool state, UT_Bool* pProp)
{
	UT_Bool ok = FlushStoredChars();
	*pProp = state;
	return ok;
}

UT_Bool IE_Imp_RTF::HandleDeleted(UT_Bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_deleted);
}

UT_Bool IE_Imp_RTF::HandleBold(UT_Bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_bold);
}

UT_Bool IE_Imp_RTF::HandleItalic(UT_Bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_italic);
}

UT_Bool IE_Imp_RTF::HandleUnderline(UT_Bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_underline);
}

UT_Bool IE_Imp_RTF::HandleOverline(UT_Bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_overline);
}

UT_Bool IE_Imp_RTF::HandleStrikeout(UT_Bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_strikeout);
}

UT_Bool IE_Imp_RTF::HandleSuperscript(UT_Bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_superscript);
}

UT_Bool IE_Imp_RTF::HandleSubscript(UT_Bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_subscript);
}

UT_Bool IE_Imp_RTF::HandleFontSize(long sizeInHalfPoints)
{
	UT_Bool ok = FlushStoredChars();

	m_currentRTFState.m_charProps.m_fontSize = sizeInHalfPoints*0.5;

	return ok;
}


UT_Bool IE_Imp_RTF::HandleU32CharacterProp(UT_uint32 val, UT_uint32* pProp)
{
	UT_Bool ok = FlushStoredChars();
	*pProp = val;
	return ok;
}

UT_Bool IE_Imp_RTF::HandleFace(UT_uint32 fontNumber)
{
	return HandleU32CharacterProp(fontNumber, &m_currentRTFState.m_charProps.m_fontNumber);
}

UT_Bool IE_Imp_RTF::HandleColour(UT_uint32 colourNumber)
{
	return HandleU32CharacterProp(colourNumber, &m_currentRTFState.m_charProps.m_colourNumber);
}




UT_Bool IE_Imp_RTF::SetParaJustification(RTFProps_ParaProps::ParaJustification just)
{
	m_currentRTFState.m_paraProps.m_justification = just;

	return UT_TRUE;
}

UT_Bool IE_Imp_RTF::AddTabstop(UT_sint32 stopDist)
{
	m_currentRTFState.m_paraProps.m_tabStops.addItem((void*)stopDist);	// convert from twip to inch
	m_currentRTFState.m_paraProps.m_tabTypes.addItem((void*)RTFProps_ParaProps::ttLeft);

	return UT_TRUE;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Imp_RTF::pasteFromBuffer(PD_DocumentRange * pDocRange,
								 unsigned char * pData, UT_uint32 lenData)
{
	UT_ASSERT(m_pDocument == pDocRange->m_pDoc);
	UT_ASSERT(pDocRange->m_pos1 == pDocRange->m_pos2);

	m_newParaFlagged = UT_FALSE;
	m_newSectionFlagged = UT_FALSE;

	UT_DEBUGMSG(("Pasting %d bytes of RTF\n",lenData));

	m_pPasteBuffer = pData;
	m_lenPasteBuffer = lenData;
	m_pCurrentCharInPasteBuffer = pData;
	m_dposPaste = pDocRange->m_pos1;
	
	// to do a paste, we set the fp to null and let the
	// read-a-char routines know about our paste buffer.
	
	UT_ASSERT(m_pImportFile==NULL);

	// note, we skip the _writeHeader() call since we don't
	// want to assume that selection starts with a section
	// break.
	_parseFile(NULL);
	
	m_pPasteBuffer = NULL;
	m_lenPasteBuffer = 0;
	m_pCurrentCharInPasteBuffer = NULL;

	return;
}

