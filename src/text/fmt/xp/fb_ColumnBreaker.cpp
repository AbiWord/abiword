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
#include "fp_ContainerObject.h"
#include "fp_TableContainer.h"
#include "fp_FootnoteContainer.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "ut_assert.h"
#include "fl_ContainerLayout.h"
#include "fl_FootnoteLayout.h"
#include "fp_Page.h"

fb_ColumnBreaker::fb_ColumnBreaker() :
	m_pStartPage(NULL),
	m_bStartFromStart(true)
{
}

void fb_ColumnBreaker::setStartPage(fp_Page * pPage)
{
	if(!m_bStartFromStart)
	{
		m_pStartPage = pPage;
	}
	if(pPage == NULL)
	{
		m_bStartFromStart = true;
		m_pStartPage = NULL;
	}
	return;
}
	
/*!
  Layout sections on pages
  \return zero

  This is the function that controls how sections and thereby columns,
  blocks, and lines are laid out on the pages. Doing so it refers to
  the various layout configurations such as orphan/widow controls and
  break Runs embedded in the text.
*/
UT_sint32 fb_ColumnBreaker::breakSection(fl_DocSectionLayout * pSL)
{
	m_bReBreak = false;

	fl_ContainerLayout* pFirstLayout = NULL;
	fp_Container* pOuterContainer = NULL;
	fp_Column* pCurColumn = NULL;
	UT_ASSERT(pSL->needsSectionBreak());
	pSL->setNeedsSectionBreak(false,m_pStartPage);
	pFirstLayout = pSL->getFirstLayout();
	if (!pFirstLayout)
	{
		m_pStartPage = NULL;
		m_bStartFromStart = false;
		return 0;
	}
	pOuterContainer = pFirstLayout->getFirstContainer();
	pCurColumn = static_cast<fp_Column*>(pSL->getFirstContainer());
	xxx_UT_DEBUGMSG(("fb_ColumnBreaker: For DocSec %x first column %x \n",pSL,pCurColumn));
	if(m_pStartPage)
	{
		xxx_UT_DEBUGMSG(("Layout from page %x \n",m_pStartPage));
		pCurColumn = m_pStartPage->getNthColumnLeader(0);
		pOuterContainer = pCurColumn->getFirstContainer();
		if(pOuterContainer && pOuterContainer->getContainerType() == FP_CONTAINER_LINE)
		{
			
			pFirstLayout = static_cast<fl_ContainerLayout *>(static_cast<fp_Line *>(pOuterContainer)->getBlock());
		}
		else if(pOuterContainer)
		{
			UT_ASSERT(pOuterContainer->getContainerType() == FP_CONTAINER_TABLE);
			pFirstLayout = static_cast<fl_ContainerLayout *>(pOuterContainer->getSectionLayout());
		}
	}

	while (pCurColumn)
	{
		fp_Container* pFirstContainerToKeep = pOuterContainer;
		xxx_UT_DEBUGMSG(("SEVIOR: first to keep 1 %x \n",pFirstContainerToKeep));
		fp_Container* pLastContainerToKeep = NULL;
		fp_Container* pOffendingContainer = NULL;
		UT_sint32 iMaxSecCol = pSL->getMaxSectionColumnHeight();
 		UT_sint32 iMaxColHeight = pCurColumn->getMaxHeight();
		UT_sint32 iFootnoteHeight = 0;
		bool bEquivColumnBreak = false;
		xxx_UT_DEBUGMSG(("SEVIOR: iMaxSecCol = %d iMaxColHeight = %d \n",iMaxSecCol,iMaxColHeight));
		if((iMaxSecCol > 0) && (iMaxSecCol < iMaxColHeight))
		{
			iMaxColHeight = iMaxSecCol;
		    bEquivColumnBreak = true;
		}
		UT_sint32 iWorkingColHeight = 0;

		fp_Container* pCurContainer = pFirstContainerToKeep;
		

		// Special handling of columns that should be skipped due to
		// page breaks. If the previous line contains a page break,
		// skip the present column if it is on the same page.
		if (pCurContainer)
		{
			fp_Container* pPrevContainer = 
				static_cast<fp_Container *>(pCurContainer->getPrev());
			if(pPrevContainer && 
			   pPrevContainer->getContainerType() == FP_CONTAINER_LINE)
			{
				fp_Line * pL = static_cast<fp_Line *>(pPrevContainer);
				{
					if (pL->containsForcedPageBreak()
				&& (pCurColumn->getPage() == pL->getContainer()->getPage()))
					{
						pCurColumn = static_cast<fp_Column *>(pCurColumn->getNext());
						continue;
					}
				}
			}
		}
		bool bBreakOnColumnBreak = false;
		bool bBreakOnPageBreak = false;
		// plam: this var is redundant if the #if 0 remains dead.
		//bool bDoTableBreak = false;
		UT_sint32 iTotalContainerSpace = 0;
		fp_Container * pPrevWorking = pCurContainer;
		while (pCurContainer)
		{
			xxx_UT_DEBUGMSG(("curContainer %x type %d \n",pCurContainer,pCurContainer->getContainerType()));
			if(pCurContainer->getContainerType() == FP_CONTAINER_FOOTNOTE)
			{
//
// skip this! We've already taken it's height into account.
//
				pCurContainer = pCurContainer->getNextContainerInSection();
				continue;
			}
			UT_sint32 iContainerHeight = 0;
			if(pCurContainer->getContainerType() == FP_CONTAINER_TABLE)
			{
				iContainerHeight = static_cast<fp_TableContainer *>(pCurContainer)->getHeight();
#if 0
				if(!pTab->isThisBroken() && pTab->getFirstBrokenTable())
				{
					pTab = pTab->getFirstBrokenTable();
				}
				if(pTab->containsFootnoteReference() )
				{
					// Ok.  Now, deduct the proper amount from iMaxColHeight.

					// OK get a vector of the footnote containers in this line.
					UT_Vector vecFootnotes;
					pTab->getFootnoteContainers(&vecFootnotes);
					fp_Page *pCurPage = pCurColumn->getPage();
					// Now loop through all these and add them to the height.
					UT_sint32 i =0;
					for(i=0; i< static_cast<UT_sint32>(vecFootnotes.getItemCount());i++)
					{
						fp_FootnoteContainer * pFC = static_cast<fp_FootnoteContainer *>(vecFootnotes.getNthItem(i));
						if(pFC->getPage() == NULL || pFC->getPage() != pCurPage)
						{
							iFootnoteHeight += pFC->getHeight();
						}				
					}	
					UT_DEBUGMSG(("got Table footnote section height %d\n", iFootnoteHeight));
					iWorkingColHeight += iFootnoteHeight;
				}
#endif
			}
			else
				iContainerHeight = pCurContainer->getHeight();

			UT_sint32 iContainerMarginAfter = pCurContainer->getMarginAfter();
			iTotalContainerSpace = iContainerHeight + iContainerMarginAfter;
			if (pCurContainer && 
				pCurContainer->getContainerType() == FP_CONTAINER_LINE)
			{
				fp_Line* pCurLine = static_cast<fp_Line *>(pCurContainer);
				// Excellent.  If we have a footnote, we can start deducting
				// from the working height if the footnote container is not on
				// this page.
				if (pCurLine->containsFootnoteReference())
				{
					// Ok.  Now, deduct the proper amount from iMaxColHeight.

					// OK get a vector of the footnote containers in this line.
					UT_Vector vecFootnotes;
					pCurLine->getFootnoteContainers(&vecFootnotes);
					fp_Page *pCurPage = pCurColumn->getPage();
					// Now loop through all these and add them to the height.
					UT_sint32 i =0;
					for(i=0; i< static_cast<UT_sint32>(vecFootnotes.getItemCount());i++)
					{
						fp_FootnoteContainer * pFC = static_cast<fp_FootnoteContainer *>(vecFootnotes.getNthItem(i));
						if(pFC->getPage() == NULL || pFC->getPage() != pCurPage)
						{
							iFootnoteHeight += pFC->getHeight();
						}				
					}	
					UT_DEBUGMSG(("got footnote section height %d\n", iFootnoteHeight));
					iWorkingColHeight += iFootnoteHeight;
				}
			}

			if ((iWorkingColHeight + iTotalContainerSpace) > iMaxColHeight)
			{
				pOffendingContainer = pCurContainer;

				/*
				  We have found the offending container (the first one
				  which won't fit in the column) and we now need to
				  decide whether we can break the column just before
				  it.  

				  First, check to see if the offending container is a
				  table and if it can be broken to fit in the column.
				*/

				xxx_UT_DEBUGMSG(("SEVIOR: iWorkingColHeight %d iTotalContainerSpace %d iMaxColHeight %d pCurContainer %x height %d \n",iWorkingColHeight,iTotalContainerSpace,iMaxColHeight,   pCurContainer,  iContainerHeight));
				if(pOffendingContainer->getContainerType() == FP_CONTAINER_TABLE)
				{
					xxx_UT_DEBUGMSG(("fb_ColumnBreak 1 Broken Table num %d \n",(static_cast<fp_TableContainer *>(pOffendingContainer))->getBrokenNumber()));
					if (_breakTable(pOffendingContainer,
									pLastContainerToKeep,
									iMaxColHeight, iWorkingColHeight, 
									iContainerMarginAfter))
					{
						pPrevWorking = pCurContainer;
						pCurContainer = pOffendingContainer;
					}
					else
					{
//
// Can't break the table so bump it.
//
						pCurContainer = pOffendingContainer;
						fp_TableContainer * pTabOffend = static_cast<fp_TableContainer *>(pOffendingContainer);
						pLastContainerToKeep = pTabOffend->getPrevContainerInSection();
						xxx_UT_DEBUGMSG(("Can't break table. pCurContainer %x pTabOffend %x pLastContainerToKeep %x \n",pCurContainer,pTabOffend,pLastContainerToKeep));
						break;
					}
				}
				if (pOffendingContainer == pFirstContainerToKeep)
				{
					// Wow!  The very first line in this column won't
					// fit.  Big line.  (or maybe a small column)
					// TODO: what should we do here?  For now, we
					// force it.

					pLastContainerToKeep = pFirstContainerToKeep;
					xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 2 %x \n",pLastContainerToKeep));
				}
				else
				{
					UT_uint32 iWidows = 0;
					UT_uint32 iOrphans = 0;
					bool bIsTable = false;
					fl_BlockLayout* pBlock = NULL;
					fl_ContainerLayout * pConLayout = NULL;
					if(pOffendingContainer->getContainerType() == FP_CONTAINER_LINE)
					{
						pBlock = static_cast<fp_Line *>(pOffendingContainer)->getBlock();
						iWidows = pBlock->getProp_Widows();
						iOrphans = pBlock->getProp_Orphans();
						pConLayout = static_cast<fl_ContainerLayout *>(pBlock);
					}
					else
					{
						bIsTable = true;
						pConLayout = static_cast<fl_ContainerLayout *>(pOffendingContainer->getSectionLayout());
					}
					if(bIsTable)
					{

						pLastContainerToKeep = pOffendingContainer->getPrevContainerInSection();
						xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 3 %x \n",pLastContainerToKeep));
						UT_ASSERT(pLastContainerToKeep);
//
// All this deals with widows and orphans. We don't need this for
// tables
//
						break;
					}
					UT_uint32 iNumContainersBeforeOffending = 0;
					UT_uint32 iNumContainersAfterOffending = 0;
					bool bFoundOffending = false;
					fp_Container* pFirstContainerInBlock = 
						pConLayout->getFirstContainer();
					pCurContainer = pFirstContainerInBlock;
					while (pCurContainer)
					{
						if (bFoundOffending)
						{
							iNumContainersAfterOffending++;
						}
						else
						{
							if (pCurContainer == pOffendingContainer)
							{
								iNumContainersAfterOffending = 1;
								bFoundOffending = true;
							}
							else
							{
								iNumContainersBeforeOffending++;
							}
						}
						pCurContainer = static_cast<fp_Container *>(pCurContainer->getNext());
					}

					UT_uint32 iNumContainersInLayout = 
					 iNumContainersBeforeOffending+iNumContainersAfterOffending;

					UT_uint32 iNumLayoutContainersInThisColumn = 0;
					pCurContainer = static_cast<fp_Container *>(pOffendingContainer->getPrev());
					while (pCurContainer)
					{
						iNumLayoutContainersInThisColumn++;
						if (pCurContainer == pFirstContainerToKeep)
							break;

						pCurContainer=static_cast<fp_Container *>(pCurContainer->getPrev());
					}
					if (pBlock && pBlock->getProp_KeepTogether()
						&& (iNumContainersBeforeOffending == 
							iNumLayoutContainersInThisColumn)
						&& (pBlock->getFirstContainer() != 
							pFirstContainerToKeep))
					{
						/*
						  This block or Table wants to be kept all in the same column.
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

						pLastContainerToKeep = static_cast<fp_Container *>(pFirstContainerInBlock->getPrevContainerInSection());
						xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 4 %x \n",pLastContainerToKeep));
					}
					else if ((iNumContainersInLayout < (iWidows + iOrphans))
						&& (iNumContainersBeforeOffending == iNumLayoutContainersInThisColumn)
						)
					{
						/*
						  There are not enough lines to divide between the
						  two columns while still satisfying both constraints.
						  Bump the whole block to the next column.
						*/

						pLastContainerToKeep = 
							static_cast<fp_Container *>(pFirstContainerInBlock->getPrevContainerInSection());
//
// If this happens to be a table better get the LAST of the broken tables
//
						if(pLastContainerToKeep->getContainerType() == 
						                    FP_CONTAINER_TABLE && 
						   pFirstContainerToKeep->getContainerType() == 
						                    FP_CONTAINER_TABLE)
						{
							//
							// Can get into trouble if the pFirstContainer
							// points to a broken table after
							// this. Check for this.
							//
							for (fp_TableContainer * pTab = 
								 static_cast<fp_TableContainer *>(pLastContainerToKeep);
								 pTab; 
								 pTab=static_cast<fp_TableContainer *>(pTab->getNext()))
							{
								if(pTab == pFirstContainerToKeep)
								{
									xxx_UT_DEBUGMSG(("SEVIOR: FirstContainer l325 %x \n",pFirstContainerToKeep)); 
									pFirstContainerToKeep = 
										pLastContainerToKeep;
									break;
								}
								
							}
						}
						xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 5 %x \n",pLastContainerToKeep));
					}
					else if (
						(iNumContainersBeforeOffending < iOrphans)
						&& (iNumContainersBeforeOffending == iNumLayoutContainersInThisColumn)
						)
					{
						/*
						  We're leaving too few lines in the current column.
						  Bump the whole block.
						*/

						pLastContainerToKeep = static_cast<fp_Container *>(pFirstContainerInBlock->getPrevContainerInSection());
						xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 6 %x \n",pLastContainerToKeep));
					}
					else if (
						(iNumContainersAfterOffending < iWidows)
						&& ((iWidows - iNumContainersAfterOffending) < iNumLayoutContainersInThisColumn)
						)
					{
						/*
						  There aren't going to be enough lines in the next
						  column.  Bump just enough.
						*/

						UT_uint32 iNumContainersNeeded = (iWidows - iNumContainersAfterOffending);

						pLastContainerToKeep = static_cast<fp_Container *>(pOffendingContainer->getPrevContainerInSection());
						xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 7 %x \n",pLastContainerToKeep));
						for (UT_uint32 iBump = 0; iBump < iNumContainersNeeded; iBump++)
						{

							pLastContainerToKeep = static_cast<fp_Container *>(pLastContainerToKeep->getPrevContainerInSection());
							xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 8 %x \n",pLastContainerToKeep));
						}
					}
					else
					{

						pLastContainerToKeep = static_cast<fp_Container *>(pOffendingContainer->getPrevContainerInSection());
						xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 9 %x \n",pLastContainerToKeep));
					}
				}
				break;
			}
			else
			{
				iWorkingColHeight += iTotalContainerSpace;
				if(pCurContainer->getContainerType() == FP_CONTAINER_LINE)
				{
					fp_Line * pL = static_cast<fp_Line *>(pCurContainer);
					if (pL->containsForcedColumnBreak()
						|| pL->containsForcedPageBreak())
					{

						pLastContainerToKeep = pCurContainer;
						xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 10 %x \n",pLastContainerToKeep));
						bBreakOnColumnBreak = (pL->containsForcedColumnBreak()) ;
						bBreakOnPageBreak = pL->containsForcedPageBreak();
						if(iWorkingColHeight >= iMaxColHeight)
							bBreakOnColumnBreak = false;
						break;
					}
				}
			}
			pCurContainer = static_cast<fp_Container *>(pCurContainer->getNextContainerInSection());
		}
//
// End of inner while loop here. After this we've found LastContainerToKeep
//
		bEquivColumnBreak = bEquivColumnBreak && ( iMaxColHeight < (iWorkingColHeight + iTotalContainerSpace));
		if (pLastContainerToKeep)
			pOuterContainer = static_cast<fp_Container *>(pLastContainerToKeep->getNextContainerInSection());
		else
			pOuterContainer = NULL;

//
// OK fill our column with content between pFirstContainerToKeep and pLastContainerToKeep
//
		pCurContainer = pFirstContainerToKeep;
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pFirstContainerToKeep);
		UT_sint32 conPos = 0;
		while (pCurContainer)
		{
			xxx_UT_DEBUGMSG(("Container %x is in Column %x Type %d \n",pCurContainer,pCurColumn,pCurContainer->getContainerType()));
			if(pCurContainer->getContainerType() == FP_CONTAINER_FOOTNOTE)
			{
//
// Skip this. It doesn't go in this column at all.
//
				pCurContainer = pCurContainer->getNextContainerInSection();
				continue;
			}
			if (pCurContainer->getContainer() != pCurColumn || (pCurColumn->findCon(pCurContainer) < 0) )
			{
				UT_ASSERT(pCurContainer->getContainer());
				if(pCurContainer->getContainer()->findCon(pCurContainer) >= 0)
				{
					static_cast<fp_VerticalContainer *>(pCurContainer->getContainer())->removeContainer(pCurContainer);
				}
				pCurColumn->addContainer(pCurContainer);
			}
//
// Now make sure footnotes are on the same page as the reference.
//
			if(pCurContainer->getContainerType() == FP_CONTAINER_LINE)
			{
				fp_Line * pCurLine = static_cast<fp_Line *>(pCurContainer);
				xxx_UT_DEBUGMSG(("About to call containerFootnoteReferenced \n"));
				if(pCurLine->containsFootnoteReference())
				{
					// OK get a vector of the footnote containers in this line.
					UT_Vector vecFootnotes;
					pCurLine->getFootnoteContainers(&vecFootnotes);
				
					// Now loop through all these and check they're on this
					// page. If not add them.
					fp_Page * pCurPage = pCurColumn->getPage();
					UT_ASSERT(pCurPage);
					UT_sint32 i =0;
					for(i=0; i< static_cast<UT_sint32>(vecFootnotes.getItemCount());i++)
					{
						fp_FootnoteContainer * pFC = static_cast<fp_FootnoteContainer *>(vecFootnotes.getNthItem(i));
						fp_Page * myPage = pFC->getPage();
						xxx_UT_DEBUGMSG(("Footnote %x is on Page %x \n",pFC,myPage));
						if(myPage != pCurPage)
						{
							if(myPage == NULL)
							{
								pCurPage->insertFootnoteContainer(pFC);
							}
							else
							{
								myPage->removeFootnoteContainer(pFC);
								pCurPage->insertFootnoteContainer(pFC);
							}
						}
					}
				}
			}
//
// Do the same for footnotes inside broken tables
//
			if(pCurContainer->getContainerType() == FP_CONTAINER_TABLE)
			{
				fp_TableContainer * pCurTable = static_cast<fp_TableContainer *>(pCurContainer);
				if(!pCurTable->isThisBroken() && pCurTable->getFirstBrokenTable())
				{
					pCurTable = pCurTable->getFirstBrokenTable();
				}
				if(pCurTable->isThisBroken())
				{
					if(pCurTable->containsFootnoteReference())
					{
						// OK get a vector of the footnote containers in this line.
						UT_Vector vecFootnotes;
						pCurTable->getFootnoteContainers(&vecFootnotes);
						
					// Now loop through all these and check they're on this
					// page. If not add them.
						fp_Page * pCurPage = pCurColumn->getPage();
						UT_ASSERT(pCurPage);
						UT_sint32 i =0;
						for(i=0; i< static_cast<UT_sint32>(vecFootnotes.getItemCount());i++)
						{
							UT_DEBUGMSG(("Found reference %d in broken table %x \n",i,pCurTable));
							fp_FootnoteContainer * pFC = static_cast<fp_FootnoteContainer *>(vecFootnotes.getNthItem(i));
							fp_Page * myPage = pFC->getPage();
							UT_DEBUGMSG(("Footnote %x is on Page %x \n",pFC,myPage));
							if(myPage != pCurPage)
							{
								UT_DEBUGMSG((" Moving anchor from %x to %x \n",myPage,pCurPage));
								if(myPage == NULL)
								{
									pCurPage->insertFootnoteContainer(pFC);
								}
								else
								{
									myPage->removeFootnoteContainer(pFC);
									pCurPage->insertFootnoteContainer(pFC);
								}
							}
						}
					}
				}
			}
//
// Code to fix order in the column
//
			if((pCurColumn->findCon(pCurContainer) >= 0) && (pCurColumn->findCon(pCurContainer)  != conPos))
			{
				xxx_UT_DEBUGMSG(("fb_ColumnBreaker:Container out of order. Should be at %d is at %d \n",conPos,pCurColumn->findCon(pCurContainer)));
				xxx_UT_DEBUGMSG(("fb_ColumnBreak: Fixing this now \n"));
				static_cast<fp_VerticalContainer *>(pCurContainer->getContainer())->removeContainer(pCurContainer);
				pCurColumn->insertConAt(pCurContainer,conPos);
//				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
			conPos++;
			if (pCurContainer == pLastContainerToKeep)
				break;
			else
			{
				if((pLastContainerToKeep!=NULL) && (pCurContainer->getNextContainerInSection()==NULL))
				{
					UT_DEBUGMSG(("Non null LastContainerToKeep yet next container is NULL!!!!!!!!!!!! \n"));
					UT_DEBUGMSG((" CurContainer %x type %d \n",pCurContainer,pCurContainer->getContainerType()));
					UT_DEBUGMSG((" FirstContainer to keep %x Last container to keep %x \n",pTab,pLastContainerToKeep));
					UT_DEBUGMSG(("Try to recover.... \n"));
//					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					pLastContainerToKeep = NULL;
					break;
				}
				pCurContainer = static_cast<fp_Container *>(pCurContainer->getNextContainerInSection());
			}
		}

		if (pLastContainerToKeep
			&& pCurColumn->getLastContainer() != pLastContainerToKeep)
		{
			if(pLastContainerToKeep->getColumn()!=pCurColumn)
			{
				UT_DEBUGMSG(("4111 bug \n"));
			}
			UT_ASSERT(pLastContainerToKeep->getColumn()==pCurColumn);
			fp_Page* pPrevPage = pCurColumn->getPage();
			fp_Column* pNextColumn = pCurColumn;

			do
			{
				// Make sure there is a next column and that it
				// falls on the next page if there's a page break.
				pNextColumn = static_cast<fp_Column *>(pNextColumn->getNext());
				if(bBreakOnColumnBreak || bEquivColumnBreak)
				{
					if((pNextColumn != NULL) && 
					   (pNextColumn != pCurColumn->getFollower()) && 
					   (pNextColumn->getPage() != pCurColumn->getPage()))
						pNextColumn = NULL;
				}
				if (!pNextColumn)
				{
					if(bBreakOnColumnBreak || bEquivColumnBreak)
						pNextColumn = (fp_Column*) 
							pSL->getNewContainer
							(static_cast<fp_Container *>(pLastContainerToKeep->
							 getNextContainerInSection()));
					else
						pNextColumn = static_cast<fp_Column*>(pSL->getNewContainer(NULL));
				}
			}
			while (pLastContainerToKeep->getContainerType()
				             == FP_CONTAINER_LINE &&
				   static_cast<fp_Line *>(pLastContainerToKeep)
				        ->containsForcedPageBreak() &&
				   (pNextColumn->getPage() == pPrevPage));

			// Bump content down the columns
			while (pCurColumn != NULL && pCurColumn != pNextColumn)
			{
				pCurColumn->bumpContainers(pLastContainerToKeep);
				pCurColumn->layout();
//
// Layout might delete a broken table. Check for this.
//
				fp_Container * pCon = pCurColumn->getLastContainer();
				if(pCon && pCon->getContainerType() == FP_CONTAINER_TABLE)
				{
					pOuterContainer = pCon->getNextContainerInSection();
				}
				pCurColumn = static_cast<fp_Column *>(pCurColumn->getNext());

					// This is only relevant for the initial column. All
					// other columns should flush their entire content.
				pLastContainerToKeep = NULL;
			}

		}
		else
		{
			UT_ASSERT((!pLastContainerToKeep) || 
					  (pCurColumn->getLastContainer() == pLastContainerToKeep));

			bool bTableTest = false;
			if(pOuterContainer)
				bTableTest = (pOuterContainer->getContainerType() == 
							  FP_CONTAINER_TABLE);

			pCurColumn->layout();

			// pCurColumn->layout() might delete a broken table; fixup.
			if(bTableTest &&
			   pOuterContainer != pCurColumn->getLastContainer())
			{
				pOuterContainer = 
				 pCurColumn->getLastContainer()->getNextContainerInSection();
				bTableTest = true;
			}
			pCurColumn = static_cast<fp_Column *>(pCurColumn->getNext());
			if(pCurColumn == NULL && bTableTest && pOuterContainer != NULL)
			{
				if(pOuterContainer->getContainerType() == FP_CONTAINER_TABLE)
				{
					pCurColumn = static_cast<fp_Column *>(pSL->getNewContainer(NULL));
				}
				else
				{
					pCurColumn = static_cast<fp_Column *>(pSL->getNewContainer(pOuterContainer));
				}
			}
			else if(pCurColumn == NULL && pLastContainerToKeep && pLastContainerToKeep->getNextContainerInSection())
			{
				fp_Container * pCon = pLastContainerToKeep->getNextContainerInSection();
				if(pCon->getContainerType() == FP_CONTAINER_TABLE)
				{
					pCurColumn =  static_cast<fp_Column *>(pSL->getNewContainer(NULL));
				}
				else
				{
					pCurColumn =  static_cast<fp_Column *>(pSL->getNewContainer(pCon));
				}
			}
		}
		if(pCurColumn == NULL && pLastContainerToKeep && pLastContainerToKeep->getNextContainerInSection())
		{
			fp_Container * pCon = pLastContainerToKeep->getNextContainerInSection();
			if(pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				pCurColumn =  static_cast<fp_Column *>(pSL->getNewContainer(NULL));
			}
			else
			{
				pCurColumn =  static_cast<fp_Column *>(pSL->getNewContainer(pCon));
			}
		}
		if(pCurColumn == NULL)
		{
			UT_ASSERT(pOuterContainer == NULL);
		}
//
// Loop back for next pCurContainer. Finish if pCurColumn == NULL.
//
		xxx_UT_DEBUGMSG(("fb_ColumnBreaker:: Finished this column, doing next now. \n"));
	}
//
// End of massive while loop here
//
	m_pStartPage = NULL;
	m_bStartFromStart = false;
	if(m_bReBreak)
	{
		breakSection(pSL);
	}
//
// Look if the next DocSectionLayout needs section break.(
// This happens if a column height changes. If so, do it!
//
	if(pSL->getNextDocSection() && pSL->needsSectionBreak())
	{
		pSL->completeBreakSection();
	}
	return 0; // TODO return code
}

