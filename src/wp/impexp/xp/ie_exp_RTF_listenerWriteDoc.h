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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef IE_EXP_RTF_LISTENERWRITEDOC
#define IE_EXP_RTF_LISTENERWRITEDOC
#include "ie_exp_RTF.h"
#include "ut_wctomb.h"
#include "ut_stack.h"
#include "ie_Table.h"
#include "pp_PropertyMap.h"

#include <list>
#include <string>

class PX_ChangeRecord_Object;

/******************************************************************
** This file is considered private to ie_exp_RTF.cpp
** This is a PL_Listener.  It's purpose is to actually write
** the contents of the document to the RTF file.
******************************************************************/

class ABI_EXPORT s_RTF_ListenerWriteDoc : public PL_Listener
{
	friend class IE_Exp_RTF;
public:
	s_RTF_ListenerWriteDoc(PD_Document * pDocument,
						   IE_Exp_RTF * pie,
						   bool bToClipboard,
						   bool bHasMultiBlock);
	virtual ~s_RTF_ListenerWriteDoc();

	virtual bool		populate(fl_ContainerLayout* sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord * pcr,
									  fl_ContainerLayout* * psfh);

	virtual bool		change(fl_ContainerLayout* sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(fl_ContainerLayout* sfh,
									const PX_ChangeRecord * pcr,
									pf_Frag_Strux* sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
															PL_ListenerId lid,
															fl_ContainerLayout* sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

protected:
	UT_sint32            getRightOfCell(UT_sint32 row,UT_sint32 col);
	void				_closeSection(void);
	void				_closeBlock(PT_AttrPropIndex nextApi = 0);
	void				_closeSpan();
	void                _openFrame(PT_AttrPropIndex api);
	void                _closeFrame(void);
	void                _writeSPNumProp(const char * prop, UT_sint32 val);
	void				_openSpan(PT_AttrPropIndex apiSpan,const PP_AttrProp * pSpanAP = NULL );
	void				_openTag(const char * szPrefix, const char * szSuffix,
								 bool bNewLineAfter, PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length, PT_DocPosition pos, bool bIgnorePos);
	void                _writeTOC(PT_AttrPropIndex api);
	bool                _isListBlock(void) const { return m_bIsListBlock;}
	bool                _isTabEaten(void) const { return m_bIsTabEaten;}
	void                _setListBlock( bool bListBlock)
		                             { m_bIsListBlock = bListBlock;}
	void                _setTabEaten( bool bTabEaten)
		                             { m_bIsTabEaten = bTabEaten;}
	void                            _rtf_info (void);
	void				_rtf_docfmt(void);
	void				_rtf_open_section(PT_AttrPropIndex api);
	void				_rtf_open_block(PT_AttrPropIndex api);
	void				_writeImageInRTF(const PX_ChangeRecord_Object * pcro);
	void                _writeBookmark(const PX_ChangeRecord_Object * pcro);
	void                _writeRDFAnchor(const PX_ChangeRecord_Object * pcro);
	void                _writeHyperlink(const PX_ChangeRecord_Object * pcro);
    void                _writeAnnotation(const PX_ChangeRecord_Object * pcro);
    void                _writeFieldPreamble(const PP_AttrProp * pSpanAP);
	void				_writeEmbedData (const std::string & Name, const UT_ByteBuf * pbb, const std::string & mime_type);
	const UT_UCSChar *  _getFieldValue(void);
	void                _writeFieldTrailer(void);
	void                _close_cell(void);
	void                _close_table(void);
	void                _open_cell(PT_AttrPropIndex api);
	void                _open_table(PT_AttrPropIndex api, bool bIsCell = false);
	void                _export_AbiWord_Table_props(PT_AttrPropIndex api);
	void                _fillTableProps(PT_AttrPropIndex api, UT_String & sTableProps);
	void                _export_AbiWord_Cell_props(PT_AttrPropIndex api,bool bFill);
	void                _fillCellProps(PT_AttrPropIndex api, UT_String & sCellProps);
	void                _exportCellProps(PT_AttrPropIndex  api, UT_String & sTableProps);
	void                _exportTableProps(PT_AttrPropIndex  api);
	void                _getPropString(const UT_String sPropString, const char * szProp, UT_String & sVal);
	void                _newRow(void);
	void                _outputTableBorders(UT_sint32 iThick);
	void                _outputCellBorders(UT_sint32 iThick);
	double              _getColumnWidthInches(void);
 private:
	PD_Document *		m_pDocument;
	IE_Exp_RTF *		m_pie;
	bool				m_bInSpan;
	bool                m_bInBlock;
	bool				m_bJustStartingDoc;
	bool				m_bJustStartingSection;
	bool				m_bToClipboard;
	PT_AttrPropIndex	m_apiLastSpan;
	bool                m_bIsListBlock;

	bool                m_bIsTabEaten;
	PT_AttrPropIndex	m_apiThisSection;
	PT_AttrPropIndex	m_apiThisBlock;
	PT_AttrPropIndex	m_apiThisFrame;
	bool                m_bInFrame;
	bool                m_bJustOpennedFrame;
	UT_Wctomb		    m_wctomb;
	pf_Frag_Strux*   m_sdh;
	UT_uint32           m_currID;
	PT_DocPosition      m_posDoc;
	bool                m_bBlankLine;
	bool                m_bStartedList;
	ie_Table            m_Table;
	bool                m_bNewTable;
	UT_sint32           m_iCurRow;
	UT_sint32           m_iLeft;
	UT_sint32           m_iRight;
	UT_sint32           m_iTop;
	UT_sint32           m_iBot;
	PT_AttrPropIndex	m_apiSavedBlock;
	pf_Frag_Strux*   m_sdhSavedBlock;
	bool                m_bOpennedFootnote;
	PP_PropertyMap::TypeLineStyle    m_LastLinestyle;
	UT_String           m_sLastColor;
	UT_sint32           m_iFirstTop;
	bool                m_bHyperLinkOpen;
	bool                m_bRDFAnchorOpen;
	bool                m_bOpenBlockForSpan;
	bool                m_bTextBox;
	//
	bool                m_bAnnotationOpen;
	UT_uint32           m_iAnnotationNumber;
	UT_ByteBuf *        m_pAnnContent;
	UT_ByteBuf *        m_pSavedBuf;
	UT_UTF8String       m_sAnnTitle;
	UT_UTF8String       m_sAnnAuthor;
	UT_UTF8String       m_sAnnDate;

    std::list< std::string > m_rdfAnchorStack;
};

#endif /* IE_EXP_RTF_LISTENERWRITEDOC */



