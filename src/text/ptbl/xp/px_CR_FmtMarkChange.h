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


#ifndef PX_CHANGERECORD_FMTMARKCHANGE_H
#define PX_CHANGERECORD_FMTMARKCHANGE_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// PX_ChangeRecord_FmtMarkChange describes a PXT_ChangeFmtMark
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
// the FmtMark at the time the change was made.


class ABI_EXPORT PX_ChangeRecord_FmtMarkChange : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_FmtMarkChange(PXType type,
				      PT_DocPosition position,
				      PT_AttrPropIndex indexOldAP,
				      PT_AttrPropIndex indexNewAP,
				      PT_BlockOffset blockOffset);
	~PX_ChangeRecord_FmtMarkChange();

	virtual PX_ChangeRecord * reverse(void) const;
	PT_AttrPropIndex		getOldIndexAP(void) const;
	PT_BlockOffset			getBlockOffset(void) const;
	void                      AdjustBlockOffset(PT_BlockOffset iBlk) const
	{ m_blockOffset = iBlk;}

protected:
	PT_AttrPropIndex		m_indexOldAP;
mutable	PT_BlockOffset			m_blockOffset; /* offset of span from beginning of paragraph */
};

#endif /* PX_CHANGERECORD_FMTMARKCHANGE_H */