/*!
 * Breaks the given table, if appropriate.  
 * \return true iff the table was broken.
 */
bool fb_ColumnBreaker::_breakTable(fp_Container*& pOffendingContainer,
								   fp_Container*& pLastContainerToKeep,
								   int iMaxColHeight, 
								   int iWorkingColHeight,
								   int iContainerMarginAfter)
{
	bool bDoTableBreak;

    UT_DEBUGMSG(("breakTable:!!!!!!!!!!!! Offending container is table %x \n",pOffendingContainer));
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pOffendingContainer);
	if(!pTab->isThisBroken())
	{
//
// This is the first of this table set. Clear the old broken tables
// and rebreak.
//
		pTab->deleteBrokenTables();
		bDoTableBreak = true;
		xxx_UT_DEBUGMSG(("SEVIOR: Need Table Break 1 \n"));
		xxx_UT_DEBUGMSG(("firstbroke %x lastbroke %x \n",pTab->getFirstBrokenTable(),pTab->getLastBrokenTable()));
	}
	else
	{
		bDoTableBreak = true;
	}
//
// If we don't break the table, the heights of the broken table's
// will be adjusted at the setY() in the layout stage.
// PLAM: broken for pango? 
//
	fp_TableContainer * pBroke = NULL;
	UT_sint32 iAvail = iMaxColHeight - iWorkingColHeight - iContainerMarginAfter;
	UT_sint32 iBreakAt = pTab->wantVBreakAt(iAvail-1);
	UT_DEBUGMSG(("breakTable column: iAvail %d actual break at %d \n",iAvail,iBreakAt));
