/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
				  UT_uint32 length,
				  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux();

	PTStruxType				getStruxType(void) const;
	PL_StruxFmtHandle		getFmtHandle(PL_ListenerId lid) const;
	UT_Bool					setFmtHandle(PL_ListenerId lid, PL_StruxFmtHandle sfh);
	
	virtual UT_Bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
													  PT_DocPosition dpos) const;
	PT_AttrPropIndex		getIndexAP(void) const;
	void					setIndexAP(PT_AttrPropIndex indexNewAP);

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const = 0;
#endif
	
protected:
	PTStruxType				m_struxType;
	PT_AttrPropIndex		m_indexAP;
	UT_Vector				m_vecFmtHandle;
};

#endif /* PF_FRAG_STRUX_H */
