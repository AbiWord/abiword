/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
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

#include <stdlib.h>

#include "fb_LineBreaker.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fp_Column.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"

fb_LineBreaker::fb_LineBreaker()
:
	m_pFirstRunToKeep(NULL),
	m_pLastRunToKeep(NULL),
	m_iMaxLineWidth(0),
	m_iWorkingLineWidth(0)
{
	xxx_UT_DEBUGMSG(("fb_LineBreaker %x created \n",this));
}

fb_LineBreaker::~fb_LineBreaker(void)
{
	xxx_UT_DEBUGMSG(("fb_LineBreaker %x destroyed \n",this));
}


/*!
  Break paragraph of text into lines
  \param pBlock Paragraph (block) of text
  \return 0

  LineBreaker shouldn't break a line until it finds a non-blank
  item past the end of the line.
  All trailing spaces should remain on the end of the line.
*/
UT_sint32
fb_LineBreaker::breakParagraph(fl_BlockLayout* pBlock, fp_Line * pLineToStartAt)
{
	// FIXME: This should:
	//	 o Check the Runs for state signalling if they changed since
	//	   last paragraph layout.
	//	 o Only layout lines starting with, and below, the first line
	//	   which contain changed Runs.
	//	 o If the block bounding box or layout properties change, it
	//	   should force a full layout.
	//	 o Also see fix me at end of loop
	fp_Line* pLine = static_cast<fp_Line *>(pBlock->getFirstContainer());
	UT_ASSERT(pLine);
	if(pLineToStartAt)
	{
		while(pLine && pLine != pLineToStartAt)
		{
			pLine = static_cast<fp_Line *>(pLine->getNext());
#if DEBUG
			pLine->assertLineListIntegrity();
#endif
		}
	}

	while (pLine)
	{
#if DEBUG
		pLine->assertLineListIntegrity();
#endif
		UT_uint32 iIndx = 0;
		if (pLine->countRuns() > 0)
		{

			fp_Run *pOriginalFirstOnLine = pLine->getFirstRun();
			fp_Run *pOriginalLastOnLine = pLine->getLastRun();

			pLine->resetJustification();

			m_pFirstRunToKeep = pLine->getFirstRun();
			m_pLastRunToKeep = NULL;
			
			m_iMaxLineWidth = pLine->getMaxWidth();

			m_iWorkingLineWidth = 0;

//			bool bFoundBreakAfter = false;
//			bool bFoundSplit = false;

//			fp_TextRun* pRunToSplit = NULL;
//			fp_TextRun* pOtherHalfOfSplitRun = NULL;

			fp_Run* pOffendingRun = NULL;

			fp_Run* pCurrentRun = m_pFirstRunToKeep;
			fp_Run* pPreviousRun = NULL;

			while (true)
			{
				// If this run is past the end of the line...

				bool bRunIsNonBlank = true;
				if(pCurrentRun)
				{
					if(!pCurrentRun->doesContainNonBlankData())
					{
						bRunIsNonBlank = false;
					}
				}
				if (bRunIsNonBlank && (m_iWorkingLineWidth	> m_iMaxLineWidth))
				{
					// This is the first run which will start past the
					// end of the line

					UT_sint32 iTrailingSpace = 0;

					fp_Run * pArun = (pPreviousRun ? pPreviousRun : pCurrentRun);
					iTrailingSpace = _moveBackToFirstNonBlankData(pArun,
											  &pOffendingRun);

					m_iWorkingLineWidth -= iTrailingSpace;
					if (m_iWorkingLineWidth > m_iMaxLineWidth)
					{
						// This run needs splitting.

						UT_ASSERT(pOffendingRun);
						_splitAtOrBeforeThisRun(pOffendingRun);
						goto done_with_run_loop;
					}
					else
					{
						if(pCurrentRun)
						{
							_splitAtNextNonBlank(pCurrentRun);
						}
						goto done_with_run_loop;
					}
				}
				if(!pCurrentRun)
					break;
				
				m_iWorkingLineWidth += pCurrentRun->getWidth();

				unsigned char iCurRunType = pCurrentRun->getType();

				switch (iCurRunType)
				{
				case FPRUN_FORCEDCOLUMNBREAK:
				{
					m_pLastRunToKeep = pCurrentRun;
					goto done_with_run_loop;
				}
				case FPRUN_FORCEDPAGEBREAK:
				{
					m_pLastRunToKeep = pCurrentRun;
					goto done_with_run_loop;
				}
				case FPRUN_FORCEDLINEBREAK:
				{
					m_pLastRunToKeep = pCurrentRun;
					goto done_with_run_loop;
				}
				case FPRUN_ENDOFPARAGRAPH:
				{
					m_pLastRunToKeep = pCurrentRun;
					goto done_with_run_loop;
				}

				case FPRUN_TAB:
				{
					/*
						if this run is not on the current line, we have a problem
						because the width of the run is dependent on its position
						on the line and the  postion can only be determined after
						the run has been added to the line. So we have to take
						care of this and we also have to add any runs between the
						last run on the line and this one
					*/

					fp_Line * pRunsLine = pCurrentRun->getLine();
					UT_ASSERT(pRunsLine);

					if(pRunsLine != pLine)
					{
						xxx_UT_DEBUGMSG(("fb_LineBreaker::breakLine: Tab run (0x%x) belonging to different line\n"
									 "		 pLine 0x%x, pRunsLine 0x%x\n"
									 ,pCurrentRun, pLine, pRunsLine));

						if(pOriginalLastOnLine)
						{
							// if there are some runs between this tab ran and the last run that the line knows
							// belongs to it; we have to add these as well
							fp_Run * pRun = pOriginalLastOnLine->getNext();
							while(pRun)
							{
								fp_Line * pL = pRun->getLine();
								if(pL)
								{
									pL->removeRun(pRun,true);
									pLine->addRun(pRun);
								}
								if(pRun == pCurrentRun)
									break;

								pRun = pRun->getNext();
							}
#if DEBUG
							pLine->assertLineListIntegrity();
#endif
						}
					}

					FL_WORKING_DIRECTION eWorkingDirection;
					FL_WHICH_TABSTOP eUseTabStop;
					
					m_iWorkingLineWidth -= pCurrentRun->getWidth();

					pLine->getWorkingDirectionAndTabstops(eWorkingDirection, eUseTabStop);
					pLine->calculateWidthOfRun(m_iWorkingLineWidth,iIndx,eWorkingDirection,eUseTabStop);
					break;
				}
				case FPRUN_FMTMARK:
					break;

				case FPRUN_FIELD:
				case FPRUN_IMAGE:
				case FPRUN_FIELDSTARTRUN:
				case FPRUN_FIELDENDRUN:
				case FPRUN_BOOKMARK:
				case FPRUN_TEXT:
				case FPRUN_HYPERLINK:
				case FPRUN_DIRECTIONMARKER:
				{

					break;
				}
				default:
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				} // switch

				pPreviousRun = pCurrentRun;
				pCurrentRun = pCurrentRun->getNext();

				iIndx++;

			} // the run loop

		done_with_run_loop:
			/*
			  OK, we've gone through the run loop.	If a run was to
			  be split, it has already been split.	m_pLastRunToKeep
			  should now be set to the last run which should be on
			  this line.  We need to make sure that all runs from
			  the first one on the line up until m_pLastRunToKeep are
			  actually on this line.  Furthermore, we need to make
			  sure that no other runs are on this line.
			*/

			_breakTheLineAtLastRunToKeep(pLine, pBlock);

			/*
			  Now we know all the runs which belong on this line.
			  However, those runs are not properly positioned.	We
			  call the line to do the actual layout.
			*/

			// First clear the line if it has been modified.

			if(
				pOriginalFirstOnLine != pLine->getFirstRun() ||
				pOriginalLastOnLine != pLine->getLastRun()
				)
			{
				pLine->clearScreen();
			}
			// FIXME: This should go in the block above. But only when
			// we are guaranteed a full re-layout on block changes.
			pLine->layout();

			pLine = static_cast<fp_Line *>(pLine->getNext());
		} // if countruns > 0
		else
		{
			// if there are no runs left on this line, we have to remove the line
			// from the block; we cannot leave this to the block format rutine as we
			// have done in the past, since if the next line becomes first line as the
			// result of this, its width and starting offest changes and have to be
			// recalculated _before_ this loop proceeds (this is take care off by
			// fl_BlockLayout::_removeLine()
			fp_Line *pOldLine = pLine;
			pLine = static_cast<fp_Line *>(pLine->getNext());

			pBlock->_removeLine(pOldLine);
		}


	}

	return 0; // TODO return code
}

