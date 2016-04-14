/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#include "fl_Squiggles.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "fp_Run.h"

/*! \page squiggle_overview Squiggles

Squiggles are used to underline miss-spelled words. Instead of simply
erasing all squiggles and rechecking all words in a block when the
block is changed, the squiggles are handled much like the words they
underline.

The word currently being edited is the <b>pending word</b>. When the
cursor does not touch the pending word anymore (due to being moved
away or the user typing a word separator), the word is
spell-checked. If it is miss-spelled, it will be squiggled.

When text is added to the block, fl_Squiggles::textInserted is called
with information of where in the block the text was added, and how
much. It will then remove any squiggle located at that offset, move
all following squiggles (so they end up aligned with the words they
should underline) and spell-checks words in the added text (via
fl_BlockLayout::_recalcPendingWord).

When text is deleted from the block, fl_Squiggles::textDeleted is
called with information of where in the block text was deleted, and
how much. It removes squiggles intersecting with that area, moves all
following squiggles and makes pending the word at the deletion point
since two words may have been joined, or a word lost part of its
letters.

When a block is split in two, fl_Squiggles::split is called with
information of where the block was split, and a pointer to the new
block. The squiggles from the old block are split between it and the
new block. The word at the end of the old block (which may have been
broken), is spell-checked, and the first word of the new block is made
the pending word.

When two blocks are merged into one, fl_Squiggles::join is called with
information of the offset where squiggles from the second block should
be joined onto the first block. The word at the merge point is made
the pending word.

\fixme There's one known buglet: typing "gref' " correctly squiggles
       the word when typing ' since it's a word separator. However,
       deleting the s in "gref's" leaves gref unsquiggled because the
       word gref was not pending when the ' changed from a word
       character to a word delimiter. (hard to explain - just try it)

*/


/*!
 Constructor
 \param pOwner The owning block
*/
fl_Squiggles::fl_Squiggles(fl_BlockLayout* pOwner,FL_SQUIGGLE_TYPE iType) :
  m_pOwner(pOwner),
  m_iSquiggleType(iType)
{
}

/*!
 Destructor

 Only purges the vector. It is assumed the squiggles have already been
 cleared from the screen.
*/
fl_Squiggles::~fl_Squiggles(void)
{
	_purge();
}

/*!
 Purge squiggles
 Purges the squiggle list. This does not clear the squiggles
 on the display.
*/
void
fl_Squiggles::_purge(void)
{
	UT_VECTOR_PURGEALL(fl_PartOfBlock *, m_vecSquiggles);

	m_vecSquiggles.clear();
}

/*!
 Find first squiggle after the given offset
 \param iOffset Offset
 \result iIndex Index of POB, or index of last POB+1
 \return True if found, otherwise false

 \note Callers may use the iIndex result even if the search fails:
       this allows them to look at the last POB on the line which may
       span the point they are looking for (remember this function
       finds the squiggle past a given offset, not the one spanning it
       - there may well be a squiggle spanning a point without there
       being further squiggles behind it).

 \fixme This function should be rewritten using binary search
*/
bool
fl_Squiggles::_findFirstAfter(UT_sint32 iOffset, UT_sint32& iIndex) const
{
	bool bRes = false;
	UT_sint32 iSquiggles = _getCount();
	fl_PartOfBlock* pPOB;
	UT_sint32 j;
	for (j = 0; j < iSquiggles; j++)
	{
		// Look for the first POB past the offset
		pPOB = getNth(j);
		if (pPOB->getOffset() > iOffset)
		{
			bRes = true;
			break;
		}
	}

	iIndex = j;
	return bRes;
}

