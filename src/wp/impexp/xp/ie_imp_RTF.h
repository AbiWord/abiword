/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
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
#include "ut_bytebuf.h"
#include "ut_growbuf.h"
#include "ut_vector.h"
#include "ut_stack.h"
#include "pt_Types.h"
#include "pd_Document.h"
#include "ut_mbtowc.h"
#include "fl_AutoLists.h"
#include "fl_AutoNum.h"
#include "fl_BlockLayout.h"
#include "fribidi/fribidi.h"
#include  "ie_Table.h"

class IE_Imp_RTF;
class RTF_msword97_list;
class ie_imp_cell;
class ie_imp_table;
class ie_imp_table_control;

// Font table entry
struct ABI_EXPORT RTFFontTableItem
{
 public:
	enum FontFamilyEnum { ffNone, ffRoman, ffSwiss, ffModern, ffScript, ffDecorative, ffTechnical, ffBiDirectional};
	enum FontPitch { fpDefault, fpFixed, fpVariable};

	RTFFontTableItem(FontFamilyEnum fontFamily, int charSet, int codepage, FontPitch pitch,
						unsigned char* panose, char* pFontName, char* pAlternativeFontName);
	~RTFFontTableItem();

	FontFamilyEnum m_family;
	int m_charSet;
	int m_codepage;
	const char* m_szEncoding;
	FontPitch m_pitch;
	unsigned char m_panose[10];
	char* m_pFontName;
	char* m_pAlternativeFontName;
};

// Set true if Character properties have been changed in list structure.
class ABI_EXPORT RTFProps_CharProps
{
public:
	RTFProps_CharProps(void);
	~RTFProps_CharProps(void);
	bool	m_deleted;
	bool	m_bold;
	bool	m_italic;
	bool	m_underline;
	bool    m_overline;
	bool	m_strikeout;
	bool	m_topline;
	bool	m_botline;
	bool    m_superscript;
	double    m_superscript_pos;       // unit is pt. if 0.0, ignore
	bool    m_subscript;
	double    m_subscript_pos;         // unit is pt. if 0.0, ignore
	double    m_fontSize;			// font size in points
	UT_uint32    m_fontNumber;		// index into font table
	bool    m_hasColour;        // if false, ignore colour number
    UT_uint32    m_colourNumber;	// index into colour table
	bool    m_hasBgColour; // if false, ignore colour number
	UT_uint32    m_bgcolourNumber; // index into colour table
	UT_sint32  m_styleNumber ; //index into the style table
	UT_uint32  m_listTag; // tag for lists to hang off
	const char * m_szLang;
};

class ABI_EXPORT RTFProps_bCharProps
{
public:
	RTFProps_bCharProps(void);
	~RTFProps_bCharProps(void);

	bool	bm_deleted;
	bool	bm_bold;
	bool	bm_italic;
	bool	bm_underline;
	bool    bm_overline;
	bool	bm_strikeout;
	bool	bm_topline;
	bool	bm_botline;
	bool bm_superscript;
	bool bm_superscript_pos;       // unit is pt. if 0.0, ignore
	bool bm_subscript;
	bool bm_subscript_pos;         // unit is pt. if 0.0, ignore
	bool bm_fontSize;			// font size in points
	bool bm_fontNumber;		// index into font table
	bool    bm_hasColour;        // if false, ignore colour number
	bool bm_colourNumber;	// index into colour table
	bool bm_hasBgColour; // if false, ignore colour number
	bool bm_bgcolourNumber; // index into colour table
	bool bm_listTag; // tag for lists to hanfg off
};

struct ABI_EXPORT _rtfListTable
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
	UT_uint32 iWord97Override;
	UT_uint32 iWord97Level;
};


struct ABI_EXPORT RTFProps_CellProps
{
	RTFProps_CellProps();
	RTFProps_CellProps & operator=(const RTFProps_CellProps&);
	bool      m_bVerticalMerged;
	bool      m_bVerticalMergedFirst;
	UT_sint32 m_iCellx;
};

