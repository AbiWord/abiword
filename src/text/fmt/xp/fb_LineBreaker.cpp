 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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

/*
  The lines have already been created for this paragraph.  Rather
  than just trash them all and start over, we actually need to iterate
  over the lines so we can figure out if the runs contained in the line
  have moved on the screen.  If they have indeed moved, we need to repaint
  them
*/
int fb_SimpleLineBreaker::reLayoutParagraph(fl_BlockLayout* pBlock)
{
	UT_Bool bDone = UT_FALSE;
	fp_Line* pLine = pBlock->getFirstLine();
	fp_RunSplitInfo si;
	UT_Bool bRedrawLine = UT_FALSE;
	UT_Bool bLineOnScreen = UT_TRUE;
	
	while (pLine && (bDone == UT_FALSE))
	{
		UT_sint32 iMaxWidth = pLine->getMaxWidth();
		UT_Bool bAddLine = UT_FALSE;
		
		// This line has been affected by the edit ...
		if (pLine->m_bDirty)
		{
			UT_sint32 xoff, yoff, screenWidth, screenHeight;

			if (bLineOnScreen == UT_TRUE)
			{
				if (pLine->countRuns() != 0)
				{
					fp_Run* pRun = pLine->getFirstRun();
					pLine->getScreenOffsets(pRun, pRun->getLineData(), xoff,
											yoff, screenWidth, screenHeight);

					if (yoff > screenHeight)
						bLineOnScreen = UT_FALSE;
				}
			}
			
			pLine->m_bDirty = UT_FALSE;

			int iWidth = 0;
			// this line no longer fits ...
			if (pLine->getWidth() > pLine->getMaxWidth())
			{
				fp_Run* pRun = pLine->getFirstRun();
				fp_Run* pLastRun = pLine->getLastRun();
				fp_Run* pNewLastRun = NULL;
				
				while (pRun && pRun != pLastRun->getNext())
				{
//					if (bRedrawLine && bLineOnScreen)
//						pRun->clearScreen();
					
					iWidth += pRun->getWidth();

					// This is the run that needs to be broken
					if (iWidth > iMaxWidth)
					{
						// subtract the width back off for breaking ...
						UT_sint32 rWidth = iWidth - pRun->getWidth();

						int bFoundSplit = pRun->findMaxLeftFitSplitPoint(iMaxWidth - rWidth, si);

						pNewLastRun = pRun;
						
						if (bFoundSplit)
						{
							// it fits.  split it and move on
							pRun->split(si);	// TODO err check this
							UT_ASSERT((UT_sint32)pRun->getWidth() == si.iLeftWidth);

							// We've found a split, but it's not our last
							// run - remove all runs after this one.
							if (pRun != pLine->getLastRun())
							{
								fp_Run *tmp = pRun->getNext();

								while (tmp != pLine->getLastRun())
								{
									pLine->removeRun(tmp);
									tmp = tmp->getNext();
								}
								pLine->removeRun(pLine->getLastRun());
							}
							pRun = pRun->getNext();
							pLine->shrink(pRun->getWidth());
						}
						else
						{
							// this run can't be split - remove it and
							// all following runs to the next line.
							// TODO However, if we are the first run on the
							// TODO line, this line can't be broken.  Skip
							// TODO it ?

							if (pRun == pLine->getFirstRun())
							{
								pLine->align();
								pLine->draw(pRun->getGraphics());
								return 0;
							}
							 
							UT_ASSERT(pLastRun == pLine->getLastRun());
							
							pNewLastRun = pRun->getPrev();

							// This is the last run on the line - just remove
							// it
							if (pRun == pLastRun)
							{
								pLine->removeRun(pRun);
							}
							else
							{
								fp_Run* pTmpRun = pLastRun;
							
								UT_ASSERT(pLastRun == pLine->getLastRun());
								while (pTmpRun != pNewLastRun)
								{
									pLine->removeRun(pTmpRun);
									pTmpRun = pTmpRun->getPrev();
								}
							}
						}
						
						// after a split, there is always a next.
						UT_ASSERT(pRun);

						pLine->align();
						pLine->draw(pRun->getGraphics());
						
						pLine = pLine->getNext();

						if (!pLine)
						{
							UT_uint32 iGuessLineHeight = pRun->getHeight();
							
							UT_uint32 iMaxLineWidth = pBlock->requestLineSpace(iGuessLineHeight);
							UT_ASSERT(iMaxLineWidth > 0);
							
							pLine = new fp_Line(iMaxLineWidth);
							pLine->m_bDirty = UT_TRUE;
							bAddLine = UT_TRUE;
						}
						
						UT_ASSERT(pLine);

						iMaxWidth = pLine->getMaxWidth();

						fp_Run* pTmpRun = pLastRun;
							
						// if this is the newly created run from
						// a split, we need to create a new
						// line data pointer for it
						if (pLastRun == pNewLastRun && bFoundSplit)
						{
							pLine->insertRun(pRun, (bAddLine == UT_FALSE), UT_TRUE);
						}

						while (pTmpRun != pNewLastRun)
						{
							if (!pTmpRun->getLineData())
							{
								pLine->insertRun(pTmpRun,
												 (bAddLine == UT_FALSE),
												 UT_TRUE);
							}
							else
							{
								pLine->insertRun(pTmpRun,
												 (bAddLine == UT_FALSE),
												 UT_FALSE);
							}

							pTmpRun = pTmpRun->getPrev();
						}

						if (bAddLine)
						{
							pBlock->addLine(pLine);
							bAddLine = UT_FALSE;
						}
						
						// This line just had an element inserted at the
						// beginning of it - redraw the whole thing
						bRedrawLine = UT_TRUE;
						
						pLine->m_bDirty = UT_TRUE;

						iWidth = 0;
						pLastRun = pLine->getLastRun();
						continue;
					}

					pRun = pRun->getNext();
				}

				// if width < maxWidth, we're done for this line, now
				// check the runs on the next line to see if they will fit
				if (iWidth <= iMaxWidth)
				{
					pLine->align();
					pLine->draw(pLine->getFirstRun()->getGraphics());
					return 0;
				}
			}
			else
			{
				fp_Line* pRealNextLine = pLine->getNext();
				fp_Line* pNextLine = pRealNextLine;
				UT_uint32 iWidthLooking = 0;
				UT_uint32 iSpaceLeft = pLine->getMaxWidth() - pLine->getWidth();
				
				if (pRealNextLine)
				{
					while (pNextLine && pNextLine->countRuns() == 0)
					{
						pNextLine->m_bDirty = UT_TRUE;
						pNextLine = pNextLine->getNext();
					}

					if (!pNextLine)
					{
						// TODO: remove empty lines from
						// TODO: pRealNextLine to pNextLine
						return 0;
					}

					fp_Run* pCurRun = pNextLine->getFirstRun();
					fp_Run* pRunLooking = pCurRun;
					fp_Run* pLastRun = pNextLine->getLastRun();
					
					fp_RunSplitInfo si;

					while (iWidthLooking < iSpaceLeft &&
						   pRunLooking != pLastRun->getNext())
					{
						iWidthLooking += pRunLooking->getWidth();

						// This run will fit in the space we have left
						// snarf it up if we can break after it
						if (iSpaceLeft >= iWidthLooking)
						{
							if (!pRunLooking->getNext() ||
								pRunLooking->canBreakAfter())
							{
								if (bLineOnScreen)
									pNextLine->clearScreen();
								
								// snarf up any runs previous to us who
								// said they couldn't be broken after ...
								fp_Run* pTmp = pCurRun;
								while (pTmp != pRunLooking)
								{
									pNextLine->removeRun(pTmp);
									pLine->addRun(pTmp);
									pTmp = pTmp->getNext();
								}
								pNextLine->removeRun(pRunLooking);
								pLine->addRun(pRunLooking);

								if (pRealNextLine != pNextLine)
								{
									if (bLineOnScreen)
									{
										pRealNextLine->clearScreen();
										pRealNextLine->align();
										pRealNextLine->draw(pRunLooking->getGraphics());
									}
								}
								
								if (bLineOnScreen)
								{
									pNextLine->align();
									pNextLine->draw(pRunLooking->getGraphics());
									pLine->align();
									pLine->draw(pRunLooking->getGraphics());
								}
								
								pRealNextLine->m_bDirty = UT_TRUE;
								pNextLine->m_bDirty = UT_TRUE;

								if (pRunLooking != pLastRun)
									pRunLooking = pRunLooking->getNext();
								else
								{
									pLine->m_bDirty = UT_TRUE;
									break;
								}
								
								pCurRun = pRunLooking;

								if ((iSpaceLeft - iWidthLooking) == 0)
								{
									pLine->m_bDirty = UT_FALSE;
									break;
								}
								else
								{
									pLine->m_bDirty = UT_TRUE;
								}

								if (!pCurRun)
								{
									UT_ASSERT(0);
									return 0;
								}

/*								if (pNextLine->countRuns() == 0)
								{
									pLine->m_bDirty = UT_TRUE;
									break;
									}*/
							}
							else
							{
								iWidthLooking += pRunLooking->getWidth();
								pRunLooking = pRunLooking->getNext();
								pLine->m_bDirty = UT_TRUE;
							}
						}
						else if	(pRunLooking->findMaxLeftFitSplitPoint(iSpaceLeft - (iWidthLooking - pRunLooking->getWidth()), si))
						{
							if (bLineOnScreen)
								pNextLine->clearScreen();
							
							pNextLine->removeRun(pRunLooking);
							
							pRunLooking->split(si);
							
							fp_Run* pOtherRun = pRunLooking->getNext();
							UT_ASSERT(pOtherRun);
							
							fp_Run* pTmp = pCurRun;
							while (pTmp != pRunLooking)
							{
								pNextLine->removeRun(pTmp);
								iSpaceLeft -= pTmp->getWidth();
								pLine->addRun(pTmp);
								pTmp = pTmp->getNext();
							}

							pLine->addRun(pRunLooking);
							
							if (bLineOnScreen)
							{
								pLine->align();
								pLine->draw(pCurRun->getGraphics());
							}
							
							pRealNextLine->insertRun(pOtherRun, UT_TRUE,
													 UT_TRUE);
							
							if (pRealNextLine != pNextLine)
							{
								pRealNextLine->align();
								pRealNextLine->draw(pOtherRun->getGraphics());
							}
							
							if (bLineOnScreen)
							{
								pNextLine->align();
								pNextLine->draw(pOtherRun->getGraphics());
							}
							pRealNextLine->m_bDirty = UT_TRUE;
							pLine->m_bDirty = UT_FALSE;
							pNextLine->m_bDirty = UT_TRUE;
							break;
						}
						else
						{
							if (bLineOnScreen)
							{
								pLine->align();
								pLine->draw(pCurRun->getGraphics());
							}
							
							pLine->m_bDirty = UT_FALSE;
						}
					}
					
					if (pLine->countRuns() != 0)
					{
						pLine->align();
						pLine->draw(pLine->getFirstRun()->getGraphics());
					}
				}
				else
				{
					if (pLine->countRuns() != 0)
					{
						pLine->align();
						pLine->draw(pLine->getFirstRun()->getGraphics());
					}
				}
			}
		}
		else
		{
			pLine = pLine->getNext();
		}
	}
	return 0;
}