UT_sint32 fb_LineBreaker::_moveBackToFirstNonBlankData(fp_Run *pCurrentRun, fp_Run **pOffendingRun)
{
	// need to move back untill we find the first non blank character and
	// return the distance back to this character.

	UT_sint32 iTrailingBlank = 0;

	do
	{
		if(!pCurrentRun->doesContainNonBlankData())
		{
			iTrailingBlank += pCurrentRun->getWidth();
		}
		else
		{
			iTrailingBlank += pCurrentRun->findTrailingSpaceDistance();
			break;
		}

		pCurrentRun = pCurrentRun->getPrev();
	}
	while(pCurrentRun);

	*pOffendingRun = pCurrentRun;

	return iTrailingBlank;
}


bool fb_LineBreaker::_splitAtOrBeforeThisRun(fp_Run *pCurrentRun)
{

	// This run is past the end of the line.

	// Reminder: m_iWorkingLineWidth = Length including this run minus any trailing spaces in this run.

	// Set m_iWorkingLineWidth to length including this run.
	m_iWorkingLineWidth += pCurrentRun->findTrailingSpaceDistance();
	m_iWorkingLineWidth -= pCurrentRun->getWidth();

	fp_Run *pOffendingRun = pCurrentRun;
	fp_RunSplitInfo splitInfo;
	fp_TextRun *pRunToSplit = NULL;

	bool bFoundBreakAfter = false;
	bool bFoundSplit = pOffendingRun->findMaxLeftFitSplitPoint(m_iMaxLineWidth - m_iWorkingLineWidth, splitInfo);
	if (bFoundSplit)
	{
		UT_ASSERT(pOffendingRun->getType() == FPRUN_TEXT);
		pRunToSplit = static_cast<fp_TextRun*>(pOffendingRun);
	}
	else
	{

		/*
		  The run we wanted to split (the one which pushes
		  this line over the limit) cannot be split.  We need
		  to work backwards along the line to find a split
		  point.  As we stop at each run along the way, we'll
		  first check to see if we can break the line after
		  that run.  If not, we'll try to split that run.
		*/

		fp_Run* pRunLookingBackwards = pCurrentRun;
		while (pRunLookingBackwards != m_pFirstRunToKeep)
		{
			pRunLookingBackwards = pRunLookingBackwards->getPrev();

			if ( !pRunLookingBackwards )
			  {
				bFoundBreakAfter = false;
				m_pLastRunToKeep = pCurrentRun;
				break;
			  }
			else if (pRunLookingBackwards->canBreakAfter())
			{
				/*
				  OK, we can break after this
				  run.	Move all the runs after this one
				  onto the next line.
				*/

				bFoundBreakAfter = true;
				m_pLastRunToKeep = pRunLookingBackwards;

				break;
			}
			else
			{
				/*
				  Can't break after this run.  Let's
				  see if we can split this run to get
				  something which will fit.
				*/
				bFoundSplit = pRunLookingBackwards->findMaxLeftFitSplitPoint(pRunLookingBackwards->getWidth(), splitInfo);

				if (bFoundSplit)
				{
					// a suitable split was found.

					UT_ASSERT(pRunLookingBackwards->getType() == FPRUN_TEXT);

					pRunToSplit = static_cast<fp_TextRun*>(pRunLookingBackwards);
					break;
				}
			}
		}
	}

	if (!(bFoundSplit || bFoundBreakAfter))
	{
		/*
		  OK.  There are no valid break points on this line,
		  anywhere.  We can't break after any of the runs, nor
		  can we split any of the runs.  We're going to need
		  to force a split of the Offending Run.
		*/
		bFoundSplit = pOffendingRun->findMaxLeftFitSplitPoint(m_iMaxLineWidth - m_iWorkingLineWidth, splitInfo, true);

		if (bFoundSplit)
		{
			UT_ASSERT(pOffendingRun->getType() == FPRUN_TEXT);
			pRunToSplit = static_cast<fp_TextRun*>(pOffendingRun);
		}
		else
		{
			/*
			  Wow!	This is a very resilient run.  It is the
			  run which no longer fits, and yet it cannot be
			  split.  It might be a single-character run.
			  Perhaps it's an image.  Anyway, we still have to
			  try as hard as we can to find a line break.
			*/

			if (pOffendingRun != m_pFirstRunToKeep)
			{
				/*
				  Force a break right before the offending run.
				*/
				m_pLastRunToKeep = pOffendingRun->getPrev();

				bFoundBreakAfter = true;
			}
			else
			{
				// nothing else we can do but this.
				m_pLastRunToKeep = pOffendingRun;
				bFoundBreakAfter = true;
			}
		}
	}

	if (bFoundSplit)
	{
		UT_ASSERT(!bFoundBreakAfter);

		_splitRunAt(pRunToSplit, splitInfo);

	}


	return true;
}

