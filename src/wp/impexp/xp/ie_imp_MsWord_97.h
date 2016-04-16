/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <dominicl@seas.upenn.edu>
 * Copyright (C) 2001-2003 Tomas Frydrych
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef IE_IMP_MSWORD_H
#define IE_IMP_MSWORD_H

// The importer/reader for Microsoft Word Documents

#include "ie_imp.h"
#include "ut_string_class.h"
#include "fl_DocLayout.h"
#include "fl_AutoLists.h"
#include "ut_units.h"
//
// forward decls so that we don't have to #include "wv.h" here
//
typedef struct _wvParseStruct wvParseStruct;
typedef struct _Blip Blip;
typedef struct _CHP CHP;
typedef struct _PAP PAP;
class PD_Document;
class pf_Frag;
class UT_Stack;

struct field;

struct bookmark
{
	gchar * name;
	UT_uint32  pos;
	bool	   start;
};

struct footnote
{
	UT_uint32  type;
	UT_uint32  ref_pos;
	UT_uint32  txt_pos;
	UT_uint32  txt_len;
	UT_uint32  pid;
};


struct textbox
{
	UT_uint32  lid;
	UT_uint32  ref_pos;
	UT_uint32  txt_pos;
	UT_uint32  txt_len;
	UT_sint32  iLeft;
	UT_sint32  iWidth;
	UT_sint32  iTop;
	UT_sint32  iHeight;
	UT_sint32  iPosType;
	UT_sint32  iBorderWidth;
};

struct textboxPos
{
	UT_uint32 lid;
	pf_Frag * endFrame;
};

typedef enum
	{
		HF_HeaderFirst = 0,
		HF_FooterFirst,
		HF_HeaderOdd,
		HF_FooterOdd,
		HF_HeaderEven,
		HF_FooterEven,
		HF_Unsupported
	}_headerTypes;


struct header
{
	_headerTypes type;
	UT_uint32    pos;
	UT_uint32    len;
	UT_uint32    pid;

	struct _d
	{
		UT_Vector hdr;
		UT_Vector frag;
	}d;
};

class ABI_EXPORT MsColSpan
{
public:
	MsColSpan(void):iLeft(0),iRight(0),width(0){}
	virtual ~MsColSpan(void) {}
	UT_sint32 iLeft;
	UT_sint32 iRight;
	UT_sint32 width;
};

class ABI_EXPORT emObject
{
public:
	UT_String props1;
	UT_String props2;
	PTObjectType objType;
};

//
// The Sniffer/Manager/Creator Class for DOC
//
class ABI_EXPORT IE_Imp_MsWord_97_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_MsWord_97_Sniffer();
	virtual ~IE_Imp_MsWord_97_Sniffer() {}

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence ();
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
	virtual UT_Confidence_t recognizeContents (GsfInput * input);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);
};

// how many chars to buffer in our fields implementation
#define FLD_SIZE 40000

//
// The import class for the MSFT Word DOC format
//
class ABI_EXPORT IE_Imp_MsWord_97 : public IE_Imp
{
public:
	IE_Imp_MsWord_97 (PD_Document * pDocument);
	~IE_Imp_MsWord_97 ();

	virtual bool        supportsLoadStylesOnly() {return true;}

	// wv's callbacks need access to these, so they have to be public
	int 			_specCharProc (wvParseStruct *ps, UT_uint16 eachchar,
								   CHP * achp);
	int 			_charProc (wvParseStruct *ps, UT_uint16 eachchar,
							   UT_Byte chartype,  UT_uint16 lid);
	int 			_docProc  (wvParseStruct *ps, UT_uint32 tag);
	int 			_eleProc  (wvParseStruct *ps, UT_uint32 tag,
							   void *props, int dirty);

protected:

	UT_Error			_loadFile (GsfInput * input);

private:

	void       _handleMetaData(wvParseStruct *ps);

	int 	   _beginSect (wvParseStruct *ps, UT_uint32 tag,
						   void *props, int dirty);
	int 	   _endSect (wvParseStruct *ps, UT_uint32 tag,
						 void *props, int dirty);

