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


#ifndef PF_FRAG_TEXT_H
#define PF_FRAG_TEXT_H

#include "ut_types.h"
#include "pt_Types.h"
#include "pf_Frag.h"

/*!
 pf_Frag_Text represents a fragment of text in the document.
 Note that it does not contain a PT_DocPosition -- the fragment
 does not know where it is in the document; it only knows its
 buffer position.
*/

class pf_Frag_Text : public pf_Frag
{
public:
	pf_Frag_Text(pt_PieceTable * pPT,
				 PT_BufIndex bufIndex,
				 UT_uint32 length,
				 PT_AttrPropIndex indexAP,
                 fd_Field * m_pField);
	virtual ~pf_Frag_Text();
	
	virtual bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
													  PT_DocPosition dpos,
													  PT_BlockOffset blockOffset) const;
	virtual bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
													  PT_DocPosition dpos,
													  PT_BlockOffset blockOffset,
													  PT_BlockOffset startFragOffset,
													  PT_BlockOffset endFragOffset) const;
	inline PT_BufIndex getBufIndex(void) const
	{
	    return m_bufIndex;
	}
	inline PT_AttrPropIndex getIndexAP(void) const
	{
	    return m_indexAP;
	}

	void					setIndexAP(PT_AttrPropIndex indexNewAP);
	void					changeLength(UT_uint32 newLength);
	void					adjustOffsetLength(PT_BufIndex bi, UT_uint32 newLength);
    void                    setField(fd_Field * pField);
#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif

protected:
	PT_BufIndex				m_bufIndex;
	PT_AttrPropIndex		m_indexAP;
};

#endif /* PF_FRAG_TEXT_H */