/*!
 Find squiggle spanning offset
 \param iOffset Offset
 \return Index of squiggle spanning the offset, or -1 if there is no
         squiggle at the offset.
*/
UT_sint32
fl_Squiggles::_find(UT_sint32 iOffset) const
{
	UT_sint32 i = 0;
	UT_sint32 iSquiggles = _getCount();
	fl_PartOfBlock* pPOB;
	for(i=0;i<iSquiggles;i++)
	{
		pPOB = getNth(i);
		xxx_UT_DEBUGMSG((" i %d pob->offset %d pob->length %d offset %d \n",i,pPOB->getOffset(),pPOB->getLength(),iOffset));
		if((pPOB->getOffset() <= iOffset) && (iOffset <= (pPOB->getOffset() + pPOB->getPTLength())))
		{
			break;
		}
	}
	if(i>=iSquiggles)
	{
		return -1;
	}
	return i;
}

/*!
 Move squiggles to new block
 \param iOffset Offset at which to split
 \param chg Offset change. If >0 it's a new absolute position,
            if <0 it's relative to the current offset.
 \param pNewBlock New block the squiggles should be moved to,
                  or NULL to keep them in the current block.

 Move existing squiggles to reflect insert/delete at iOffset.
 All subsequent squiggles should be switched to (non-null) pBlock.

 \note Only squiggles after the offset are moved. Squiggles spanning
       the offset must be handled elsewhere.
*/
void
fl_Squiggles::_move(UT_sint32 iOffset, UT_sint32 chg,
					fl_BlockLayout* pNewBlock /* =NULL */)
{
	xxx_UT_DEBUGMSG(("fl_Squiggles::_move(%d, %d, %p)\n",
					 iOffset, chg, pNewBlock));

	UT_sint32 target = (chg > 0) ? iOffset : (iOffset - chg);

	UT_sint32 iSquiggles = _getCount();
	UT_sint32 j;
	for (j = iSquiggles-1; j >= 0; j--)
	{
		fl_PartOfBlock* pPOB = getNth(j);

		// Only interested in squiggles after change, and since they
		// are sorted, stop searching when first one before target is
		// found.
		if (pPOB->getOffset() < target) break;

		// Clear the squiggle before moving it
		clear(pPOB);
		pPOB->setOffset(pPOB->getOffset() + chg);

		// Move squiggle to another block if requested
		if (pNewBlock)
		{
			UT_ASSERT(pNewBlock != m_pOwner);
			pNewBlock->getSpellSquiggles()->add(pPOB);
			m_vecSquiggles.deleteNthItem(j);
		}	
	}
}

/*!
 * Update the offsets in the POB's. We shifts the offsets around after text
 * inside an emebdded section (like a footnote is changed).
\param iFirstOffset this is the first offset that is changed.
\param iShift this is the amount that the text is shifted.
 */
void fl_Squiggles::updatePOBs(UT_sint32 iFirstOffset, UT_sint32 iShift)
{
	UT_sint32 i =0;
	for(i=0; i< m_vecSquiggles.getItemCount(); i++)
	{
		fl_PartOfBlock * pPOB = m_vecSquiggles.getNthItem(i);
		if(pPOB->getOffset() >= iFirstOffset)
		{
			pPOB->setOffset(pPOB->getOffset() + iShift);
		}
	}
}

