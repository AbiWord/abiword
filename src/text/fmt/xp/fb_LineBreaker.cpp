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



#include <stdlib.h>

#include "fb_LineBreaker.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"

#include "ut_assert.h"

fb_LineBreaker::fb_LineBreaker()
{
}

UT_sint32 fb_LineBreaker::breakParagraph(fl_BlockLayout* pBlock)
{
	fp_Line* pLine = pBlock->getFirstLine();
	UT_ASSERT(pLine);

	fp_RunSplitInfo si;

	while (pLine)
	{
		if (pLine->countRuns() > 0)
		{
			fp_Run* pFirstRunToKeep = pLine->getFirstRun();
			fp_Run*	pLastRunToKeep = NULL;
			
			UT_sint32 iMaxLineWidth = pLine->getMaxWidth();
			UT_sint32 iWorkingLineWidth = 0;
			
			UT_Bool bFoundBreakAfter = UT_FALSE;
			UT_Bool bFoundSplit = UT_FALSE;
			
			fp_Run* pRunToSplit = NULL;
			fp_Run* pOtherHalfOfSplitRun = NULL;
			fp_Run* pOffendingRun = NULL;
			
			fp_Run* pCurrentRun = pFirstRunToKeep;
			while (pCurrentRun)
			{
				unsigned char iCurRunType = pCurrentRun->getType();

				switch (iCurRunType)
				{
				case FPRUN_FORCEDLINEBREAK:
				{
					pLastRunToKeep = pCurrentRun;
					goto done_with_run_loop;
				}
				case FPRUN_TAB:
				{
					/*
					  find the position of this tab and its type.
					  if it's a left tab, then add its width to the iWorkingLineWidth
					*/

					UT_sint32 iPos;
					unsigned char iType;

					UT_Bool bRes = pLine->findNextTabStop(iWorkingLineWidth, iPos, iType);
					if (bRes)
					{
						UT_ASSERT(iType == FL_TAB_LEFT);	// TODO right and center tabs

						UT_ASSERT(iPos > iWorkingLineWidth);
					
						iWorkingLineWidth = iPos;
					}
					else
					{
						// tab won't fit.  bump it to the next line
						UT_ASSERT(pCurrentRun->getPrev());
						UT_ASSERT(pCurrentRun != pFirstRunToKeep);
						
						pLastRunToKeep = pCurrentRun->getPrev();
						goto done_with_run_loop;
					}
					break;
				}
				case FPRUN_FIELD:
				case FPRUN_IMAGE:
				case FPRUN_TEXT:
				{
					/* if this run doesn't fit on the line... */
					if ((iWorkingLineWidth + pCurrentRun->getWidth()) > iMaxLineWidth)
					{
						// This is the first run which doesn't fit on the line
						pOffendingRun = pCurrentRun;
					
						bFoundSplit = pOffendingRun->findMaxLeftFitSplitPoint(iMaxLineWidth - iWorkingLineWidth, si);
						if (bFoundSplit)
						{
							pRunToSplit = pOffendingRun;
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
							while (pRunLookingBackwards != pFirstRunToKeep)
							{
								pRunLookingBackwards = pRunLookingBackwards->getPrev();

								if (pRunLookingBackwards->canBreakAfter())
								{
									/*
									  OK, we can break after this
									  run.  Move all the runs after this one
									  onto the next line.
									*/

									bFoundBreakAfter = UT_TRUE;
									pLastRunToKeep = pRunLookingBackwards;

									break;
								}
								else
								{
									/*
									  Can't break after this run.  Let's
									  see if we can split this run to get
									  something which will fit.
									*/
									bFoundSplit = pRunLookingBackwards->findMaxLeftFitSplitPoint(pRunLookingBackwards->getWidth(), si);

									if (bFoundSplit)
									{
										// a suitable split was found.
										
										pRunToSplit = pRunLookingBackwards;
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

							bFoundSplit = pOffendingRun->findMaxLeftFitSplitPoint(iMaxLineWidth - iWorkingLineWidth, si, UT_TRUE);
							if (bFoundSplit)
							{
								pRunToSplit = pOffendingRun;
							}
							else
							{
								/*
								  Wow!  This is a very resilient run.  It is the
								  run which no longer fits, and yet it cannot be
								  split.  It might be a single-character run.
								  Perhaps it's an image.  Anyway, we still have to
								  try as hard as we can to find a line break.
								*/

								if (pOffendingRun != pFirstRunToKeep)
								{
									/*
									  Force a break right before the offending run.
									*/
									pLastRunToKeep = pOffendingRun->getPrev();

									bFoundBreakAfter = UT_TRUE;
								}
							}
						}

						if (bFoundSplit)
						{
							UT_ASSERT(!bFoundBreakAfter);
				
							pRunToSplit->split(si);	// TODO err check this
							UT_ASSERT((UT_sint32)pRunToSplit->getWidth() == si.iLeftWidth);

							pOtherHalfOfSplitRun = pRunToSplit->getNext();
							UT_ASSERT(pOtherHalfOfSplitRun);
							UT_ASSERT(!(pOtherHalfOfSplitRun->getLine()));
							pLastRunToKeep = pRunToSplit;
						}
						
						goto done_with_run_loop;
						
					} /* if this run doesn't fit on the line */

					iWorkingLineWidth += pCurrentRun->getWidth();
				
					break;
				}
				default:
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				} /* switch */
				
				pCurrentRun = pCurrentRun->getNext();
			} /* the run loop */

		done_with_run_loop:
			
			/*
			  OK, we've gone through the run loop.  If a run was to
			  be split, it has already been split.  pLastRunToKeep
			  should now be set to the last run which should be on
			  this line.  We need to make sure that all runs from
			  the first one on the line up until pLastRunToKeep are
			  actually on this line.  Furthermore, we need to make
			  sure that no other runs are on this line.
			*/

			/*
			  If pLastRunToKeep is NULL here, that means that
			  all remaining runs in this block will fit on this
			  line.
			*/
			
			pCurrentRun = pFirstRunToKeep;
			while (pCurrentRun)
			{
				if (pCurrentRun->getLine() != pLine)
				{
					fp_Line* pOtherLine = pCurrentRun->getLine();
					UT_ASSERT(pOtherLine);

					pOtherLine->removeRun(pCurrentRun);
					pLine->addRun(pCurrentRun);
				}

				if (pCurrentRun == pLastRunToKeep)
				{
					break;
				}
				else
				{
					pCurrentRun = pCurrentRun->getNext();
				}
			}

			fp_Line* pNextLine = NULL;

			if (
				pOtherHalfOfSplitRun
				|| (
					pLastRunToKeep
					&& (pLine->getLastRun() != pLastRunToKeep)
					)
				)
			{
				// make sure there is a next line
				pNextLine = pLine->getNext();
				if (!pNextLine)
				{
					fp_Line* pNewLine  = pBlock->getNewLine(pLine->getFirstRun()->getHeight());
					UT_ASSERT(pNewLine);	// TODO check for outofmem
							
					pNextLine = pNewLine;
				}

				fp_Run* pRunToBump = pLine->getLastRun();
				while (pLine->getLastRun() != pLastRunToKeep)
				{
					UT_ASSERT(pRunToBump->getLine() == pLine);

					pLine->removeRun(pRunToBump);
					pNextLine->insertRun(pRunToBump);

					pRunToBump = pRunToBump->getPrev();
				}

				if (pOtherHalfOfSplitRun)
				{
					pNextLine->insertRun(pOtherHalfOfSplitRun);
				}
			}

			UT_ASSERT((!pLastRunToKeep) || (pLine->getLastRun() == pLastRunToKeep));
			
			/*
			  Now we know all the runs which belong on this line.
			  However, those runs are not properly positioned.  We
			  call the line to do the actual layout.
			*/

			pLine->layout();
			  
		} /* if countruns > 0 */
		
		pLine = pLine->getNext();
	}

	return 0; // TODO return code
}

