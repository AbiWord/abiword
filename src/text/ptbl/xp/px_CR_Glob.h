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


#ifndef PX_CHANGERECORD_GLOB_H
#define PX_CHANGERECORD_GLOB_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// PX_ChangeRecord_Glob describes an insertSpan or
// deleteSpan change made to the document.
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


class PX_ChangeRecord_Glob : public PX_ChangeRecord
{
public:
	typedef enum _PXFlags { PXF_Null=				0x00,
							PXF_MultiStepStart=		0x01, /* display-atomic */
							PXF_MultiStepEnd=		0x02,
							PXF_UserAtomicStart=	0x04, /* user-level-atomic */
							PXF_UserAtomicEnd=		0x08 } PXFlags;

	PX_ChangeRecord_Glob(PXType type,
						 UT_Byte flags);
	~PX_ChangeRecord_Glob();

	virtual PX_ChangeRecord * reverse(void) const;

	UT_Byte					getFlags(void) const;
	UT_Byte					getRevFlags(void) const;
	
protected:
	UT_Byte					m_flags;			/* see PXFlags above */
};

#endif /* PX_CHANGERECORD_GLOB_H */
