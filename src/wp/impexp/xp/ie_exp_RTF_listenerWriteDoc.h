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

#ifndef IE_EXP_RTF_LISTENERWRITEDOC
#define IE_EXP_RTF_LISTENERWRITEDOC
#include "ie_exp_RTF.h"
#include "ut_wctomb.h"
class PX_ChangeRecord_Object;

/******************************************************************
** This file is considered private to ie_exp_RTF.cpp
** This is a PL_Listener.  It's purpose is to actually write
** the contents of the document to the RTF file.
******************************************************************/

class s_RTF_ListenerWriteDoc : public PL_Listener
{
public:
	s_RTF_ListenerWriteDoc(PD_Document * pDocument,
						   IE_Exp_RTF * pie,
						   bool bToClipboard);
	virtual ~s_RTF_ListenerWriteDoc();

	virtual bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

protected:
	void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_openSpan(PT_AttrPropIndex apiSpan);
	void				_openTag(const char * szPrefix, const char * szSuffix,
								 bool bNewLineAfter, PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	bool                _isListBlock(void) const { return m_bIsListBlock;}
	bool                _isTabEaten(void) const { return m_bIsTabEaten;}
	void                _setListBlock( bool bListBlock) 
		                             { m_bIsListBlock = bListBlock;}
	void                _setTabEaten( bool bTabEaten)
		                             { m_bIsTabEaten = bTabEaten;} 
	void				_rtf_docfmt(void);
	void				_rtf_open_section(PT_AttrPropIndex api);
	void				_rtf_open_block(PT_AttrPropIndex api);
	void				_writeImageInRTF(const PX_ChangeRecord_Object * pcro);

	PD_Document *		m_pDocument;
	IE_Exp_RTF *		m_pie;
	bool				m_bInSpan;
	bool				m_bJustStartingDoc;
	bool				m_bJustStartingSection;
	bool				m_bToClipboard;
	PT_AttrPropIndex	m_apiLastSpan;
	bool                m_bIsListBlock;
	bool                m_bIsTabEaten;
	PT_AttrPropIndex	m_apiThisSection;
	PT_AttrPropIndex	m_apiThisBlock;
	UT_Wctomb		m_wctomb;
	PL_StruxDocHandle       m_sdh;
};

#endif /* IE_EXP_RTF_LISTENERWRITEDOC */



