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
#include <math.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ie_types.h"
#include "ie_imp_RTF.h"
#include "pd_Document.h"
#include "wv.h"
#include "xap_EncodingManager.h"

class fl_AutoNum;


//////////////////////////////////////////////////////////////////////
// Two Useful List arrays
/////////////////////////////////////////////////////////////////////
//
// SEVIOR: FIXME these definitions are included here as well as in
// src/text/fmt/xp/fl_BlockLayout.cpp
//
//
// We need to find a way to include these definitions in 
// src/text/fmt/xp/fl_AutoLists.h without raising a whole
// see of "unused variable" warnings.
//
// C/C++ gods please advise
//

static const XML_Char * xml_Lists[] = { XML_NUMBERED_LIST, 
			   XML_LOWERCASE_LIST, 
			   XML_UPPERCASE_LIST, 
			   XML_UPPERROMAN_LIST,
			   XML_LOWERROMAN_LIST,
			   XML_BULLETED_LIST,
			   XML_DASHED_LIST,
			   XML_SQUARE_LIST,
			   XML_TRIANGLE_LIST,
			   XML_DIAMOND_LIST,
			   XML_STAR_LIST,
			   XML_IMPLIES_LIST,
			   XML_TICK_LIST,
			   XML_BOX_LIST,
			   XML_HAND_LIST,
			   XML_HEART_LIST };
#if 0
// these aren't used currently but might be useful
static const char     * fmt_Lists[] = { fmt_NUMBERED_LIST, 
			   fmt_LOWERCASE_LIST,
			   fmt_UPPERCASE_LIST,
			   fmt_UPPERROMAN_LIST,
			   fmt_LOWERROMAN_LIST,
			   fmt_BULLETED_LIST,
			   fmt_DASHED_LIST };
#endif

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// End List definitions
//////////////////////////////////////////////////////////////////////////



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
	m_deleted = false;
	m_bold = false;
	m_italic = false;
	m_underline = false;
	m_overline = false;
	m_strikeout = false;
	m_superscript = false;
	m_subscript = false;
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
	m_lineSpaceExact = false;
	m_lineSpaceVal = 240;
	m_isList = false;
	m_level = 0;
	memset(&m_pszStyle, 0, sizeof(m_pszStyle)); 
	m_rawID = 0;
	m_rawParentID = 0;
	memset(m_pszListDecimal,0,sizeof(m_pszListDecimal)) ;
	memset(m_pszListDelim,0,sizeof(m_pszListDelim)) ;
	memset(m_pszFieldFont,0,sizeof(m_pszFieldFont)) ;
	m_startValue = 0;
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
		m_isList = other.m_isList;
		m_level = other.m_level;
 	        strcpy((char *) m_pszStyle, (char *) other.m_pszStyle); 
	        m_rawID = other.m_rawID;
		m_rawParentID = other.m_rawParentID;
 	        strcpy((char *) m_pszListDecimal, (char *) other.m_pszListDecimal); 
 	        strcpy((char *) m_pszListDelim, (char *) other.m_pszListDelim); 
 	        strcpy((char *) m_pszFieldFont, (char *) other.m_pszFieldFont); 
		m_startValue = other.m_startValue;
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
	m_newParaFlagged(false),
	m_newSectionFlagged(false),
	m_cbBin(0),
	m_pImportFile(NULL),
	deflangid(0)
{
	m_pPasteBuffer = NULL;
	m_lenPasteBuffer = 0;
	m_pCurrentCharInPasteBuffer = NULL;
	m_numLists = 0;
	memset(m_rtfAbiListTable,0,sizeof( m_rtfAbiListTable));
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
	m_newParaFlagged = true;
	m_newSectionFlagged = true;

	FILE *fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}
	
	UT_Error error = _writeHeader(fp);
	if (!error)
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

	bool ok = true;
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
						ok = ParseChar(b,0);
						cNibble = 2;
						b = 0;
						m_currentRTFState.m_internalState = RTFStateStore::risNorm;
                        // actually don't handle the following space since
                        // this is NOT a delimiter
						// see bug #886
					}
				}
			}
		}
	}

	if (ok)
	{
		ok = FlushStoredChars(true);
	}
	return ok ? UT_OK : UT_ERROR;
}

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_RTF::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	if ( iNumbytes < 5 )
	{
		return(false);
	}
	if ( strncmp( szBuf, "{\\rtf", 5 ) == 0 )
	{
		return(true) ;
	}
	return(false);
}

bool IE_Imp_RTF::RecognizeSuffix(const char * szSuffix)
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

bool	IE_Imp_RTF::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "Rich Text Format (.rtf)";
	*pszSuffixList = "*.rtf";
	*ft = IEFT_RTF;
	return true;
}

bool IE_Imp_RTF::SupportsFileType(IEFileType ft)
{
	return (IEFT_RTF == ft);
}


// flush any remaining text in the previous para and flag
// a new para to be started.  Don't actually start a new
// para as it may turn out to be empty
// 
bool IE_Imp_RTF::StartNewPara()
{
	bool ok = FlushStoredChars(true);
	
	m_newParaFlagged = true;

	return ok;
}


// flush any remaining text in the previous sction and
// flag a new section to be started.  Don't actually 
// start a new section as it may turn out to be empty
// 
bool IE_Imp_RTF::StartNewSection()
{
	bool ok = FlushStoredChars(true);
	
	m_newParaFlagged = true;
	m_newSectionFlagged = true;

	return ok;
}


// add a new character.  Characters are actually cached and 
// inserted into the document in batches - see FlushStoredChars
// 
bool IE_Imp_RTF::AddChar(UT_UCSChar ch)
{
	return m_gbBlock.ins(m_gbBlock.getLength(), &ch, 1);
}