//
// Look to see if the table can be broken. If iBreakAt < 0 we have to bump 
// the whole table into the next column.
//
	if(iBreakAt < 1)
	{
		UT_DEBUGMSG(("breakTable Col: Can't break this table %d \n",iBreakAt));
		return false;
	}

	UT_ASSERT(iBreakAt <= (iAvail-1));

	if(bDoTableBreak && (iBreakAt + iWorkingColHeight <= iMaxColHeight))
	{
//
// OK we can break this table and keep some of it in this column. The
// rest goes into the next column.
//
// Look to see if this table is broken.
//
		if(!pTab->isThisBroken())
		{
//
// Break it at 0 first.
//
			xxx_UT_DEBUGMSG(("SEVIOR: Breaking MAster iBreakAt %d yloc = %d \n",iBreakAt,pTab->getY()));
			xxx_UT_DEBUGMSG(("SEVIOR: iBreakLO %d iWorkingColHeight %d iMaxColHeight %d Container Height %d MArginAfter %d \n",iBreakLO,iWorkingColHeight,iMaxColHeight,pTab->getHeight() , iContainerMarginAfter ));
			fp_Container * pNext = static_cast<fp_Container *>(pTab->getNext());
			xxx_UT_DEBUGMSG(("SEVIOR: getNext %x \n",pNext));
			if(pNext)
			{
				xxx_UT_DEBUGMSG(("SEVIOR: Container of next %d \n",pNext->getContainerType()));
			}
			pTab->deleteBrokenTables();
			pTab->VBreakAt(0);
		}
//
// Now get a broken table and break it again.
//
		if(!pTab->isThisBroken())
		{
			pBroke = pTab->getFirstBrokenTable();
		}
		else
		{
			pBroke = pTab;
		}
//
// Look to see if we have to move the table out of this container.
//
		if(iBreakAt < 30)
		{
			pOffendingContainer = pTab;
		}
		else
		{
//
// When we break the table, the bit broken will be placed after the current
// table in the column. This then becomes the offending container and will
// be bumped into the next column. Pretty cool eh ? :-)
//
			pOffendingContainer = static_cast<fp_Container *>(pBroke->VBreakAt(iBreakAt));
		    xxx_UT_DEBUGMSG(("SEVIOR: Created broken table %x \n",pOffendingContainer));
			pLastContainerToKeep = static_cast<fp_Container *>(pTab);
			xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 1 %x \n",pLastContainerToKeep));
		}
		return true;
	}
	return false;