struct ABI_EXPORT RTFProps_TableProps
{
	RTFProps_TableProps();
	RTFProps_TableProps& operator=(const RTFProps_TableProps&);
	bool      m_bAutoFit;
};

// Paragraph properties
struct ABI_EXPORT RTFProps_ParaProps
{
	enum ParaJustification { pjLeft, pjCentre, pjRight, pjFull};

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
	UT_Vector m_tabLeader;
	bool         m_isList;       // TRUE if para is an element of a list
	UT_sint32       m_level;        // Level of list in para
	char            m_pszStyle[30]; // Type of List
	UT_uint32       m_rawID;        // raw ID of list
	UT_uint32       m_rawParentID;        // raw Parent ID of list
	char            m_pszListDecimal[64]; // char between levels
	char            m_pszListDelim[64];   // char between levels
	char            m_pszFieldFont[64];   // field font name
	UT_uint32       m_startValue;         // Start value of the list
	eTabType        m_curTabType;        // Current Tab type
	eTabLeader      m_curTabLeader;       // Current Tab Leader
	UT_uint32       m_iOverride;          // 1's index to override table
	UT_uint32       m_iOverrideLevel;     // 0's index to the level
	_rtfListTable   m_rtfListTable;
	UT_sint32  m_styleNumber ; //index into the style table
	FriBidiCharType m_dom_dir;
	UT_sint32       m_tableLevel; //nesting level of the paragram in a table.
};

// These are set true if changed in list definitions.
class ABI_EXPORT RTFProps_bParaProps
{
public:
    RTFProps_bParaProps(void);
    ~RTFProps_bParaProps(void);

	bool        bm_justification;
	bool    	bm_spaceBefore;	// space above paragraph in twips
	bool    	bm_spaceAfter;	// space above paragraph in twips
    bool    	bm_indentLeft;	// left indent in twips
    bool    	bm_indentRight;	// right indent in twips
    bool    	bm_indentFirst;	// first line indent in twips
	bool	    bm_lineSpaceVal;		// line spaceing value
	bool	    bm_lineSpaceExact;	// TRUE if m_lineSpaceVal is an exact value, FALSE if multiple
	bool        bm_tabStops;
	bool        bm_tabTypes;
	bool        bm_tabLeader;
	bool        bm_curTabType;        // Current Tab type
	bool        bm_curTabLeader;       // Current Tab Leader
	bool        bm_rtfListTable;
	bool        bm_dom_dir;
};


//typedef struct sect_prop
//{
//    int cCols;                  // number of columns
//    SBK sbk;                    // section break type
//    int xaPgn;                  // x position of page number in twips
//    int yaPgn;                  // y position of page number in twips
//    PGN pgnFormat;              // how the page number is formatted
//} SEP;                  // SEction Properties


// Lists Level class
class ABI_EXPORT RTF_msword97_level
{
public:
    RTF_msword97_level(	RTF_msword97_list * pmsword97List, UT_uint32 level);
	~RTF_msword97_level();
	void buildAbiListProperties( const char ** szListID,
								 const char ** szParentID,
								 const char ** szLevel,
								 const char ** szStartat,
								 const char ** szFieldFont,
								 const char ** szListDelim,
								 const char ** szListDecimal,
								 const char ** szAlign,
								 const char ** szIndent,
								 const char ** szListStyle);
	bool ParseLevelText(const UT_String & szLevelText,const UT_String & szLevelNumbers, UT_uint32 iLevel);
	UT_sint32 m_levelStartAt;
	UT_uint32 m_AbiLevelID;
	static UT_uint32 m_sLastAssignedLevelID;
	static UT_uint32 m_sPreviousLevel;
	UT_uint32 m_RTFListType;
	UT_String m_listDelim;
	char      m_cLevelFollow;
	bool m_bStartNewList;
	bool m_bRestart;
    RTFProps_ParaProps * m_pParaProps;
	RTFProps_CharProps *  m_pCharProps;
    RTFProps_bParaProps * m_pbParaProps;
	RTFProps_bCharProps *  m_pbCharProps;
private:
	UT_uint32 m_localLevel;
	RTF_msword97_list * m_pMSWord97_list ;
};

