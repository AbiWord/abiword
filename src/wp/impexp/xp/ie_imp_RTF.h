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

#ifndef IE_IMP_RTF_H
#define IE_IMP_RTF_H

#include <stdio.h>
#include "ie_imp.h"
#include "ut_growbuf.h"
#include "ut_vector.h"
#include "ut_stack.h"
class PD_Document;



// Font table entry
class RTFFontTableItem
{
public:
	enum FontFamilyEnum { ffNone, ffRoman, ffSwiss, ffModern, ffScript, ffDecorative, ffTechnical, ffBiDirectional};
	enum FontPitch { fpDefault, fpFixed, fpVariable};

	RTFFontTableItem(FontFamilyEnum fontFamily, int charSet, int codepage, FontPitch pitch,
						unsigned char* panose, char* pFontName, char* pAlternativeFontName);
	~RTFFontTableItem();

public:
	FontFamilyEnum m_family;
	int m_charSet;
	int m_codepage;
	FontPitch m_pitch;
	unsigned char m_panose[10];
	char* m_pFontName;
	char* m_pAlternativeFontName;
};



// Character properties
class RTFProps_CharProps
{
public:
	RTFProps_CharProps();

	UT_Bool	m_bold;
	UT_Bool	m_italic;
	UT_Bool	m_underline;
	UT_Bool	m_strikeout;
	double	m_fontSize;
	UT_uint32 m_fontNumber;
	UT_uint32 m_colourNumber;
};                  


// RTFStateStore
class RTFStateStore
{
public:
	RTFStateStore();
	
//	RTFStateStore& operator=(const RTFStateStore&);

	enum DestinationStateTypes { rdsNorm, rdsSkip };
	enum InternalStateTypes { risNorm, risBin };

	DestinationStateTypes	m_destinationState;		// Reading or skipping text
	InternalStateTypes		m_internalState;		// Normal, binary or hex
	RTFProps_CharProps		m_charProps;			// Character properties
};


/*
class RTFProps_ParaProps
{

};

typedef enum {justL, justR, justC, justF } JUST;
typedef struct para_prop
{
    int xaLeft;                 // left indent in twips
    int xaRight;                // right indent in twips
    int xaFirst;                // first line indent in twips
    JUST just;                  // justification
} PAP;                  // PAragraph Properties



typedef enum {justL, justR, justC, justF } JUST;
typedef struct para_prop
{
    int xaLeft;                 // left indent in twips
    int xaRight;                // right indent in twips
    int xaFirst;                // first line indent in twips
    JUST just;                  // justification
} PAP;                  // PAragraph Properties

typedef enum {sbkNon, sbkCol, sbkEvn, sbkOdd, sbkPg} SBK;
typedef enum {pgDec, pgURom, pgLRom, pgULtr, pgLLtr} PGN;
typedef struct sect_prop
{
    int cCols;                  // number of columns
    SBK sbk;                    // section break type
    int xaPgn;                  // x position of page number in twips
    int yaPgn;                  // y position of page number in twips
    PGN pgnFormat;              // how the page number is formatted
} SEP;                  // SEction Properties

typedef struct doc_prop
{
    int xaPage;                 // page width in twips
    int yaPage;                 // page height in twips
    int xaLeft;                 // left margin in twips
    int yaTop;                  // top margin in twips
    int xaRight;                // right margin in twips
    int yaBottom;               // bottom margin in twips
    int pgnStart;               // starting page number in twips
    char fFacingp;              // facing pages enabled?
    char fLandscape;            // landscape or portrait??
} DOP;                  // DOcument Properties

typedef enum { rdsNorm, rdsSkip } RDS;              // Rtf Destination State
typedef enum { risNorm, risBin, risHex } RIS;       // Rtf Internal State

typedef struct save             // property save structure
{
    struct save *pNext;         // next save
    CHP chp;
    PAP pap;
    SEP sep;
    DOP dop;
    RDS rds;
    RIS ris;
} SAVE;

// What types of properties are there?
typedef enum {ipropBold, ipropItalic, ipropUnderline, ipropLeftInd,
              ipropRightInd, ipropFirstInd, ipropCols, ipropPgnX,
              ipropPgnY, ipropXaPage, ipropYaPage, ipropXaLeft,
              ipropXaRight, ipropYaTop, ipropYaBottom, ipropPgnStart,
              ipropSbk, ipropPgnFormat, ipropFacingp, ipropLandscape,
              ipropJust, ipropPard, ipropPlain, ipropSectd,
              ipropMax } IPROP;

typedef enum {actnSpec, actnByte, actnWord} ACTN;
typedef enum {propChp, propPap, propSep, propDop} PROPTYPE;

typedef struct propmod
{
    ACTN actn;              // size of value
    PROPTYPE prop;          // structure containing value
    int  offset;            // offset of value from base of structure
} PROP;

typedef enum {ipfnBin, ipfnHex, ipfnSkipDest } IPFN;
typedef enum {idestPict, idestSkip } IDEST;
typedef enum {kwdChar, kwdDest, kwdProp, kwdSpec} KWD;

typedef struct symbol
{
    char *szKeyword;        // RTF keyword
    int  dflt;              // default value to use
    UT_Bool fPassDflt;         // true to use default value from this table
    KWD  kwd;               // base action to take
    int  idx;               // index into property table if kwd == kwdProp
                            // index into destination table if kwd == kwdDest
                            // character to print if kwd == kwdChar
} SYM;


*/