/*!
 Add squiggle
 \param POB for squiggle
 Insert POB sorted by offset in vector.
*/
void
fl_Squiggles::add(fl_PartOfBlock* pPOB)
{
	xxx_UT_DEBUGMSG(("fl_Squiggles::add(%p) [%d:%d]\n", pPOB,
					 pPOB->getOffset(), 
					 pPOB->getOffset() + pPOB->getLength()));
	UT_ASSERT(pPOB->getOffset() >= 0);
	UT_sint32 iIndex;

	if (_findFirstAfter(pPOB->getOffset(), iIndex))
	{
		m_vecSquiggles.insertItemAt(pPOB, iIndex);
	}
	else
	{
		m_vecSquiggles.addItem(pPOB);
	}
	// Handle extension / merging of squiggles
	if (iIndex > 0)
	{
		fl_PartOfBlock* pPrev = getNth(iIndex-1);

		if (pPOB->getOffset() == pPrev->getOffset() && (getSquiggleType() == FL_SQUIGGLE_SPELL))
		{
			// Handle extension of existing squiggles. This happens
			// because ' changes from being a word separator to not
			// being one when characters are added after it. So while
			// typing "gest'" >gest< will be squiggled, and after
			// "gest's" the entire >gest's< is squiggled.
			pPrev->setPTLength(pPOB->getPTLength());
			_deleteNth(iIndex--);
			markForRedraw(pPrev);
		}
		else if ((pPOB->getOffset() == pPrev->getOffset() + pPrev->getPTLength()) &&
				 (getSquiggleType() == FL_SQUIGGLE_SPELL))
		{
			// Handle merging of two squiggles - this happens e.g. in
			// overwrite mode when two misspelled words are joined by
			// typing a character ' between them.
			pPrev->setPTLength(pPrev->getPTLength() + pPOB->getPTLength());
			_deleteNth(iIndex--);
			markForRedraw(pPrev);
		}
		else
		{
			markForRedraw(pPOB);
		}

	}
	else
	{
	        markForRedraw(pPOB);
	}

#ifdef DEBUG
	UT_sint32 iSquiggles = _getCount();
	if (iSquiggles <= 1) return;
	if(getSquiggleType() == FL_SQUIGGLE_SPELL)
	{
	  //
	  // Grammar squiggles can over lap.
	  //
	  if (iIndex > 0)
	    {
	      UT_ASSERT((getNth(iIndex-1)->getOffset() + getNth(iIndex-1)->getPTLength())
			< getNth(iIndex)->getOffset());
	    }
	  if (iSquiggles > (iIndex+1))
	    {
	      UT_ASSERT((getNth(iIndex)->getOffset() + getNth(iIndex)->getPTLength())
			< getNth(iIndex+1)->getOffset());
	    }
	}
#endif
}

/*!
 Delete Nth squiggle
 \param iIndex Index of squiggle to delete
 Clear squiggle from screen and g_free the POB's memory
*/
void
fl_Squiggles::_deleteNth(UT_sint32 iIndex)
{
	xxx_UT_DEBUGMSG(("fl_Squiggles::delelteNth(%d)\n", iIndex));
	fl_PartOfBlock* pPOB = getNth(iIndex);
	m_vecSquiggles.deleteNthItem(iIndex);
	clear(pPOB);
	delete pPOB;
}


/*!
 Delete all squiggles
 \return True if display should be updated, otherwise false
 Clear all squiggles from display, and purge the list.
*/
bool
fl_Squiggles::deleteAll(void)
{
	xxx_UT_DEBUGMSG(("fl_Squiggles::deleteAll()\n"));

	// Remove any existing squiggles from the screen...
	UT_sint32 iSquiggles = _getCount();
	UT_sint32 j;
	for (j = iSquiggles-1; j >= 0 ; j--)
	{
		_deleteNth(j);
	}

	return (0 == iSquiggles) ? false : true;
}

/*!
 Delete squiggle at offset
 \param iOffset Offset
 \return True if a squiggle was deleted, otherwise false

 If a squiggle spans the offset, delete it.
*/
bool
fl_Squiggles::_deleteAtOffset(UT_sint32 iOffset)
{
	xxx_UT_DEBUGMSG(("fl_Squiggles::_deleteAtOffset(%d)\n", iOffset));

	bool res = false;
	if(getSquiggleType() == FL_SQUIGGLE_GRAMMAR)
	{
	  fl_PartOfBlock* pPOB = 0;
	  UT_sint32 i = 0;
	  UT_sint32 iLow = 0;
	  UT_sint32 iHigh = 0;
	  for(i=0; i< _getCount();)
	  {
	    pPOB = getNth(i);
	    if(pPOB->isInvisible() && ((pPOB->getOffset() <= iOffset) &&
				       pPOB->getOffset()+ pPOB->getPTLength() >= iOffset))
	    {
	      iLow = pPOB->getOffset();
	      iHigh = pPOB->getOffset() + pPOB->getPTLength();
	    }
	    if(iOffset >= iLow && iOffset <= iHigh)
	    {
	      _deleteNth(i);
	      res = true;
	    }
	    else
	    {
	      i++;
	    }
	  }
	}
	if(res)
	  return res;
	UT_sint32 iIndex = _find(iOffset);
	if (iIndex >= 0)
	{
		_deleteNth(iIndex);
		res = true;
	}

	return res;
}


