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
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fp_ContainerObject.h"
#include "fp_TableContainer.h"
#include "fp_FootnoteContainer.h"
#include "fp_TOCContainer.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "ut_assert.h"
#include "fl_ContainerLayout.h"
#include "fl_FootnoteLayout.h"
#include "fp_Page.h"

fb_ColumnBreaker::fb_ColumnBreaker(fl_DocSectionLayout * pDSL) :
	m_pStartPage(NULL),
	m_bStartFromStart(true),
	m_bReBreak(false),
	m_pDocSec(pDSL),
	m_pCurrentBlock(NULL)
{
}

void fb_ColumnBreaker::setStartPage(fp_Page * pPage)
{
  //	if(!m_bStartFromStart)
	{
		if(m_pStartPage == NULL)
		{
			m_pStartPage = pPage;
			return;
		}
		FL_DocLayout * pDL = m_pDocSec->getDocLayout();
		UT_sint32 iCurPage = pDL->findPage(m_pStartPage);
		UT_sint32 iNewPage = pDL->findPage(pPage);
		if(iCurPage < 0 && iNewPage >= 0)
		{
			m_pStartPage = pPage;
		}
		else if( (iNewPage >= 0) && (iNewPage < iCurPage))
		{
			m_pStartPage = pPage;
		}
		else if( (iNewPage < 0) && (iCurPage < 0))
		{
			m_pStartPage = NULL;
			m_bStartFromStart = true;
		}
	}
	if(pPage == NULL)
	{
		m_bStartFromStart = true;
		m_pStartPage = NULL;
	}
	return;
}

/*!
 * Returns true if Annotations are to be display
 */
bool fb_ColumnBreaker::_displayAnnotations(void)
{
  return m_pDocSec->getDocLayout()->displayAnnotations();
}

/*!
 * Returns NULL if no rebreak is required. 
 Otherwise returns a pointer to the page requiring the rebreak.
*/
fp_Page * fb_ColumnBreaker::needsRebreak(void)
{
    fl_ContainerLayout * pCL = m_pDocSec->getLastLayout();
    fl_BlockLayout * pBL = NULL;
    if(pCL && (pCL->getContainerType() == FL_CONTAINER_BLOCK))
    {
        pBL = static_cast<fl_BlockLayout *>(pCL);
    }
    else if(pCL)
    {
        pBL = pCL->getPrevBlockInDocument();
    }
    else
    {
	return _getLastValidPage();
    }
    if(pBL)
    {
        fp_Line * pLine = static_cast<fp_Line *>(pBL->getLastContainer());
	if(pLine == NULL)
	{
	     return _getLastValidPage();
	}
	fp_Page * pPage = pLine->getPage();
	if(pPage == NULL)
	{
	     return _getLastValidPage();
	}
	else if(pLine->getY() >  m_pDocSec->getActualColumnHeight())
	{
	     return pPage;
	}
	else
	{
	    fp_Column * pCol = pPage->getNthColumnLeader(0);
	    if(pCol->getHeight() >  m_pDocSec->getActualColumnHeight())
	    {
		return pPage;
	    }
	}
    }
    return NULL;
}

	
/*!
  Layout sections on pages
  \return zero

  This is the function that controls how sections and thereby columns,
  blocks, and lines are laid out on the pages. Doing so it refers to
  the various layout configurations such as orphan/widow controls and
  break Runs embedded in the text.
  At the eof the break we check to see that that all the text has been layed
 out.
 If it hasn't we rebreak from invalid page forward.
 FIXME we should try to detect the error in the code rather than rebreak.
*/

UT_sint32 fb_ColumnBreaker::breakSection()
{
  fp_Page * pStartPage = m_pStartPage;
  m_pDocSec->setNeedsSectionBreak(false,m_pStartPage);
  FL_DocLayout * pDL =  m_pDocSec->getDocLayout();
  m_bStartFromStart = true;
  UT_sint32 iPage = pDL->findPage(pStartPage);  
  if(iPage < 0)
       pStartPage = NULL;
  UT_sint32 iVal = _breakSection(pStartPage);
  UT_sint32 icnt = 0;
  pStartPage = needsRebreak();
  if(m_pStartPage)
  {
      icnt = 0;
      pStartPage = m_pStartPage;
  }
  while(pStartPage && (icnt < 50))
  {

      UT_DEBUGMSG(("Try to get a decent break Column %d times \n",icnt));
      iPage = pDL->findPage(pStartPage);
      if(iPage < 0)
      {
	  pStartPage = NULL;
      }
      else
      {
	  UT_DEBUGMSG(("Retrying from page %d \n",iPage));
	  if(icnt >15)
	  {
	      if(pStartPage->getAvailableHeight() <0)
	      {
		  while(pStartPage->countFootnoteContainers()>0)
		  {
		      fp_FootnoteContainer* pFC = pStartPage->getNthFootnoteContainer(0);
		      pStartPage->removeFootnoteContainer(pFC);
		  }
	      }
	  }
      }

      iVal = _breakSection(pStartPage);
      pStartPage = needsRebreak();
      if(m_pStartPage)
      {
  	  pStartPage = m_pStartPage;
	  if(icnt > 10)
	  {
	      pStartPage = pStartPage->getPrev();
	  }
      }
      icnt++;
  }
  pDL->deleteEmptyColumnsAndPages();
  return iVal;
}

fp_Page * fb_ColumnBreaker::_getLastValidPage(void)
{
  fp_Page * pPage = NULL;
  fp_Page * pFoundPage = NULL;
  UT_sint32 i = 0;
  FL_DocLayout * pDL = m_pDocSec->getDocLayout();
  for(i=0; i<pDL->countPages();i++)
  {
      pPage = pDL->getNthPage(i);
      if(pPage->getOwningSection() == m_pDocSec)
      {
	  pFoundPage = pPage;
      }
      else if(pFoundPage != NULL)
	break;
  }
  return pFoundPage;
}


