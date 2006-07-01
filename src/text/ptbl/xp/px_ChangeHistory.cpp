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


#include "ut_types.h"
#include "ut_vector.h"
#include "px_ChangeRecord.h"
#include "px_ChangeHistory.h"
#include "px_CR_Span.h"
#include "pt_PieceTable.h"
#include "ut_debugmsg.h"


// m_undoPosition is the position of the undo pointer.
// a value of zero means no undo history.
// the first undo item is at ...[m_undoPosition-1]
// the first redo item is at ...[m_undoPosition] if present.

px_ChangeHistory::px_ChangeHistory(pt_PieceTable * pPT): 
  m_undoPosition(0),
  m_savePosition(0),
  m_pPT(pPT),
  m_iAdjustOffset(0)
{
}

px_ChangeHistory::~px_ChangeHistory()
{
	UT_VECTOR_PURGEALL(PX_ChangeRecord *,m_vecChangeRecords);
}

// this function is used when restoring an older version of a document
// when maintaining full history, since that makes the info in the
// undo meaningless (and causes crashes)
void px_ChangeHistory::clearHistory()
{
	UT_VECTOR_PURGEALL(PX_ChangeRecord *,m_vecChangeRecords);
	m_vecChangeRecords.clear();
	m_undoPosition = 0;
	m_savePosition = 0;
	m_iAdjustOffset = 0;
}


void px_ChangeHistory::_invalidateRedo(void)
{
	UT_uint32 kLimit = m_vecChangeRecords.getItemCount();
	UT_return_if_fail (m_undoPosition <= kLimit);
	UT_uint32 k;

	// walk backwards (most recent to oldest) from the end of the
	// history and delete any redo records.
	// we do the math a little odd here because they are unsigned.
	
	for (k=kLimit; (k > m_undoPosition); k--)
	{
		PX_ChangeRecord * pcrTemp = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(k-1);
		if (!pcrTemp)
			break;

		// Don't remove record from remote documents

		if(!pcrTemp->isFromThisDoc())
		  continue;
		delete pcrTemp;
		m_vecChangeRecords.deleteNthItem(k-1);
	}

	if (m_savePosition > (UT_sint32) m_undoPosition)
		m_savePosition = -1;
}

PD_Document * px_ChangeHistory::getDoc(void) const
{
  return m_pPT->getDocument();
}
        
bool px_ChangeHistory::addChangeRecord(PX_ChangeRecord * pcr)
{
	// add a change record to the history.
	// blow away any redo, since it is now invalid.
	UT_DEBUGMSG(("Add CR Pos %d Type %d indexAP %x \n",pcr->getPosition(),pcr->getType(),pcr->getIndexAP()));
	if(pcr && pcr->getDocument() == NULL)
	{
	    pcr->setDocument(getDoc());
	}
	if(pcr && pcr->isFromThisDoc())
	{
	    m_iAdjustOffset = 0;
	}
	else
	{
	    m_iAdjustOffset++;
	}
	if(!m_pPT->isDoingTheDo())
	{
		_invalidateRedo();
	
		bool bResult = (m_vecChangeRecords.insertItemAt(pcr,m_undoPosition++) == 0);
		UT_ASSERT_HARMLESS(bResult);
		UT_DEBUGMSG(("Undo pos %d savepos %d \n",m_undoPosition,m_savePosition));
		return bResult;
	}
	else
	{
//
// Just save the cr for later deletion with the PT
//
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		m_vecChangeRecords.addItem(pcr);
		return true;
	}
}

bool px_ChangeHistory::canDo(bool bUndo) const
{
	PX_ChangeRecord * pcr;
	UT_uint32 iAdj = m_iAdjustOffset;
	bool b = (bUndo ? getUndo(&pcr) : getRedo(&pcr));
	m_iAdjustOffset = iAdj;
	return b;
}

UT_sint32 px_ChangeHistory::getSavePosition(void) const
{
	return m_savePosition;
}

UT_uint32 px_ChangeHistory::getUndoPos(void) const
{
        return (m_undoPosition - m_iAdjustOffset);
}

void px_ChangeHistory::setSavePosition(UT_sint32 savePosition)
{
	m_savePosition = savePosition;
}