/*!
 * Mark all the runs overlapping with the POB for Redraw.
 */
void fl_Squiggles::markForRedraw(fl_PartOfBlock* pPOB)
{
	PT_DocPosition pos1 = pPOB->getOffset();
	PT_DocPosition pos2 = pos1 + pPOB->getPTLength();
	//
	// Make sure the runs in this POB get redrawn.
	//
	fp_Run * pRun = m_pOwner->getFirstRun();
	while(pRun && (pRun->getBlockOffset() <= pos2))
	{
	    if((pRun->getBlockOffset() + pRun->getLength()) >= pos1)
	    {
	         pRun->markAsDirty();
	    }
	    pRun = pRun->getNextRun();
	}
}

/*!
 Get squiggle at offset
 \param iOffset Offset
 \return Pointer to POB or NULL if there is no squiggle at the offset
*/
fl_PartOfBlock*
fl_Squiggles::get(UT_sint32 iOffset) const
{
	fl_PartOfBlock* pPOB = NULL;
	UT_sint32 i = _find(iOffset);

	if (i >= 0)
		pPOB = getNth(i);

	return pPOB;
}

/*!
 Clear squiggle
 \param pPOB Part of block to clear squiggle for
 This clears the squiggle graphics from the screen.
*/
void
fl_Squiggles::clear(fl_PartOfBlock* pPOB)
{
	if(!m_pOwner->isOnScreen())
	{
		return;
	}
	FV_View* pView = m_pOwner->getDocLayout()->getView();
	PT_DocPosition pos1 = m_pOwner->getPosition() + pPOB->getOffset();
	PT_DocPosition pos2 = pos1 + pPOB->getPTLength();
	if(pView->getDocument()->isPieceTableChanging())
	{
	  //
	  // Make sure the runs in this POB get redrawn.
	  //
	  markForRedraw(pPOB);
	  return;
	}
	PT_DocPosition posEOD = 0;
	m_pOwner->getDocument()->getBounds(true,posEOD);
	if(pos2 > posEOD)
	{
		pos2 = posEOD;
	}
	if(pos1 > pos2)
	{
		pos1 = pos2 -1;
	}
	pView->_clearBetweenPositions(pos1, pos2, true);
	xxx_UT_DEBUGMSG(("fl_Squiggles::clear posl %d pos2 %d \n", pos1,pos2));
}

/*!
 Text inserted - update squiggles
 \param iOffset Location at which insertion happens
 \param iLength Length of inserted text
*/
void
fl_Squiggles::textInserted(UT_sint32 iOffset, UT_sint32 iLength)
{
	// Ignore operations on shadow blocks
	if (m_pOwner->isHdrFtr())
		return;

	// Return if auto spell-checking disabled
	if (!m_pOwner->getDocLayout()->getAutoSpellCheck())
		return;

	xxx_UT_DEBUGMSG(("fl_Squiggles::textInserted(%d, %d)\n", 
					 iOffset, iLength));

	UT_sint32 chg = iLength;

	// Delete squiggle broken by this insert
	_deleteAtOffset(iOffset);

	// Move all trailing squiggles
	_move(iOffset, chg);

	// Deal with pending word, if any
  
	if (m_pOwner->getDocLayout()->isPendingWordForSpell() && (getSquiggleType() ==  FL_SQUIGGLE_SPELL) )
	{
		// If not affected by insert, check it
		if (!m_pOwner->getDocLayout()->touchesPendingWordForSpell(m_pOwner, iOffset, 0))
		{
			fl_PartOfBlock* pPending = m_pOwner->getDocLayout()->getPendingWordForSpell();
			// If pending word is later in the block, adjust its
			// offset according to the change
			if (pPending->getOffset() > iOffset)
				pPending->setOffset(pPending->getOffset() + chg);

#if 0
			m_pOwner->getDocLayout()->checkPendingWordForSpell();
#else
//
// Remove the pending word. Trying to spellcheck it is giving us troubles
//

// What kind of trouble? Please refer a Bug # or symptom when
// disabling code like this... Also see Bug 4453    jskov 2003.01.05

		m_pOwner->getDocLayout()->setPendingWordForSpell(NULL,NULL);
#endif 
		}
	}

	// Recheck word at boundary
	if(getSquiggleType() ==  FL_SQUIGGLE_SPELL) 
	{
	  m_pOwner->_recalcPendingWord(iOffset, chg);
	}
}