// flush any stored text into the document
// 
bool IE_Imp_RTF::FlushStoredChars(bool forceInsertPara)
{
	// start a new para if we have to
	bool ok = true;
        if (m_newSectionFlagged && (forceInsertPara || (m_gbBlock.getLength() > 0)) )
	{
		ok = ApplySectionAttributes();
		m_newSectionFlagged = false;
	}

	if (ok  &&  m_newParaFlagged  &&  (forceInsertPara  ||  (m_gbBlock.getLength() > 0)) )
	{
		ok = ApplyParagraphAttributes();
		m_newParaFlagged = false;
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
bool IE_Imp_RTF::PushRTFState(void)
{
	// Create a new object to store the state in
	RTFStateStore* pState = new RTFStateStore;
	if (pState == NULL)
	{
	    return false;
	}
	*pState = m_currentRTFState;
	m_stateStack.push(pState);

	// Reset the current state
	m_currentRTFState.m_internalState = RTFStateStore::risNorm;

	return true;
}



// Pops the state off the top of the RTF stack and set the current state to it.
//
bool IE_Imp_RTF::PopRTFState(void)
{
	RTFStateStore* pState = NULL;
	m_stateStack.pop((void**)&pState);

	if (pState != NULL)
	{
		bool ok = FlushStoredChars();
		m_currentRTFState = *pState;
		delete pState;

		m_currentRTFState.m_unicodeInAlternate = 0;
		
		return ok;
	}
	else
	{
		UT_ASSERT(pState != NULL);	// state stack should not be empty
		return false;
	}
}


// Process a single character from the RTF stream
//
bool IE_Imp_RTF::ParseChar(UT_UCSChar ch,bool no_convert)
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
			return true;
		case RTFStateStore::rdsNorm:
			if (m_currentRTFState.m_unicodeInAlternate > 0)
			{
				m_currentRTFState.m_unicodeInAlternate--;
				return true;
			}
			// Insert a character into the story
            if ((ch >= 32  ||  ch == 9 || ch == UCS_FF || ch == UCS_LF)  &&  !m_currentRTFState.m_charProps.m_deleted)
			{
				if (no_convert==0 && ch<=0xff) 
				{	
					wchar_t wc;
					if (m_mbtowc.mbtowc(wc,(UT_Byte)ch))
						return AddChar(wc);
				} else
					return AddChar(ch);
			}
		default:
			// handle other destinations....
			return true;
	}
}


// Reads and proccesses a RTF control word and its parameter
//
bool IE_Imp_RTF::ParseRTFKeyword()
{
	unsigned char keyword[256];
	long parameter = 0;
	bool parameterUsed = false;

	if (ReadKeyword(keyword, &parameter, &parameterUsed))
		return TranslateKeyword(keyword, parameter, parameterUsed);
	else
		return false;
}


bool IE_Imp_RTF::ReadKeyword(unsigned char* pKeyword, long* pParam, bool* pParamUsed)
{
	bool fNegative = false;
	*pParam = 0;
	*pParamUsed = false;
	*pKeyword = 0;
	const unsigned int max_param = 256;
	unsigned char parameter[max_param];
	unsigned int count = 0;

	// Read the first character of the control word
	unsigned char ch;
	if (!ReadCharFromFileWithCRLF(&ch))
		return false;

	// If it's a control symbol there is no delimiter, its just one character
	if (!isalpha(ch))
	{
		pKeyword[0] = ch;
		pKeyword[1] = 0;
		return true;
	}

	// Read in the rest of the control word
	while (isalpha(ch))
	{
		*pKeyword = ch;
		pKeyword++;
		if (!ReadCharFromFileWithCRLF(&ch))
			return false;
	}
	*pKeyword = 0;

    // If the delimeter was '-' then the following parameter is negative
    if (ch == '-')
    {
        fNegative = true;
		if (!ReadCharFromFileWithCRLF(&ch))
			return false;
    }

    // Read the numeric parameter (if there is one)
	if (isdigit(ch))
	{
		*pParamUsed = true;
		while (isdigit(ch))
		{
			// Avoid buffer overflow
			if (count == max_param )
			{
				UT_DEBUGMSG(("Parameter too large. Bogus RTF!\n"));
				return false;
			}

			parameter[count++] = ch;
			if (!ReadCharFromFileWithCRLF(&ch))
				return false;
		}
		parameter[count] = 0;

		*pParam = atol((char*)parameter);
		if (fNegative)
			*pParam = -*pParam;
	}

	// If the delimeter was non-whitespace then this character is part of the following text!
	if ((ch != ' ') && (ch != 10) && (ch != 13))
	{
		SkipBackChar(ch);
	}

	return true;
}


// Reads a character from the file. Doesn't ignore CR and LF
bool IE_Imp_RTF::ReadCharFromFileWithCRLF(unsigned char* pCh)
{
	
	bool ok = false;

	if (m_pImportFile)					// if we are reading a file
	{
		if (fread(pCh, 1, sizeof(unsigned char), m_pImportFile) > 0)
		{
			ok = true;
		}		
	}
	else								// else we are pasting from a buffer
	{
		if (m_pCurrentCharInPasteBuffer < m_pPasteBuffer+m_lenPasteBuffer)
		{
			*pCh = *m_pCurrentCharInPasteBuffer++;
			ok = true;
		}
	}

	return ok;
}

// Reads a character from the file ignoring CR and LF
bool IE_Imp_RTF::ReadCharFromFile(unsigned char* pCh)
{
	// line feed and cr should be ignored in RTF files
	do
	{
		if (ReadCharFromFileWithCRLF(pCh) == false) {
			return false;
		}
	} while (*pCh == 10  ||  *pCh == 13);
	
	return true;

}


bool IE_Imp_RTF::SkipBackChar(unsigned char ch)
{
	if (m_pImportFile)					// if we are reading a file
	{
		// TODO - I've got a sneaking suspicion that this doesn't work on the Macintosh
		return (ungetc(ch, m_pImportFile) != EOF);
	}
	else								// else we are pasting from a buffer
	{
		bool bStatus = (m_pCurrentCharInPasteBuffer > m_pPasteBuffer);
		if (bStatus)
			m_pCurrentCharInPasteBuffer--;
		return bStatus;
	}
}