#if 0 // BLAH!
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pOffendingContainer);
	if(!pTab->isThisBroken())
	{
        // This is the first of this table set. Clear the old broken
        // tables and rebreak.
		pTab->deleteBrokenTables();
		xxx_UT_DEBUGMSG(("SEVIOR: Need Table Break 1 \n"));
		xxx_UT_DEBUGMSG(("firstbroke %x lastbroke %x \n",pTab->getFirstBrokenTable(),pTab->getLastBrokenTable()));
	}

    // If we don't break the table, the heights of the broken table's
    // will be adjusted at the setY() in the layout stage.
    // PLAM: broken for pango?
	fp_TableContainer * pBroke = NULL;
	UT_sint32 iAvail = iMaxColHeight - iWorkingColHeight - 
		iContainerMarginAfter;

	UT_sint32 iBreakAt = pTab->wantVBreakAt(iAvail-1);

	UT_ASSERT(iBreakAt <= (iAvail-1));

	if(iBreakAt + iWorkingColHeight <= iMaxColHeight)
	{
        // We can break this table and keep some of it in this
        // column. The rest goes into the next column.

		if(!pTab->isThisBroken())
		{
			// Break it at 0 first.
			UT_DEBUGMSG(("SEVIOR: Breaking MAster iBreakAt %d yloc = %d \n",
						 iBreakAt,pTab->getY()));
			UT_DEBUGMSG(("SEVIOR: iBreakLO %d iWorkingColHeight %d "
						 "iMaxColHeight %d Container Height %d "
						 "MArginAfter %d \n",iBreakLO,iWorkingColHeight,
						 iMaxColHeight,pTab->getHeight(), 
						 iContainerMarginAfter ));
			fp_Container * pNext = static_cast<fp_Container *>(pTab->getNext());
			UT_DEBUGMSG(("SEVIOR: getNext %x \n",pNext));
			if(pNext)
				UT_DEBUGMSG(("SEVIOR: Container of next %d \n",pNext->getContainerType()));
			pTab->deleteBrokenTables();
			pTab->VBreakAt(0);
		}

        // Now get a broken table and break it again.
		if(!pTab->isThisBroken())
			pBroke = pTab->getFirstBrokenTable();
		else
			pBroke = pTab;

        // Look to see if we have to move the table out of this container.
		if(iBreakAt < 30)
		{
			pOffendingContainer = pTab;
		}
		else
		{
            // When we break the table, the bit broken will be placed
            // after the current table in the column. This then
            // becomes the offending container and will be bumped into
            // the next column. Pretty cool eh ? :-)
			pOffendingContainer = static_cast<fp_Container *>(pBroke->VBreakAt(iBreakAt));
			UT_DEBUGMSG(("SEVIOR: Created broken table %x \n",
							 pOffendingContainer));
			pLastContainerToKeep = static_cast<fp_Container *>(pTab);
			UT_DEBUGMSG(("SEVIOR: Set lasttokeep 1 %x \n",
							 pLastContainerToKeep));
		}
		return true;
	}
	return false;
#endif
}