bool fb_LineBreaker::_splitAtNextNonBlank(fp_Run *pCurrentRun)
{
	fp_RunSplitInfo splitInfo;
	bool	bCanSplit = pCurrentRun->findFirstNonBlankSplitPoint(splitInfo);

	if(bCanSplit)
	{
		_splitRunAt(pCurrentRun, splitInfo);
	}
	else
	{
		// cannot split run so split before.

		m_pLastRunToKeep = pCurrentRun->getPrev();
	}

	return true;
}

void fb_LineBreaker::_splitRunAt(fp_Run *pCurrentRun, fp_RunSplitInfo &splitInfo)
{
	UT_ASSERT(pCurrentRun->getType() == FPRUN_TEXT);
	fp_TextRun *pRunToSplit = static_cast<fp_TextRun*>(pCurrentRun);

	pRunToSplit->split(splitInfo.iOffset + 1);	// TODO err check this
	UT_ASSERT(pRunToSplit->getNext());
	UT_ASSERT(pRunToSplit->getNext()->getType() == FPRUN_TEXT);

	fp_TextRun *pOtherHalfOfSplitRun;
	pOtherHalfOfSplitRun = static_cast<fp_TextRun*>(pRunToSplit->getNext());

// todo decide if we need to call recalcWidth() now on the 2 pieces.
//							pRunToSplit->recalcWidth();
//							pOtherHalfOfSplitRun->recalcWidth();
	UT_ASSERT(static_cast<UT_sint32>(pRunToSplit->getWidth()) == splitInfo.iLeftWidth);

	UT_ASSERT(pOtherHalfOfSplitRun);
	UT_ASSERT(pOtherHalfOfSplitRun->getLine() == pRunToSplit->getLine());
	m_pLastRunToKeep = pRunToSplit;
}