// Test the keyword against all the known handlers
bool IE_Imp_RTF::TranslateKeyword(unsigned char* pKeyword, long param, bool fParam)
{
	// switch on the first char to reduce the number of string comparisons
	// NB. all RTF keywords are lowercase.
	switch (*pKeyword)
	{
	case 'a':
		if (strcmp((char*)pKeyword, "ansicpg") == 0)
		{
			m_mbtowc.setInCharset(XAP_EncodingManager::instance->charsetFromCodepage((UT_uint32)param));
		}
		break;		
	case 'b':
		if (strcmp((char*)pKeyword, "b") == 0)
		{
			// bold - either on or off depending on the parameter
			return HandleBold(fParam ? false : true);
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
			return HandleDeleted(fParam ? false : true);
		}
		break;
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
			return HandleItalic(fParam ? false : true);
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
		else if (strcmp((char*)pKeyword, "line") == 0)
		{
			return ParseChar(UCS_LF);
		}	
		break;
	case 'o': 
	        if (strcmp((char*)pKeyword,"ol") == 0)
	        {
		         return HandleOverline(fParam ? (param != 0) : true);
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
	        else if (strcmp((char*)pKeyword, "page") == 0)
       		{
           	 return ParseChar(UCS_FF);
        	}
	        else if (strcmp((char*)pKeyword, "pntext") == 0 && m_numLists > 0 )
       		{
		  //
		  // skip this!
		  //
			m_currentRTFState.m_destinationState = RTFStateStore::rdsSkip;
			break;
		}
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
			return HandleStrikeout(fParam ? (param != 0) : true);
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
			return HandleSuperscript(fParam ? false : true);
		}	
		else if (strcmp((char*)pKeyword, "sub") == 0)
		{
			return HandleSubscript(fParam ? false : true);
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
			return HandleUnderline(fParam ? (param != 0) : true);
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
			bool bResult = ParseChar((UT_UCSChar)param);
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
		  bool parameterUsed_star = false;
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
					      (parameter_star != 0): true);
                               }
      		               else if( strcmp((char*)keyword_star,"pn") == 0)
			       { 
			            return HandleLists();
                               }
      		               else if( strcmp((char*)keyword_star,"abilist") == 0)
			       { 
			            return HandleAbiLists();
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
		// TODO handle optional hyphen. Currently simply ignore them.
		break;
	case '_':
		// currently simply make a standard hyphen
		ParseChar('-');	// TODO - make these optional and nonbreaking
		break;
	}

	return true;
}


