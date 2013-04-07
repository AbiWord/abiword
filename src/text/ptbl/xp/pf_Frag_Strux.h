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


#ifndef PF_FRAG_STRUX_H
#define PF_FRAG_STRUX_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pt_Types.h"
#include "pd_Document.h"

class fl_ContainerLayout;

/*!
 pf_Frag_Strux represents structure information (such as a
 paragraph or section) in the document.

 pf_Frag_Strux is descended from pf_Frag, but is a base
 class for pf_Frag_Strux_Block and pf_Frag_Strux_Section.

 We use an enum to remember type, rather than use any of the
 run-time stuff.
*/

class ABI_EXPORT pf_Frag_Strux : public pf_Frag
{
public:
	pf_Frag_Strux(pt_PieceTable * pPT,
				  PTStruxType struxType,
				  UT_uint32 length,
				  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux();

	PTStruxType				getStruxType(void) const;
	fl_ContainerLayout*		getFmtHandle(PL_ListenerId lid) const;
	bool					setFmtHandle(PL_ListenerId lid, fl_ContainerLayout* sfh);
	void                    clearAllFmtHandles() {m_vecFmtHandle.clear();}

	virtual bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
													  PT_DocPosition dpos) const;

	virtual bool            usesXID() const;
	bool                    isMatchingType(PTStruxType e) const;
	bool                    isMatchingType(const pf_Frag * p) const;

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const = 0;
#endif

protected:

	virtual bool            _isContentEqual(const pf_Frag &f2) const;
	PTStruxType				m_struxType;
	UT_GenericVector<fl_ContainerLayout*>	m_vecFmtHandle;
};

#endif /* PF_FRAG_STRUX_H */