int fb_SimpleLineBreaker::breakParagraph(fl_BlockLayout* pBlock)
{
	fp_Run*	pCurRun = pBlock->getFirstRun();

	/*
	  As long as there are runs left, we try to build lines.
	*/
	while (pCurRun)
	{
		// start a new line
		UT_uint32 iGuessLineHeight = pCurRun->getHeight();

		UT_uint32 iMaxLineWidth = pBlock->requestLineSpace(iGuessLineHeight);
		UT_ASSERT(iMaxLineWidth > 0);

		fp_Line*	pLine = new fp_Line(iMaxLineWidth);
		
		UT_Bool bDoneWithLine = 0;
		UT_uint32 iCurLineWidth = 0;
		fp_RunSplitInfo si;

		fp_Run*		pRunLooking = pCurRun;
		UT_uint32	iWidthLooking = 0;
		
		while (!bDoneWithLine)
		{
			if (!pRunLooking)
			{
				if (iWidthLooking > 0)
				{
					UT_ASSERT(pCurRun);
					
					iCurLineWidth += iWidthLooking;
					
					fp_Run* pRunLoop = pCurRun;

					while (pRunLoop)
					{
						pLine->addRun(pRunLoop);
						pRunLoop = pRunLoop->getNext();
					}
				}
					
				pCurRun = NULL;
				break;
			}
			
			// find the next place we can put a line break
			UT_uint32 iRunWidth = pRunLooking->getWidth();
			if ((iCurLineWidth + iWidthLooking + iRunWidth) <= iMaxLineWidth)
			{
				// this run would fit.  can we break after it?
				if (pRunLooking->canBreakAfter())
				{
					/*
					  we can break after this run, and everything we've
					  been looking through will fit.  Add it all
					  to the line.
					*/
					
					iCurLineWidth += iWidthLooking;
					iCurLineWidth += iRunWidth;
					
					fp_Run* pRunLoop = pCurRun;

					for (;;)
					{
						pLine->addRun(pRunLoop);
						if (pRunLoop == pRunLooking)
						{
							break;
						}
						pRunLoop = pRunLoop->getNext();
					}
					
					pCurRun = pRunLooking->getNext();
					pRunLooking = pCurRun;
					iWidthLooking = 0;
				}
				else
				{
					// keep looking
					iWidthLooking += iRunWidth;
					pRunLooking = pRunLooking->getNext();
				}
			}
			else
			{
				// the run won't fit.  Can we split it?
				int bFoundSplit = pRunLooking->findMaxLeftFitSplitPoint(iMaxLineWidth - (iCurLineWidth + iWidthLooking), si);

				if (bFoundSplit)
				{
					// we found a split which fits.  split it and move on
					pRunLooking->split(si);	// TODO err check this
					UT_ASSERT((UT_sint32)pRunLooking->getWidth() == si.iLeftWidth);
					UT_ASSERT(pRunLooking->getNext());

					iCurLineWidth += iWidthLooking;
					iCurLineWidth += iRunWidth;
					
					fp_Run* pRunLoop = pCurRun;

					for (;;)
					{
						UT_ASSERT(pRunLoop);
						pLine->addRun(pRunLoop);
						if (pRunLoop == pRunLooking)
						{
							break;
						}
						pRunLoop = pRunLoop->getNext();
					}
					
					pCurRun = pRunLooking->getNext();
					pRunLooking = pCurRun;
					iWidthLooking = 0;

					// if we had to split, then we should just assume that the line is done
					bDoneWithLine = UT_TRUE;
				}
				else
				{
					/*
					  Nothing in this run will fit on this line.
					*/

					iWidthLooking = 0;
					bDoneWithLine = UT_TRUE;

					if (pLine->countRuns() == 0)
					{
						/*
						  nothing at all was placed on this line.  We should try harder.
						*/
						
						int bFoundSplit = pCurRun->findMinLeftFitSplitPoint(si);

						if (bFoundSplit)		
						{
							// it fits.  split it and move on
							pCurRun->split(si);	// TODO err check this

							pLine->addRun(pCurRun);
							iCurLineWidth += pCurRun->getWidth();

							pCurRun = pCurRun->getNext();
							UT_ASSERT(pCurRun);	// after a split, there is always a next.
							bDoneWithLine = UT_TRUE;
						}
						else
						{
							// nothing could be split.  We just force it and add anyway.  this is a bug.
							pLine->addRun(pCurRun);
							pCurRun = pCurRun->getNext();
							bDoneWithLine = UT_TRUE;
						}
					}
				}
			}
		}

		UT_ASSERT(pLine->getHeight() > 0);
		UT_ASSERT(pLine->getWidth() > 0);

		// The line is done.  Let's see if the size is going to be okay.
		if (pLine->getHeight() > iGuessLineHeight)
		{
			// Ooops!!!	TODO - we really should check to see if this is still okay.  This is a bug
			pBlock->addLine(pLine);
		}
		else
		{
			pBlock->addLine(pLine);
		}
	}

	return 0;	// TODO return code
}