UT_sint32 fb_ColumnBreaker::_breakSection(fp_Page * pStartPage)
{
	m_bReBreak = false;
	m_pStartPage = NULL;
	FL_DocLayout * pDocLayout = m_pDocSec->getDocLayout();
	fl_ContainerLayout* pFirstLayout = NULL;
	fp_Container* pOuterContainer = NULL;
	fp_Column* pCurColumn = NULL;
	fp_Column* pPrevColumn = NULL;
	UT_sint32 iCurPage = 0;
	UT_sint32 iColCons =  0;
	xxx_UT_DEBUGMSG(("Doing ColumnBreak for section %x at page %x \n",m_pDecSec,m_pStartPage));
//	UT_ASSERT(m_pDocSec->needsSectionBreak());
	pFirstLayout = m_pDocSec->getFirstLayout();
	if (!pFirstLayout)
	{
		m_pStartPage = NULL;
		m_bStartFromStart = false;
		return 0;
	}
	if(pStartPage)
	{
	    iCurPage =  pDocLayout->findPage(pStartPage);
	    if(iCurPage == 0)
	        pStartPage=NULL;
	}
	pOuterContainer = pFirstLayout->getFirstContainer();
	if(pOuterContainer
		&& pOuterContainer->getContainerType() == FP_CONTAINER_FRAME)
	{
	            fl_ContainerLayout * pCL = pOuterContainer->getSectionLayout()->getNext();
		    if(pCL)
		    {
			pOuterContainer = static_cast<fp_Container *>(pCL->getFirstContainer());
		    }

	}
	pCurColumn = static_cast<fp_Column*>(m_pDocSec->getFirstContainer());
	xxx_UT_DEBUGMSG(("fb_ColumnBreaker: For DocSec %x first column %x \n",m_pDecSec,pCurColumn));
	if(pStartPage && (pStartPage->getNthColumnLeader(0)))
	{
		xxx_UT_DEBUGMSG(("Layout from page %x \n",pStartPage));
		pCurColumn = pStartPage->getNthColumnLeader(0);
		pOuterContainer = pCurColumn->getFirstContainer();
		if(pOuterContainer && pOuterContainer->getContainerType() == FP_CONTAINER_LINE)
		{
			
			pFirstLayout = static_cast<fl_ContainerLayout *>(static_cast<fp_Line *>(pOuterContainer)->getBlock());
		}
		else if(pOuterContainer && pOuterContainer->getContainerType() == FP_CONTAINER_ENDNOTE)
		{
			pFirstLayout = static_cast<fl_ContainerLayout *>(pOuterContainer->getSectionLayout());
		}
		else if(pOuterContainer &&pOuterContainer->getContainerType() == FP_CONTAINER_TABLE )
		{
			UT_ASSERT(pOuterContainer->getContainerType() == FP_CONTAINER_TABLE);
			pFirstLayout = static_cast<fl_ContainerLayout *>(pOuterContainer->getSectionLayout());
		}
		else if(pOuterContainer)
		{
			UT_ASSERT(pOuterContainer->getContainerType() == FP_CONTAINER_TOC);
			pFirstLayout = static_cast<fl_ContainerLayout *>(pOuterContainer->getSectionLayout());
		}
	}
	fp_Page * pPrevPage = NULL;
	// attachment 3627 from bug 9878 has pCurColumn == NULL
	if (pCurColumn) {
		pPrevPage = pCurColumn->getPage();
	}
	xxx_UT_DEBUGMSG(("Starting ColumnBreak now pOuterContainer %p \n",pOuterContainer));
	while (pCurColumn)
	{
	        if(  m_pStartPage != NULL)
		{
		  // Abort and start again
		    break;
		}
		iColCons = -1;
		if(pCurColumn && pCurColumn->getPrev())
		{
		    fp_Column * pPrevCol = static_cast<fp_Column *>(pCurColumn->getPrev());
		    fp_Page * pPrevPrevPage = pPrevCol->getPage();
		    if((pPrevCol->getHeight() == 0) && !pPrevCol->containsPageBreak())
		    {
		        bool bPageBreak = false;
		        fp_Column * pCol2 = static_cast<fp_Column *>(pPrevCol->getPrev());
			while(pCol2 && (pCol2->getPage() == pPrevPrevPage) && !pCol2->containsPageBreak())
			{
			    pCol2 = static_cast<fp_Column *>(pCol2->getPrev());
			}
			bPageBreak = (pCol2 && pCol2->containsPageBreak());
		      // Abort and start again from this page
			if(!bPageBreak)
			{
			    m_pStartPage = pPrevPrevPage;
			    break;
			}
		    }
		    if(pPrevCol->getHeight() >  m_pDocSec->getActualColumnHeight())
		    {
		      // Abort and start again from this page

			m_pStartPage = pPrevPrevPage;
			break;
		    }
		}
	        xxx_UT_DEBUGMSG(("New column pOuterContainer %p \n",pOuterContainer));
		bool bSaveset = false;
		if(pOuterContainer)
		{
		        pDocLayout->setSaveContainerPointer(pOuterContainer); 
			bSaveset = true;
		}
		if(pCurColumn->getPage() != pPrevPage)
		{
			if(pPrevPage)
			{
//
// check and update wrapped text around positioned objects
//
			        UT_sint32 iPage = m_pDocSec->getDocLayout()->findPage(pPrevPage);
				pDocLayout->setFramePageNumbers(iPage);
				//pOuterContainer->setAllowDelete(false);
				// 
				// pOuterContainer can be deleted by this call
				//
				fp_Container * pNextContainer = pPrevPage->updatePageForWrapping(pCurColumn);
				//pOuterContainer->setAllowDelete(true);
				xxx_UT_DEBUGMSG(("Returned container updatePage %x \n",pNextContainer));
				if(pNextContainer != NULL)
				{
					pOuterContainer = pNextContainer;
				}
			}
		}
		if((pDocLayout->getRebuiltBlock() == NULL) && bSaveset)
		{
		    pDocLayout->getSavedContainerPointer()->setAllowDelete(true);
		}
		else if(bSaveset)
		{
		  //
		  // pOuterContainer has been deleted!
		  //
		    if(pDocLayout->getSavedContainerPointer() == pOuterContainer)
		    {
		        UT_DEBUGMSG(("AbiWord Saved from a dangling pointer in fb_ColumnBreak \n"));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			pOuterContainer =  pDocLayout->getRebuiltBlock()->getFirstContainer();
		    }
		}
		pPrevPage = pCurColumn->getPage();
		fp_Container* pFirstContainerToKeep = pOuterContainer;
		xxx_UT_DEBUGMSG(("SEVIOR: first to keep 1 %x \n",pFirstContainerToKeep));
		fp_Container* pLastContainerToKeep = NULL;
		fp_Container* pOffendingContainer = NULL;
		UT_sint32 iMaxSecCol = m_pDocSec->getActualColumnHeight();
 		UT_sint32 iMaxColHeight = pCurColumn->getMaxHeight();
		UT_sint32 iFootnoteHeight = 0;
		bool bEquivColumnBreak = false;
		xxx_UT_DEBUGMSG(("fb_ColumnBreaker: iMaxSecCol = %d iMaxColHeight = %d \n",iMaxSecCol,iMaxColHeight));
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
				static_cast<fp_Container *>(pCurContainer->getPrevContainerInSection());
			if(pPrevContainer && 
			   (pPrevContainer->getContainerType() == FP_CONTAINER_LINE))
			{
				fp_Line * pL = static_cast<fp_Line *>(pPrevContainer);
				
				if (pL && pL->containsForcedPageBreak() && pL->getContainer() 
				&& (pCurColumn->getPage() == pL->getContainer()->getPage()))
				{
					  //						pCurColumn->validate();
				  pCurColumn = static_cast<fp_Column *>(pCurColumn->getNext());
				  continue;
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
		  //		        pCurContainer->setMyBrokenContainer(NULL);
		        iColCons++;
		        xxx_UT_DEBUGMSG(("curContainer pointer %p type %s \n",iLoop,pCurContainer,pCurContainer->getContainerString()));
			if(pCurContainer->getDocSectionLayout() != m_pDocSec)
			{
			        break;
			}
			if(pCurContainer->getContainerType() == FP_CONTAINER_FOOTNOTE || ((pCurContainer->getContainerType() == FP_CONTAINER_ANNOTATION)))
			{
//
// skip this! We've already taken it's height into account.
//
				pCurContainer = _getNext(pCurContainer);
				continue;
			}
			if(pCurContainer->getContainerType() == FP_CONTAINER_FRAME)
			{
//
// skip this! It's height is ignored in the layout
//
				pCurContainer = _getNext(pCurContainer);
				continue;
			}
			UT_sint32 iContainerHeight = 0;
			UT_sint32 iContainerMarginAfter = pCurContainer->getMarginAfter();
			fp_Container * pVTab = NULL;
			if(pCurContainer->isVBreakable())
			{
				pVTab = static_cast<fp_Container *>(pCurContainer);
				UT_sint32 iOldBreak = _getLastWantedVBreak(pVTab); 
				xxx_UT_DEBUGMSG(("pVTab height is %d \n",pVTab->getHeight()));
				if( (iOldBreak > 0) && !_isThisBroken(pVTab))
				{
					UT_sint32 iAvail = iMaxColHeight - iWorkingColHeight - iContainerMarginAfter;
					UT_sint32 iBreakAt = pVTab->wantVBreakAt(iAvail-1);
					if(iBreakAt != iOldBreak)
					{
//
// Rebreak it
//
						if(pVTab->getContainerType() == FP_CONTAINER_TABLE)
						{
						        xxx_UT_DEBUGMSG(("Deleting broken tables for %p \n",pVTab));
						        UT_ASSERT(static_cast<fp_TableContainer *>(pVTab)->getMasterTable() == NULL);
							static_cast<fp_TableContainer *>(pVTab)->deleteBrokenTables(true,true);
						}
						else
						{
							static_cast<fp_TOCContainer *>(pVTab)->deleteBrokenTOCs(true);
						}
						pVTab->VBreakAt(0);
						_setLastWantedVBreak(pVTab,iBreakAt);
					}
				}
				iContainerHeight = pVTab->getHeight();
				if(pVTab->getContainerType() == FP_CONTAINER_TOC)
				{
					iContainerHeight = static_cast<fp_TOCContainer *>(pVTab)->getHeight();
				}
			}
			else
			{
				iContainerHeight = pCurContainer->getHeight();
			}
			iTotalContainerSpace = iContainerHeight + iContainerMarginAfter;
			if (pCurContainer && 
				pCurContainer->getContainerType() == FP_CONTAINER_LINE)
			{
				fp_Line* pCurLine = static_cast<fp_Line *>(pCurContainer);
				if(pCurLine->isSameYAsPrevious())
				{
//
// No vertical changes to skip this
//
					pCurContainer = _getNext(pCurContainer);
					continue;
				}
				// Excellent.  If we have a footnote or annotation, we can start deducting
				// from the working height if the footnote container is not on
				// this page.
				if (pCurLine->containsFootnoteReference())
				{
					// Ok.  Now, deduct the proper amount from iMaxColHeight.

					// OK get a vector of the footnote containers in this line.
				        UT_sint32 iTheseFootnotes = 0; 
					UT_GenericVector<fp_FootnoteContainer*> vecFootnotes;
					pCurLine->getFootnoteContainers(&vecFootnotes);
					fp_Page *pCurPage = pCurColumn->getPage();
					// Now loop through all these and add them to the height.
					UT_sint32 i =0;
					for(i=0; i< vecFootnotes.getItemCount();i++)
					{
						fp_FootnoteContainer * pFC = vecFootnotes.getNthItem(i);
						if(pFC->getHeight() > iMaxSecCol)
						{
						    UT_DEBUGMSG(("Footnote container height %d \n",pFC->getHeight()));
						    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
						    pFC->setHeight(iMaxSecCol -600);
						}
						if(pFC && ((pFC->getPage() == NULL) || (pFC->getPage() != pCurPage)))
						{
						        iTheseFootnotes += pFC->getHeight();
							iFootnoteHeight += pFC->getHeight();
						}
					}	
					xxx_UT_DEBUGMSG(("got footnote section height %d\n", iFootnoteHeight));
					iWorkingColHeight += iTheseFootnotes;
				}
				if(pCurLine->containsAnnotations() && _displayAnnotations())
				  {
					// Ok.  Now, deduct the proper amount from iMaxColHeight.

					// OK get a vector of the Annotation containers in this line.
				        UT_sint32 iTheseAnnotations = 0; 
				 
					UT_GenericVector<fp_AnnotationContainer*> vecAnnotations;
					pCurLine->getAnnotationContainers(&vecAnnotations);
					fp_Page *pCurPage = pCurColumn->getPage();
					// Now loop through all these and add them to the height.
					UT_sint32 i =0;
					for(i=0; i< vecAnnotations.getItemCount();i++)
					{
						fp_AnnotationContainer * pAC = vecAnnotations.getNthItem(i);
						if(pAC && ((pAC->getPage() == NULL) || (pAC->getPage() != pCurPage)))
						{
						        iTheseAnnotations += pAC->getHeight();
							iFootnoteHeight += pAC->getHeight();
						}				
					}	
					iWorkingColHeight += iTheseAnnotations;

				  }
			}
			xxx_UT_DEBUGMSG(("WorkingColHeight = %d \n",iWorkingColHeight));
			xxx_UT_DEBUGMSG(("iTotalContainerSpace = %d MarginAfter %d \n",iTotalContainerSpace,iContainerMarginAfter));
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
				if(pOffendingContainer == NULL)
				{
				        break;
				}
				if(pOffendingContainer->isVBreakable())
				{
					xxx_UT_DEBUGMSG(("fb_ColumnBreak 1 Broken Container num %d \n",(static_cast<fp_TableContainer *>(pOffendingContainer))->getBrokenNumber()));
					if (_breakCON(pOffendingContainer,
									pLastContainerToKeep,
									iMaxColHeight, iWorkingColHeight, 
									iContainerMarginAfter))
					{
						pPrevWorking = pCurContainer;
						pCurContainer = pOffendingContainer;
					}
					else if(pOffendingContainer && pOffendingContainer->getContainerType() == FP_CONTAINER_TABLE)
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
					else if( pOffendingContainer == NULL)
					{
					        break;
					}
					else
					{
//
// Can't break the TOC so bump it.
//
						pCurContainer = pOffendingContainer;
						UT_ASSERT(pCurContainer->getContainerType() == FP_CONTAINER_TOC);
						fp_TOCContainer * pTOCOffend = static_cast<fp_TOCContainer *>(pOffendingContainer);
						pLastContainerToKeep = pTOCOffend->getPrevContainerInSection();
					    UT_DEBUGMSG(("Can't break TOC. pCurContainer %p pTabOffend %p pLastContainerToKeep %p \n",pCurContainer,pTOCOffend,pLastContainerToKeep));
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
					bool bIsTableOrTOC = false;
					fl_BlockLayout* pBlock = NULL;
					fl_ContainerLayout * pConLayout = NULL;
					if(pOffendingContainer->getContainerType() == FP_CONTAINER_LINE)
					{
						pBlock = static_cast<fp_Line *>(pOffendingContainer)->getBlock();
						iWidows = pBlock->getProp_Widows();
						iOrphans = pBlock->getProp_Orphans();
						pConLayout = static_cast<fl_ContainerLayout *>(pBlock);
					}
					else if(pOffendingContainer->getContainerType() == FP_CONTAINER_ENDNOTE)
					{
						pConLayout = static_cast<fl_ContainerLayout *>(pOffendingContainer->getSectionLayout());
						pLastContainerToKeep = pOffendingContainer->getPrevContainerInSection();
						if(pLastContainerToKeep == NULL)
						{
							fl_DocSectionLayout * pDSL = pConLayout->getDocSectionLayout();
							UT_ASSERT(pDSL);
							UT_ASSERT(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION);
							pLastContainerToKeep = pDSL->getLastContainer();
						}
						UT_ASSERT(pLastContainerToKeep);
//
// All this deals with widows and orphans. We don't need this for
// tables or TOCs
//
						break;
					}
					else
					{
						bIsTableOrTOC = true;
						pConLayout = static_cast<fl_ContainerLayout *>(pOffendingContainer->getSectionLayout());
					}
					if(bIsTableOrTOC)
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
						if(pLastContainerToKeep && pLastContainerToKeep->getContainerType() == 
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
//
// If this happens to be a TOC better get the LAST of the broken TOCs
//
						if(pLastContainerToKeep && pLastContainerToKeep->getContainerType() == 
						                    FP_CONTAINER_TOC && 
						   pFirstContainerToKeep->getContainerType() == 
						                    FP_CONTAINER_TOC)
						{
							//
							// Can get into trouble if the pFirstContainer
							// points to a broken table after
							// this. Check for this.
							// Not sure why this code is meant to work at all
#if 0
							for (fp_TOCContainer * pTOC = 
								 static_cast<fp_TOCContainer *>(pLastContainerToKeep);
								 pTOC;
								 pTOC=static_cast<fp_TOCContainer *>(pTOC->getNext()))
							{
								if(pTOC == pFirstContainerToKeep)
								{
									xxx_UT_DEBUGMSG(("SEVIOR: FirstContainer l325 %x \n",pFirstContainerToKeep)); 
									pFirstContainerToKeep = 
										pLastContainerToKeep;
									break;
								}
								
							}
#endif
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
			pCurContainer = _getNext(pCurContainer);
		}
//
// End of inner while loop here. After this we've found LastContainerToKeep
//
		bEquivColumnBreak = bEquivColumnBreak && ( iMaxColHeight < (iWorkingColHeight + iTotalContainerSpace));
		if (pLastContainerToKeep)
		{
		        while(pLastContainerToKeep && ((pLastContainerToKeep->getContainerType() == FP_CONTAINER_FOOTNOTE )
						 || (pLastContainerToKeep->getContainerType() == FP_CONTAINER_ANNOTATION)))
			{
			        pLastContainerToKeep = pLastContainerToKeep->getPrevContainerInSection();
			}
			if(pLastContainerToKeep)
			{
				pOuterContainer = _getNext(pLastContainerToKeep);
			}
			else
			{
				pOuterContainer = NULL;
			}
		}
		else
			pOuterContainer = NULL;
//
// OK fill our column with content between pFirstContainerToKeep and pLastContainerToKeep
//
		xxx_UT_DEBUGMSG(("Doing column fill now pCurContainer %x pFirstContainer %x \n",pCurContainer,pFirstContainerToKeep));
		pCurContainer = pFirstContainerToKeep;
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pFirstContainerToKeep);
		UT_UNUSED(pTab); // only for debug.
		UT_sint32 conPos = 0;
		while (pCurContainer)
		{
			xxx_UT_DEBUGMSG(("Container %x is in Column %x Type %d numCons %d \n",pCurContainer,pCurColumn,pCurContainer->getContainerType(),pCurColumn->countCons()));
			if(pCurContainer->getContainerType() == FP_CONTAINER_FOOTNOTE  || 
			   (pCurContainer->getContainerType() == FP_CONTAINER_ANNOTATION) )
			{
//
// Skip this. It doesn't go in this column at all.
//
				pCurContainer = _getNext(pCurContainer);
				continue;
			}
			if(pCurContainer->getDocSectionLayout() != m_pDocSec)
			{
			    break;
			}
			if (pCurContainer->getContainer() != pCurColumn || (pCurColumn->findCon(pCurContainer) < 0) )
			{
//
// Endnotes don't get placed in columns until here
//
				if(pCurContainer->getContainerType() == FP_CONTAINER_ENDNOTE)
				{
					if(pCurContainer->getContainer() == NULL)
					{
						pCurColumn->addContainer(pCurContainer);
					}
				}
//				UT_ASSERT(pCurContainer->getContainer());
				if(pCurContainer->getContainer() && pCurContainer->getContainer()->findCon(pCurContainer) >= 0)
				{
					fp_VerticalContainer *pVert = static_cast<fp_VerticalContainer *>(pCurContainer->getContainer());
#if 0
					if(pCurContainer->getContainer() != pCurColumn)
					{
						pCurColumn->addContainer(pCurContainer);
					}
#endif
					pVert->removeContainer(pCurContainer,true);
				}
				if((pCurContainer->getContainer() != pCurColumn) && (pCurContainer->getDocSectionLayout() == pCurColumn->getDocSectionLayout()))
				{
					if(pCurContainer->getContainerType() != FP_CONTAINER_FRAME)
					{
						pCurColumn->addContainer(pCurContainer);
					}
					else
					{
						fp_Page * pPage = pCurContainer->getPage();
						fp_Page * pNextPage = pCurColumn->getPage();
						if(pNextPage && pPage && (pNextPage != pPage))
						{
							fp_FrameContainer * pFC = reinterpret_cast<fp_FrameContainer *>(pCurContainer);
							if(( pDocLayout->findPage(pPage) >= 0) && ( pDocLayout->findPage(pNextPage) >= 0))
							{
								if((pPage->findFrameContainer(pFC) >=0) &&
								   (pNextPage->findFrameContainer(pFC) < 0))
								{
									pPage->removeFrameContainer(pFC);
									pNextPage->insertFrameContainer(pFC);
								}				
							}			
						}
					}
				}
				
			}
//
// Now make sure footnotes and annotations are on the same page as the reference.
//
			if(pCurContainer->getContainerType() == FP_CONTAINER_LINE)
			{
				fp_Line * pCurLine = static_cast<fp_Line *>(pCurContainer);
				xxx_UT_DEBUGMSG(("About to call containerFootnoteReferenced \n"));
				if(pCurLine->containsFootnoteReference())
				{
					// OK get a vector of the footnote containers in this line.
					UT_GenericVector<fp_FootnoteContainer*> vecFootnotes;
					pCurLine->getFootnoteContainers(&vecFootnotes);
				
					// Now loop through all these and check they're on this
					// page. If not add them.
					fp_Page * pCurPage = pCurColumn->getPage();
					UT_ASSERT(pCurPage);
					UT_sint32 i =0;
					for(i=0; i< vecFootnotes.getItemCount();i++)
					{
						fp_FootnoteContainer * pFC = vecFootnotes.getNthItem(i);
						if(pFC != NULL)
						{
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
				if(pCurLine->containsAnnotations() && _displayAnnotations())
				{
					// OK get a vector of the footnote containers in this line.
					UT_GenericVector<fp_AnnotationContainer*> vecAnnotations;
					pCurLine->getAnnotationContainers(&vecAnnotations);
				
					// Now loop through all these and check they're on this
					// page. If not add them.
					fp_Page * pCurPage = pCurColumn->getPage();
					UT_ASSERT(pCurPage);
					UT_sint32 i =0;
					for(i=0; i< vecAnnotations.getItemCount();i++)
					{
						fp_AnnotationContainer * pAC = vecAnnotations.getNthItem(i);
						if(pAC != NULL)
						{
							fp_Page * myPage = pAC->getPage();
							UT_DEBUGMSG(("Annotation %x is on Page %x \n",pAC,myPage));
							if(myPage != pCurPage)
							{
								if(myPage == NULL)
								{
									pCurPage->insertAnnotationContainer(pAC);
								}
								else
								{
									myPage->removeAnnotationContainer(pAC);
									pCurPage->insertAnnotationContainer(pAC);
								}
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
						UT_GenericVector<fp_FootnoteContainer*> vecFootnotes;
						pCurTable->getFootnoteContainers(&vecFootnotes);
						
					// Now loop through all these and check they're on this
					// page. If not add them.
						fp_Page * pCurPage = pCurColumn->getPage();
						UT_ASSERT(pCurPage);
						UT_sint32 i =0;
						for(i=0; i< vecFootnotes.getItemCount();i++)
						{
							xxx_UT_DEBUGMSG(("Found reference %d in broken table %x \n",i,pCurTable));
							fp_FootnoteContainer * pFC = vecFootnotes.getNthItem(i);
							fp_Page * myPage = pFC->getPage();
							xxx_UT_DEBUGMSG(("Footnote %x is on Page %x \n",pFC,myPage));
							if(myPage != pCurPage)
							{
								xxx_UT_DEBUGMSG((" Moving anchor from %x to %x \n",myPage,pCurPage));
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
					if(pCurTable->containsAnnotations()  && _displayAnnotations())
					{
						// OK get a vector of the footnote containers in this line.
						UT_GenericVector<fp_AnnotationContainer*> vecAnnotations;
						pCurTable->getAnnotationContainers(&vecAnnotations);
						
					// Now loop through all these and check they're on this
					// page. If not add them.
						fp_Page * pCurPage = pCurColumn->getPage();
						UT_ASSERT(pCurPage);
						UT_sint32 i =0;
						for(i=0; i< vecAnnotations.getItemCount();i++)
						{
							xxx_UT_DEBUGMSG(("Found reference %d in broken table %x \n",i,pCurTable));
							fp_AnnotationContainer * pAC = vecAnnotations.getNthItem(i);
							fp_Page * myPage = pAC->getPage();
							xxx_UT_DEBUGMSG(("Annotation %x is on Page %x \n",pAC,myPage));
							if(myPage != pCurPage)
							{
								xxx_UT_DEBUGMSG((" Moving anchor from %x to %x \n",myPage,pCurPage));
								if(myPage == NULL)
								{
									pCurPage->insertAnnotationContainer(pAC);
								}
								else
								{
									myPage->removeAnnotationContainer(pAC);
									pCurPage->insertAnnotationContainer(pAC);
								}
							}
						}
					}
				}
			}
//
// Code to fix order in the column
// 
#if 1
			if((pCurColumn->findCon(pCurContainer) >= 0) && (pCurColumn->findCon(pCurContainer)  != conPos))
			{
				xxx_UT_DEBUGMSG(("fb_ColumnBreaker:Container out of order. Should be at %d is at %d \n",conPos,pCurColumn->findCon(pCurContainer)));
				xxx_UT_DEBUGMSG(("fb_ColumnBreak: Fixing this now orig num cons %d \n",pCurColumn->countCons()));
				static_cast<fp_VerticalContainer *>(pCurContainer->getContainer())->removeContainer(pCurContainer);
				pCurColumn->insertConAt(pCurContainer,conPos);
				pCurContainer->setContainer(pCurColumn);
				xxx_UT_DEBUGMSG(("fb_ColumnBreak: Insert at %d now num cons \n",conPos,pCurColumn->countCons()));
//				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
#endif
			conPos++;
			if (pCurContainer == pLastContainerToKeep)
			{
				break;
			}
			else
			{
				if((pLastContainerToKeep!=NULL) && (_getNext(pCurContainer) == NULL))
				{
					UT_DEBUGMSG(("Non null LastContainerToKeep yet next container is NULL!!!!!!!!!!!! \n"));
					UT_DEBUGMSG((" CurContainer %p type %d \n",pCurContainer,pCurContainer->getContainerType()));
					UT_DEBUGMSG((" FirstContainer to keep %p Last container to keep %p \n",pTab,pLastContainerToKeep));
					UT_DEBUGMSG(("Try to recover.... \n"));
					//	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					pLastContainerToKeep = NULL;
					break;
				}
				pCurContainer = _getNext(pCurContainer);
			}
		}

		//
		// end of add container to columns
		//
		if (pLastContainerToKeep
			&& pCurColumn->getLastContainer() != pLastContainerToKeep)
		{
			if(pLastContainerToKeep->getColumn()!=pCurColumn)
			{
				UT_DEBUGMSG(("4111 bug \n"));
			}
			UT_ASSERT(pLastContainerToKeep->getColumn()==pCurColumn);
			fp_Page* pPrevPage2 = pCurColumn->getPage();
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
						pNextColumn = static_cast<fp_Column*>
						 (m_pDocSec->getNewContainer(_getNext(pLastContainerToKeep)));
					else
						pNextColumn = static_cast<fp_Column*>(m_pDocSec->getNewContainer(NULL));
				}
			}
			while (pLastContainerToKeep->getContainerType()
				             == FP_CONTAINER_LINE &&
				   static_cast<fp_Line *>(pLastContainerToKeep)
				        ->containsForcedPageBreak() &&
				   (pNextColumn->getPage() == pPrevPage2));
			if(pCurColumn != NULL && (pCurColumn == pNextColumn))
			{
				pCurColumn->layout();
			}
			// Bump content down the columns
			while (pCurColumn != NULL && pCurColumn != pNextColumn)
			{
				xxx_UT_DEBUGMSG(("Start of bump loop pNextColumn %x \n",pNextColumn));
				bool isTOCTABLE = false;
				if(pOuterContainer && ((pOuterContainer->getContainerType() == FP_CONTAINER_TABLE) || (pOuterContainer->getContainerType() == FP_CONTAINER_TABLE)))
				        isTOCTABLE= true;
				pCurColumn->bumpContainers(pLastContainerToKeep);

				pCurColumn->layout();
//
// Layout might delete a broken table or TOC. Check for this.
//
				fp_Container * pCon = pCurColumn->getLastContainer();
				bool bDoneTabTOC = false;
				if(pCon && pCon->getContainerType() == FP_CONTAINER_TABLE)
				{
				        bDoneTabTOC = true;
					pOuterContainer = _getNext(pCon);
				}
				if(pCon && pCon->getContainerType() == FP_CONTAINER_TOC)
				{
					xxx_UT_DEBUGMSG(("Last Con was TOC \n"));
				        bDoneTabTOC = true;
					pOuterContainer = _getNext(pCon);
				}
				if(!bDoneTabTOC && isTOCTABLE)
				{
				        if(pCon)
					{
					      pOuterContainer = _getNext(pCon);
					}
					else
					{
					      pOuterContainer = NULL;
					      pPrevColumn = pCurColumn;
					      pCurColumn = NULL;
					      pLastContainerToKeep = NULL;
					      break;
					}
				}
//				pCurColumn->validate();
				pPrevColumn = pCurColumn;
				pCurColumn = static_cast<fp_Column *>(pCurColumn->getNext());
					// This is only relevant for the initial column. All
					// other columns should flush their entire content.
				pLastContainerToKeep = NULL;
				xxx_UT_DEBUGMSG(("Last of bump loop pCurColumn %x \n",pCurColumn));
			}

		}
		else
		{
			UT_ASSERT((!pLastContainerToKeep) || 
					  (pCurColumn->getLastContainer() == pLastContainerToKeep));

			bool bTableTest = false;
			bool bTOCTest = false;
			if(pOuterContainer)
				bTableTest = (pOuterContainer->getContainerType() == 
							  FP_CONTAINER_TABLE);
			if(pOuterContainer)
				bTOCTest = (pOuterContainer->getContainerType() == 
							  FP_CONTAINER_TOC);
			pCurColumn->layout();

			// pCurColumn->layout() might delete a broken table or TOC; fixup.
			if(bTableTest &&
			   pOuterContainer != pCurColumn->getLastContainer())
			{
				pOuterContainer =  _getNext(pCurColumn->getLastContainer());
				bTableTest = true;
			}
			if(bTOCTest &&
			   pOuterContainer != pCurColumn->getLastContainer())
			{
				pOuterContainer =  _getNext(pCurColumn->getLastContainer());
				bTOCTest = true;
			}
			//			pCurColumn->validate();
			pPrevColumn = pCurColumn;
			pCurColumn = static_cast<fp_Column *>(pCurColumn->getNext());
			if(pCurColumn == NULL && (bTableTest || bTOCTest) && pOuterContainer != NULL)
			{
				if(pOuterContainer->isVBreakable())
				{
					pCurColumn = static_cast<fp_Column *>(m_pDocSec->getNewContainer(NULL));
				}
				else
				{
					pCurColumn = static_cast<fp_Column *>(m_pDocSec->getNewContainer(pOuterContainer));
				}
			}
			else if(pCurColumn == NULL && pLastContainerToKeep && _getNext(pLastContainerToKeep))
			{
				fp_Container * pCon = _getNext(pLastContainerToKeep);
				if(pCon->isVBreakable())
				{
					pCurColumn =  static_cast<fp_Column *>(m_pDocSec->getNewContainer(NULL));
				}
				else
				{
					pCurColumn =  static_cast<fp_Column *>(m_pDocSec->getNewContainer(pCon));
				}
			}
		}
		if(pCurColumn == NULL && pLastContainerToKeep && _getNext(pLastContainerToKeep))
		{
			fp_Container * pCon = _getNext(pLastContainerToKeep);
			if(pCon->isVBreakable())
			{
				pCurColumn =  static_cast<fp_Column *>(m_pDocSec->getNewContainer(NULL));
			}
			else
			{
				pCurColumn =  static_cast<fp_Column *>(m_pDocSec->getNewContainer(pCon));
			}
		}
//
// check for wrapped objects on last page
//
		if(pCurColumn == NULL)
		{
			UT_ASSERT(pOuterContainer == NULL);
			fp_Page * pPage = pPrevColumn->getPage();
			UT_sint32 iPage = m_pDocSec->getDocLayout()->findPage(pPage);
			m_pDocSec->getDocLayout()->setFramePageNumbers(iPage);
			fp_Container * pNextContainer = pPage->updatePageForWrapping(pCurColumn);
			xxx_UT_DEBUGMSG(("Returned container updatePage %x \n",pNextContainer));
			if(pNextContainer == NULL)
			{
				pCurColumn = NULL;
				pOuterContainer = NULL;
			}
			else
			{
				pOuterContainer = pNextContainer;
			}
		}
		if(pCurContainer && (pCurContainer->getDocSectionLayout() != m_pDocSec))
		{
		        pCurColumn = NULL;
		}
//
// Loop back for next pCurContainer. Finish if pCurColumn == NULL.
//
		xxx_UT_DEBUGMSG(("fb_ColumnBreaker:: Finished this column, doing next now. \n"));
	}
//
// End of massive while loop here
//
	if(m_bReBreak)
	{
		UT_ASSERT(0);
		m_pStartPage = NULL;
		m_bStartFromStart = false;
		breakSection();
	}
//
// Look if the next DocSectionLayout needs section break.(
// This happens if a column height changes. If so, do it!
//
#if 0
	if(m_pDocSec->getNextDocSection() && m_pDocSec->needsSectionBreak())
	{
		m_pDocSec->completeBreakSection();
	}
#endif
	return 0; // TODO return code
}


bool fb_ColumnBreaker::_isThisBroken(fp_Container * pCon)
{
	if(pCon->getContainerType() == FP_CONTAINER_TABLE)
	{
		return static_cast<fp_TableContainer *>(pCon)->isThisBroken();
	}
	else
	{
		return static_cast<fp_TOCContainer *>(pCon)->isThisBroken();
	}
}

void fb_ColumnBreaker::_setLastWantedVBreak(fp_Container * pCon, UT_sint32 iBreakAt)
{
	if(pCon->getContainerType() == FP_CONTAINER_TABLE)
	{
		static_cast<fp_TableContainer *>(pCon)->setLastWantedVBreak(iBreakAt);
	}
	else
	{
		static_cast<fp_TOCContainer *>(pCon)->setLastWantedVBreak(iBreakAt);
	}
}


UT_sint32 fb_ColumnBreaker::_getLastWantedVBreak(fp_Container * pCon)
{
	if(pCon->getContainerType() == FP_CONTAINER_TABLE)
	{
		return static_cast<fp_TableContainer *>(pCon)->getLastWantedVBreak();
	}
	fp_TOCContainer * pTOC = static_cast<fp_TOCContainer *>(pCon);
	UT_sint32 i = pTOC->getLastWantedVBreak();
	return i;
}

/*!
 * Breaks the given breakabke container, if appropriate.  
 * \return true iff the table was broken.
 */
bool fb_ColumnBreaker::_breakCON(fp_Container*& pOffendingContainer,
								   fp_Container*& pLastContainerToKeep,
								   int iMaxColHeight, 
								   int iWorkingColHeight,
								   int iContainerMarginAfter)
{
	if(pOffendingContainer->getContainerType() == FP_CONTAINER_TABLE)
	{
		return _breakTable(pOffendingContainer,pLastContainerToKeep,
						   iMaxColHeight,iWorkingColHeight,
						   iContainerMarginAfter);
	}
	return _breakTOC(pOffendingContainer,pLastContainerToKeep,
						   iMaxColHeight,iWorkingColHeight,
						   iContainerMarginAfter);
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

        xxx_UT_DEBUGMSG(("breakTable:!!!!!!!!!!!! Offending Table is %p \n",pOffendingContainer));
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pOffendingContainer);
	if(!pTab->isThisBroken())
	{
//
// This is the first of this table set. Clear the old broken tables
// and rebreak.
//
		pTab->deleteBrokenTables(true,true);
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
//
	fp_TableContainer * pBroke = NULL;
	UT_sint32 iAvail = iMaxColHeight - iWorkingColHeight - iContainerMarginAfter;
	UT_sint32 iBreakAt = pTab->wantVBreakAt(iAvail-1);
	pTab->setLastWantedVBreak(iBreakAt);
	xxx_UT_DEBUGMSG(("breakTable column: iAvail %d actual break at %d \n",iAvail,iBreakAt));
//
// Look to see if the table can be broken. If iBreakAt < 0 we have to bump 
// the whole table into the next column.
//
	if(iBreakAt < 1)
	{
		xxx_UT_DEBUGMSG(("breakTable Col: Can't break this table %d \n",iBreakAt));
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
			fp_Container * pNext = static_cast<fp_Container *>(pTab->getNext());
			xxx_UT_DEBUGMSG(("SEVIOR: getNext %x \n",pNext));
			if(pNext)
			{
				xxx_UT_DEBUGMSG(("SEVIOR: Container of next %d \n",pNext->getContainerType()));
			}
			pTab->deleteBrokenTables(true,true);
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
		if(pBroke == NULL)
		{
			pBroke = pTab;
		}
//
// Look to see if we have to move the table out of this container.
//
		if(iBreakAt < 30)
		{
		        xxx_UT_DEBUGMSG(("Break too small %d , bumping container height of Tab %d \n",iBreakAt,pTab->getHeight()));
		        pLastContainerToKeep = static_cast<fp_Container *>(pTab->getPrevContainerInSection());
		        pOffendingContainer = static_cast<fp_Container *>(pBroke);
		}
		else
		{
//
// When we break the table, the bit broken will be placed after the current
// table in the column. This then becomes the offending container and will
// be bumped into the next column. Pretty cool eh ? :-)
//
			fp_TableContainer * pNewTab = static_cast<fp_TableContainer *>(pBroke->VBreakAt(iBreakAt));
			pOffendingContainer = static_cast<fp_Container *>(pNewTab);
			pLastContainerToKeep = static_cast<fp_Container *>(pTab);
                        if(pNewTab == NULL)
			{
			  return false;
			}
		        xxx_UT_DEBUGMSG(("SEVIOR: Created broken table %p height %d \n",pOffendingContainer,pOffendingContainer->getHeight()));
			UT_ASSERT(pBroke->getHeight() > 0);
			UT_ASSERT(pNewTab->getHeight() > 0);
			xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 1 %x \n",pLastContainerToKeep));
		}
		return true;
	}
	return false;
}



/*!
 * Breaks the given TOC, if appropriate.  
 * \return true iff the TOC was broken.
 */
bool fb_ColumnBreaker::_breakTOC(fp_Container*& pOffendingContainer,
								   fp_Container*& pLastContainerToKeep,
								   int iMaxColHeight, 
								   int iWorkingColHeight,
								   int iContainerMarginAfter)
{
	bool bDoTOCBreak;

    UT_DEBUGMSG(("breakTOC:!!!!!!!!!!!! Offending TOC is %p \n",pOffendingContainer));
	fp_TOCContainer * pTOC = static_cast<fp_TOCContainer *>(pOffendingContainer);
	if(!pTOC->isThisBroken())
	{
//
// This is the first of this table set. Clear the old broken tables
// and rebreak.
//
		pTOC->deleteBrokenTOCs(true);
		bDoTOCBreak = true;
		xxx_UT_DEBUGMSG(("SEVIOR: Need TOC Break 1 \n"));
		xxx_UT_DEBUGMSG(("firstbroke %x lastbroke %x \n",pTOC->getFirstBrokenTOC(),pTOC->getLastBrokenTOC()));
	}
	else
	{
		bDoTOCBreak = true;
	}
//
// If we don't break the table, the heights of the broken table's
// will be adjusted at the setY() in the layout stage.
//
	fp_TOCContainer * pBroke = NULL;
	UT_sint32 iAvail = iMaxColHeight - iWorkingColHeight - iContainerMarginAfter;
	UT_sint32 iBreakAt = pTOC->wantVBreakAt(iAvail-1);
	pTOC->setLastWantedVBreak(iBreakAt);
	UT_DEBUGMSG(("breakTOC column: iAvail %d actual break at %d \n",iAvail,iBreakAt));
//
// Look to see if the table can be broken. If iBreakAt < 0 we have to bump 
// the whole table into the next column.
//
	if(iBreakAt < 1)
	{
		xxx_UT_DEBUGMSG(("breakTOC Col: Can't break this table %d \n",iBreakAt));
		return false;
	}

	UT_ASSERT(iBreakAt <= (iAvail-1));

	if(bDoTOCBreak && (iBreakAt + iWorkingColHeight <= iMaxColHeight))
	{
//
// OK we can break this table and keep some of it in this column. The
// rest goes into the next column.
//
// Look to see if this table is broken.
//
		if(!pTOC->isThisBroken())
		{
//
// Break it at 0 first.
//
			xxx_UT_DEBUGMSG(("SEVIOR: Breaking MAster iBreakAt %d yloc = %d \n",iBreakAt,pTOC->getY()));
			fp_Container * pNext = static_cast<fp_Container *>(pTOC->getNext());
			xxx_UT_DEBUGMSG(("SEVIOR: getNext %x \n",pNext));
			if(pNext)
			{
				xxx_UT_DEBUGMSG(("SEVIOR: Container of next %d \n",pNext->getContainerType()));
			}
			pTOC->deleteBrokenTOCs(true);
			pTOC->VBreakAt(0);
		}
//
// Now get a broken table and break it again.
//
		if(!pTOC->isThisBroken())
		{
			pBroke = pTOC->getFirstBrokenTOC();
		}
		else
		{
			pBroke = pTOC;
		}
//
// Look to see if we have to move the table out of this container.
//
		if(iBreakAt < 30)
		{
			pOffendingContainer = static_cast<fp_Container *>(pTOC);
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
			fp_TOCContainer * pNewTOC = static_cast<fp_TOCContainer *>(pOffendingContainer);
			UT_ASSERT(pBroke->getHeight() > 0);
			UT_UNUSED(pNewTOC);
			UT_ASSERT(pNewTOC && pNewTOC->getHeight() > 0);
			pLastContainerToKeep = static_cast<fp_Container *>(pTOC);
			xxx_UT_DEBUGMSG(("SEVIOR: Set lasttokeep 1 %x \n",pLastContainerToKeep));
		}
		return true;
	}

	return false;
}

fp_Container * fb_ColumnBreaker::_getNext(fp_Container * pCon)
{
        UT_return_val_if_fail(pCon,NULL);
	fp_Container * pNext = NULL;
	if(pCon->getContainerType() != FP_CONTAINER_ENDNOTE)
	{
		pNext = pCon->getNextContainerInSection();
		if(pNext)
		{
		    UT_return_val_if_fail(pNext->getDocSectionLayout() == m_pDocSec,NULL);
		}
		if(pNext != NULL)
		{
		        xxx_UT_DEBUGMSG(("_getNext: 1 Returning %p \n",pNext));
			return pNext;
		}
		xxx_UT_DEBUGMSG(("_getNext: Returning First endnote \n"));
		pNext = m_pDocSec->getFirstEndnoteContainer();
		xxx_UT_DEBUGMSG(("_getNext: Returning endnote pNext %x \n",pNext));
#if DEBUG
		if(pNext)
		{
			UT_ASSERT(pNext->getContainerType() == FP_CONTAINER_ENDNOTE);
		}
#endif
	}
	else
	{
		xxx_UT_DEBUGMSG(("_getNext: Returning next endnote \n"));
		pNext = static_cast<fp_Container *>(pCon->getNext());
		xxx_UT_DEBUGMSG(("_getNext: Returning endnote pNext %x \n",pNext));
#if DEBUG
		if(pNext)
		{
			UT_ASSERT(pNext->getContainerType() == FP_CONTAINER_ENDNOTE);
		}
#endif
	}
#if DEBUG
	if(pNext)
	{
	    UT_ASSERT(pNext->getContainerType() != FP_CONTAINER_FRAME);
	}
#endif
	xxx_UT_DEBUGMSG(("_getNext: 2 Returning %p \n",pNext));

	return pNext;
}
#if 0	
bool fb_ColumnBreaker::_verifyAllInColumns(void)
{
  fp_Container * pCon = m_pDocSec->getFirstContainer();
  while(pCon)
    {
      if((pCon->getContainerType() == FP_CONTAINER_LINE) || (pCon->getContainerType() == FP_CONTAINER_TABLE))
	{
	  
	} 
    }
}
#endif