/*!
 Text deleted - update squiggles
 \param iOffset Offset of deletion 
 \param iLength Length of deletion
*/
void
fl_Squiggles::textDeleted(UT_sint32 iOffset, UT_sint32 iLength)
{
	// Ignore operations on shadow blocks
	if (m_pOwner->isHdrFtr())
		return;

	// Return if auto spell-checking disabled
	if (!m_pOwner->getDocLayout()->getAutoSpellCheck())
		return;

	xxx_UT_DEBUGMSG(("fl_Squiggles::textDeleted(%d, %d)\n",
					 iOffset, iLength));

	UT_sint32 chg = -(UT_sint32)iLength;

	UT_sint32 iFirst, iLast;
	if (findRange(iOffset, iOffset+iLength, iFirst, iLast))
	{
		while ((iLast >= 0) && (iLast >= iFirst))
		{
			_deleteNth(iLast--);
		}
	}

	// Move all trailing squiggles
	_move(iOffset, chg);

	// Deal with pending word, if any
	if (m_pOwner->getDocLayout()->isPendingWordForSpell() && (getSquiggleType() ==  FL_SQUIGGLE_SPELL) )
	{
		// If not affected by delete, check it
		if (!m_pOwner->getDocLayout()->touchesPendingWordForSpell(m_pOwner, iOffset, chg))
		{
			fl_PartOfBlock* pPending = m_pOwner->getDocLayout()->getPendingWordForSpell();
			// If pending word is later in the block, adjust its
			// offset according to the change
			if (pPending->getOffset() > iOffset)
				pPending->setOffset(pPending->getOffset() + chg);

#if 0
			m_pOwner->getDocLayout()->checkPendingWordForSpell();
//
// Remove the pending word. Trying to spellcheck it is giving us troubles
//

// What kind of trouble? Please refer a Bug # or symptom when
// disabling code like this... Also see Bug 4453    jskov 2003.01.05
		m_pOwner->getDocLayout()->setPendingWordForSpell(NULL,NULL);
#endif
		}
	}

	// Recheck at boundary
	if(getSquiggleType() ==  FL_SQUIGGLE_SPELL) 
	  m_pOwner->_recalcPendingWord(iOffset, chg);
}

/*!
 change of fmt that impacts on spelling (e.g., delete in revisions mode, or undo of delete
 in revisions mode)
 
 \param iOffset Location at which insertion happens
 \param iLength Length of inserted text
*/
void
fl_Squiggles::textRevised(UT_sint32 iOffset, UT_sint32 iLength)
{
	// Ignore operations on shadow blocks
	if (m_pOwner->isHdrFtr())
		return;

	// Return if auto spell-checking disabled
	if (!m_pOwner->getDocLayout()->getAutoSpellCheck())
		return;

	xxx_UT_DEBUGMSG(("fl_Squiggles::textRevised(%d, %d)\n", 
					 iOffset, iLength));

	UT_sint32 chg = iLength;

	// Delete squiggle broken by this insert
	_deleteAtOffset(iOffset);

	if (m_pOwner->getDocLayout()->isPendingWordForSpell() && (getSquiggleType() ==  FL_SQUIGGLE_SPELL) )
	{
		// If not affected by insert, remove it
		if (!m_pOwner->getDocLayout()->touchesPendingWordForSpell(m_pOwner, iOffset, 0))
		{
			//fl_PartOfBlock* pPending = m_pOwner->getDocLayout()->getPendingWordForSpell();

			m_pOwner->getDocLayout()->setPendingWordForSpell(NULL,NULL);
		}
	}

	// Recheck word at boundary
	if(getSquiggleType() ==  FL_SQUIGGLE_SPELL) 
	{
	  m_pOwner->_recalcPendingWord(iOffset, chg);
	}
}