bool px_ChangeHistory::getUndo(PX_ChangeRecord ** ppcr) const
{
	bool bGotOne = false;
	PX_ChangeRecord * pcr = NULL;
	bool bCorrect = false;
	while(!bGotOne)
	{
	  if ((static_cast<UT_sint32>(m_undoPosition) - static_cast<UT_sint32>(m_iAdjustOffset)) <= 0)
	    {
	      return false;
	    }
	    pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition-m_iAdjustOffset-1);
	    UT_ASSERT_HARMLESS(pcr);
	    if(pcr && !pcr->isFromThisDoc())
	    {
		bCorrect = true;
		m_iAdjustOffset++;
	    }
	    else
	    {
		bGotOne = true;
	    }
	}
	PX_ChangeRecord * pcrOrig = pcr;
	if(bCorrect)
	{
	    pcr->setAdjustment(0);
	    PT_DocPosition pos = pcr->getPosition();
	    UT_sint32 i = static_cast<UT_sint32>(m_iAdjustOffset)-1;
	    UT_sint32 iAdj= 0;
	    for(i=i; i>=0;i--)
	    {
		pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition-i-1);
		if(!pcr->isFromThisDoc())
		{
		    if(pcr->getPosition() <= static_cast<PT_DocPosition>(static_cast<UT_sint32>(pos) + iAdj))
		    {
			iAdj += getDoc()->getAdjustmentForCR(pcr);
		    }
		}
	    }
	    pcrOrig->setAdjustment(iAdj);
	}
	*ppcr = pcrOrig;
	return true;
}

bool px_ChangeHistory::getUndo(PX_ChangeRecord ** ppcr, UT_uint32 undoNdx) const
{
	if (m_undoPosition <= undoNdx)
		return false;

	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition-undoNdx-1);
	UT_ASSERT_HARMLESS(pcr);
	
	*ppcr = pcr;
	return true;
}

bool px_ChangeHistory::getRedo(PX_ChangeRecord ** ppcr) const
{
	if (m_undoPosition >= m_vecChangeRecords.getItemCount())
		return false;
	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition);
	if (!pcr)
		return false;
	*ppcr = pcr;
	return true;
}

bool px_ChangeHistory::didUndo(void)
{
	xxx_UT_DEBUGMSG((" Doing Undo void in PT undopos %d savePos pos %d \n",m_undoPosition,m_savePosition));
	if (m_undoPosition == 0)
		return false;
	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition-1);

	// leave records from external documents in place so we can correct

	if(pcr && !pcr->isFromThisDoc())
	  return true;
	m_undoPosition--;
	pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition);
	if (pcr && !pcr->getPersistance())
	{
		UT_return_val_if_fail(m_savePosition > 0,false);
		m_savePosition--;
	}
	return true;
}

bool px_ChangeHistory::didRedo(void)
{
	xxx_UT_DEBUGMSG((" Doing Redo void in PT undopos %d savePos pos %d \n",m_undoPosition,m_savePosition));
	if (m_undoPosition >= m_vecChangeRecords.getItemCount())
		return false;
	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition);

	// leave records from external documents in place so we can correct

	if(pcr && !pcr->isFromThisDoc())
	  return true;
	m_undoPosition++;
	if (pcr && !pcr->getPersistance())
		m_savePosition++;
	return true;
}

void px_ChangeHistory::coalesceHistory(const PX_ChangeRecord * pcr)
{
	// coalesce this record with the current undo record.

	PX_ChangeRecord * pcrUndo;
	bool bResult;
	UT_sint32 iAdj = m_iAdjustOffset;
	bResult = getUndo(&pcrUndo);
	UT_return_if_fail (bResult);
	UT_return_if_fail (pcr->getType() == pcrUndo->getType());

	switch (pcr->getType())
	{
	default:
	        m_iAdjustOffset = iAdj;
		UT_ASSERT_HARMLESS(0);
		return;
		
	case PX_ChangeRecord::PXT_InsertSpan:
	case PX_ChangeRecord::PXT_DeleteSpan:
		{
			const PX_ChangeRecord_Span * pcrSpan = static_cast<const PX_ChangeRecord_Span *>(pcr);
			PX_ChangeRecord_Span * pcrSpanUndo = static_cast<PX_ChangeRecord_Span *>(pcrUndo);

			_invalidateRedo();
			pcrSpanUndo->coalesce(pcrSpan);
			if(pcr->isFromThisDoc())
			  m_iAdjustOffset = 0;
			else if(iAdj > 0) 
			  m_iAdjustOffset = iAdj - 1;
		}
		return;
	}
}

void px_ChangeHistory::setClean(void)
{
	m_savePosition = (UT_sint32) m_undoPosition;
}

bool px_ChangeHistory::isDirty(void) const
{
	return (m_savePosition != (UT_sint32) m_undoPosition);
}


