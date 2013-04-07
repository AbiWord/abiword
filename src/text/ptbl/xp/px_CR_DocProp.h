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


#ifndef PX_CHANGERECORD_DOCPROP_H
#define PX_CHANGERECORD_DOCPROP_H

#include "ut_types.h"
#include "px_ChangeRecord.h"


class ABI_EXPORT PX_ChangeRecord_DocProp : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_DocProp(PXType type,
						  PT_DocPosition position,
						  PT_AttrPropIndex indexAP,
				UT_uint32 iXID);
	~PX_ChangeRecord_DocProp();

	virtual PX_ChangeRecord * reverse(void) const;
};

#endif /* PX_CHANGERECORD_DOCPROP_H */