/*!
 Split squiggles
 \param iOffset Offset of split
 \param pNewBL New block

 Move squiggles after the offset to the new block. If there's a
 squiggle spanning the offset, delete it. 

 If the old block is pending a background spell-check, check
 both it and the new block.

 Any pending word is forgotten (since we're splitting the word) and
 the word (if any) at the end of the line is checked, while the word
 at the start of the new line (if any) is made pending.
*/
void
fl_Squiggles::split(UT_sint32 iOffset, fl_BlockLayout* pNewBL)
{
	// Ignore operations on shadow blocks
	if (m_pOwner->isHdrFtr())
		return;

	// Return if auto spell-checking disabled
	if (!m_pOwner->getDocLayout()->getAutoSpellCheck() && (getSquiggleType() ==  FL_SQUIGGLE_SPELL) )
		return;

	xxx_UT_DEBUGMSG(("fl_Squiggles::split(%d, %p)\n", iOffset, pNewBL));

	// When inserting block break, squiggles move in opposite direction
	UT_sint32 chg = -(UT_sint32)iOffset;

	// Check pending word - this is necessary to avoid forgetting
	// words after an undo operation (which undos a block
	// merge). Unfortunately it makes the word under the cursor
	// squiggled (if badly spelled) instead of just pending - but it's
	// hard to do anything about.

	if (m_pOwner->getDocLayout()->isPendingWordForSpell()&& (getSquiggleType() ==  FL_SQUIGGLE_SPELL) )
	{
		fl_PartOfBlock *pPending, *pPOB;
		const fl_BlockLayout *pBL;
		pPending = m_pOwner->getDocLayout()->getPendingWordForSpell();
		pBL = m_pOwner->getDocLayout()->getPendingBlockForSpell();
		// Copy details from pending POB - but don't actually use
		// the object since it's owned by the code handling the
		// pending word.
		pPOB = new fl_PartOfBlock(pPending->getOffset(),
								  pPending->getPTLength());
		// Clear pending word
		m_pOwner->getDocLayout()->setPendingWordForSpell(NULL, NULL);
		if (pBL == m_pOwner)
		{
			if(pPOB->getOffset() >= iOffset)
			{
				// If pending word is in this block after split,
				// adjust details of the copy
				pPOB->setOffset(pPOB->getOffset() + chg);
				pBL = pNewBL;
			}
			else if (pPOB->getOffset() + pPOB->getPTLength() > iOffset)
			{
				// If pending word spans offset, adjust its length
				pPOB->setPTLength(iOffset - pPOB->getOffset());
			}
		}
		pBL->checkWord(pPOB);
	}

	if(getSquiggleType() ==  FL_SQUIGGLE_SPELL)
	{
	  if (m_pOwner->getDocLayout()->dequeueBlockForBackgroundCheck(m_pOwner))
	  {
		// This block was queuing for spell-checking. Do a check of
		// both blocks, but clear any squiggle added at IP.
	    deleteAll();
	    m_pOwner->checkSpelling();
	    pNewBL->checkSpelling();
	    fl_Squiggles * pSq = pNewBL->getSpellSquiggles();
	    UT_return_if_fail( pSq );
	    pSq->_deleteAtOffset(0);
	  }
	  else
	  {
		// This block was already spell-checked, so just move the
		// squiggles around.

		// Remove squiggle broken by this insert
		_deleteAtOffset(iOffset);

		// Move all following squiggles to the new block
		_move(0, chg, pNewBL);

		// Find bounds of word at end of this block and check it.
		// Use _recalcPendingWord which is a bit overkill, but gets
		// the job done.
		if(getSquiggleType() ==  FL_SQUIGGLE_SPELL) 
		  m_pOwner->_recalcPendingWord(iOffset, 0);
		if (m_pOwner->getDocLayout()->isPendingWordForSpell() && (getSquiggleType() ==  FL_SQUIGGLE_SPELL) )
		{
			fl_PartOfBlock *pPending, *pPOB;
			pPending = m_pOwner->getDocLayout()->getPendingWordForSpell();
			// Copy details from pending POB - but don't actually use
			// the object since it's owned by the code handling the
			// pending word.
			pPOB = new fl_PartOfBlock(pPending->getOffset(),
									  pPending->getPTLength());
			m_pOwner->getDocLayout()->setPendingWordForSpell(NULL, NULL);
			m_pOwner->checkWord(pPOB);
		}
	  }
	  m_pOwner->getDocLayout()->setPendingBlockForGrammar(m_pOwner);
	}

	// Set start of new block to be pending word.
	if(getSquiggleType() ==  FL_SQUIGGLE_SPELL) 
	  pNewBL->_recalcPendingWord(0, 0);
}