// The importer/reader for Rich Text Format files

class IE_Imp_RTF : public IE_Imp
{
public:
	IE_Imp_RTF(PD_Document * pDocument);
	~IE_Imp_RTF();

	IEStatus			importFile(const char * szFilename);

	static UT_Bool		RecognizeSuffix(const char * szSuffix);
	static IEStatus		StaticConstructor(PD_Document * pDocument,
										  IE_Imp ** ppie);
	static UT_Bool		GetDlgLabels(const char ** pszDesc,
									 const char ** pszSuffixList);
	static UT_Bool 		SupportsFileType(IEFileType ft);
	
protected:
	IEStatus			_parseFile(FILE * fp);
	IEStatus			_writeHeader(FILE * fp);


// importer helper methods
private:
	UT_Bool AddChar(UT_UCSChar ch);
	UT_Bool FlushStoredChars(UT_Bool forceInsertPara = UT_FALSE);
	UT_Bool StartNewPara();
	UT_Bool PushRTFState();
	UT_Bool PopRTFState();
	UT_Bool ParseRTFKeyword();
	UT_Bool ParseChar(UT_UCSChar ch);
	UT_Bool ReadCharFromFile(unsigned char* pCh);
	UT_Bool SkipBackChar(unsigned char ch);
	UT_Bool ReadKeyword(unsigned char* pKeyword, long* pParam, UT_Bool* pParamUsed);
	UT_Bool TranslateKeyword(unsigned char* pKeyword, long param, UT_Bool fParam);
	UT_Bool ReadColourTable();
	UT_Bool ReadFontTable();
	UT_Bool ReadOneFontFromTable();

	// Character propery handlers
	UT_Bool ResetCharacterAttributes();
	UT_Bool ApplyCharacterAttributes();
	UT_Bool HandleBoolCharacterProp(UT_Bool state, UT_Bool* pProp);
		UT_Bool HandleBold(UT_Bool state);
		UT_Bool HandleItalic(UT_Bool state);
		UT_Bool HandleUnderline(UT_Bool state);
		UT_Bool HandleStrikeout(UT_Bool state);
	UT_Bool HandleFontSize(long sizeInHalfPoints);
	UT_Bool HandleU32CharacterProp(UT_uint32 val, UT_uint32* pProp);
		UT_Bool HandleFace(UT_uint32 fontNumber);
		UT_Bool HandleColour(UT_uint32 colourNumber);


// import member vars
private:
	UT_GrowBuf m_gbBlock;

	int m_groupCount;
	UT_Bool m_newParaFlagged;
	int m_cbBin;

	UT_Stack m_stateStack;
	RTFStateStore m_currentRTFState;

	UT_Vector m_fontTable;
	UT_Vector m_colourTable;

	FILE* m_pImportFile;
};

#endif /* IE_IMP_RTF_H */