// List Header Class
class ABI_EXPORT RTF_msword97_list
{
public:
	RTF_msword97_list(	IE_Imp_RTF * pie_rtf);
    ~RTF_msword97_list();
	UT_uint32 m_RTF_listID;
	UT_uint32 m_RTF_listTemplateID;
	RTF_msword97_level * m_RTF_level[9];
private:
	IE_Imp_RTF * m_pie_rtf;
};

// List Header Override
class ABI_EXPORT RTF_msword97_listOverride
{
public:
	RTF_msword97_listOverride(	IE_Imp_RTF * pie_rtf);
    ~RTF_msword97_listOverride();
	void buildAbiListProperties( const char ** szListID,
								 const char ** szParentID,
								 const char ** szLevel,
								 const char ** szStartat,
								 const char ** szFieldFont,
								 const char ** szListDelim,
								 const char ** szListDecimal,
								 const char ** szAlign,
								 const char ** szIndent,
								 const char ** szListStyle,
								 UT_uint32 iLevel);
	UT_uint32 m_RTF_listID;
	UT_uint32 m_OverrideCount;
	RTFProps_ParaProps * m_pParaProps;
	RTFProps_CharProps * m_pCharProps;
	RTFProps_bParaProps * m_pbParaProps;
	RTFProps_bCharProps * m_pbCharProps;
	bool setList(void);
	bool isTab(UT_uint32 iLevel);
	UT_Vector * getTabStopVect(UT_uint32 iLevel);
	UT_Vector * getTabTypeVect(UT_uint32 iLevel);
	UT_Vector * getTabLeaderVect(UT_uint32 iLevel);
	bool isDeletedChanged(UT_uint32 iLevel);
	bool getDeleted(UT_uint32 iLevel);
	bool isBoldChanged(UT_uint32 iLevel);
	bool getBold(UT_uint32 iLevel);
	bool isItalicChanged(UT_uint32 iLevel);
	bool getItalic(UT_uint32 iLevel);
	bool isUnderlineChanged(UT_uint32 iLevel);
	bool getUnderline(UT_uint32 iLevel);
	bool isStrikeoutChanged(UT_uint32 iLevel);
	bool getStrikeout(UT_uint32 iLevel);
	bool isSuperscriptChanged(UT_uint32 iLevel);
	bool getSuperscript(UT_uint32 iLevel);
	bool isSuperscriptPosChanged(UT_uint32 iLevel);
	double getSuperscriptPos(UT_uint32 iLevel);
	bool isSubscriptChanged(UT_uint32 iLevel);
	bool getSubscript(UT_uint32 iLevel);
	bool isSubscriptPosChanged(UT_uint32 iLevel);
	double getSubscriptPos(UT_uint32 iLevel);
 	bool isFontSizeChanged(UT_uint32 iLevel);
 	double getFontSize(UT_uint32 iLevel);
 	bool isHasColourChanged(UT_uint32 iLevel);
 	bool getHasColour(UT_uint32 iLevel);
 	bool isColourNumberChanged(UT_uint32 iLevel);
 	UT_uint32 getColourNumber(UT_uint32 iLevel);
 	bool isHasBgColourChanged(UT_uint32 iLevel);
 	bool getHasBgColour(UT_uint32 iLevel);
	bool isBgColourNumberChanged(UT_uint32 iLevel);
 	UT_uint32 getBgColourNumber(UT_uint32 iLevel);
 	bool isFontNumberChanged(UT_uint32 iLevel);
 	UT_uint32 getFontNumber(UT_uint32 iLevel);

private:
	IE_Imp_RTF * m_pie_rtf;
	RTF_msword97_list* m_pList;
};

struct ABI_EXPORT RTFProps_ImageProps
{
	enum IPSizeType { ipstNone, ipstGoal, ipstScale };