void fb_LineBreaker::_breakTheLineAtLastRunToKeep(fp_Line *pLine,
												  fl_BlockLayout *pBlock)
{

	/*
	  If m_pLastRunToKeep is NULL here, that means that
	  all remaining runs in this block will fit on this
	  line.
	*/

	fp_Run *pCurrentRun = m_pFirstRunToKeep;
	while (pCurrentRun)
	{
		if (pCurrentRun->getLine() != pLine)
		{
			fp_Line* pOtherLine = pCurrentRun->getLine();
			UT_ASSERT(pOtherLine);

			pOtherLine->removeRun(pCurrentRun, true);
			pLine->addRun(pCurrentRun);
		}
		if (pCurrentRun == m_pLastRunToKeep)
		{
			break;
		}
		else
		{
			pCurrentRun = pCurrentRun->getNext();
		}
	}

	fp_Line* pNextLine = NULL;
	xxx_UT_DEBUGMSG(("fb_LineBreaker::_breakThe ... \n"));
	if ( m_pLastRunToKeep != NULL
		&& (pLine->getLastRun() != m_pLastRunToKeep)
		)
	{
		// make sure there is a next line
		pNextLine = static_cast<fp_Line *>(pLine->getNext());
		if (!pNextLine)
		{
			fp_Line* pNewLine  = static_cast<fp_Line *>(pBlock->getNewContainer());
			UT_ASSERT(pNewLine);	// TODO check for outofmem
			pNextLine = pNewLine;
			xxx_UT_DEBUGMSG(("!!!! Generated a new Line \n"));
		}
		else
		{
			UT_ASSERT(pNextLine->getContainerType() == FP_CONTAINER_LINE);
			xxx_UT_DEBUGMSG(("fb_LineBreaker::_breakThe ... pLine 0x%x, pNextLine 0x%x, blocks last 0x%x\n",
			pLine, pNextLine, pBlock->getLastContainer()));
			if(pBlock->getLastContainer() == static_cast<fp_Container *>(pLine))
				pBlock->setLastContainer(pNextLine);
		}

		fp_Run* pRunToBump = pLine->getLastRun();
		UT_ASSERT(pRunToBump);
		xxx_UT_DEBUGMSG(("!!!RunToBump %x Type %d Offset %d Length %d \n",pRunToBump,pRunToBump->getType(),pRunToBump->getBlockOffset(),pRunToBump->getLength()));

		while (pRunToBump && pLine->getNumRunsInLine() && (pLine->getLastRun() != m_pLastRunToKeep))
		{
			UT_ASSERT(pRunToBump->getLine() == pLine);
			xxx_UT_DEBUGMSG(("RunToBump %x Type %d Offset %d Length %d \n",pRunToBump,pRunToBump->getType(),pRunToBump->getBlockOffset(),pRunToBump->getLength()));
			pLine->removeRun(pRunToBump);
			
			UT_ASSERT(pLine->getLastRun()->getType() != FPRUN_ENDOFPARAGRAPH);
			pNextLine->insertRun(pRunToBump);

			pRunToBump = pRunToBump->getPrev();
			xxx_UT_DEBUGMSG(("Next runToBump %x \n",pRunToBump));
		}
	}

	UT_ASSERT((!m_pLastRunToKeep) || (pLine->getLastRun() == m_pLastRunToKeep));
#if DEBUG
	pLine->assertLineListIntegrity();
	if(pNextLine)
	{
		pNextLine->assertLineListIntegrity();
	}
#endif
}