	int 	   _beginPara (wvParseStruct *ps, UT_uint32 tag,
						   void *props, int dirty);
	int 	   _endPara (wvParseStruct *ps, UT_uint32 tag,
						 void *props, int dirty);

	int 	   _beginChar (wvParseStruct *ps, UT_uint32 tag,
						   void *props, int dirty);
	int 	   _endChar (wvParseStruct *ps, UT_uint32 tag,
						 void *props, int dirty);
	int 	   _beginComment (wvParseStruct *ps, UT_uint32 tag,
						   void *props, int dirty);
	int 	   _endComment (wvParseStruct *ps, UT_uint32 tag,
						 void *props, int dirty);
	gchar * _getBookmarkName(const wvParseStruct * ps, UT_uint32 pos);
	bool	   _insertBookmarkIfAppropriate(UT_uint32 iPos);
	bool	   _insertBookmark(bookmark * bm);
	UT_Error   _handleImage (Blip *, long width, long height, long cropt, long cropb, long cropl, long cropr);
	UT_Error   _handlePositionedImage (Blip *, UT_String & sImageName);
	bool	   _handleCommandField (char *command);
	bool	   _handleFieldEnd (char * command, UT_uint32 iPos);
	int 	   _fieldProc (wvParseStruct *ps, UT_uint16 eachchar,
						   UT_Byte chartype, UT_uint16 lid);
	void	   _appendChar (UT_UCSChar ch);
	void	   _flush ();

	void		_table_open();
	void		_table_close(const wvParseStruct *ps, const PAP *apap);
	void		_row_open(const wvParseStruct *ps);
	void		_row_close();
	void		_cell_open(const wvParseStruct *ps, const PAP *apap);
	void		_cell_close();
	void        _handleStyleSheet(const wvParseStruct *ps);
	void        _generateCharProps(UT_String &s, const CHP * achp, wvParseStruct *ps);
	void        _generateParaProps(UT_String &s, const PAP * apap, wvParseStruct *ps);
	int         _handleBookmarks(const wvParseStruct *ps);
	void        _handleNotes(const wvParseStruct *ps);
	void        _handleTextBoxes(const wvParseStruct *ps);
	bool        _insertNoteIfAppropriate(UT_uint32 iDocPosition,UT_UCS4Char c);
	bool        _insertFootnote(const footnote * f, UT_UCS4Char c);
	bool        _insertEndnote(const footnote * f, UT_UCS4Char c);
	bool        _handleNotesText(UT_uint32 iPos);
	bool        _handleTextboxesText(UT_uint32 iPos);
	bool        _findNextTextboxSection();
	bool        _findNextFNoteSection();
	bool        _findNextENoteSection();
	bool        _shouldUseInsert()const;
	bool        _ensureInBlock();
	bool        _appendStrux(PTStruxType pts, const PP_PropertyVector & attributes);
	bool        _appendObject(PTObjectType pto, const PP_PropertyVector & attributes);
	bool        _appendSpan(const UT_UCSChar * p, UT_uint32 length);
	bool        _appendStruxHdrFtr(PTStruxType pts, const PP_PropertyVector & attributes);
	bool        _appendObjectHdrFtr(PTObjectType pto, const PP_PropertyVector & attributes);
	bool        _appendSpanHdrFtr(const UT_UCSChar * p, UT_uint32 length);
	bool		_appendFmt(const PP_PropertyVector & attributes);
	void        _handleHeaders(const wvParseStruct *ps);
	bool        _handleHeadersText(UT_uint32 iPos, bool bDoBlockIns);
	bool        _insertHeaderSection(bool bDoBlockIns);
	bool        _build_ColumnWidths(UT_NumberVector & colWidths);
	bool        _isVectorFull(UT_NumberVector & vec);
	void        setNumberVector(UT_NumberVector & vec, UT_sint32 i, UT_sint32 val);
	bool        findMatchSpan(UT_sint32 iLeft,UT_sint32 iRight);
	bool        _ignorePosition(UT_uint32 pos);

	bool        _isTOCsupported(field *f);
	bool        _insertTOC(field *f);


