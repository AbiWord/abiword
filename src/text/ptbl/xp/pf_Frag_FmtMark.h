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


#ifndef PF_FRAG_FMTMARK_H
#define PF_FRAG_FMTMARK_H

#include "ut_types.h"
#include "pt_Types.h"
#include "pf_Frag.h"

/*!
 pf_Frag_FmtMark represents a zero-length place-holder fragment
 in the document.  It is used to hold a formating information
 in anticipation of future text.  That is, we use it when
 the user does a 'toggle bold' or similar and before any text has
 been entered.  As soon as text is entered we will remove this
 marker (we may also remove it if the user moves the cursor
 away before actually entering any text).
*/

class pf_Frag_FmtMark : public pf_Frag
{
public:
	pf_Frag_FmtMark(pt_PieceTable * pPT,
					PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_FmtMark();
	
	virtual UT_Bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
													  PT_DocPosition dpos,
													  PT_BlockOffset blockOffset) const;
	PT_AttrPropIndex		getIndexAP(void) const;
	void					setIndexAP(PT_AttrPropIndex indexNewAP);
	
#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif

protected:
	PT_AttrPropIndex		m_indexAP;
};

#endif /* PF_FRAG_FMTMARK_H */
