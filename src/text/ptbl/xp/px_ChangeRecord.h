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


#ifndef PX_CHANGERECORD_H
#define PX_CHANGERECORD_H

#include "ut_types.h"
#include "pt_Types.h"
#include "pd_Document.h"

// PX_ChangeRecord describes a change made to the document.
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
// PX_ChangeRecord is an abstract base class.
// We use an enum to remember type, rather than use any of
// the run-time stuff.

class PX_ChangeRecord
{
public:
	typedef enum _PXType { PXT_GlobMarker=-1,
						   PXT_InsertSpan=0, 		PXT_DeleteSpan=1,		PXT_ChangeSpan=2,
						   PXT_InsertStrux=3,		PXT_DeleteStrux=4,		PXT_ChangeStrux=5,
						   PXT_InsertObject=6,		PXT_DeleteObject=7,		PXT_ChangeObject=8,
						   PXT_InsertFmtMark=9,		PXT_DeleteFmtMark=10,	PXT_ChangeFmtMark=11,
			       PXT_ChangePoint=12,  PXT_ListUpdate=13, 
			       PXT_StopList=14,  PXT_DontChangeInsPoint=15,
			       PXT_AllowChangeInsPoint=16, PXT_UpdateField=17,
                               PXT_RemoveList=18
	} PXType;

	PX_ChangeRecord(PXType type,
					PT_DocPosition position,
					PT_AttrPropIndex indexNewAP);

	virtual ~PX_ChangeRecord();

	PXType					getType(void) const;
	PT_DocPosition			getPosition(void) const;
	PT_AttrPropIndex		getIndexAP(void) const;
	UT_Bool 				getPersistance(void) const;

	inline void				setPersistance(UT_Bool persistant) { m_persistant = persistant; }

	virtual PX_ChangeRecord * reverse(void) const;
	PXType					getRevType(void) const;

#ifdef PT_TEST
	virtual void			__dump(void) const;
#endif
	
protected:
	PXType					m_type;
	PT_DocPosition			m_position;			/* absolute document position of the change */
	PT_AttrPropIndex		m_indexAP;
	UT_Bool					m_persistant;
};

#endif /* PX_CHANGERECORD_H */


