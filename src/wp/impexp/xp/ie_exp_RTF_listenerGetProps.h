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

#ifndef IE_EXP_RTF_LISTENERGETPROPS
#define IE_EXP_RTF_LISTENERGETPROPS
#include "ie_exp_RTF.h"

/******************************************************************
** This class is to be considered private to ie_exp_RTF.cpp
** This is a PL_Listener.  It's purpose is to gather information
** from the document (the font table and color table and anything
** else) that must be written to the rtf header.
******************************************************************/

class s_RTF_ListenerGetProps : public PL_Listener
{
public:
	s_RTF_ListenerGetProps(PD_Document * pDocument,
						   IE_Exp_RTF * pie);
	virtual ~s_RTF_ListenerGetProps();

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
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_compute_span_properties(const PP_AttrProp * pSpanAP,
												 const PP_AttrProp * pBlockAP,
												 const PP_AttrProp * pSectionAP);
	
	PD_Document *		m_pDocument;
	IE_Exp_RTF *		m_pie;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	PT_AttrPropIndex	m_apiLastSpan;

	PT_AttrPropIndex	m_apiThisSection;
	PT_AttrPropIndex	m_apiThisBlock;
};

#endif /* IE_EXP_RTF_LISTENERGETPROPS */
