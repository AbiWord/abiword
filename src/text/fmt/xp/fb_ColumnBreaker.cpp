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

#include "fb_ColumnBreaker.h"
#include "fl_SectionLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "ut_assert.h"

fb_ColumnBreaker::fb_ColumnBreaker()
{
}

UT_sint32 fb_ColumnBreaker::breakSection(fl_SectionLayout* pSL)
{
	fl_BlockLayout* pFirstBlock = pSL->getFirstBlock();
	if (!pFirstBlock)
	{
		return 0;
	}

	fp_Line* pCurrentLine = pFirstBlock->getFirstLine();
		
	fp_Column* pCurColumn = pSL->getFirstColumn();
	UT_ASSERT(pCurColumn);

	while (pCurColumn)
	{
		fp_Line* pFirstLineToKeep = pCurrentLine;
		fp_Line* pLastLineToKeep = NULL;
		fp_Line* pOffendingLine = NULL;
		
		UT_sint32 iMaxColHeight = pCurColumn->getMaxHeight();
		UT_sint32 iWorkingColHeight = 0;

		fp_Line* pCurLine = pFirstLineToKeep;
		while (pCurLine)
		{
			UT_sint32 iLineHeight = pCurLine->getHeight();
			UT_sint32 iLineMarginAfter = pCurLine->getMarginAfter();
			UT_sint32 iTotalLineSpace = iLineHeight + iLineMarginAfter;

			if ((iWorkingColHeight + iTotalLineSpace) > iMaxColHeight)
			{
				pOffendingLine = pCurLine;

				/*
				  We have found the offending line (the first one which won't fit in the
				  column) and we now need to decide whether we can break the column
				  just before it.
				*/

				if (pOffendingLine == pFirstLineToKeep)
				{
					/*
					  Wow!  The very first line in this column won't fit.
					  
					  Big line.  (or maybe a small column)
					  
					  TODO what should we do here?  For now, we force it.
					*/
					pLastLineToKeep = pFirstLineToKeep;
				}
				else
				{
					fl_BlockLayout* pBlock = pOffendingLine->getBlock();
					UT_uint32 iWidows = pBlock->getProp_Widows();
					UT_uint32 iOrphans = pBlock->getProp_Orphans();

					UT_uint32 iNumLinesBeforeOffending = 0;
					UT_uint32 iNumLinesAfterOffending = 0;
					UT_Bool bFoundOffending = UT_FALSE;
					
					fp_Line* pFirstLineInBlock = pBlock->getFirstLine();
					pCurLine = pFirstLineInBlock;
					while (pCurLine)
					{
						if (bFoundOffending)
						{
							iNumLinesAfterOffending++;
						}
						else
						{
							if (pCurLine == pOffendingLine)
							{
								iNumLinesAfterOffending = 1;
								bFoundOffending = UT_TRUE;
							}
							else
							{
								iNumLinesBeforeOffending++;
							}
						}
						
						pCurLine = pCurLine->getNext();
					}

					UT_uint32 iNumLinesInBlock = iNumLinesBeforeOffending + iNumLinesAfterOffending;

					UT_uint32 iNumBlockLinesInThisColumn = 0;
					pCurLine = pOffendingLine->getPrev();
					while (pCurLine)
					{
						iNumBlockLinesInThisColumn++;
						if (pCurLine == pFirstLineToKeep)
						{
							break;
						}

						pCurLine = pCurLine->getPrev();
					}

					if (
						pBlock->getProp_KeepTogether()
						&& (iNumLinesBeforeOffending == iNumBlockLinesInThisColumn)
						&& (pBlock->getFirstLine() != pFirstLineToKeep)
						)
					{
						/*
						  This block wants to be kept all in the same column.
						  Bump the whole block to the next column.
						*/

						/*
						  If the block is simply too big to fit in a
						  single column, then we can spawn an infinite
						  loop by continually punting it to the next
						  one.  So, we assume that if the first line
						  in the block is the first line in this
						  column, we just keep it and cope.  This will
						  be slightly incorrect in cases where pushing
						  it to the next column would allow the block
						  to try to live in a larger column, thus
						  staying all together.
						*/
						
						pLastLineToKeep = pFirstLineInBlock->getPrevLineInSection();
					}
					else if (
						(iNumLinesInBlock < (iWidows + iOrphans))
						&& (iNumLinesBeforeOffending == iNumBlockLinesInThisColumn)
						)
					{
						/*
						  There are not enough lines to divide between the
						  two columns while still satisfying both constraints.
						  Bump the whole block to the next column.
						*/
						
						pLastLineToKeep = pFirstLineInBlock->getPrevLineInSection();
					}
					else if (
						(iNumLinesBeforeOffending < iOrphans)
						&& (iNumLinesBeforeOffending == iNumBlockLinesInThisColumn)
						)
					{
						/*
						  We're leaving too few lines in the current column.
						  Bump the whole block.
						*/

						pLastLineToKeep = pFirstLineInBlock->getPrevLineInSection();
					}
					else if (
						(iNumLinesAfterOffending < iWidows)
						&& ((iWidows - iNumLinesAfterOffending) < iNumBlockLinesInThisColumn)
						)
					{
						/*
						  There aren't going to be enough lines in the next
						  column.  Bump just enough.
						*/

						UT_uint32 iNumLinesNeeded = (iWidows - iNumLinesAfterOffending);
						pLastLineToKeep = pOffendingLine->getPrevLineInSection();
						for (UT_uint32 iBump = 0; iBump < iNumLinesNeeded; iBump++)
						{
							pLastLineToKeep = pLastLineToKeep->getPrevLineInSection();
						}
					}
					else
					{
						pLastLineToKeep = pOffendingLine->getPrevLineInSection();
					}
				}
				break;
			}
			else
			{
				iWorkingColHeight += iTotalLineSpace;
				if (
					pCurLine->containsForcedColumnBreak()
					|| pCurLine->containsForcedPageBreak()
					)
				{
					pLastLineToKeep = pCurLine;
					break;
				}
			}

			pCurLine = pCurLine->getNextLineInSection();
		}

		if (pLastLineToKeep)
		{
			pCurrentLine = pLastLineToKeep->getNextLineInSection();
		}
		else
		{
			pCurrentLine = NULL;
		}
		
		pCurLine = pFirstLineToKeep;
		while (pCurLine)
		{
			if (pCurLine->getColumn() != pCurColumn)
			{
				pCurLine->getColumn()->removeLine(pCurLine);
				pCurColumn->addLine(pCurLine);
			}

			if (pCurLine == pLastLineToKeep)
			{
				break;
			}
			else
			{
				pCurLine = pCurLine->getNextLineInSection();
			}
		}

		fp_Column* pNextColumn = NULL;
		
		if (pLastLineToKeep)
		{
			UT_ASSERT(pLastLineToKeep->getColumn() == pCurColumn);
			
			if (pCurColumn->getLastLine() != pLastLineToKeep)
			{
				// make sure there is a next column
				pNextColumn = pCurColumn->getNext();
				if (!pNextColumn)
				{
					pNextColumn = pSL->getNewColumn();
				}

				pCurColumn->bumpLines(pLastLineToKeep);
			}
		}

		UT_ASSERT((!pLastLineToKeep) || (pCurColumn->getLastLine() == pLastLineToKeep));
			
		pCurColumn->layout();

		pCurColumn = pCurColumn->getNext();
	}

	return 0; // TODO return code
}