bool IE_Imp_RTF::ApplyCharacterAttributes()
{
	XML_Char* pProps = "props";
	XML_Char propBuffer[1024];	//TODO is this big enough?  better to make it a member and stop running all over the stack
	XML_Char tempBuffer[130];

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
	sprintf(tempBuffer, "; font-size:%spt", std_size_string((float)m_currentRTFState.m_charProps.m_fontSize));	
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

	bool ok;
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


bool IE_Imp_RTF::ResetCharacterAttributes()
{
	bool ok = FlushStoredChars();

	m_currentRTFState.m_charProps = RTFProps_CharProps();

	return ok;
}

  ///
  /// OK if we are pasting into the text we have to decide if the list we paste
  /// should be a new list or an old list. The user might want to swap paragraphs
  /// in a list for example.
  ///
  /// Use the following algorithim to decide. If the docpos of the paste is 
  /// within a list of the same ID as our list or if the docpos is immediately
  /// before or after a list of the same ID reuse the ID. Otherwise change it.
  ///
UT_uint32 IE_Imp_RTF::mapID(UT_uint32 id)
{
        UT_uint32 mappedID = id;
	if (m_pImportFile)  // if we are reading a file - dont remap the ID
	{
	        return id;
	}
	///
	/// Now look to see if the ID has been remapped.
	///
	UT_uint32 i,j;
	for(i=0; i<m_numLists; i++)
	{
	        if(m_rtfAbiListTable[i].orig_id == id)
		{ 
		          if(m_rtfAbiListTable[i].hasBeenMapped == true )
			  {
			           mappedID =  m_rtfAbiListTable[i].mapped_id;
			  }
			  else
			    ///
			    /// Do the remapping!
			    ///
			  {
			           fl_AutoNum * pMapAuto = NULL;
				   UT_uint32 nLists = m_pDocument->getListsCount();
				   UT_uint32 highestLevel = 0;
				   PL_StruxDocHandle sdh;
				   m_pDocument->getStruxOfTypeFromPosition(m_dposPaste, PTX_Block,&sdh);
				   for(j=0; j< nLists; j++)
				   {
				           fl_AutoNum * pAuto = m_pDocument->getNthList(j);
					   if(pAuto->isContainedByList(sdh) == true)
					   {
					           if(highestLevel < pAuto->getLevel())
						   {
						           highestLevel = pAuto->getLevel();
							   pMapAuto = pAuto;
						   }
					   }
				   }
				   UT_DEBUGMSG(("SEVIOR: m_rtfAbiListTable[i].orig_parentid = \n",m_rtfAbiListTable[i].orig_parentid));
				   if(pMapAuto == NULL )
				          mappedID = rand();
				   else if( m_rtfAbiListTable[i].level <= pMapAuto->getLevel() && pMapAuto->getID() != 0)
				          mappedID = pMapAuto->getID();
				   else
				          mappedID = rand();
				   m_rtfAbiListTable[i].hasBeenMapped = true;
				   m_rtfAbiListTable[i].mapped_id = mappedID;	  
				   if(highestLevel > 0)
				   {
				          m_rtfAbiListTable[i].mapped_parentid =  m_rtfAbiListTable[i].orig_parentid;
				   }				   
				   else
				   {
				          m_rtfAbiListTable[i].mapped_parentid = 0;
					  m_rtfAbiListTable[i].orig_parentid = 0;
					  m_rtfAbiListTable[i].level = 1;
				   }

				   ///
				   /// Now look to see if the parent ID has been remapped, if so update mapped_parentid
				   ///
				   for(j = 0;  j<m_numLists; j++)
				   {
				         if(m_rtfAbiListTable[j].orig_id == m_rtfAbiListTable[i].orig_parentid)
				         {
						  m_rtfAbiListTable[i].mapped_parentid = m_rtfAbiListTable[j].mapped_id;
					 }
				   }
			  }
		}
	}
	return mappedID;

}

UT_uint32 IE_Imp_RTF::mapParentID(UT_uint32 id)
{
  //
  // OK if we are pasting into the text we have to decide if the list we paste
  // should be a new list or an old list. The user might want to swap paragraphs
  // for example.
  //
  // For the parent ID we have to look to see if the parent ID has been remapped
  //
        UT_uint32 mappedID;
	mappedID = id;
	if (m_pImportFile)  // if we are reading a file
	{
	        return id;
	}
	UT_uint32 i;
	UT_DEBUGMSG(("SEVIOR: Looking for id %d \n",id));
	for(i=0; (i<m_numLists) && (m_rtfAbiListTable[i].orig_id == id); i++)
	{
	}
	if( i < m_numLists && m_rtfAbiListTable[i].orig_id == id)
	{
	    mappedID =  m_rtfAbiListTable[i].mapped_id;
	    UT_DEBUGMSG(("SEVIOR: Found it! mapping id %d to %d \n",id,mappedID));
	}
	return mappedID;
}

bool IE_Imp_RTF::ApplyParagraphAttributes()
{
	XML_Char* pProps = "props";
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

			sprintf(tempBuffer, "%s/L", UT_convertInchesToDimensionString(DIM_IN,tabIn,"04"));	// TODO - left tabs only
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
			UT_ASSERT(false);	// so what is it?
		case RTFProps_ParaProps::pjLeft:
			strcat(propBuffer, "left");
			break;
	}
	strcat(propBuffer, "; ");

	// indents - first, left and right, top and bottom
	sprintf(tempBuffer, "margin-top:%s; ",		UT_convertInchesToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_spaceBefore/1440));
	strcat(propBuffer, tempBuffer);
	sprintf(tempBuffer, "margin-bottom:%s; ",	UT_convertInchesToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_spaceAfter/1440));
	strcat(propBuffer, tempBuffer);
	sprintf(tempBuffer, "margin-left:%s; ",		UT_convertInchesToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_indentLeft/1440));
	strcat(propBuffer, tempBuffer);
	sprintf(tempBuffer, "margin-right:%s; ",	UT_convertInchesToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_indentRight/1440));
	strcat(propBuffer, tempBuffer);
	sprintf(tempBuffer, "text-indent:%s; ",		UT_convertInchesToDimensionString(DIM_IN, (double)m_currentRTFState.m_paraProps.m_indentFirst/1440));
	strcat(propBuffer, tempBuffer);
	
	// line spacing
	if(m_currentRTFState.m_paraProps.m_isList == true)
	{
	        if (m_currentRTFState.m_paraProps.m_lineSpaceExact)
                {
		// ABIWord doesn't (yet) support exact line spacing we'll just fall back to single
		       sprintf(tempBuffer, "line-height:1.0;");
		}
		else
	        {
		       sprintf(tempBuffer, "line-height:%s;",	UT_convertToDimensionlessString(fabs(m_currentRTFState.m_paraProps.m_lineSpaceVal/240)));
		}

	}
	else
        {
	        if (m_currentRTFState.m_paraProps.m_lineSpaceExact)
                {
		// ABIWord doesn't (yet) support exact line spacing we'll just fall back to single
		       sprintf(tempBuffer, "line-height:1.0");
		}
		else
	        {
		       sprintf(tempBuffer, "line-height:%s",	UT_convertToDimensionlessString(fabs(m_currentRTFState.m_paraProps.m_lineSpaceVal/240)));
		}

	}
	strcat(propBuffer, tempBuffer);

	// Lists. If the paragraph has a list element handle it.
	const XML_Char* propsArray[3];
	const XML_Char** attribs = NULL;
	UT_Vector v;
	static char pszLevel[8];
	static char pszStyle[20];
	static char pszListID[15];
	static char pszParentID[15];
	static char pszStartValue[15];
	UT_uint32 id,pid,startValue;
	UT_uint32 attribsCount;
	if(m_currentRTFState.m_paraProps.m_isList == true)
	{
	  //
	  // First off assemble the list attributes
	  //
	        sprintf(pszStyle,"%s",m_currentRTFState.m_paraProps.m_pszStyle);
	        v.addItem((void *) "style"); v.addItem( (void *) pszStyle);
	        id = mapID(m_currentRTFState.m_paraProps.m_rawID);
	        sprintf(pszListID,"%d",id);
	        v.addItem((void *) "listid"); v.addItem( (void *) pszListID);
	        pid = mapParentID(m_currentRTFState.m_paraProps.m_rawParentID);
	        sprintf(pszParentID,"%d",pid);
	        v.addItem((void *) "parentid"); v.addItem( (void *) pszParentID);
	        if(pid == 0)
		       m_currentRTFState.m_paraProps.m_level = 1;
	        sprintf(pszLevel,"%d",m_currentRTFState.m_paraProps.m_level);
	        v.addItem((void *) "level"); v.addItem( (void *) pszLevel);

	        UT_uint32 counta = v.getItemCount() + 3;
	        attribs = (const XML_Char **) calloc(counta, sizeof(XML_Char *));
	        for(attribsCount=0; attribsCount<v.getItemCount();attribsCount++)
	        {
		       attribs[attribsCount] = (XML_Char *) v.getNthItem(attribsCount);
	        }
	        //
	        // Now handle the List properties
	        //
	        sprintf(tempBuffer, "list-decimal:%s; ",m_currentRTFState.m_paraProps.m_pszListDecimal);
	        strcat(propBuffer, tempBuffer);
	        sprintf(tempBuffer, "list-delim:%s; ",m_currentRTFState.m_paraProps.m_pszListDelim);
	        strcat(propBuffer, tempBuffer);
	        sprintf(tempBuffer, "field-font:%s; ",m_currentRTFState.m_paraProps.m_pszFieldFont);
	        strcat(propBuffer, tempBuffer);
		startValue = m_currentRTFState.m_paraProps.m_startValue;
		sprintf(pszStartValue,"%d",startValue);
	        sprintf(tempBuffer, "start-value:%s ",pszStartValue);
	        strcat(propBuffer, tempBuffer);

	        attribs[attribsCount++] = pProps;
	        attribs[attribsCount++] = propBuffer;
	        attribs[attribsCount] = NULL;
	}
	propsArray[0] = pProps;
	propsArray[1] = propBuffer;
	propsArray[2] = NULL;

	if (m_pImportFile)					// if we are reading a file
	{
	        if(m_currentRTFState.m_paraProps.m_isList == true)
		{
		        bool bret = m_pDocument->appendStrux(PTX_Block, attribs);
			//
			// Insert a list-label field??
			//
			FREEP(attribs);
			const XML_Char* fielddef[3];
			fielddef[0] ="type";
			fielddef[1] = "list_label";
                        fielddef[2] = NULL;
			bret =   m_pDocument->appendObject(PTO_Field,fielddef);
			return bret;
		}
		else
		{
		        return m_pDocument->appendStrux(PTX_Block, propsArray);
		}
	}
	else
	{
	        bool bSuccess = true;
		if (bSuccess)
		{
	                if(m_currentRTFState.m_paraProps.m_isList == true)
			{
				UT_DEBUGMSG(("SEVIOR: Pasting list id %d \n",id));
				bool bSuccess = m_pDocument->insertStrux(m_dposPaste,PTX_Block);
				m_dposPaste++;
				PL_StruxDocHandle sdh_cur,sdh_next;
				PT_DocPosition pos_next;
				m_pDocument->getStruxOfTypeFromPosition(m_dposPaste, PTX_Block, & sdh_cur);
				UT_uint32 j;
				fl_AutoNum * pAuto = m_pDocument->getListByID(id);
				if(pAuto == NULL) 
				/*
				 * Got to create a new list here. 
				 * Old one may have been cut out or ID may have
				 * been remapped.
				 */
				{
				  UT_DEBUGMSG(("SEVIOR: Creating a new list \n"));
				       List_Type lType = NOT_A_LIST;
				       UT_uint32 size_xml_lists = sizeof(xml_Lists)/sizeof(xml_Lists[0]);
				       for(j=0; j< size_xml_lists; j++)
				       {
				                if( UT_XML_strcmp(pszStyle,xml_Lists[j]) ==0)
						{
						        break;
						}
				       }
				       if(j < size_xml_lists)
				             lType = (List_Type) j;
				       else
				             lType = (List_Type) 0;
				       pAuto = new fl_AutoNum(id, pid, lType, startValue,(XML_Char *)  m_currentRTFState.m_paraProps.m_pszListDelim,(XML_Char *)  m_currentRTFState.m_paraProps.m_pszListDecimal, m_pDocument);
				       UT_DEBUGMSG(("SEVIOR: Created new list in Paste \n"));
				       m_pDocument->addList(pAuto);
				       pAuto->fixHierarchy(m_pDocument);
				}
				bSuccess = m_pDocument->getStruxOfTypeFromPosition(m_dposPaste,PTX_Block,&sdh_cur);
				///
				/// Now insert this into the pAuto List
				///
				if(pAuto->isEmpty() == true)
				{
				        pAuto->addItem(sdh_cur);
				}
				else
				{
				        j= 0;
					sdh_next = pAuto->getNthBlock(j);
					pos_next = m_pDocument->getStruxPosition(sdh_next);
					while(sdh_next != NULL && pos_next < m_dposPaste)
					{
					        j++;
						sdh_next = pAuto->getNthBlock(j);
						if(sdh_next != NULL)
						       pos_next = m_pDocument->getStruxPosition(sdh_next);
					}
					if(sdh_next != NULL)
					{
					         pAuto->prependItem(sdh_cur,sdh_next);
					}
					else
					{
					         pAuto->addItem(sdh_cur);
					}
				}
				if(pid != 0)
				{
				        pAuto->findAndSetParentItem();
					pAuto->markAsDirty();
					pid = pAuto->getParentID();
					sprintf(pszParentID,"%d",pid);
				}
			        bSuccess = m_pDocument->changeStruxFmt(PTC_AddFmt,m_dposPaste,m_dposPaste,attribs, NULL,PTX_Block);
  				FREEP(attribs);
			}
			else
			{
				bSuccess = m_pDocument->insertStrux(m_dposPaste,PTX_Block);
				m_dposPaste++;
			        bSuccess = m_pDocument->changeStruxFmt(PTC_AddFmt,m_dposPaste,m_dposPaste, propsArray,NULL,PTX_Block);
			}
		}
		return bSuccess;
	}
}


