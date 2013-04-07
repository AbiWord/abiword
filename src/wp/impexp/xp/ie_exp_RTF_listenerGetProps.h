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

#ifndef IE_EXP_RTF_LISTENERGETPROPS
#define IE_EXP_RTF_LISTENERGETPROPS
#include "ie_exp_RTF.h"

/******************************************************************
** This class is to be considered private to ie_exp_RTF.cpp
** This is a PL_Listener.  It's purpose is to gather information
** from the document (the font table and color table and anything
** else) that must be written to the rtf header.
******************************************************************/

class ABI_EXPORT s_RTF_ListenerGetProps : public PL_Listener
{
public:
	s_RTF_ListenerGetProps(PD_Document * pDocument,
						   IE_Exp_RTF * pie);
	virtual ~s_RTF_ListenerGetProps();

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
	bool				hasBlock() { return m_bHasBlock; }

protected:
	void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_openSpan(PT_AttrPropIndex apiSpan);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_compute_span_properties(const PP_AttrProp * pSpanAP,
												 const PP_AttrProp * pBlockAP,
												 const PP_AttrProp * pSectionAP);

	void                _check_revs_for_color(const PP_AttrProp * pAP1,
											  const PP_AttrProp * pAP2,
											  const PP_AttrProp * pAP3);

	void                _check_revs_for_font (const PP_AttrProp * pAP1,
											  const PP_AttrProp * pAP2,
											  const PP_AttrProp * pAP3);

	void                _searchTableAPI(PT_AttrPropIndex api);
	void                _searchCellAPI(PT_AttrPropIndex api);
 private:
	PD_Document *		m_pDocument;
	IE_Exp_RTF *		m_pie;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	PT_AttrPropIndex	m_apiLastSpan;

	PT_AttrPropIndex	m_apiThisSection;
	PT_AttrPropIndex	m_apiThisBlock;
	PT_AttrPropIndex	m_apiSavedBlock;
	/*! true if we have a multi-block paste. */
	bool				m_bHasBlock;
};

#endif /* IE_EXP_RTF_LISTENERGETPROPS */
