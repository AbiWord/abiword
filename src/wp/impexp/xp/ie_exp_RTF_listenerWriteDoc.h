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
						   UT_Bool bToClipboard);
	virtual ~s_RTF_ListenerWriteDoc();

	virtual UT_Bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual UT_Bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual UT_Bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual UT_Bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual UT_Bool		signal(UT_uint32 iSignal);

protected:
	void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_openSpan(PT_AttrPropIndex apiSpan);
	void				_openTag(const char * szPrefix, const char * szSuffix,
								 UT_Bool bNewLineAfter, PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	
	void				_rtf_docfmt(void);
	void				_rtf_open_section(PT_AttrPropIndex api);
	void				_rtf_open_block(PT_AttrPropIndex api);
	void				_writeImageInRTF(const PX_ChangeRecord_Object * pcro);

	PD_Document *		m_pDocument;
	IE_Exp_RTF *		m_pie;
	UT_Bool				m_bInSpan;
	UT_Bool				m_bJustStartingDoc;
	UT_Bool				m_bJustStartingSection;
	UT_Bool				m_bToClipboard;
	PT_AttrPropIndex	m_apiLastSpan;

	PT_AttrPropIndex	m_apiThisSection;
	PT_AttrPropIndex	m_apiThisBlock;
};

#endif /* IE_EXP_RTF_LISTENERWRITEDOC */
