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

#ifndef IE_EXP_WML_H
#define IE_EXP_WML_H

#include "ie_exp.h"
#include "ie_Table.h"
#include "pl_Listener.h"
#include "pp_AttrProp.h"

class PD_Document;
class s_WML_Listener;

// the exporter/writer for WML 1.1

class IE_Exp_WML_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_WML_Sniffer (const char * name);
	virtual ~IE_Exp_WML_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

class IE_Exp_WML : public IE_Exp
{
public:
	IE_Exp_WML(PD_Document *pDocument);
	virtual ~IE_Exp_WML();

protected:
	virtual UT_Error	_writeDocument(void);

 private:
	s_WML_Listener *	m_pListener;
};

class IE_TOCHelper;
class s_WML_Listener : public PL_Listener
{
public:
	s_WML_Listener(PD_Document * pDocument,
		       IE_Exp_WML * pie);
	virtual ~s_WML_Listener();

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
	void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_closeAnchor(void);
	void				_closeHyperlink(void);
	void				_openParagraph(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api);
	void				_openSpan(PT_AttrPropIndex api);

	void				_openTable(PT_AttrPropIndex api);
	void				_closeTable(void);
	void				_openCell(void);
	void				_openRow(void);
	void				_closeCell(void);
	void				_closeRow(void);

	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_outputDataUnchecked(const UT_UCSChar * p, UT_uint32 length);
	void				_handleDataItems(void);
	void				_handleBookmark(PT_AttrPropIndex api);
	void				_handleEmbedded(PT_AttrPropIndex api);
	void				_handleField(const PX_ChangeRecord_Object * pcro, PT_AttrPropIndex api);
	void				_handleHyperlink(PT_AttrPropIndex api);
	void				_handleImage(PT_AttrPropIndex api, bool bPos = false);
	void				_handleMath(PT_AttrPropIndex api);
	void				_handleMetaData(void);

	void				_emitTOC (PT_AttrPropIndex api);
	bool				_styleDescendsFrom(const char * style_name, const char * base_name);

	PD_Document *		m_pDocument;
	IE_Exp_WML *		m_pie;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	bool				m_bInAnchor;
	bool				m_bInHyperlink;
	bool				m_bInCell;
	bool				m_bInRow;
	bool				m_bInTable;
	bool				m_bPendingClose;
	bool				m_bWasSpace;
	UT_uint32			m_iCards;
	UT_uint32			m_iTableDepth;

	const PP_AttrProp*	m_pAP_Span;

	UT_Vector			m_utvDataIDs;	// list of data ids for image enumeration
	ie_Table			mTableHelper;
	IE_TOCHelper *		m_toc;
	UT_uint32			m_heading_count;
};

#endif /* IE_EXP_WML_H */