/*!
 Join squiggles
 \param iOffset Offset to where squiggles are moved
 \param pPrevBlock Block they should be moved to

 This function is called when a paragrah break is deleted and two
 blocks are joined.

 If either block is pending a background spell-check, the combined
 block is checked in full.

 Any squiggle touching the IP is deleted and the word touching the IP
 becomes the pending word. The previously pending word, if any,
 becomes irrelevant.

*/
void
fl_Squiggles::join(UT_sint32 iOffset, fl_BlockLayout* pPrevBL)
{
	// Ignore operations on shadow blocks
	if (m_pOwner->isHdrFtr())
		return;

	// Return if auto spell-checking disabled
	if (!m_pOwner->getDocLayout()->getAutoSpellCheck() && (getSquiggleType() ==  FL_SQUIGGLE_SPELL) )
		return;

	xxx_UT_DEBUGMSG(("fl_Squiggles::join(%d, %p)\n", iOffset, pPrevBL));

	bool bFullCheck = m_pOwner->getDocLayout()->dequeueBlockForBackgroundCheck(m_pOwner);
	bFullCheck |= m_pOwner->getDocLayout()->dequeueBlockForBackgroundCheck(pPrevBL);

	if (bFullCheck)
	{
		// This or the previous block was queuing for
		// spell-checking. Clear all existing squiggles and do a check
		// of the combined block.
		deleteAll();
		pPrevBL->getSpellSquiggles()->deleteAll();
		pPrevBL->checkSpelling();
	}
	else
	{
		// If there is a squiggle first in this block, delete it to
		// prevent having to handle a merge with a squiggle last in
		// the previous block (which we could, but only to delete it
		// below).
		_deleteAtOffset(0);
		// Move all squiggles from this block to the previous block.
		_move(0, iOffset, pPrevBL);
	}
	m_pOwner->getDocLayout()->setPendingBlockForGrammar(m_pOwner);

	// Delete squiggle touching IP
	if(getSquiggleType() ==  FL_SQUIGGLE_SPELL) 
	{
	  fl_Squiggles * pSq = pPrevBL->getSpellSquiggles();
	  UT_return_if_fail( pSq );
	
	  pSq->_deleteAtOffset(iOffset);

	// Update pending word

	  pPrevBL->_recalcPendingWord(iOffset, 0);
	}
}

