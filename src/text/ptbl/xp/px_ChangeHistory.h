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


#ifndef PX_CHANGEHISTORY_H
#define PX_CHANGEHISTORY_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// px_ChangeHistory implements a history mechanism of PX_ChangeRecords.
// This is used for undo and redo.  We implement a first-order undo/redo.
// (We do not put undo commands into the undo history like emacs.)
// As an editing operation is performed, one or more ChangeRecords will
// be generated.  These are appended to the history.  The undo command
// will take the last ChangeRecord and reverse its effect and decrement
// the undo position.  ChangeRecords which are undone are not deleted
// from the end of the history until a non-redo command is performed
// (and thus invalidating what needs to be redone).  A redo command will
// re-apply the ChangeRecord and advance the undo position.

class px_ChangeHistory
{
public:
	px_ChangeHistory();
	~px_ChangeHistory();

	// addChangeRecord -- append the given cr to the history
	//                    at the current position and advance
	//                    the current position.  also deletes
	//                    any (now) invalid redo items beyond
	//                    the current position.
	// getUndo -- fetch the cr immediately prior to the current
	//            position.
	// getRedo -- fetch the cr immediately after the current position.
	//
	// didUndo -- decrement the current position (to compensate for
	//            having just performed the undo).
	// didRedo -- increment the current position (to compensate for
	//            having just performed the redo).
	
	bool					addChangeRecord(PX_ChangeRecord * pcr);
	bool					canDo(bool bUndo) const;
	
	void					setSavePosition(UT_sint32 savePosition);
	UT_sint32				getSavePosition(void) const;

	UT_uint32                               getUndoPos(void);

	bool					getUndo(PX_ChangeRecord ** ppcr) const;
	bool					getUndo(PX_ChangeRecord ** ppcr, UT_uint32 undoNdx) const;
	bool					getRedo(PX_ChangeRecord ** ppcr) const;
	bool					didUndo(void);
	bool					didRedo(void);
	void					coalesceHistory(const PX_ChangeRecord * pcr);

	void					setClean(void);
	bool					isDirty(void) const;

protected:
	void					_invalidateRedo(void);
	
	UT_Vector				m_vecChangeRecords;
	UT_uint32				m_undoPosition;
	UT_sint32				m_savePosition;
};

#endif /* PX_CHANGEHISTORY_H */