	UT_UCS4String		m_pTextRun;
	//UT_uint32			m_iImageCount;
	UT_uint32			m_nSections;
	bool				m_bSetPageSize;
#if 0
	UT_UCS2Char m_command [FLD_SIZE];
	UT_UCS2Char m_argument [FLD_SIZE];
	UT_UCS2Char *m_fieldWhich;
	UT_sint32	m_fieldI;
	char *		m_fieldC;
	UT_sint32	m_fieldRet;
	UT_sint32	m_fieldDepth;
#else
	UT_Stack    m_stackField;
#endif
	//char *	  m_fieldA;
	bool	   m_bIsLower;

	bool m_bInSect;
	bool m_bInPara;
	bool m_bLTRCharContext;
	bool m_bLTRParaContext;
	UT_BidiCharType  m_iOverrideIssued;
	bool m_bBidiMode;
	bool m_bInLink;
	bookmark * m_pBookmarks;
	UT_uint32  m_iBookmarksCount;
	footnote * m_pFootnotes;
	UT_uint32  m_iFootnotesCount;
	footnote * m_pEndnotes;
	UT_uint32  m_iEndnotesCount;
	textbox *  m_pTextboxes;
	UT_sint32  m_iTextboxCount;
	UT_Vector  m_vLists;
	UT_uint32  m_iListIdIncrement[9];
	UT_uint32  m_iMSWordListId;

	bool m_bEncounteredRevision;
	bool		m_bInTable;						// are we in a table ?
	int			m_iRowsRemaining;				// number of rows left to process
	int			m_iCellsRemaining;				// number of cells left to process in the current row
	int			m_iCurrentRow;					//
	int			m_iCurrentCell;					//
	bool		m_bRowOpen;						// row strux open ?
	bool		m_bCellOpen;					// cell strux open ?
	UT_NumberVector	m_vecColumnSpansForCurrentRow;	// placeholder for horizontal cell spans
	UT_GenericVector<MsColSpan *>	m_vecColumnWidths;
	UT_GenericVector<emObject*>   m_vecEmObjects;               // Objects between cell
											  // struxes
	UT_NumberVector m_vecColumnPositions;
	UT_String   m_charProps;
	UT_String   m_charRevs;
	UT_String   m_charStyle;
	UT_String   m_paraProps;
	UT_String   m_paraStyle;

	UT_uint32   m_iFootnotesStart;
	UT_uint32   m_iFootnotesEnd;
	UT_uint32   m_iEndnotesStart;
	UT_uint32   m_iEndnotesEnd;
	UT_uint32   m_iNextFNote;
	UT_uint32   m_iNextENote;
	bool        m_bInFNotes;
	bool        m_bInENotes;
	pf_Frag *   m_pNotesEndSection;
	header *    m_pHeaders;
	UT_uint32   m_iHeadersCount;
	UT_uint32   m_iHeadersStart;
	UT_uint32   m_iHeadersEnd;
	UT_uint32   m_iCurrentHeader;
	bool        m_bInHeaders;
	UT_uint32   m_iCurrentSectId;
	UT_uint32   m_iAnnotationsStart;
	UT_uint32   m_iAnnotationsEnd;
	UT_uint32   m_iMacrosStart;
	UT_uint32   m_iMacrosEnd;
	UT_uint32   m_iTextStart;
	UT_uint32   m_iTextEnd;
	bool        m_bPageBreakPending;
	bool        m_bLineBreakPending;
	UT_NumberVector m_vListIdMap;
	bool        m_bSymbolFont;
	UT_Dimension m_dim;
	UT_sint32    m_iLeft;
	UT_sint32    m_iRight;
	UT_uint32    m_iTextboxesStart;
	UT_uint32    m_iTextboxesEnd;
	UT_sint32    m_iNextTextbox;
	UT_uint32    m_iPrevHeaderPosition;
	bool         m_bEvenOddHeaders;

	UT_sint32    m_bInTOC;
	bool         m_bTOCsupported;
	bool         m_bInTextboxes;
	pf_Frag *    m_pTextboxEndSection;
	UT_GenericVector<textboxPos *> m_vecTextboxPos;
	UT_sint32    m_iLeftCellPos;
	UT_uint32    m_iLastAppendedHeader;
};

#endif /* IE_IMP_MSWORD_H */
