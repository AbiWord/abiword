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
#include "ut_mbtowc.h"
#include "fl_AutoLists.h"
#include "fl_AutoNum.h"

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

	bool	m_deleted;
	bool	m_bold;
	bool	m_italic;
	bool	m_underline;
	bool m_overline;
	bool	m_strikeout;
	bool m_superscript;
	bool m_subscript;
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
    UT_sint32	m_indentLeft;	// left indent in twips
    UT_sint32	m_indentRight;	// right indent in twips
    UT_sint32	m_indentFirst;	// first line indent in twips
	double	m_lineSpaceVal;		// line spaceing value
	bool	m_lineSpaceExact;	// TRUE if m_lineSpaceVal is an exact value, FALSE if multiple
	UT_Vector m_tabStops;
	UT_Vector m_tabTypes;
	bool         m_isList;       // TRUE if para is an element of a list
	UT_sint32       m_level;        // Level of list in para
	char            m_pszStyle[30]; // Type of List
	UT_uint32       m_rawID;        // raw ID of list
	UT_uint32       m_rawParentID;        // raw Parent ID of list
	char            m_pszListDecimal[64]; // char between levels
	char            m_pszListDelim[64];   // char between levels
	char            m_pszFieldFont[64];   // field font name
	UT_uint32       m_startValue;         // Start value of the list
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

	UT_uint32				m_unicodeAlternateSkipCount;	// value of N in "\ucN"
	UT_uint32				m_unicodeInAlternate;			// chars left in alternate "\u<u><A>"
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

	virtual UT_Error	importFile(const char * szFilename);
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
										unsigned char * pData, UT_uint32 lenData);

	static bool		RecognizeContents(const char * szBuf, UT_uint32 iNumbytes);
	static bool		RecognizeSuffix(const char * szSuffix);
	static UT_Error		StaticConstructor(PD_Document * pDocument,
										  IE_Imp ** ppie);
	static bool		GetDlgLabels(const char ** pszDesc,
									 const char ** pszSuffixList,
									 IEFileType * ft);
	static bool 		SupportsFileType(IEFileType ft);

protected:
	UT_Error			_parseFile(FILE * fp);
	UT_Error			_writeHeader(FILE * fp);


// importer helper methods
private:
	bool AddChar(UT_UCSChar ch);
	bool FlushStoredChars(bool forceInsertPara = false);
	bool StartNewPara();
	bool StartNewSection();
	bool PushRTFState();
	bool PopRTFState();
	bool ParseRTFKeyword();
	bool ParseChar(UT_UCSChar ch,bool no_convert=1);
  bool ReadCharFromFileWithCRLF(unsigned char* pCh);
	bool ReadCharFromFile(unsigned char* pCh);
	bool SkipBackChar(unsigned char ch);
	bool ReadKeyword(unsigned char* pKeyword, long* pParam, bool* pParamUsed);
	bool TranslateKeyword(unsigned char* pKeyword, long param, bool fParam);
	bool ReadColourTable();
	bool ReadFontTable();
	bool ReadOneFontFromTable();
	
	RTFFontTableItem* GetNthTableFont(UT_uint32 fontNum);
	UT_uint32 GetNthTableColour(UT_uint32 colNum);

	// Character property handlers
	bool ResetCharacterAttributes();
	bool ApplyCharacterAttributes();
	bool HandleBoolCharacterProp(bool state, bool* pProp);
	bool HandleDeleted(bool state);
	bool HandleBold(bool state);
	bool HandleItalic(bool state);
	bool HandleUnderline(bool state);
	bool HandleOverline(bool state);
	bool HandleStrikeout(bool state);
	bool HandleSuperscript(bool state);
	bool HandleSubscript(bool state);
	bool HandleFontSize(long sizeInHalfPoints);
	bool HandleU32CharacterProp(UT_uint32 val, UT_uint32* pProp);
	bool HandleFace(UT_uint32 fontNumber);
	bool HandleColour(UT_uint32 colourNumber);

	// Paragraph property handlers
	bool ResetParagraphAttributes();
	bool ApplyParagraphAttributes();
	bool SetParaJustification(RTFProps_ParaProps::ParaJustification just);
	bool AddTabstop(UT_sint32 stopDist);

	bool HandleAbiLists(void);
	bool HandleLists(void);
        UT_uint32 mapID(UT_uint32 id);
	UT_uint32 mapParentID(UT_uint32 id);

	// Section property handlers
	bool ApplySectionAttributes();
	bool ResetSectionAttributes();


// import member vars
private:
	UT_GrowBuf m_gbBlock;

	int m_groupCount;
	bool m_newParaFlagged;
	bool m_newSectionFlagged;

	int m_cbBin;

	UT_Stack m_stateStack;
	RTFStateStore m_currentRTFState;

	UT_Vector m_fontTable;
	UT_Vector m_colourTable;

	struct _rtfAbiListTable
        {      
	       UT_uint32 orig_id;
	       UT_uint32 orig_parentid;
	       UT_uint32 start_value;
	       UT_uint32 level;
	       bool hasBeenMapped;
	       UT_uint32 mapped_id;
	       UT_uint32 mapped_parentid;
	};
	_rtfAbiListTable m_rtfAbiListTable[2000]; // this will be a vector eventually
	UT_uint32 m_numLists;
	bool m_bisAbiList;
        
	struct _rtfListTable
        {      
	       UT_uint32 start_value;
	       UT_uint32 level;
	       bool bullet;
	       bool simple;
	       bool continueList;
	       bool hangingIndent;
	       List_Type type;
	       bool bold;
	       bool italic;
	       bool caps;
	       bool scaps;
	       bool underline;
	       bool nounderline;
	       bool strike;
	       bool isList;
	       UT_uint32 forecolor;
	       UT_uint32 font;
	       UT_uint32 fontsize;
	       UT_uint32 indent;
	       bool prevlist;
	       char textbefore[129];
	       char textafter[129];
	};
	_rtfListTable m_rtfListTable; 
 
	FILE* m_pImportFile;

	unsigned char *		m_pPasteBuffer;
	UT_uint32 			m_lenPasteBuffer;
	unsigned char *		m_pCurrentCharInPasteBuffer;
	PT_DocPosition		m_dposPaste;
	UT_uint32		deflangid;
	UT_Mbtowc		m_mbtowc;
};

#endif /* IE_IMP_RTF_H */

