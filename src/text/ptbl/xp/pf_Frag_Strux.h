 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef PF_FRAG_STRUX_H
#define PF_FRAG_STRUX_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pt_Types.h"
#include "pd_Document.h"

// pf_Frag_Strux represents structure information (such as a
// paragraph or section) in the document.
//
// pf_Frag_Strux is descended from pf_Frag, but is a base
// class for _Section, etc.
// We use an enum to remember type, rather than use any of the
// run-time stuff.

class pf_Frag_Strux : public pf_Frag
{
public:
	pf_Frag_Strux(pt_PieceTable * pPT,
				  PTStruxType struxType,
				  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux();

	PTStruxType				getStruxType(void) const;
	PL_StruxFmtHandle		getFmtHandle(PL_ListenerId lid) const;
	UT_Bool					setFmtHandle(PL_ListenerId lid, PL_StruxFmtHandle sfh);
	
	virtual UT_Bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr) const;
	PT_AttrPropIndex		getIndexAP(void) const;
	void					setIndexAP(PT_AttrPropIndex indexNewAP);

	virtual void			dump(FILE * fp) const = 0;
	
protected:
	PTStruxType				m_struxType;
	PT_AttrPropIndex		m_indexAP;
	UT_Vector				m_vecFmtHandle;
};

#endif /* PF_FRAG_STRUX_H */
