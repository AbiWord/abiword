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


#ifndef PF_FRAG_TEXT_H
#define PF_FRAG_TEXT_H

#include "ut_types.h"
#include "pt_Types.h"
#include "pf_Frag.h"
#include <string>

/*!
 pf_Frag_Text represents a fragment of text in the document.
 Note that it does not contain a PT_DocPosition -- the fragment
 does not know where it is in the document; it only knows its
 buffer position.
*/

class ABI_EXPORT pf_Frag_Text : public pf_Frag
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

	void					changeLength(UT_uint32 newLength);
	void					adjustOffsetLength(PT_BufIndex bi, UT_uint32 newLength);
    void                    setField(fd_Field * pField);

	virtual bool            usesXID() const {return false;}

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif

    std::string            toString() const;

protected:
	virtual bool            _isContentEqual(const pf_Frag & f2) const;
	PT_BufIndex				m_bufIndex;
};

#endif /* PF_FRAG_TEXT_H */