	RTFProps_ImageProps ();
	IPSizeType sizeType;
	UT_uint32 wGoal;
	UT_uint32 hGoal;
	UT_uint16 scaleX;
	UT_uint16 scaleY;
	UT_uint32 width;
	UT_uint32 height;
};

// Section properties
struct ABI_EXPORT RTFProps_SectionProps
{
	enum ESectionBreak {sbkNone, sbkColumn, sbkEven, sbkOdd, sbkPage};
	enum EPageNumber {pgDecimal, pgURoman, pgLRoman, pgULtr, pgLLtr};

	RTFProps_SectionProps();

	UT_uint32		m_numCols;
	bool m_bColumnLine;
	ESectionBreak	m_breakType;
	EPageNumber		m_pageNumFormat;
	UT_sint32       m_leftMargTwips;
	UT_sint32       m_rightMargTwips;
	UT_sint32       m_topMargTwips;
	UT_sint32       m_bottomMargTwips;
	UT_sint32       m_headerYTwips;
	UT_sint32       m_footerYTwips;
	UT_sint32       m_colSpaceTwips;
	FriBidiCharType m_dir;
};


/*!
  Stores a RTF header and footer.
  headers and footer are NOT section properties. But they are defined
  before any section data begins.
  \todo add right and left headers and footer. Not yet supported by AbiWord
 */
struct ABI_EXPORT RTFHdrFtr
{
	enum HdrFtrType {hftNone,
					 hftHeader,
					 hftHeaderEven,
					 hftHeaderFirst,
					 hftHeaderLast,
					 hftFooter,
					 hftFooterEven,
					 hftFooterFirst,
					 hftFooterLast };

	RTFHdrFtr () : m_type(hftNone), m_id(0), m_buf(1024) {}

	HdrFtrType      m_type;
	UT_uint32       m_id;
	UT_ByteBuf      m_buf;
};

// RTFStateStore
struct ABI_EXPORT RTFStateStore
{
	RTFStateStore();

//	RTFStateStore& operator=(const RTFStateStore&);

	enum DestinationStateTypes { rdsNorm, rdsSkip, rdsFootnote, rdsHeader, rdsFooter, rdsField };
	enum InternalStateTypes { risNorm, risBin, risHex };

