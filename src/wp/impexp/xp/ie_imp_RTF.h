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
#include "pt_Types.h"
#include "pd_Document.h"



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
	double	m_fontSize;			// font size in points
	UT_uint32 m_fontNumber;		// index into font table
	UT_uint32 m_colourNumber;	// index into colour table
};                  


// Paragraph properties
class RTFProps_ParaProps
{
public:
	enum ParaJustification { pjLeft, pjCentre, pjRight, pjFull};
	enum TabTypes { ttLeft, ttCentre, ttRight, ttDecimal};

	RTFProps_ParaProps();
	RTFProps_ParaProps& operator=(const RTFProps_ParaProps&);

	ParaJustification	m_justification;
	UT_uint32	m_spaceBefore;	// space above paragraph in twips
	UT_uint32	m_spaceAfter;	// space above paragraph in twips
    UT_uint32	m_indentLeft;	// left indent in twips
    UT_uint32	m_indentRight;	// right indent in twips
    UT_uint32	m_indentFirst;	// first line indent in twips
	double	m_lineSpaceVal;		// line spaceing value
	UT_Bool	m_lineSpaceExact;	// TRUE if m_lineSpaceVal is an exact value, FALSE if multiple
	UT_Vector m_tabStops;
	UT_Vector m_tabTypes;
};                  


//typedef struct sect_prop
//{
//    int cCols;                  // number of columns
//    SBK sbk;                    // section break type
//    int xaPgn;                  // x position of page number in twips
//    int yaPgn;                  // y position of page number in twips
//    PGN pgnFormat;              // how the page number is formatted
//} SEP;                  // SEction Properties


// Section properties
class RTFProps_SectionProps
{
public:
	enum SectionBreak {sbkNone, sbkColumn, sbkEven, sbkOdd, sbkPage};
	enum PageNumber {pgDecimal, pgURoman, pgLRoman, pgULtr, pgLLtr};

	RTFProps_SectionProps();
	RTFProps_SectionProps& operator=(const RTFProps_SectionProps&);

	UT_uint32		m_numCols;
	SectionBreak	m_breakType;
	PageNumber		m_pageNumFormat;
};


// RTFStateStore
class RTFStateStore
{
public:
	RTFStateStore();
	
//	RTFStateStore& operator=(const RTFStateStore&);

	enum DestinationStateTypes { rdsNorm, rdsSkip };
	enum InternalStateTypes { risNorm, risBin, risHex };

	DestinationStateTypes	m_destinationState;		// Reading or skipping text
	InternalStateTypes		m_internalState;		// Normal, binary or hex
	RTFProps_CharProps		m_charProps;			// Character properties
	RTFProps_ParaProps		m_paraProps;			// Paragraph properties
	RTFProps_SectionProps	m_sectionProps;			// Section properties
};


/*
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
*/



// The importer/reader for Rich Text Format files

class IE_Imp_RTF : public IE_Imp
{
public:
	IE_Imp_RTF(PD_Document * pDocument);
	~IE_Imp_RTF();

	virtual IEStatus	importFile(const char * szFilename);
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
										unsigned char * pData, UT_uint32 lenData);

	static UT_Bool		RecognizeSuffix(const char * szSuffix);
	static IEStatus		StaticConstructor(PD_Document * pDocument,
										  IE_Imp ** ppie);
	static UT_Bool		GetDlgLabels(const char ** pszDesc,
									 const char ** pszSuffixList,
									 IEFileType * ft);
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
	
	RTFFontTableItem* GetNthTableFont(UT_uint32 fontNum);
	UT_uint32 GetNthTableColour(UT_uint32 colNum);

	// Character property handlers
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

	// Paragraph property handlers
	UT_Bool ResetParagraphAttributes();
	UT_Bool ApplyParagraphAttributes();
	UT_Bool SetParaJustification(RTFProps_ParaProps::ParaJustification just);
	UT_Bool AddTabstop(UT_sint32 stopDist);

	// Section property handlers
	UT_Bool ApplySectionAttributes();


// import member vars
private:
	UT_GrowBuf m_gbBlock;

	UT_Bool m_newParaFlagged;
	UT_Bool m_newSectionFlagged;
	int m_groupCount;
	int m_cbBin;

	UT_Stack m_stateStack;
	RTFStateStore m_currentRTFState;

	UT_Vector m_fontTable;
	UT_Vector m_colourTable;

	FILE* m_pImportFile;

	unsigned char *		m_pPasteBuffer;
	UT_uint32 			m_lenPasteBuffer;
	unsigned char *		m_pCurrentCharInPasteBuffer;
	PT_DocPosition		m_dposPaste;
};

#endif /* IE_IMP_RTF_H */