/*!
 Find squiggles intersecting with region
 \param iStart Start offset of region
 \param iEnd End offset of region
 \result iFirst Index of first squiggle intersecting with region
 \result iLast Index of last squiggle intersecting with region
 \return True if range is not empty, otherwise false
*/
bool
fl_Squiggles::findRange(UT_sint32 iStart, UT_sint32 iEnd,
						UT_sint32& iFirst, UT_sint32& iLast, bool bDontExpand) const
{
	xxx_UT_DEBUGMSG(("fl_Squiggles::findRange(%d, %d)\n", iStart, iEnd));

	UT_sint32 iSquiggles = _getCount();
	if (0 == iSquiggles) return false;
	fl_PartOfBlock* pPOB = 0;
	UT_sint32 s, e;

	if((getSquiggleType() == FL_SQUIGGLE_GRAMMAR) && !bDontExpand)
	{
	  // Grammar squiggles are preceded by a POB that covers the whole of the sentence.
	  // Expand the end point to cover it.

	  UT_sint32 i = 0;
	  for(i=0; i< iSquiggles;i++)
	  {
	    pPOB = getNth(i);
	    if((iStart >= pPOB->getOffset()) && (iStart <= pPOB->getOffset() + pPOB->getPTLength()) && pPOB->isInvisible())
	    {
	      iStart = pPOB->getOffset();
	    }
	    if((iEnd  >= pPOB->getOffset()) && (iEnd <= pPOB->getOffset() + pPOB->getPTLength()) && pPOB->isInvisible())
	    {
	      iEnd = pPOB->getOffset() + pPOB->getPTLength();
	    }
	  }
	}
	// Look for the first POB.start that is higher than the end offset
	_findFirstAfter(iEnd, e);
	// Note that the return value is not checked: either there is no
	// POBs at all, in which case we'll catch it in the statement
	// below (first POB's offset past end point). Otherwise we want to
	// look at the last POB on the line since it may span past
	// iOffset.

	// Return with empty set if the offset of the first POB on the
	// line is higher than the region end (i.e. there is no previous
	// POB that could span the region end).
	if (0 == e)
	{
	  UT_ASSERT(getNth(0)->getOffset() > iEnd);
	  return false;
	}
	// Adjust to be the first POB inside the region.
	e--;

	// FIXME: this should also use _findFirstAfter
	// Look for the last POB.end that is lower than the start offset
	for (s = e; s >= 0; s--)
	{
	  pPOB = getNth(s);
	  if ((pPOB->getOffset() + pPOB->getPTLength()) < iStart) break;
	}
	// Return with empty set if the last POB's end offset is lower
	// than the region start.
	if (s == e)
	{
	  UT_ASSERT((pPOB->getOffset() + pPOB->getPTLength()) < iStart);
	  return false;
	}
	//Adjust to be the first POB inside the region
	s++;
	UT_ASSERT(s >= 0 && s < iSquiggles);
	UT_ASSERT(e >= 0 && e < iSquiggles);
	UT_ASSERT(s <= e);
	
	iFirst = s;
	iLast = e;

	return true;
}


/*!
 Recheck ignored words
 \param pBlockText The block's text 
 \return True if any words squiggled, false otherwise
*/
bool
fl_Squiggles::recheckIgnoredWords(const UT_UCSChar* pBlockText)
{
	xxx_UT_DEBUGMSG(("fl_Squiggles::recheckIgnoredWords(%p)\n", pBlockText));

	bool bUpdate = false;

	UT_sint32 iSquiggles = (UT_sint32) _getCount();
	UT_sint32 i;
	for (i = iSquiggles-1; i >= 0; i--)
	{
		fl_PartOfBlock*	pPOB = getNth((UT_uint32) i);
		
		if (m_pOwner->_doCheckWord(pPOB, pBlockText, false))
		{
			// Word squiggled
			bUpdate = true;
		}
		else
		{
			// Word not squiggled, remove from squiggle list
			_deleteNth((UT_uint32) i);
		}
	}

	return bUpdate;
}

fl_SpellSquiggles::fl_SpellSquiggles(fl_BlockLayout* pOwner) : fl_Squiggles(pOwner,FL_SQUIGGLE_SPELL)
{
}


fl_GrammarSquiggles::fl_GrammarSquiggles(fl_BlockLayout* pOwner) : fl_Squiggles(pOwner,FL_SQUIGGLE_GRAMMAR)
{
}