	DestinationStateTypes	m_destinationState;		// Reading or skipping text
	InternalStateTypes		m_internalState;		// Normal, binary or hex
	RTFProps_CharProps		m_charProps;			// Character properties
	RTFProps_ParaProps		m_paraProps;			// Paragraph properties
	RTFProps_SectionProps	m_sectionProps;			// Section properties
	RTFProps_CellProps      m_cellProps;            // Cell properties
    RTFProps_TableProps     m_tableProps;           // Table properties

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

class ABI_EXPORT IE_Imp_RTF_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_RTF_Sniffer() {}
	virtual ~IE_Imp_RTF_Sniffer() {}

	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class ABI_EXPORT IE_Imp_RTF : public IE_Imp
{
public:
	IE_Imp_RTF(PD_Document * pDocument);
	~IE_Imp_RTF();

	virtual UT_Error	importFile(const char * szFilename);
	virtual void		pasteFromBuffer(PD_DocumentRange * pDocRange,
										unsigned char * pData, UT_uint32 lenData, const char * szEncoding = 0);
	UT_sint32 get_vecWord97ListsCount(void) { return m_vecWord97Lists.getItemCount();}
	RTF_msword97_list *  get_vecWord97NthList(UT_sint32 i) { return (RTF_msword97_list *) m_vecWord97Lists.getNthItem(i);}
    bool  isWord97Lists(void) const { return (m_vecWord97Lists.getItemCount() > 0);}
protected:
	UT_Error			_parseFile(FILE * fp);
	UT_Error			_writeHeader(FILE * fp);
	UT_Error            _parseHdrFtr ();

// importer helper methods
private:
	enum PictFormat {
		picNone,
		picPNG,
		picJPEG,
		picBMP,
		picWMF,
		picPICT,
		picEMF,
		picGIF
	};

	typedef enum {
		RBT_START = 0,
		RBT_END
	} RTFBookmarkType;

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
	bool ReadKeyword(unsigned char* pKeyword, long* pParam, bool* pParamUsed,
					 UT_uint32 keywordBuffLen);
	bool TranslateKeyword(unsigned char* pKeyword, long param, bool fParam);
	bool ReadColourTable();
	bool ReadFontTable();
	bool ReadOneFontFromTable();
	bool HandlePicture();
	bool HandleObject();
	bool HandleField();
	bool HandleHyperlink();
	bool HandleStyleDefinition(void);
	bool HandleHeaderFooter(RTFHdrFtr::HdrFtrType hftype, UT_uint32 & headerID);
	bool SkipCurrentGroup(bool bConsumeLastBrace = false);
	bool StuffCurrentGroup(UT_ByteBuf & buf);
	bool LoadPictData(PictFormat format, const char * image_name,
					  struct RTFProps_ImageProps & imgProps);
	bool InsertImage (const UT_ByteBuf * buf, const char * image_name,
					  const struct RTFProps_ImageProps & imgProps);

	RTFFontTableItem* GetNthTableFont(UT_uint32 fontNum);
	UT_uint32 GetNthTableColour(UT_uint32 colNum);
	UT_sint32 GetNthTableBgColour(UT_uint32 colNum);

// ListTable handlers.
	bool ReadListTable(void);
	bool HandleListLevel(RTF_msword97_list * pList, UT_uint32 levelCount  );
	bool HandleTableList(void);
	char * getCharsInsideBrace(void);
	bool ParseCharParaProps( unsigned char * pKeyword, long param, bool fParam, RTFProps_CharProps * pChars, RTFProps_ParaProps * pParas, RTFProps_bCharProps * pbChars, RTFProps_bParaProps * pbParas);
	bool ReadListOverrideTable(void);
	bool HandleTableListOverride(void);

	bool buildAllProps( char * propBuffer,  RTFProps_ParaProps * pParas,
					   RTFProps_CharProps * pChars,
					   RTFProps_bParaProps * pbParas,
					   RTFProps_bCharProps * pbChars);

	
	// Character property handlers
	bool ResetCharacterAttributes();
	bool buildCharacterProps(UT_String & propBuffer);
	bool ApplyCharacterAttributes();
	bool HandleBoolCharacterProp(bool state, bool* pProp);
	bool HandleDeleted(bool state);
	bool HandleBold(bool state);
	bool HandleItalic(bool state);
	bool HandleUnderline(bool state);
	bool HandleOverline(bool state);
	bool HandleStrikeout(bool state);
	bool HandleTopline(bool state);
	bool HandleBotline(bool state);
	bool HandleSuperscript(bool state);
	bool HandleSuperscriptPosition(UT_uint32 pos);
	bool HandleSubscript(bool state);
	bool HandleSubscriptPosition(UT_uint32 pos);
	bool HandleFontSize(long sizeInHalfPoints);
	bool HandleBookmark (RTFBookmarkType type);
	bool HandleListTag(long id);

	// Generic handlers
	bool HandleFloatCharacterProp(double val, double* pProp);
	bool HandleU32CharacterProp(UT_uint32 val, UT_uint32* pProp);
	bool HandleFace(UT_uint32 fontNumber);
	bool HandleColour(UT_uint32 colourNumber);
	bool HandleBackgroundColour (UT_uint32 colourNumber);

	// Paragraph property handlers
	bool ResetParagraphAttributes();
	bool ApplyParagraphAttributes();
	bool SetParaJustification(RTFProps_ParaProps::ParaJustification just);
	bool AddTabstop(UT_sint32 stopDist, eTabType tabType, eTabLeader tableader);
	bool AddTabstop(UT_sint32 stopDist, eTabType tabType, eTabLeader tabLeader,  RTFProps_ParaProps * pParas);

	bool HandleAbiLists(void);
	bool HandleLists(_rtfListTable & rtfListTable );
        UT_uint32 mapID(UT_uint32 id);
	UT_uint32 mapParentID(UT_uint32 id);

// Table methods
    bool           ResetCellAttributes(void);
	bool           ResetTableAttributes(void);
    ie_imp_table * getTable(void);
	ie_imp_cell *  getCell(void);
	void           FlushCellProps(void);
	void           FlushTableProps(void);
	void           OpenTable(void);
	void           CloseTable(void);
    void           HandleCell(void);
	void           HandleCellX(UT_sint32 cellx);
    void           HandleRow(void);

	// Section property handlers
	bool ApplySectionAttributes();
	bool ResetSectionAttributes();
	typedef enum {
	    RTF_TOKEN_NONE = 0,
	    RTF_TOKEN_OPEN_BRACE,
	    RTF_TOKEN_CLOSE_BRACE,
	    RTF_TOKEN_KEYWORD,
	    RTF_TOKEN_DATA,
	    RTF_TOKEN_ERROR = -1
	} RTFTokenType;
	RTFTokenType NextToken (unsigned char *pKeyword, long* pParam,
							bool* pParamUsed, UT_uint32 len, bool bIgnoreWhiteSpace=false);


// import member vars
private:
	UT_GrowBuf m_gbBlock;
	char *m_szFileDirName;

	int m_groupCount;
	bool m_newParaFlagged;
	bool m_newSectionFlagged;
	int m_cbBin;

	// headers and footers
	// headers and footers are NOT part of the state. They change each time
	// they are defined and sections inherit them from the previous
	// this is not part of section properties, they are not reset by \sectd
	// TODO: handle \titlepg and \facingpg cases.
	UT_uint32       m_currentHdrID;     // these are numbers.
	UT_uint32       m_currentFtrID;
	UT_uint32       m_currentHdrEvenID;     // these are numbers.
	UT_uint32       m_currentFtrEvenID;
	UT_uint32       m_currentHdrFirstID;     // these are numbers.
	UT_uint32       m_currentFtrFirstID;
	UT_uint32       m_currentHdrLastID;     // these are numbers.
	UT_uint32       m_currentFtrLastID;


	UT_Stack m_stateStack;
	RTFStateStore m_currentRTFState;

	UT_Vector m_fontTable;
	UT_Vector m_colourTable;
	UT_Vector m_hdrFtrTable;
	UT_Vector m_styleTable;

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
	UT_Vector m_vecAbiListTable;
	_rtfAbiListTable * getAbiList( UT_uint32 i) {return (_rtfAbiListTable *) m_vecAbiListTable.getNthItem(i);}

	RTF_msword97_listOverride* _getTableListOverride(UT_uint32 id);

	UT_uint32 m_numLists;
	bool m_bisAbiList;
	bool m_bisNOTList; // true if the current stream does not have  abi list extensions
	bool m_bParaHasRTFList;
	bool m_bParaHasRTFContinue;

	FILE* m_pImportFile;

	unsigned char *		m_pPasteBuffer;
	UT_uint32 			m_lenPasteBuffer;
	unsigned char *		m_pCurrentCharInPasteBuffer;
	PT_DocPosition		m_dposPaste;
	UT_uint32		    deflangid;
	UT_UCS4_mbtowc		m_mbtowc;
	bool                m_parsingHdrFtr;
	UT_uint32           m_icurOverride;
	UT_uint32           m_icurOverrideLevel;
	UT_Vector           m_vecWord97Lists;
	UT_Vector           m_vecWord97ListOverride;
	void _appendHdrFtr ();
	bool _appendField (const XML_Char *xmlField);
	XML_Char *_parseFldinstBlock (UT_ByteBuf & buf, XML_Char *xmlField, bool & isXML);
	bool                m_bAppendAnyway;
	RTFProps_SectionProps m_sectdProps ;
	ie_imp_table_control  m_TableControl;
	PL_StruxDocHandle     m_lastBlockSDH;
	PL_StruxDocHandle     m_lastCellSDH;
	bool                  m_bNestTableProps;
	bool                  m_bParaWrittenForSection;
};

#endif /* IE_IMP_RTF_H */










