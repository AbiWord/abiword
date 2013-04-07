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


// undo/redo-related functions for class pt_PieceTable.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_FmtMark.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_FmtMark.h"
#include "px_CR_FmtMarkChange.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "px_CR_Glob.h"
#include "fd_Field.h"
/****************************************************************/
/****************************************************************/

#define DONE()  (bUndo ? m_history.didUndo() : m_history.didRedo());
#define UNDO_return_val_if_fail(cond,val) if (!(cond)) { UT_ASSERT(cond); m_bDoingTheDo = false; return (val); }

bool pt_PieceTable::_doTheDo(const PX_ChangeRecord * pcr, bool bUndo)
{
	// actually do the work of the undo or redo.
	m_bDoingTheDo = true;
	switch (pcr->getType())
	{

	//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////

	case PX_ChangeRecord::PXT_GlobMarker:
		DONE();
		m_bDoingTheDo = false;
		return true;
		
	//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////

	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrSpan = static_cast<const PX_ChangeRecord_Span *>(pcr);
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			UT_DEBUGMSG(("_undo insertspan Pos = %d \n",pcr->getPosition()));
			bool bFound = getFragFromPosition(pcrSpan->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFound,false);

			pf_Frag_Strux * pfs = NULL;
			bool bFoundStrux = _getStruxFromFrag(pf,&pfs);
			UNDO_return_val_if_fail (bFoundStrux,false);
			if(isEndFootnote(static_cast<pf_Frag *>(pfs)))
			{
				bFoundStrux = _getStruxFromFragSkip(static_cast<pf_Frag *>(pfs),&pfs);
				UNDO_return_val_if_fail (bFoundStrux,false);
			}
			PT_BlockOffset newOffset = pcrSpan->getPosition() - pfs->getPos() -1;
			if (!_insertSpan(pf,pcrSpan->getBufIndex(),fragOffset,
							 pcrSpan->getLength(),pcrSpan->getIndexAP(),
                             pcrSpan->getField()))
				return false;

			DONE();
			pcrSpan->AdjustBlockOffset(newOffset);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		m_bDoingTheDo = false;
		return true;
		
	case PX_ChangeRecord::PXT_DeleteSpan:
		{
			// Our deleteSpan is much simpler than the main routine.
			// We can do this becase the change history is composed
			// of atomic operations, whereas the main routine has to
			// to deal with whatever the user chose to do (and cut
			// it into a series of steps).

			const PX_ChangeRecord_Span * pcrSpan = static_cast<const PX_ChangeRecord_Span *>(pcr);
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFound = getFragFromPosition(pcrSpan->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFound,false);
			UNDO_return_val_if_fail (pf->getType() == pf_Frag::PFT_Text,false);

			pf_Frag_Strux * pfs = NULL;
			bool bFoundStrux = _getStruxFromFrag(pf,&pfs);
			UNDO_return_val_if_fail (bFoundStrux, false);
			if(isEndFootnote(static_cast<pf_Frag *>(pfs)))
			{
				bFoundStrux = _getStruxFromFragSkip(static_cast<pf_Frag *>(pfs),&pfs);
				UNDO_return_val_if_fail (bFoundStrux, false);
			}
			UNDO_return_val_if_fail (bFoundStrux, false);
			UT_sint32 newOffset = pcrSpan->getPosition() - pfs->getPos() -1; //was -2 
			if(newOffset < 0)
			  newOffset = 0;
			pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);
			UNDO_return_val_if_fail (pft->getIndexAP() == pcrSpan->getIndexAP(),false);
			xxx_UT_DEBUGMSG(("deletespan in _doTheDo length %d \n",pcrSpan->getLength()));
			UT_uint32 iLenDeleted = fragOffset + pft->getLength();
			if(iLenDeleted >= pcrSpan->getLength())
			{
			    _deleteSpan(pft,fragOffset,pcrSpan->getBufIndex(),pcrSpan->getLength(),NULL,NULL);
			}
			else
			{
			    pf_Frag_Text * pftNext = static_cast<pf_Frag_Text *>(pft->getNext());
			    UT_uint32 iLenDel = pft->getLength() - fragOffset;
			    iLenDeleted = 0;
			    while(pft && (pft ->getType() == pf_Frag::PFT_Text) && (iLenDeleted <pcrSpan->getLength()) )
			    {
				_deleteSpan(pft,fragOffset,pcrSpan->getBufIndex(),iLenDel,NULL,NULL);
				pft = pftNext;
				pftNext = static_cast<pf_Frag_Text *>(pft->getNext());
				iLenDeleted += iLenDel;
				iLenDel = pcrSpan->getLength() - iLenDeleted;
				if(iLenDel > pft->getLength())
				  iLenDel = pft->getLength();
				fragOffset = 0;
			    }
			}
			UT_DEBUGMSG(("newOffset %d spanBlockOffset %d \n",newOffset,pcrSpan->getBlockOffset()));
			pcrSpan->AdjustBlockOffset(newOffset);
			m_pDocument->notifyListeners(pfs,pcr);

			DONE();
		}
		m_bDoingTheDo = false;
		return true;

	case PX_ChangeRecord::PXT_ChangeSpan:
		{
			// ChangeSpan is it's own inverse.  similarly, we have a much simpler
			// job than the main routine, because we have broken up the user's
			// request into atomic operations.

			const PX_ChangeRecord_SpanChange * pcrs = static_cast<const PX_ChangeRecord_SpanChange *>(pcr);

			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFound = getFragFromPosition(pcrs->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFound, false);
			UNDO_return_val_if_fail (pf->getType() == pf_Frag::PFT_Text, false);

			pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);

			pf_Frag_Strux * pfs = NULL;
			bool bFoundStrux = _getStruxFromFrag(pf,&pfs);
			if(isEndFootnote(static_cast<pf_Frag *>(pfs)))
			{
				bFoundStrux = _getStruxFromFragSkip(static_cast<pf_Frag *>(pfs),&pfs);
				UNDO_return_val_if_fail (bFoundStrux, false);
			}
			UNDO_return_val_if_fail (bFoundStrux, false);
			PT_BlockOffset newOffset = pcrs->getPosition() - pfs->getPos() -1;

			// we need to loop here, because even though we have a simple (atomic) change,
			// the document may be fragmented slightly differently (or rather, it may not
			// yet be possible to coalesce it (until the end of the loop)).
			
			pf_Frag * pfEnd;
			UT_uint32 fragOffsetEnd;
			UT_uint32 length = pcrs->getLength();
			while (length)
			{
				UT_uint32 lengthInFrag = pft->getLength() - fragOffset;
				UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);

				_fmtChangeSpan(pft,fragOffset,lengthThisStep,pcrs->getIndexAP(),&pfEnd,&fragOffsetEnd);

				length -= lengthThisStep;
				if (length == 0)
					break;

				UNDO_return_val_if_fail (pfEnd->getType() == pf_Frag::PFT_Text, false);
				pft = static_cast<pf_Frag_Text *> (pfEnd);
				fragOffset = fragOffsetEnd;
			}
			
			DONE();
			pcrs->AdjustBlockOffset(newOffset);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		m_bDoingTheDo = false;
		return true;
			
	//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////

	case PX_ChangeRecord::PXT_InsertStrux:
		{
			const PX_ChangeRecord_Strux * pcrStrux = static_cast<const PX_ChangeRecord_Strux *>(pcr);
			pf_Frag_Strux * pfsNew = NULL;
			if (!_createStrux(pcrStrux->getStruxType(),pcrStrux->getIndexAP(),&pfsNew))
				return false;

			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFoundFrag = getFragFromPosition(pcrStrux->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFoundFrag, false);

			// get the strux containing the given position.
	
// TODO see if we can avoid this call to _getStruxFromPosition ??
			pf_Frag_Strux * pfsContainer = NULL;
			bool bFoundContainer = _getStruxFromPosition(pcrStrux->getPosition(),&pfsContainer);
			UNDO_return_val_if_fail (bFoundContainer,false);
			if(isEndFootnote(static_cast<pf_Frag *>(pfsContainer)))
			{
				bool bFoundStrux = _getStruxFromFragSkip(static_cast<pf_Frag *>(pfsContainer),&pfsContainer);
				UNDO_return_val_if_fail (bFoundStrux, false);
			}
			_insertStrux(pf,fragOffset,pfsNew);
			// insert frag in the embedded_strux list if needed
			PTStruxType pts = pcrStrux->getStruxType();
			if ((pts == PTX_EndFootnote) || (pts == PTX_EndEndnote) || (pts == PTX_EndAnnotation)) 
			{
			    _insertNoteInEmbeddedStruxList(pfsNew);
			}
			DONE();
			m_pDocument->notifyListeners(pfsContainer,pfsNew,pcr);
		}
		return true;
		
	case PX_ChangeRecord::PXT_DeleteStrux:
		{
			const PX_ChangeRecord_Strux * pcrStrux = static_cast<const PX_ChangeRecord_Strux *>(pcr);
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFoundFrag = getFragFromPosition(pcrStrux->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFoundFrag,false);
			UNDO_return_val_if_fail (pf->getType() == pf_Frag::PFT_Strux,false);

			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);
			UNDO_return_val_if_fail (pcrStrux->getStruxType() == pfs->getStruxType(),false);
			bool bResult = _unlinkStrux(pfs,NULL,NULL);
			m_pDocument->notifyListeners(pfs,pcr);
			UNDO_return_val_if_fail (bResult,false);
			DONE();
			
			delete pfs;
		}
		m_bDoingTheDo = false;
		return true;

	case PX_ChangeRecord::PXT_ChangeStrux:
		{
			// ChangeStrux is it's own inverse.

			const PX_ChangeRecord_StruxChange * pcrs = static_cast<const PX_ChangeRecord_StruxChange *>(pcr);
			pf_Frag_Strux * pfs;
			bool bFound = _getStruxFromPosition(pcrs->getPosition(),&pfs);
			UNDO_return_val_if_fail (bFound,false);
			bool bResult = _fmtChangeStrux(pfs,pcrs->getIndexAP());
			UNDO_return_val_if_fail (bResult,false);
			DONE();
			m_pDocument->notifyListeners(pfs,pcr);
		}
		m_bDoingTheDo = false;
		return true;

	//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////

	case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcrObject = static_cast<const PX_ChangeRecord_Object *>(pcr);
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFound = getFragFromPosition(pcrObject->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFound, false);

			pf_Frag_Strux * pfs = NULL;
			bool bFoundStrux2 = _getStruxFromFrag(pf,&pfs);
			UNDO_return_val_if_fail (bFoundStrux2, false);
			if(isEndFootnote(static_cast<pf_Frag *>(pfs)))
			{
				bool bFoundStrux = _getStruxFromFragSkip(static_cast<pf_Frag *>(pfs),&pfs);
				UNDO_return_val_if_fail (bFoundStrux,false);
			}
			pf_Frag_Object * pfo = NULL;
			if (!_insertObject(pf,fragOffset,pcrObject->getObjectType(),
                               pcrObject->getIndexAP(),pfo))
				return false;
			pcrObject->setObjectHandle(pfo);
			UNDO_return_val_if_fail (pfo,false);
			UT_sint32 newOffset = pcrObject->getPosition() - pfs->getPos() -1;

            
            // need to set field pointers to values of new pointer
            // as old field doesn't exist
			pf = pfo->getNext();
			while (pf&&pf->getType()==pf_Frag::PFT_Text&&
			       pf->getField())
			  {
			    pf_Frag_Text * pft = 
			      static_cast<pf_Frag_Text *>(pf);
			    pft->setField(pfo->getField());
			    pf = pft->getNext();
			  }
			DONE();            
			pcrObject->AdjustBlockOffset(newOffset);
			m_pDocument->notifyListeners(pfs,pcr);
            // don't update field until all of changes have been made
		}
		m_bDoingTheDo = false;
		return true;
		
	case PX_ChangeRecord::PXT_DeleteObject:
		{
			const PX_ChangeRecord_Object * pcrObject = static_cast<const PX_ChangeRecord_Object *>(pcr);
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFound = getFragFromPosition(pcrObject->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFound, false);
			UNDO_return_val_if_fail (pf->getType() == pf_Frag::PFT_Object,false);
			UNDO_return_val_if_fail (fragOffset == 0,false);
			
			pf_Frag_Strux * pfs = NULL;
			bool bFoundStrux2 = _getStruxFromFrag(pf,&pfs);
			UNDO_return_val_if_fail (bFoundStrux2,false);
			if(isEndFootnote(static_cast<pf_Frag *>(pfs)))
			{
				bool bFoundStrux = _getStruxFromFragSkip(static_cast<pf_Frag *>(pfs),&pfs);
				UNDO_return_val_if_fail (bFoundStrux,false);
			}
			UT_sint32 newOffset = pcrObject->getPosition() - pfs->getPos() -1; // was -2
			if(newOffset < 0)
			  newOffset = 0;
			pf_Frag_Object * pfo = static_cast<pf_Frag_Object *> (pf);
			if((pfo->getObjectType() != PTO_Math) && ((pfo->getObjectType() != PTO_Embed)))
			{
			    UNDO_return_val_if_fail (pfo->getIndexAP() == pcrObject->getIndexAP(),false);
			}
			_deleteObject(pfo,NULL,NULL);

			DONE();
			pcrObject->AdjustBlockOffset(newOffset);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		m_bDoingTheDo = false;
		return true;

	case PX_ChangeRecord::PXT_ChangeObject:
		{
			// ChangeSpan is it's own inverse.

			const PX_ChangeRecord_ObjectChange * pcro = static_cast<const PX_ChangeRecord_ObjectChange *>(pcr);

			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFound = getFragFromPosition(pcro->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFound,false);
			UNDO_return_val_if_fail (pf->getType() == pf_Frag::PFT_Object,false);
			UNDO_return_val_if_fail (fragOffset == 0, false);

			pf_Frag_Strux * pfs = NULL;
			bool bFoundStrux2 = _getStruxFromFrag(pf,&pfs);
			UNDO_return_val_if_fail (bFoundStrux2,false);
			if(isEndFootnote(static_cast<pf_Frag *>(pfs)))
			{
				bool bFoundStrux = _getStruxFromFragSkip(static_cast<pf_Frag *>(pfs),&pfs);
				UNDO_return_val_if_fail (bFoundStrux,false);
			}
			UT_sint32 newOffset = pcro->getPosition() - pfs->getPos() -1;

			pf_Frag_Object * pfo = static_cast<pf_Frag_Object *> (pf);

			_fmtChangeObject(pfo,pcro->getIndexAP(),NULL,NULL);

			DONE();
			pcro->AdjustBlockOffset(newOffset);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		m_bDoingTheDo = false;
		return true;

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	case PX_ChangeRecord::PXT_InsertFmtMark:
		{
			const PX_ChangeRecord_FmtMark * pcrFM = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFound = getFragFromPosition(pcrFM->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFound, false);

			pf_Frag_Strux * pfs = NULL;
			bool bFoundStrux2 = _getStruxFromFrag(pf,&pfs);
			UNDO_return_val_if_fail (bFoundStrux2, false);
			if(isEndFootnote(static_cast<pf_Frag *>(pfs)))
			{
				bool bFoundStrux = _getStruxFromFragSkip(static_cast<pf_Frag *>(pfs),&pfs);
				UNDO_return_val_if_fail (bFoundStrux,false);
			}
			UT_sint32 newOffset = pcrFM->getPosition() - pfs->getPos() -1;
			
			if (!_insertFmtMark(pf,fragOffset,pcrFM->getIndexAP()))
				return false;

			DONE();
			pcrFM->AdjustBlockOffset(newOffset);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		m_bDoingTheDo = false;
		return true;
	 
	case PX_ChangeRecord::PXT_DeleteFmtMark:
		{
			const PX_ChangeRecord_FmtMark * pcrFM = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFound = getFragFromPosition(pcrFM->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFound, false);

			// we backup one because we have zero length and getFragFromPosition()
			// returns the right-most thing with this document position.
			if(pf->getType() != pf_Frag::PFT_FmtMark)
			  pf = pf->getPrev();
			if(pf->getType()==pf_Frag::PFT_Strux)
			{
			    if(pf->getNext() && pf->getNext()->getType() == pf_Frag::PFT_Strux)
			    {
				DONE();
				m_bDoingTheDo = false;
				return true;
			    }
			    if(pf->getNext() && pf->getNext()->getType() == pf_Frag::PFT_Text)
			    {
			        pf = pf->getNext();
			    
				if(pf->getNext() && pf->getNext()->getType() == pf_Frag::PFT_FmtMark)
				{
				    pf = pf->getNext();
				}
			    }
			}

			UNDO_return_val_if_fail (pf->getType() == pf_Frag::PFT_FmtMark,false);
			UNDO_return_val_if_fail (fragOffset == 0,false);
			
			pf_Frag_Strux * pfs = NULL;
			bool bFoundStrux = _getStruxFromFragSkip(pf,&pfs);
			UNDO_return_val_if_fail (bFoundStrux,false);
			UT_sint32 newOffset = pcrFM->getPosition() - pfs->getPos() -1;

			pf_Frag_FmtMark * pffm = static_cast<pf_Frag_FmtMark *> (pf);
			//			UNDO_return_val_if_fail (pffm->getIndexAP() == pcrFM->getIndexAP(),false);
			_deleteFmtMark(pffm,NULL,NULL);

			DONE();
			pcrFM->AdjustBlockOffset(newOffset);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		m_bDoingTheDo = false;
		return true;

	case PX_ChangeRecord::PXT_ChangeFmtMark:
		{
			// ChangeFmt is it's own inverse.

			const PX_ChangeRecord_FmtMarkChange * pcrFMC = static_cast<const PX_ChangeRecord_FmtMarkChange *>(pcr);

			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			bool bFound = getFragFromPosition(pcrFMC->getPosition(),&pf,&fragOffset);
			UNDO_return_val_if_fail (bFound,false);

			// we backup one because we have zero length and getFragFromPosition()
			// returns the right-most thing with this document position.
			if(pf->getType() != pf_Frag::PFT_FmtMark)
			  pf = pf->getPrev();
			if(pf->getType()==pf_Frag::PFT_Strux)
			{
			    if(pf->getNext() && pf->getNext()->getType() == pf_Frag::PFT_Strux)
			    {
				DONE();
				m_bDoingTheDo = false;
				return true;
			    }
			}
			UNDO_return_val_if_fail (pf->getType() == pf_Frag::PFT_FmtMark,false);
			UNDO_return_val_if_fail (fragOffset == 0,false);

			pf_Frag_Strux * pfs = NULL;
			bool bFoundStrux = _getStruxFromFragSkip(pf,&pfs);
			UNDO_return_val_if_fail (bFoundStrux,false);
			UT_sint32 newOffset = pcrFMC->getPosition() - pfs->getPos() -1;

			pf_Frag_FmtMark * pffm = static_cast<pf_Frag_FmtMark *> (pf);

			_fmtChangeFmtMark(pffm,pcrFMC->getIndexAP(),NULL,NULL);

			DONE();
			m_bDoingTheDo = false;
			pcrFMC->AdjustBlockOffset(newOffset);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		return true;
		
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	case PX_ChangeRecord::PXT_ChangePoint:
		DONE();
		m_pDocument->notifyListeners(NULL, pcr);

		m_bDoingTheDo = false;
		return true;

	default:
		UT_ASSERT_HARMLESS(0);
		m_bDoingTheDo = false;
		return false;
	}
}

/*****************************************************************/
/*****************************************************************/

#define GETGLOBFLAGS(pcr)			(  (pcr->getType() == PX_ChangeRecord::PXT_GlobMarker)		\
									 ? static_cast<PX_ChangeRecord_Glob *>(pcr)->getFlags()		\
									 : static_cast<UT_Byte>(PX_ChangeRecord_Glob::PXF_Null))
#define GETREVGLOBFLAGS(pcr)		(  (pcr->getType() == PX_ChangeRecord::PXT_GlobMarker)		\
									 ? static_cast<PX_ChangeRecord_Glob *>(pcr)->getRevFlags()	\
									 : static_cast<UT_Byte>(PX_ChangeRecord_Glob::PXF_Null))

/*****************************************************************/
/*****************************************************************/
	
bool pt_PieceTable::canDo(bool bUndo) const
{
	return m_history.canDo(bUndo);
}

UT_uint32 pt_PieceTable::undoCount(bool bUndo) const
{
  if(bUndo)
    return m_history.getUndoPos ();
  return m_history.getSavePosition ();
}

bool pt_PieceTable::undoCmd(void)
{
	// do a user-atomic undo.
	// return false if we can't.
	
	PX_ChangeRecord * pcr;
	if (!m_history.getUndo(&pcr))
		return false;
	UT_return_val_if_fail (pcr,false);

	// the first undo record tells us whether it is
	// a simple change or a glob.  there are two kinds
	// of globs: a multi-step change (display atomic)
	// like deleting a selection that spans a paragraph
	// break; and a user-atomic glob like doing a search
	// and replace over the whole document.
	//
	// for a simple change, we just do it and return.
	// for a glob, we loop until we do the
	// corresponding other end.
	m_history.setScanningUndoGLOB(false);
	UT_Byte flagsFirst = GETGLOBFLAGS(pcr);
	do
	{
		PX_ChangeRecord * pcrRev = pcr->reverse(); // we must delete this.
		pcrRev->setAdjustment(pcr->getAdjustment());
		pcrRev->setDocument(getDocument());
		pcrRev->setCRNumber();
		UT_return_val_if_fail (pcrRev,false);
		UT_Byte flagsRev = GETGLOBFLAGS(pcrRev);
		bool bResult = _doTheDo(pcrRev, true);
		delete pcrRev;
		
		if (!bResult)
			return false;
		if (flagsRev == flagsFirst)		// stop when we have a matching end
			break;

	} while (m_history.getUndo(&pcr));
	m_history.setScanningUndoGLOB(false);
	m_pDocument->updateFields();
	return true;
}

bool pt_PieceTable::redoCmd(void)
{
	// do a user-atomic redo.
	// return false if we can't.
        m_history.setScanningUndoGLOB(false);	
	PX_ChangeRecord * pcr;
	if (!m_history.getRedo(&pcr))
		return false;
	UT_return_val_if_fail (pcr,false);

	// the first undo record tells us whether it is
	// a simple change or a glob.  there are two kinds
	// of globs: a multi-step change (display atomic)
	// like deleting a selection that spans a paragraph
	// break; and a user-atomic glob like doing a search
	// and replace over the whole document.
	//
	// for a simple change, we just do it and return.
	// for a glob, we loop until we do the
	// corresponding other end.

	UT_Byte flagsRevFirst = GETREVGLOBFLAGS(pcr);

	while (m_history.getRedo(&pcr))
	{
	        pcr->setCRNumber(); // update the CRNumber
		if (!_doTheDo(pcr, false))
			return false;
		
		if (flagsRevFirst == GETGLOBFLAGS(pcr))		// stop when we have a matching end
			break;
	}
	m_history.setScanningUndoGLOB(false);
	return true;
}

bool pt_PieceTable::isDirty(void) const
{
	bool bDirty = m_history.isDirty();

	if (!bDirty)
	{
		// TODO: this is a good spot for sanity checks of the undo mechanism
		// TODO: if we really *aren't* dirty, then doc should be identical to saved version
	}

	return bDirty;
}

void pt_PieceTable::setClean(void)
{
	m_history.setClean();
}

