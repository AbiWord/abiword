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

fb_SimpleLineBreaker::fb_SimpleLineBreaker() 
{
}

int fb_SimpleLineBreaker::breakParagraph(fl_BlockLayout* pBlock)
{
	fp_Line* pLine = pBlock->getFirstLine();
	UT_ASSERT(pLine);

	fp_RunSplitInfo si;
	UT_Bool bRepeatThisLine = UT_FALSE;
	UT_Bool bForcedLastLine = UT_FALSE;

	while (pLine)
	{
		bRepeatThisLine = UT_FALSE;

		UT_sint32 iMaxLineWidth = 0;
		UT_sint32 iCurLineWidth = 0;

		if (pLine->countRuns() == 0)
		{
			goto skip_to_next_line;
		}
		
		{
			fp_Run* pWalkingRun = pLine->getFirstRun();
			for (;;)
			{
				if (pWalkingRun->getType() == FPRUN_FORCEDLINEBREAK)
				{
					if (
						(pWalkingRun != pLine->getLastRun())
						&& (pWalkingRun->getNext())
						)
					{
						// all runs after this one have got to move
						fp_Line* pNextLine = pLine->getNext();
						if (!pNextLine)
						{
							fp_Line* pNewLine  = pBlock->getNewLine(pLine->getFirstRun()->getHeight());
							UT_ASSERT(pNewLine);	// TODO check for outofmem
							
							pNextLine = pNewLine;
						}

						fp_Run* pTempRun = pLine->getLastRun();
						while (pTempRun != pWalkingRun)
						{
							pLine->removeRun(pTempRun);
							pNextLine->insertRun(pTempRun);
							
							pTempRun = pTempRun->getPrev();
						}
					}
					break;
				}
				
				if (pWalkingRun == pLine->getLastRun())
				{
					break;
				}
				else
				{
					pWalkingRun = pWalkingRun->getNext();
				}
			}
		}
		
		iMaxLineWidth = pLine->getMaxWidth();
		iCurLineWidth = pLine->getWidth();

		if (iCurLineWidth > iMaxLineWidth)
		{
			fp_Run* pFirstRunOnLine = pLine->getFirstRun();
			fp_Run* pLastRunOnLine = pLine->getLastRun();
				
			/*
			  the current line is too long.  we need to break it
			*/
			
			fp_Run* pOffendingRun = NULL;

			/*
			  Regardless of what happens, this line is too long, and
			  we're going to break it, if at all possible.  Whatever
			  gets broken is going to move to the next line.  So,
			  we're going to need to know what our next line is.  If
			  there is no next line, we'll create one.
			*/
			fp_Line* pNextLine = pLine->getNext();
			if (!pNextLine)
			{
				fp_Line* pNewLine  = pBlock->getNewLine(pFirstRunOnLine->getHeight());
				UT_ASSERT(pNewLine);	// TODO check for outofmem
							
				pNextLine = pNewLine;
			}

			/*
			  Our first goal is to find the first run which pushes the
			  length of the line over the limit.  We'll call this the
			  OffendingRun.  If it can be split in such a way that the
			  line will fit, then we consider ourselves lucky and move
			  on.
			*/
			UT_sint32 iWorkingLineWidth = 0;
			fp_Run* pWorkingRun = pFirstRunOnLine;
			UT_Bool bFoundSplit = UT_FALSE;
			UT_Bool bFoundBreakAfter = UT_FALSE;
			
			while (pWorkingRun && (pWorkingRun != pLastRunOnLine->getNext()))
			{
				if ((iWorkingLineWidth + (UT_sint32) pWorkingRun->getWidth()) > iMaxLineWidth)
				{
					// This is the first run which doesn't fit on the line
					pOffendingRun = pWorkingRun;
					
					bFoundSplit = pWorkingRun->findMaxLeftFitSplitPoint(iMaxLineWidth - iWorkingLineWidth, si);
					break;
				}

				iWorkingLineWidth += pWorkingRun->getWidth();
				
				pWorkingRun = pWorkingRun->getNext();
			}

			UT_ASSERT(iWorkingLineWidth <= iMaxLineWidth);

			if (!bFoundSplit)
			{
				/*
				  The run we wanted to split (the one which pushes
				  this line over the limit) cannot be split.  We need
				  to work backwards along the line to find a split
				  point.  As we stop at each run along the way, we'll
				  first check to see if we can break the line after
				  that run.  If not, we'll try to split that run.
				*/

				while (pWorkingRun != pFirstRunOnLine)
				{
					pWorkingRun = pWorkingRun->getPrev();

					if (pWorkingRun->canBreakAfter())
					{
						/*
						  OK, we can break after this
						  run.  Move all the runs after this one
						  onto the next line.
						*/

						bFoundBreakAfter = UT_TRUE;

						break;
					}
					else
					{
						/*
						  Can't break after this run.  Let's
						  see if we can split this run to get
						  something which will fit.
						*/
						bFoundSplit = pWorkingRun->findMaxLeftFitSplitPoint(pWorkingRun->getWidth(), si);

						if (bFoundSplit)
						{
							// a suitable split was found.
							break;
						}
					}
				}
			}

			if (bFoundSplit || bFoundBreakAfter)
			{
				bForcedLastLine = UT_FALSE;
			}
			else
			{
				/*
				  OK.  There are no valid break points on this line,
				  anywhere.  We can't break after any of the runs, nor
				  can we split any of the runs.  We're going to need
				  to force a split of the Offending Run.
				*/

				pWorkingRun = pOffendingRun;
				bFoundSplit = pWorkingRun->findMaxLeftFitSplitPoint(iMaxLineWidth - iWorkingLineWidth, si, UT_TRUE);
				if (bFoundSplit)
				{
					bForcedLastLine = UT_TRUE;
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

					if (pWorkingRun != pFirstRunOnLine)
					{
						/*
						  Force a break right before the offending run.
						*/
						pWorkingRun = pWorkingRun->getPrev();

						bFoundBreakAfter = UT_TRUE;
						bForcedLastLine = UT_TRUE;
					}
				}
			}

			/*
			  All attempts to find a line break are complete.  It is
			  conceivable, although unlikely, that no suitable line
			  break location was found.  For example, if the line
			  contains only one run, and that run cannot be split
			  even with a force, and if that run is wider than the
			  maximum line width, then there is nothing we can do.
			  This case should be very rare, but it will come up
			  more often when we have embedded images supported.
			*/
			
			if (pWorkingRun != pLastRunOnLine)
			{
				/*
				  If the offending run was not the last one on the line,
				  we need to move the ones after it onto the next line.
				*/
				fp_Run* pTempRun = pLastRunOnLine;
				while (pTempRun != pWorkingRun)
				{
					pLine->removeRun(pTempRun);
					pNextLine->insertRun(pTempRun);
							
					pTempRun = pTempRun->getPrev();
				}
			}

			if (bFoundSplit)
			{
				UT_ASSERT(!bFoundBreakAfter);
				
				pWorkingRun->split(si);	// TODO err check this
				UT_ASSERT((UT_sint32)pWorkingRun->getWidth() == si.iLeftWidth);

				fp_Run* pFirstBumpedRun = pWorkingRun->getNext();
				UT_ASSERT(pFirstBumpedRun);
				UT_ASSERT(!(pFirstBumpedRun->getLine()));
						
				pLine->shrink(pFirstBumpedRun->getWidth());

				UT_ASSERT(pNextLine);

				/*
				  The other half of the run we split must be
				  inserted into the next line.
				*/
				pLine->removeRun(pFirstBumpedRun);
				pNextLine->insertRun(pFirstBumpedRun);
				UT_ASSERT(pLine->getWidth() <= pLine->getMaxWidth());
			}
		}
		else if (iCurLineWidth < iMaxLineWidth)
		{
			/*
			  The current line has extra room in it.  we should check
			  to see if something from the next line can be slurped
			  into this one.  Of course, if there is no next line,
			  then there's not much we can do, is there?  :-)
			*/

			if (pLine->getLastRun()->getType() == FPRUN_FORCEDLINEBREAK)
			{
				goto skip_to_next_line;
			}
			
			fp_Line* pTmpLine = pLine->getNext();
			while (pTmpLine && (pTmpLine->countRuns() == 0))
			{
				pTmpLine = pTmpLine->getNext();
			}

			if (pTmpLine)
			{
				UT_sint32 iExtraLineWidth = iMaxLineWidth - iCurLineWidth;

				{
					UT_sint32 iWorkingWidth = 0;
					fp_Run* pFirstRun = pTmpLine->getFirstRun();
					fp_Run* pWorkingRun = pFirstRun;
					fp_Run* pBoundaryRun = NULL;
					UT_Bool bFoundSplit = UT_FALSE;
					UT_Bool bFoundBreakAfter = UT_FALSE;

					while (pWorkingRun)
					{
						if ((iWorkingWidth + (UT_sint32) pWorkingRun->getWidth()) > iExtraLineWidth)
						{
							// We've gone too far.  This one won't fit.
							pBoundaryRun = pWorkingRun;
					
							bFoundSplit = pWorkingRun->findMaxLeftFitSplitPoint(iExtraLineWidth - iWorkingWidth, si);
							break;
						}

						iWorkingWidth += pWorkingRun->getWidth();
				
						pWorkingRun = pWorkingRun->getNext();
					}

					if (!pBoundaryRun)
					{
						// Apparently, all of the remaining runs in the block will fit in our extra space.
						
						UT_ASSERT(!pWorkingRun);
						UT_ASSERT(iWorkingWidth <= iExtraLineWidth);
						
						fp_Run* pTempRun = pFirstRun;
						while (pTempRun)
						{
							fp_Line* pTempLine = pTempRun->getLine();
							pTempLine->removeRun(pTempRun);
							pLine->addRun(pTempRun);
							
							pTempRun = pTempRun->getNext();
						}
					}
					else
					{
						/*
						  OK.  We found a boundary run.
						*/

						if (!bFoundSplit)
						{
							/*
							  The boundary run wouldn't split.  We'll have to look
							  backwards for a break point.
							*/
							pWorkingRun = pBoundaryRun;
							while (pWorkingRun != pFirstRun)
							{
								pWorkingRun = pWorkingRun->getPrev();

								if (pWorkingRun->canBreakAfter())
								{
										/*
										  OK, we can break after this
										  run.
										*/

									bFoundBreakAfter = UT_TRUE;
									break;
								}
								else
								{
										/*
										  Can't break after this run.  Let's
										  see if we can split this run to get
										  something which will fit.
										*/
									bFoundSplit = pWorkingRun->findMaxLeftFitSplitPoint(pWorkingRun->getWidth(), si);

									if (bFoundSplit)
									{
										// a suitable split was found.
										break;
									}
								}
							}
						}

						/*
						  OK, our slurp search is done.
						*/

						if (bFoundSplit || bFoundBreakAfter)
						{
							bForcedLastLine = UT_FALSE;
							
							fp_Run* pTempRun = pFirstRun;
							while (pTempRun)
							{
								if ((pTempRun == pWorkingRun) && bFoundSplit)
								{
									break;
								}
									
								fp_Line* pTempLine = pTempRun->getLine();
								pTempLine->removeRun(pTempRun);
								pLine->addRun(pTempRun);

								if (pTempRun == pWorkingRun)
								{
									break;
								}
							
								pTempRun = pTempRun->getNext();
							}

							if (bFoundSplit)
							{
								UT_ASSERT(!bFoundBreakAfter);
				
								fp_Line* pLineOfSplitRun = pWorkingRun->getLine();
								pLineOfSplitRun->removeRun(pWorkingRun);
								UT_ASSERT(!(pWorkingRun->getLine()));
				
								pWorkingRun->split(si);	// TODO err check this
								UT_ASSERT((UT_sint32)pWorkingRun->getWidth() == si.iLeftWidth);

								fp_Run* pRightHalfOfSplitRun = pWorkingRun->getNext();
								UT_ASSERT(pRightHalfOfSplitRun);
								UT_ASSERT(!(pRightHalfOfSplitRun->getLine()));

								pLine->addRun(pWorkingRun);
								pLineOfSplitRun->insertRun(pRightHalfOfSplitRun);
				
								UT_ASSERT(pLine->getWidth() <= pLine->getMaxWidth());
							}
						}
						else
						{
							if (!bForcedLastLine && (pLine->countRuns() > 0) && !(pLine->getLastRun()->canBreakAfter()))
							{
								/*
								  The last run on this line is telling us that a
								  line break cannot be placed after it.
								  Nonetheless, there already IS a line break after
								  it.
								*/

								fp_Run* pTempRun = pFirstRun;
								while (pTempRun)
								{
									fp_Line* pTempLine = pTempRun->getLine();
									pTempLine->removeRun(pTempRun);
									pLine->addRun(pTempRun);

									if (pTempRun == pBoundaryRun)
									{
										break;
									}
							
									pTempRun = pTempRun->getNext();
								}

								bRepeatThisLine = UT_TRUE;
							}
						}
					}
				}
			}
		}
		else
		{
			UT_ASSERT(iCurLineWidth == iMaxLineWidth);
			bForcedLastLine = UT_FALSE;
		}

skip_to_next_line:
		
		if (!bRepeatThisLine)
		{
			pLine = pLine->getNext();
		}
	}

	return 0; // TODO return code
}

