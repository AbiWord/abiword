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


#ifndef PX_CHANGERECORD_SPANCHANGE_H
#define PX_CHANGERECORD_SPANCHANGE_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// PX_ChangeRecord_SpanChange describes a PXT_ChangeSpan
// made to the document (a formatting change).
// This description should be sufficient to allow undo to
// work and sufficient to allow the formatter to do a
// partial format and screen update (if appropriate).
// The change record must be free of pointers, since it
// represents what was done to the document -- and not
// how it was done (that is, not what was done to various
// intermediate data structures).  this also lets it be
// cached to disk (for autosave and maybe multi-session
// undo).
//
// m_position contains the absolute document position of
// the text span at the time the change was made.
// m_bufIndex,m_length describe the actual contents
// of the text span.


class PX_ChangeRecord_SpanChange : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_SpanChange(PXType type,
							   PT_DocPosition position,
							   PT_AttrPropIndex indexOldAP,
							   PT_AttrPropIndex indexNewAP,
							   PTChangeFmt ptc,
							   PT_BufIndex bufIndex,
							   UT_uint32 length);
	~PX_ChangeRecord_SpanChange();

	virtual PX_ChangeRecord * reverse(void) const;

	UT_uint32				getLength(void) const;
	PT_BufIndex				getBufIndex(void) const;
	PT_AttrPropIndex		getOldIndexAP(void) const;
	
protected:
	PTChangeFmt				m_ptc;
	PT_BufIndex				m_bufIndex;	/* bufIndex to our text */
	UT_uint32				m_length;	/* length of our text */
	PT_AttrPropIndex		m_indexOldAP;
};

#endif /* PX_CHANGERECORD_SPANCHANGE_H */