bool IE_Imp_RTF::ResetParagraphAttributes()
{
	bool ok = FlushStoredChars();

	UT_DEBUGMSG(("SEVIOR: clearing all paragraph properties \n"));
	m_currentRTFState.m_paraProps = RTFProps_ParaProps();

	return ok;
}


bool IE_Imp_RTF::ResetSectionAttributes()
{
	bool ok = FlushStoredChars();

	m_currentRTFState.m_sectionProps = RTFProps_SectionProps();

	return ok;
}


bool IE_Imp_RTF::ApplySectionAttributes()
{
	XML_Char* pProps = "props";
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
		bool bSuccess = m_pDocument->insertStrux(m_dposPaste,PTX_Section);
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
bool IE_Imp_RTF::ReadFontTable()
{
	// Ensure the font table is empty before we start
	UT_ASSERT(m_fontTable.getItemCount() == 0);
	UT_VECTOR_PURGEALL(RTFFontTableItem*, m_fontTable);	

	unsigned char ch;
	if (!ReadCharFromFile(&ch))
		return false;

	if (ch == '\\')
	{
		// one entry in the font table
		// TODO - Test one item font tables!
		if (!ReadOneFontFromTable())
			return false;
	}
	else
	{
		if (ch != '{')
			return false;

		// multiple entries in font table
		while (ch != '}')
		{
			if (ch == '{')
			{
				if (!ReadCharFromFile(&ch))
					return false;
			}
			
			if (!ReadOneFontFromTable())
				return false;

			// now eat whitespace until we hit either '}' (end of font group) or '{' (another font item)
			do
			{
				if (!ReadCharFromFile(&ch))
					return false;
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
bool IE_Imp_RTF::ReadOneFontFromTable()
{
	unsigned char keyword[1024];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;

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
		return false;
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
		return false;
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
		return false;
	int nesting = 0;
	while (ch == '\\'  ||  ch == '{')
	{
		if (ch == '{')
		{
			++nesting;
			if (!ReadCharFromFile(&ch))
				return false;
		}

		if (!ReadKeyword(keyword, &parameter, &paramUsed))
			return false;

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
				return false;

			if (isdigit(ch))
			{
				 SkipBackChar(ch);
			}

			for (int i = 0; i < 10; i++)
			{
				unsigned char buf[3];

				if ( !ReadCharFromFile(&(buf[0]))  ||  !ReadCharFromFile(&(buf[1])) )
				{
					return false;
				}
				unsigned char val = (unsigned char)(atoi((char*)buf));
				panose[i] = val;
			}
		}

		//TODO - handle the other keywords

		if (!ReadCharFromFile(&ch))
			return false;

		if (ch == '}' && (nesting-- > 0))
		{
			if (!ReadCharFromFile(&ch))
				return false;
		}
	}
	//we fall back here when space between parameter of keyword and font name
	//is seen
	// Now comes the font name, terminated by either a close brace or a slash or a semi-colon
	int count = 0;
	/*
	    FIXME: CJK font names come in form \'aa\'cd\'ef - so we have to 
	    parse \'HH correctly (currently we ignore them!) - VH
	*/
	while ( ch != '}'  &&  ch != '\\'  &&  ch != ';' && ch!= '{')
	{
		keyword[count++] = ch;
		if (!ReadCharFromFile(&ch))
			return false;
	}
	if (ch=='{')
		++nesting;
		
	keyword[count] = 0;
	/*work around "helvetica" font name -replace it with "Helvetic"*/
	if (!UT_stricmp((const char*)keyword,"helvetica"))
		strcpy((char*)keyword,"Helvetic");

	if (!UT_cloneString(pFontName, (char*)keyword))
	{
		// TODO outofmem
	}			
	for(int i=0;i<=nesting;++i)
	{
		// Munch the remaining control words down to the close brace
		while (ch != '}')
		{
			if (!ReadCharFromFile(&ch))
				return false;
			if (ch=='{')
				++nesting;
		}
		if (nesting>0 && i!=nesting) //we need to skip '}' we've just seen.
			if (!ReadCharFromFile(&ch))
				return false;		
	}

	// Create the font entry and put it into the font table
	RTFFontTableItem* pNewFont = new RTFFontTableItem(fontFamily, charSet, codepage, pitch,
						panose, pFontName, pAlternativeFontName);
	if (pNewFont == NULL)
		return false;

	// ensure that the font table is large enough for this index
	while (m_fontTable.getItemCount() <= fontIndex)
	{
		m_fontTable.addItem(NULL);
	}
	void* pOld = NULL;
	UT_sint32 res = m_fontTable.setNthItem(fontIndex, pNewFont, &pOld);
	UT_ASSERT(res == 0);
	UT_ASSERT(pOld == NULL);

	return true;
}




//////////////////////////////////////////////////////////////////////////////
// Colour table reader
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::ReadColourTable()
{
	// Ensure the table is empty before we start
	UT_ASSERT(m_colourTable.getItemCount() == 0);

	unsigned char keyword[1024];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	if (!ReadCharFromFile(&ch))
		return false;

	while (ch != '}')
	{
		UT_uint32 colour = 0;
		bool tableError = false;

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
					return false;
				if (strcmp((char*)keyword, "red") == 0  &&  paramUsed)
				{
					red = parameter;

					// Read slash at start of next keyword
					if (!ReadCharFromFile(&ch) ||  ch != '\\')
						tableError = true;
				}
				else
					tableError = true;


				if (!tableError)
				{
					if (!ReadKeyword(keyword, &parameter, &paramUsed))
						return false;
					if (strcmp((char*)keyword, "green") == 0  &&  paramUsed)
					{
						green = parameter;
						if (!ReadCharFromFile(&ch) ||  ch != '\\')
							tableError = true;
					}
					else
						tableError = true;
				}

				if (!tableError)
				{
					if (!ReadKeyword(keyword, &parameter, &paramUsed))
						return false;
					if (strcmp((char*)keyword, "blue") == 0  &&  paramUsed)
					{
						blue = parameter;
						if (!ReadCharFromFile(&ch) ||  ch != ';')
							tableError = true;
					}
					else
						tableError = true;
				}

				colour = (unsigned char)red << 16 | (unsigned char)green << 8 | (unsigned char)blue;
			}
			else
				tableError = true;
		}

		if (tableError)
		{
			return false;
		}
		else
		{
			m_colourTable.addItem((void*)colour);

			// Read in the next char
			if (!ReadCharFromFile(&ch))
				return false;
		}
	}

	// Put the '}' back into the input stream
	return SkipBackChar(ch);
}




//////////////////////////////////////////////////////////////////////////////
// List table reader
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::HandleLists()
{
	unsigned char keyword[1024];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	if (!ReadCharFromFile(&ch))
		return false;

	while (ch != '}') // Outer loop
	{
	  //SkipBackChar(ch); // Put char back in stream
		if(ch == '{')  // pntxta or pntxtb
		{
			if (!ReadCharFromFile(&ch))
			         return false;
			if(!ReadKeyword(keyword, &parameter, &paramUsed))
			{
		                 return false;
			}
			else
			{
			         if (strcmp((char*)keyword, "pntxta") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
			                 int count = 0;
					 if (!ReadCharFromFile(&ch))
				                 return false;
					 while ( ch != '}'  && ch != ';')
					 {
				                 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
					                  return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_rtfListTable.textafter,(char*)keyword);
					 UT_DEBUGMSG(("FOUND pntxta in stream, copied %s to input  \n",keyword));
				 }
				 else if (strcmp((char*)keyword, "pntxtb") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
			                 int count = 0;
					 if (!ReadCharFromFile(&ch))
				                  return false;
					 while ( ch != '}'  && ch != ';' )
					 {
				                  keyword[count++] = ch;
						  if (!ReadCharFromFile(&ch))
					                  return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_rtfListTable.textbefore,(char*)keyword);
					 UT_DEBUGMSG(("FOUND pntxtb in stream,copied %s to input  \n",keyword));
				 }
				 else
			         {
				         UT_DEBUGMSG(("Unknown keyword %s found in List stream  \n",keyword));
				 }
			}
			goto nextChar;
		}
		if(!ReadKeyword(keyword, &parameter, &paramUsed))
		{
		        return false;
		}
		else
		{
		        if (strcmp((char*)keyword, "levelstartat") == 0)
			{
			         m_rtfListTable.start_value = (UT_uint32) parameter;
				 UT_DEBUGMSG(("FOUND levelstartat in stream \n"));
			}
		        if (strcmp((char*)keyword, "pnstart") == 0)
			{
			         m_rtfListTable.start_value = (UT_uint32) parameter;
				 UT_DEBUGMSG(("FOUND pnstart in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnlvl") == 0)
			{
			         m_rtfListTable.level = (UT_uint32) parameter;
				 UT_DEBUGMSG(("FOUND pnlvl in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnlvlblt") == 0)
			{
			         m_rtfListTable.bullet = true;
				 UT_DEBUGMSG(("FOUND pnlvlblt in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnlvlbody") == 0)
			{
			         m_rtfListTable.simple = true;
				 UT_DEBUGMSG(("FOUND pnlvlbody in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnlvlcont") == 0)
			{
			         m_rtfListTable.continueList = true;
				 UT_DEBUGMSG(("FOUND pnlvlcont in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnnumonce") == 0)
			{
				 UT_DEBUGMSG(("FOUND pnnumonce in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnacross") == 0)
			{
				 UT_DEBUGMSG(("FOUND pnacross in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnhang") == 0)
			{
			         m_rtfListTable.hangingIndent = true;
				 UT_DEBUGMSG(("FOUND pnhang in stream \n"));
			}
			else if (strcmp((char*)keyword, "pncard") == 0)
			{
			         m_rtfListTable.type = NUMBERED_LIST;
				 UT_DEBUGMSG(("FOUND pncard in stream \n"));
			}
			else if (strcmp((char*)keyword, "pndec") == 0)
			{
			         m_rtfListTable.type = NUMBERED_LIST;
				 UT_DEBUGMSG(("FOUND pndec in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnucltr") == 0)
			{
			         m_rtfListTable.type = UPPERCASE_LIST;
				 UT_DEBUGMSG(("FOUND pnucltr in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnuclrm") == 0)
			{
			         m_rtfListTable.type = UPPERROMAN_LIST;
				 UT_DEBUGMSG(("FOUND pnucrm in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnlcltr") == 0)
			{
			         m_rtfListTable.type = LOWERCASE_LIST;
				 UT_DEBUGMSG(("FOUND pnlctr in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnlclrm") == 0)
			{
			         m_rtfListTable.type = LOWERROMAN_LIST;
				 UT_DEBUGMSG(("FOUND pnlcrm in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnord") == 0)
			{
			         m_rtfListTable.type = NUMBERED_LIST;
				 UT_DEBUGMSG(("FOUND pnord in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnordt") == 0)
			{
			         m_rtfListTable.type = NUMBERED_LIST;
				 UT_DEBUGMSG(("FOUND pnordt in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnb") == 0)
			{
			         m_rtfListTable.bold = true;
				 UT_DEBUGMSG(("FOUND pnb in stream \n"));
			}
			else if (strcmp((char*)keyword, "pni") == 0)
			{
			         m_rtfListTable.italic = true;
				 UT_DEBUGMSG(("FOUND pni in stream \n"));
			}
			else if (strcmp((char*)keyword, "pncaps") == 0)
			{
			         m_rtfListTable.caps = true;
				 UT_DEBUGMSG(("FOUND pncaps in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnscaps") == 0)
			{
			         m_rtfListTable.scaps = true;
				 UT_DEBUGMSG(("FOUND pnscaps in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnul") == 0)
			{
			         m_rtfListTable.underline = true;
				 UT_DEBUGMSG(("FOUND pnul in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnuld") == 0)
			{
			         m_rtfListTable.underline = true;
				 UT_DEBUGMSG(("FOUND pnuld in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnuldb") == 0)
			{
			         m_rtfListTable.underline = true;
				 UT_DEBUGMSG(("FOUND pnuldb in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnulnone") == 0)
			{
			         m_rtfListTable.nounderline = true;
				 UT_DEBUGMSG(("FOUND pnulnone in stream \n"));
			}
			else if (strcmp((char*)keyword, "pnulw") == 0)
			{
				 UT_DEBUGMSG(("FOUND pnulw in stream - ignore for now \n"));
			}
			else if (strcmp((char*)keyword, "pnstrike") == 0)
			{
			         m_rtfListTable.strike = true;
				 UT_DEBUGMSG(("FOUND pnstrike in stream  \n"));
			}
			else if (strcmp((char*)keyword, "pncf") == 0)
			{
			         m_rtfListTable.forecolor =  (UT_uint32) parameter;
				 UT_DEBUGMSG(("FOUND pncf in stream  \n"));
			}
			else if (strcmp((char*)keyword, "pnf") == 0)
			{
			         m_rtfListTable.font =  (UT_uint32) parameter;
				 UT_DEBUGMSG(("FOUND pnf in stream  \n"));
			}
			else if (strcmp((char*)keyword, "pnfs") == 0)
			{
			         m_rtfListTable.fontsize =  (UT_uint32) parameter;
				 UT_DEBUGMSG(("FOUND pnfs in stream  \n"));
			}
			else if (strcmp((char*)keyword, "pnindent") == 0)
			{
			         m_rtfListTable.indent =  (UT_uint32) parameter;
				 UT_DEBUGMSG(("FOUND pnindent in stream  \n"));
			}
			else if (strcmp((char*)keyword, "pnsp") == 0)
			{
				 UT_DEBUGMSG(("FOUND pnsp in stream  - ignored for now \n"));
			}
			else if (strcmp((char*)keyword, "pnprev") == 0)
			{
			         m_rtfListTable.prevlist =  true;
				 UT_DEBUGMSG(("FOUND pnprev in stream  \n"));
			}
			else if (strcmp((char*)keyword, "pnqc") == 0)
			{
				 UT_DEBUGMSG(("FOUND pnqc in stream - ignored for now \n"));
				 // centered numbering
			}
			else if (strcmp((char*)keyword, "pnql") == 0)
			{
				 UT_DEBUGMSG(("FOUND pnql in stream - ignored for now \n"));
				 // left justified numbering
			}
			else if (strcmp((char*)keyword, "pnqr") == 0)
			{
				 UT_DEBUGMSG(("FOUND pnqr in stream - ignored for now \n"));
				 // right justified numbering
			}
			else if (strcmp((char*)keyword, "ls") == 0)
			{
				 UT_DEBUGMSG(("FOUND ls in stream - ignored for now \n"));
				 // Word 97 list table identifier
			}
			else if (strcmp((char*)keyword, "ilvl") == 0)
			{
				 UT_DEBUGMSG(("FOUND ilvl in stream - ignored for now \n"));
				 // Word 97 list level
			}
			else if (strcmp((char*)keyword, "pnrnot") == 0)
			{
				 UT_DEBUGMSG(("FOUND pnrnot in stream - ignored for now \n"));
				 // Don't know this
			}
			else
			{
				 UT_DEBUGMSG(("Unknown keyword %s found in List stream  \n",keyword));
			}
         nextChar:	if (!ReadCharFromFile(&ch))
			         return false;
		}
	}
	// Put the '}' back into the input stream
	return SkipBackChar(ch);
}



//////////////////////////////////////////////////////////////////////////////
// AbiList table reader
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::HandleAbiLists()
{
	unsigned char keyword[1024];
	unsigned char ch;
	long parameter = 0;
	bool paramUsed = false;
	if (!ReadCharFromFile(&ch))
		return false;

	while (ch != '}') // Outer loop
	{
		if(ch == '{')  // abiliststyle, abilistdecimal, abilistdelim
		{
			if (!ReadCharFromFile(&ch))
			         return false;
			if(!ReadKeyword(keyword, &parameter, &paramUsed))
			{
		                 return false;
			}
			else
			{
			         if (strcmp((char*)keyword, "abiliststyle") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
			                 int count = 0;
					 if (!ReadCharFromFile(&ch))
				                 return false;
					 while ( ch != '}'  && ch != ';')
					 {
				                 keyword[count++] = ch;
						 if (!ReadCharFromFile(&ch))
					                  return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_currentRTFState.m_paraProps.m_pszStyle,(char*)keyword);
				 }
				 else if (strcmp((char*)keyword, "abilistdecimal") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
			                 int count = 0;
					 if (!ReadCharFromFile(&ch))
				                  return false;
					 while ( ch != '}'  && ch != ';' )
					 {
				                  keyword[count++] = ch;
						  if (!ReadCharFromFile(&ch))
					                  return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_currentRTFState.m_paraProps.m_pszListDecimal,(char*)keyword);
				 }
				 else if (strcmp((char*)keyword, "abilistdelim") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
			                 int count = 0;
					 if (!ReadCharFromFile(&ch))
				                  return false;
					 while ( ch != '}'  && ch != ';' )
					 {
				                  keyword[count++] = ch;
						  if (!ReadCharFromFile(&ch))
					                  return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_currentRTFState.m_paraProps.m_pszListDelim,(char*)keyword);
				 }
				 else if (strcmp((char*)keyword, "abifieldfont") == 0)
				 {
			  // OK scan through the text until a closing delimeter is
			  // found
			                 int count = 0;
					 if (!ReadCharFromFile(&ch))
				                  return false;
					 while ( ch != '}'  && ch != ';' )
					 {
				                  keyword[count++] = ch;
						  if (!ReadCharFromFile(&ch))
					                  return false;
					 }
					 keyword[count++] = 0;
					 strcpy(m_currentRTFState.m_paraProps.m_pszFieldFont,(char*)keyword);
				 }
				 else
			         {
				         UT_DEBUGMSG(("Unknown keyword %s found in List stream  \n",keyword));
				 }
			}
			goto nextChar;
		}
		if(!ReadKeyword(keyword, &parameter, &paramUsed))
		{
		        return false;
		}
		else
		{
		        if (strcmp((char*)keyword, "abistartat") == 0)
			{
			         m_currentRTFState.m_paraProps.m_startValue= (UT_uint32) parameter;
			}
		        else if (strcmp((char*)keyword, "abilistid") == 0)
			{
			         m_currentRTFState.m_paraProps.m_rawID = (UT_uint32) parameter;
			         m_currentRTFState.m_paraProps.m_isList = true;

			}
		        else if (strcmp((char*)keyword, "abilistparentid") == 0)
			{
			         m_currentRTFState.m_paraProps.m_rawParentID = (UT_uint32) parameter;
			}
			else if (strcmp((char*)keyword, "abilistlevel") == 0)
			{
			         m_currentRTFState.m_paraProps.m_level = (UT_uint32) parameter;
			}
			else
			{
				 UT_DEBUGMSG(("Unknown keyword %s found in List stream  \n",keyword));
			}
		}
nextChar:	if (!ReadCharFromFile(&ch))
			         return false;
	}
	//
	// Increment the list mapping table if necessary
	//
	UT_uint32 i;
	if(m_currentRTFState.m_paraProps.m_rawID != 0) 
	{
	  for(i=0; i < m_numLists; i++)
	  {
	         if(m_currentRTFState.m_paraProps.m_rawID == m_rtfAbiListTable[i].orig_id)
		       break;
	  }
	  if(i >= m_numLists)
	  {
	         UT_DEBUGMSG(("SEVIOR: Found new id %d, adding it to mapping table at%d \n",m_currentRTFState.m_paraProps.m_rawID,m_numLists));
		 m_rtfAbiListTable[m_numLists].orig_id = m_currentRTFState.m_paraProps.m_rawID ; 
		 m_rtfAbiListTable[m_numLists].orig_parentid = m_currentRTFState.m_paraProps.m_rawParentID ;
		 m_rtfAbiListTable[m_numLists].level = m_currentRTFState.m_paraProps.m_level ;
		 m_rtfAbiListTable[m_numLists].hasBeenMapped = false;
		 m_numLists++;
	  }
	}

	// Put the '}' back into the input stream

	//return SkipBackChar(ch);
	return true;
}



//////////////////////////////////////////////////////////////////////////////
// Character Properties keyword handlers
//////////////////////////////////////////////////////////////////////////////

bool IE_Imp_RTF::HandleBoolCharacterProp(bool state, bool* pProp)
{
	bool ok = FlushStoredChars();
	*pProp = state;
	return ok;
}

bool IE_Imp_RTF::HandleDeleted(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_deleted);
}

bool IE_Imp_RTF::HandleBold(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_bold);
}

bool IE_Imp_RTF::HandleItalic(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_italic);
}

bool IE_Imp_RTF::HandleUnderline(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_underline);
}

bool IE_Imp_RTF::HandleOverline(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_overline);
}

bool IE_Imp_RTF::HandleStrikeout(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_strikeout);
}

bool IE_Imp_RTF::HandleSuperscript(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_superscript);
}

bool IE_Imp_RTF::HandleSubscript(bool state)
{
	return HandleBoolCharacterProp(state, &m_currentRTFState.m_charProps.m_subscript);
}

bool IE_Imp_RTF::HandleFontSize(long sizeInHalfPoints)
{
	bool ok = FlushStoredChars();

	m_currentRTFState.m_charProps.m_fontSize = sizeInHalfPoints*0.5;

	return ok;
}


bool IE_Imp_RTF::HandleU32CharacterProp(UT_uint32 val, UT_uint32* pProp)
{
	bool ok = FlushStoredChars();
	*pProp = val;
	return ok;
}

bool IE_Imp_RTF::HandleFace(UT_uint32 fontNumber)
{
	return HandleU32CharacterProp(fontNumber, &m_currentRTFState.m_charProps.m_fontNumber);
}

bool IE_Imp_RTF::HandleColour(UT_uint32 colourNumber)
{
	return HandleU32CharacterProp(colourNumber, &m_currentRTFState.m_charProps.m_colourNumber);
}




bool IE_Imp_RTF::SetParaJustification(RTFProps_ParaProps::ParaJustification just)
{
	m_currentRTFState.m_paraProps.m_justification = just;

	return true;
}

bool IE_Imp_RTF::AddTabstop(UT_sint32 stopDist)
{
	m_currentRTFState.m_paraProps.m_tabStops.addItem((void*)stopDist);	// convert from twip to inch
	m_currentRTFState.m_paraProps.m_tabTypes.addItem((void*)RTFProps_ParaProps::ttLeft);

	return true;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Imp_RTF::pasteFromBuffer(PD_DocumentRange * pDocRange,
								 unsigned char * pData, UT_uint32 lenData)
{
	UT_ASSERT(m_pDocument == pDocRange->m_pDoc);
	UT_ASSERT(pDocRange->m_pos1 == pDocRange->m_pos2);

	m_newParaFlagged = false;
	m_newSectionFlagged = false;

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






