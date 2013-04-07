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


#ifndef PX_CHANGERECORD_DATAITEM_H
#define PX_CHANGERECORD_DATAITEM_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// PX_ChangeRecord_Strux describes an insertStrux or
// deleteStrux change made to the document.
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


class ABI_EXPORT PX_ChangeRecord_DataItem : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_DataItem(PXType type,
						  PT_DocPosition position,
						  PT_AttrPropIndex indexAP,
				 UT_uint32 iXID);

	~PX_ChangeRecord_DataItem();

	virtual PX_ChangeRecord * reverse(void) const;
};

#endif /* PX_CHANGERECORD_DATAITEM_H */
