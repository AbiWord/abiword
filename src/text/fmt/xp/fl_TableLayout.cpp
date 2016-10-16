/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior (msevior@physics.unimelb.edu.au>
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

#include <string.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_string.h"
#include "fp_FrameContainer.h"
#include "ap_Prefs.h"
#include "fl_TableLayout.h"
#include "fl_SectionLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "fp_TableContainer.h"
#include "fp_ContainerObject.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "px_CR_Glob.h"
#include "fv_View.h"
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"
#include "xap_App.h"

static void s_border_properties (const char * border_color, const char * border_style, const char * border_width,
								 const char * color, PP_PropertyMap::Line & line);


static void s_border_properties_cell (const char * border_color, const char * border_style, const char * border_width,
								 const char * color, PP_PropertyMap::Line & line, const PP_PropertyMap::Line lineTable);

static void s_background_properties (const char * pszBgStyle, const char * pszBgColor,
									 const char * pszBackgroundColor,
									 PP_PropertyMap::Background & background);

fl_TableLayout::fl_TableLayout(FL_DocLayout* pLayout, pf_Frag_Strux* sdh, 
			       PT_AttrPropIndex indexAP, fl_ContainerLayout * pMyContainerLayout)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_TABLE,FL_CONTAINER_TABLE,PTX_SectionTable, pMyContainerLayout),
	  m_bNeedsRebuild(false),
	  m_iJustification(FL_TABLE_FULL),
	  m_iLeftOffset(0),
	  m_dLeftOffsetUserUnits(0.0),
	  m_iRightOffset(0),
	  m_dRightOffsetUserUnits(0.0),
	  m_iTopOffset(0),
	  m_dTopOffsetUserUnits(0.0),
	  m_iBottomOffset(0),
	  m_dBottomOffsetUserUnits(0.0),
	  m_bIsHomogeneous(true),
	  m_bSameRowOnTopOfPage(false),
	  m_iRowNumberForTop(0),
	  m_iNumberOfRows(0),
	  m_iNumberOfColumns(0),
	  m_bColumnsPositionedOnPage(false),
	  m_bRowsPositionedOnPage(false),
	  m_bIsDirty(true),
	  m_bInitialLayoutCompleted(false),
	  m_iTableWaitIndex(0),
	  m_iLineThickness(0),
	  m_iColSpacing(0),
	  m_iRowSpacing(0),
	  m_iLeftColPos(0),
	  m_bRecursiveFormat(false),
	  m_iRowHeightType(FL_ROW_HEIGHT_NOT_DEFINED),
	  m_iRowHeight(0),
	  m_iNumNestedTables(0),
	  m_bIsEndTableIn(false),
	  m_iHeightChanged(0),
      m_pNewHeightCell(NULL),
	  m_bDoingDestructor(false),
	  m_iTableWidth(0),
	  m_dTableRelWidth(0.0)
{
	UT_DEBUGMSG(("Created Table Layout %p \n",this));
	UT_ASSERT(pLayout);
	m_vecColProps.clear();
	m_vecRowProps.clear();
	createTableContainer();
}

fl_TableLayout::~fl_TableLayout()
{
	// NB: be careful about the order of these
	xxx_UT_DEBUGMSG(("SEVIOR: !!!!!!!! Deleting tableLayout  %x !! \n",this));
	m_bDoingDestructor = true;
	_purgeLayout();
	fp_TableContainer * pTC = static_cast<fp_TableContainer *>(getFirstContainer());
	DELETEP(pTC);

	setFirstContainer(NULL);
	setLastContainer(NULL);
	UT_VECTOR_PURGEALL(fl_ColProps *, m_vecColProps);
	UT_VECTOR_PURGEALL(fl_RowProps *, m_vecRowProps);
}

/*!
 * Rather than do a complete new relayout, if we have a simple change of
 * height for one cell, just updated the positions of the cells below it.
 */
void fl_TableLayout::setHeightChanged(fp_CellContainer * pCell)
{
	if(pCell != m_pNewHeightCell)
	{
		m_iHeightChanged++;
	}
	m_pNewHeightCell = pCell;
}

/*!
 * Only one Master Table container per Table Layout. Create it here.
 */
void fl_TableLayout::createTableContainer(void)
{
	lookupProperties();
	if(isHidden() >= FP_HIDDEN_FOLDED)
	{
		xxx_UT_DEBUGMSG(("Don't format coz I'm hidden! \n"));
		return;
	}

	fp_TableContainer * pTableContainer = new fp_TableContainer(static_cast<fl_SectionLayout *>(this));
	setFirstContainer(pTableContainer);
	setLastContainer(pTableContainer);
	setTableContainerProperties(pTableContainer);
	fl_ContainerLayout * pCL = myContainingLayout();
	fp_Container * pCon = pCL->getLastContainer();
	UT_sint32 iWidth = 0;
	if(pCon != NULL)
	{
		iWidth = pCon->getWidth();
	}
	if(iWidth == 0)
	{
		iWidth = getDocSectionLayout()->getWidth();
		if(pCon)
		{
			pCon->setWidth(iWidth);
		}
	}
	pTableContainer->setWidth(iWidth);
	//
	// Tell fl_DocSectionLayout this needs to be formated.
	//
	setNeedsReformat(this,0);
//
// The container of the tbale is set in getNewContainer()
//
}


UT_sint32 fl_TableLayout::getNumNestedTables(void) const
{
	return m_iNumNestedTables;
}


void fl_TableLayout::incNumNestedTables(void)
{
	m_iNumNestedTables++;
}

void fl_TableLayout::decNumNestedTables(void)
{
	m_iNumNestedTables--;
}

/*!
 * This method sets all the parameters of the table container from
 * properties of this section.
 */
void fl_TableLayout::setTableContainerProperties(fp_TableContainer * pTab)
{
	pTab->setHomogeneous(m_bIsHomogeneous);
	pTab->setColSpacings(m_iColSpacing);
	pTab->setRowSpacings(m_iRowSpacing);
	pTab->setLineThickness(m_iLineThickness);
	pTab->setRowHeightType(m_iRowHeightType);
	pTab->setRowHeight(m_iRowHeight);
}


/*!
  Create a new Table container and plug it into the linked list of Table 
  containers.
  \param If pPrevTab is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Table container
*/
fp_Container* fl_TableLayout::getNewContainer(fp_Container * /*pPrevTab*/)
{
	UT_ASSERT(getFirstContainer() == NULL);
	createTableContainer();
	fp_TableContainer * pNewTab = static_cast<fp_TableContainer *>(getFirstContainer());
//
// Master Tables do not get linked into the container linked list.
//
	pNewTab->setPrev(NULL);
	pNewTab->setNext(NULL);
//
// Now find the right place to put our new Table container within the arrangement
// of it's own container.
//
	insertTableContainer(pNewTab);
	return static_cast<fp_Container *>(pNewTab);
}

/*!
 * This method inserts the given TableContainer into its correct place in the
 * Vertical container.
 */
void fl_TableLayout::insertTableContainer( fp_TableContainer * pNewTab)
{
	fl_ContainerLayout * pUPCL = myContainingLayout();
	fl_ContainerLayout * pPrevL = static_cast<fl_ContainerLayout *>(getPrev());
	fp_Container * pPrevCon = NULL;
	fp_Container * pUpCon = NULL;
	fp_Line * pPrevLine = NULL;
	if(pPrevL != NULL )
	{
		while(pPrevL && (pPrevL != pUPCL) && ((pPrevL->getContainerType() == FL_CONTAINER_FOOTNOTE)
          || (pPrevL->getContainerType() == FL_CONTAINER_ENDNOTE)||
		  (pPrevL->getContainerType() == FL_CONTAINER_FRAME) ||
		  (pPrevL->isHidden() == FP_HIDDEN_FOLDED) ||
          (pPrevL->getLastContainer() == NULL)))
		{
			pPrevL = pPrevL->getPrev();
		}
		if(pPrevL)
		{
			if(pPrevL->getContainerType() == FL_CONTAINER_TABLE)
			{
//
// Handle if prev container is table that is broken across a page
//
				fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pPrevL);
				fp_TableContainer * pTC = static_cast<fp_TableContainer *>(pTL->getFirstContainer());
				fp_TableContainer * pFirst = pTC->getFirstBrokenTable();
				fp_TableContainer * pLast = pTC->getLastBrokenTable();
				if((pLast != NULL) && pLast != pFirst)
				{
					pPrevCon = static_cast<fp_Container *>(pLast);
					pUpCon = pLast->getContainer();
				}
				else
				{
					pPrevCon = pPrevL->getLastContainer();
					pUpCon = pPrevCon->getContainer();
				}
			}
			else if(pPrevL->getContainerType() == FL_CONTAINER_DOCSECTION)
			{
				pUpCon= static_cast<fl_DocSectionLayout *>(pPrevL)->getFirstContainer();
				pPrevCon = NULL;
			}
			else if(pPrevL->getContainerType() == FL_CONTAINER_SHADOW)
			{
				pUpCon= static_cast<fl_HdrFtrShadow *>(pPrevL)->getFirstContainer();
				pPrevCon = NULL;
			}
			else if(pPrevL->getContainerType() == FL_CONTAINER_HDRFTR)
			{
				pUpCon= static_cast<fl_HdrFtrSectionLayout *>(pPrevL)->getFirstContainer();
				pPrevCon = NULL;
			}
			else if(pPrevL->getContainerType() == FL_CONTAINER_FRAME)
			{
				pUpCon= static_cast<fl_FrameLayout *>(pPrevL)->getFirstContainer();
				pPrevCon = NULL;
			}
			else
			{
				pPrevCon = pPrevL->getLastContainer();
				if(pPrevCon)
				{
					pUpCon = pPrevCon->getContainer();
				}
				else
				{
					pPrevL = NULL;
				}
				if(pPrevCon && pPrevCon->getContainerType() == FP_CONTAINER_LINE)
				{
					pPrevLine = static_cast<fp_Line *>(pPrevCon);
					if(pPrevLine->containsForcedPageBreak())
					{
						pUpCon = pPrevLine->getContainer();
						while(pUpCon && pUpCon->getPage() == pPrevLine->getPage())
						{
							pUpCon = static_cast<fp_Container *>(pUpCon->getNext());
						}
					}
					if(pUpCon == NULL)
					{
							pUpCon = pPrevLine->getContainer();
					}
				}
			}
		}
		else
		{
			pUpCon = pUPCL->getLastContainer();
		}
		if(pUpCon == NULL)
		{
			pUpCon = pUPCL->getNewContainer(NULL);
		}
		UT_ASSERT(pUpCon);
	}
	else if((pUPCL->getContainerType() == FL_CONTAINER_HDRFTR) || (pUPCL->getContainerType() == FL_CONTAINER_SHADOW) || (pUPCL->getContainerType() == FL_CONTAINER_FRAME))
	{
		pUpCon = pUPCL->getFirstContainer();
		if(pUpCon == NULL)
		{
			pUpCon = pUPCL->getNewContainer(NULL);
		}
		UT_ASSERT(pUpCon);
		pPrevL = pUPCL;
		pPrevCon = NULL;
	} 
	else
	{
		pUpCon = pUPCL->getLastContainer();
		if(pUpCon == NULL)
		{
			pUpCon = pUPCL->getNewContainer(NULL);
		}
		UT_ASSERT(pUpCon);
	}
	if(pPrevL == NULL)
	{
		xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!! New Table %x added into %x \n",pNewTab,pUpCon));
		pUpCon->addCon(pNewTab);
		pNewTab->setContainer(pUpCon);
	}
	else
	{
		UT_sint32 i =0;
		if(pPrevCon == NULL)
		{
			pUpCon->insertConAt(pNewTab,0);
			pNewTab->setContainer(pUpCon);
		}
		else
		{
			i = pUpCon->findCon(pPrevCon);
			xxx_UT_DEBUGMSG(("SEVIOR!!!!!!!!!! New Table %x inserted into %x \n",pNewTab,pUpCon));
			if(i >= 0 && (i+1) < pUpCon->countCons())
			{
				pUpCon->insertConAt(pNewTab,i+1);
				pNewTab->setContainer(pUpCon);
			}
			else if( i >=0 &&  (i+ 1) == pUpCon->countCons())
			{
				pUpCon->addCon(pNewTab);
				pNewTab->setContainer(pUpCon);
			}
			else if( i < 0)
			{
				pUpCon->insertConAt(pNewTab,0);
				pNewTab->setContainer(pUpCon);
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
	}
}

/*!
 * Often editting ina table changes the height of just one cell. We can
 * short circuit a complete relayout if this happens and just re-adjust the
 * ypositions of the cells.
 * returns true if the row size could be successfully adjusted
 */
bool fl_TableLayout::doSimpleChange(void)
{
	if(m_pNewHeightCell == NULL)
	{
		return false;
	}
	UT_sint32 iTop = m_pNewHeightCell->getTopAttach();
	UT_sint32 iBot = m_pNewHeightCell->getBottomAttach();
	fl_CellLayout * pCL = static_cast<fl_CellLayout *>(m_pNewHeightCell->getSectionLayout());
	pCL->format();

	if(iBot > iTop +1 )
	{
		return false;
	}
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getFirstContainer());
	if(pTab == NULL)
	{
		return false;
	}
	//
	// Handle case of single Cell table. Also no point doing this for
	// small tables.
	//
	UT_sint32 iNumCells = pTab->getNumRows()*pTab->getNumCols();
	if(iNumCells < 11)
	{
		return false;
	}
	fp_CellContainer * pCell = pTab->getCellAtRowColumn(iTop,0);
	UT_sint32 iMaxHeight = 0;
	fp_Requisition Req;
	UT_sint32 iPrevRight = 0;
	while(pCell)
	{
		if((pCell->getTopAttach() != iTop) || (pCell->getBottomAttach() != iBot) ||
		   (pCell->getLeftAttach() != iPrevRight))
		{
			break;
		}
		iPrevRight = pCell->getRightAttach();
		pCell->sizeRequest(&Req);
		if(Req.height > iMaxHeight)
		{
			iMaxHeight = Req.height;
		}
		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}
	if((pCell && (pCell->getTopAttach() != iBot)) || (iPrevRight != pTab->getNumCols()))
	{
		UT_DEBUGMSG(("fl_TableLayout::doSimpleChange aborted\n"));
		return false;
	}
	UT_DEBUGMSG(("fl_TableLayout::doSimpleChange executed\n"));
	fp_TableRowColumn * pRow = pTab->getNthRow(iTop);
	UT_sint32 iAlloc = pRow->allocation;	
	iMaxHeight = pTab->getRowHeight(iTop,iMaxHeight);
	if(iAlloc == iMaxHeight)
	{
		return true;
	}
	pTab->deleteBrokenTables(true,true);
	setNeedsRedraw();
	markAllRunsDirty();
	UT_sint32 diff = iMaxHeight - iAlloc;
	pRow->allocation += diff;
	UT_sint32 kk;
	for (kk = iTop + 1; kk < pTab->getNumRows(); kk++)
	{
		pRow = pTab->getNthRow(kk);
		pRow->position += diff;
	}
	while(pCell)
	{
		pCell->setY(pCell->getY()+diff);
		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}
	pCell =  pTab->getCellAtRowColumn(iTop,0);
	while(pCell)
	{
		pCell->setLineMarkers();
		pCell = static_cast<fp_CellContainer *>(pCell->getNext());
	}
	m_pNewHeightCell->setMaxHeight(iMaxHeight);
	pTab->setHeight(pTab->getHeight() + diff);
	return true;
}


bool fl_TableLayout::needsReformat(void) const
{
	if(fl_SectionLayout::needsReformat())
	{
		return true;
	}
	fl_CellLayout * pCell = static_cast<fl_CellLayout *>(getFirstLayout());
	if(pCell == NULL)
	{
		return true;
	}
	UT_return_val_if_fail(pCell->getContainerType() == FL_CONTAINER_CELL,true);
	//
	// Cell Not set
	//
	if(pCell->needsReformat())
	{
		return true;
	}
	return false;
}

void fl_TableLayout::setDirty(void)
{
	xxx_UT_DEBUGMSG(("Table Dirty set true \n"));
	m_bIsDirty = true;
}

void fl_TableLayout::format(void)
{
	if(m_bRecursiveFormat)
	{
		return;
	}
	if((isHidden() > FP_VISIBLE) || !isTableReadyForLayout())
	{
		return;
	}
	m_bRecursiveFormat = true;
	bool bRebuild = false;
	
	fl_ContainerLayout*	pCell = NULL;
	pCell = getFirstLayout();
	//
	// Get the old height of the table
	//
	UT_sint32 iOldHeight = 0;
	if(getFirstContainer())
	{
		iOldHeight = getFirstContainer()->getHeight();
	}
//
// OK on with the formatting
//
	xxx_UT_DEBUGMSG(("!!!!!!!!!!!!TableLayout format Table !!!!!!!!!\n"));
	if(getFirstContainer() == NULL)
	{
		m_iHeightChanged = 0;
		m_pNewHeightCell = NULL;
		getNewContainer(NULL);
		bRebuild = true;
	}
	else if( getFirstContainer()->countCons() == 0)
	{
		m_iHeightChanged = 10;
		m_pNewHeightCell = NULL;
		bRebuild = true;
		m_bIsDirty = true;
	}
	else if(pCell && !static_cast<fl_CellLayout *>(pCell)->isLayedOut())
	{
		m_iHeightChanged = 10;
		m_pNewHeightCell = NULL;
		m_bIsDirty = true;
	}
	if(isDirty())
	{
		xxx_UT_DEBUGMSG(("TableLayout Mark All runs dirty \n"));
		markAllRunsDirty();
	}
	bool bSim = false;
	fl_ContainerLayout * myL = myContainingLayout();
	if((m_iHeightChanged == 1)  && !getDocument()->isDontImmediateLayout())
	{
		//
		// Simple height change due to editting. Short circuit full blown 
		// layout
		//
		bSim = doSimpleChange();
		if(bSim)
		{
			m_bIsDirty = false;
		}
		m_iHeightChanged = 0;
		m_pNewHeightCell = NULL;
	}
	if((!bSim && isDirty()) || bRebuild)
	{
		while (pCell)
		{
			pCell->format();
			if(bRebuild)
			{
				attachCell(pCell);
			}
			pCell = pCell->getNext();
		}
		if((m_iHeightChanged == 1)  && !getDocument()->isDontImmediateLayout())
		{
			//
			// Simple height change due to editting. Short circuit full blown 
			// layout
			//
			bSim = doSimpleChange();
			if(bSim)
			{
				m_bIsDirty = false;
			}
		}
		xxx_UT_DEBUGMSG(("fl_TableLayout: Finished Formatting %x isDirty %d \n",this,isDirty()));

		if((!isInitialLayoutCompleted() || ((m_iHeightChanged !=0) && isDirty())) && 
		   !getDocument()->isDontImmediateLayout())
	    {
			m_bIsDirty = false;
			xxx_UT_DEBUGMSG(("SEVIOR: Layout pass 1 \n"));
			static_cast<fp_TableContainer *>(getFirstContainer())->layout();
			setNeedsRedraw();
			markAllRunsDirty();
			if (!isInitialLayoutCompleted())
			{
				m_bInitialLayoutCompleted = true;
			}
		}
	}
//
// The layout process can trigger a width change on a cell which requires
// a second layout pass
//
	if(isDirty() && !getDocument()->isDontImmediateLayout())
	{
		static_cast<fp_TableContainer *>(getFirstContainer())->layout();
		xxx_UT_DEBUGMSG(("SEVIOR: Layout pass 2 \n"));
		setNeedsRedraw();
   		markAllRunsDirty();
		m_bIsDirty = false;
		if (!isInitialLayoutCompleted())
		{
			m_bInitialLayoutCompleted = true;
		}
	}
	UT_sint32 iNewHeight = -10;
	bool isBroken = false;
	if(getFirstContainer())
	{
		iNewHeight = getFirstContainer()->getHeight();
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getFirstContainer());
		if(pTab->getFirstBrokenTable() != NULL)
		{
			isBroken = true;
		}
	}
	if((iNewHeight != iOldHeight)  || !isBroken)
	{
		//
		// Set a section break.
		//

//
// All this code is to find the right page to do a sectionBreak from.
//
		fl_ContainerLayout * pPrevCL = getPrev();
		fp_Page * pPrevP = NULL;
		if(pPrevCL)
		{
			fp_Container * pPrevCon = pPrevCL->getFirstContainer();
			if(pPrevCon)
			{
				pPrevP = pPrevCon->getPage();
			}
		}
		if(myL && (myL->getContainerType() != FL_CONTAINER_SHADOW) &&
		   (myL->getContainerType() != FL_CONTAINER_HDRFTR))
		{
			getDocSectionLayout()->setNeedsSectionBreak(true,pPrevP);
		}
	}
	//	if(myL && (myL->getContainerType() == FL_CONTAINER_SHADOW) && !getDocument()->isDontImmediateLayout() )
   	if(myL && (myL->getContainerType() == FL_CONTAINER_SHADOW) )
	{
		m_bNeedsReformat = false;
		myL->format();
		fp_ShadowContainer * pSC = static_cast<fp_ShadowContainer *>(myL->getFirstContainer());
		if(pSC)
		{
			pSC->layout(true);
		}
	}
	if(myL && (myL->getContainerType() == FL_CONTAINER_CELL))
	{
		if(iNewHeight != iOldHeight)
		{
			fl_CellLayout * pCL = static_cast<fl_CellLayout *>(myL);
			pCL->setNeedsReformat(pCL);
			fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pCL->myContainingLayout());
			pTL->setDirty();
			fp_Container * pTCon = pTL->getFirstContainer();
			if(pTCon)
			{
				fp_Page * pTP = pTCon->getPage();
				getDocSectionLayout()->setNeedsSectionBreak(true,pTP);
			}
		}
	}
	m_bRecursiveFormat = false;
	if(!getDocument()->isDontImmediateLayout())
	{
		m_iHeightChanged = 0;
		m_pNewHeightCell = NULL;
		m_bIsDirty = false;
		m_bNeedsReformat = false;
		m_vecFormatLayout.clear();
		xxx_UT_DEBUGMSG(("TableLayout format cleared %x \n"));
	}
}

void fl_TableLayout::markAllRunsDirty(void)
{
	//
	// No need to do this during import.
	//
	if(m_pLayout->isLayoutFilling())
	{
		return;
	}
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->markAllRunsDirty();
		pCL = pCL->getNext();
	}
}

void fl_TableLayout::updateLayout(bool /*bDoAll*/)
{			
	xxx_UT_DEBUGMSG(("updateTableLayout  in TableLayout\n"));
	if(getDocument()->isDontImmediateLayout())
	{
		UT_DEBUGMSG(("updateTableLayout not allowed updates  \n"));
		return;
	}
	xxx_UT_DEBUGMSG(("updateTableLayout Doing updates  \n"));
	fl_ContainerLayout*	pBL = getFirstLayout();
	bool bNeedsFormat = false;
	m_vecFormatLayout.clear();
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			pBL->updateLayout(false);
			bNeedsFormat = true;
		}

		pBL = pBL->getNext();
	}
	if(bNeedsFormat || isDirty())
	{
		format();
	}
}

bool fl_TableLayout::isTableReadyForLayout(void) const
{
	return (isEndTableIn() && (getTableWaitIndex() == 0));
}

void fl_TableLayout::redrawUpdate(void)
{
	if(getDocument()->isDontImmediateLayout())
	{
		xxx_UT_DEBUGMSG(("redrawupdate table don't immediately update! \n"));
		return;
	}
	if(!needsRedraw())
	{
		xxx_UT_DEBUGMSG(("redrawupdate table no redraw needed! \n"));
		return;
	}
 	xxx_UT_DEBUGMSG(("Doing Redraw update in Table layout %x \n",this));
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		if (pBL->needsRedraw())
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Doing redraw in table \n"));
			pBL->redrawUpdate();
		}
		pBL = pBL->getNext();
	}
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getFirstContainer());
	if(pTab && pTab->doRedrawLines())
	{
		pTab->drawLines();
	}
	m_bNeedsRedraw = false;
 	xxx_UT_DEBUGMSG(("Finished Redraw update in Table layout %x \n",this));
}

bool fl_TableLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);
	xxx_UT_DEBUGMSG(("changeStrux: getNext() %x getPrev() %x \n",getNext(),getPrev()));
	if(getPrev())
	{
		UT_ASSERT(getPrev()->getNext() == this);
	}
	if(getNext())
	{
		UT_ASSERT(getNext()->getPrev() == this);
	}
	if(pcrxc->getStruxType() == PTX_SectionTable)
	{
		setAttrPropIndex(pcrxc->getIndexAP());
	}
	updateTable();
	xxx_UT_DEBUGMSG(("SEVIOR: getNext() %x getPrev() %x \n",getNext(),getPrev()));
	if(getPrev())
	{
		UT_ASSERT(getPrev()->getNext() == this);
	}
	if(getNext())
	{
		UT_ASSERT(getNext()->getPrev() == this);
	}
	//
	// Look to see if we're in a HfrFtr section
	//
	fl_ContainerLayout * pMyCL = myContainingLayout();
	if(pMyCL && pMyCL->getContainerType() == FL_CONTAINER_HDRFTR)
	{
		fl_HdrFtrSectionLayout * pHFSL = static_cast<fl_HdrFtrSectionLayout *>(pMyCL);
		pHFSL->bl_doclistener_changeStrux(this,pcrxc);
	}
	return true;
}

fl_SectionLayout * fl_TableLayout::getSectionLayout(void) const
{
	fl_ContainerLayout * pDSL = myContainingLayout();
	while(pDSL)
	{
		if(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			return static_cast<fl_SectionLayout *>(pDSL);
		}
		pDSL = pDSL->myContainingLayout();
	}
	return NULL;
}
void fl_TableLayout::updateTable(void)
{

	const PP_AttrProp* pAP = NULL;
	// This is a real NO-NO: must *always* call getAP()
	// bool bres = m_pDoc->getAttrProp(m_apIndex, &pAP);
	getAP(pAP);

	lookupProperties();

	// Do not collapse if the table should not be remade right away
	if (!isTableReadyForLayout())
	{
		return;
	}
 
	// clear all table content
    collapse();

	/*
	  TODO to more closely mirror the architecture we're using for BlockLayout, this code
	  should probably just set a flag, indicating the need to reformat this section.  Then,
	  when it's time to update everything, we'll actually do the format.
	*/

	FV_View * pView = m_pLayout->getView();
	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(false);
	}

	format();
	markAllRunsDirty();

	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(true);
	}

	return;
}


bool fl_TableLayout::bl_doclistener_insertBlock(fl_ContainerLayout* /*pLBlock*/,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{

	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Block);

	fl_ContainerLayout * pNewCL = NULL;
	fl_ContainerLayout * pMyCL = myContainingLayout();
	pNewCL = pMyCL->insert(sdh,this,pcrx->getIndexAP(), FL_CONTAINER_BLOCK);
	fl_BlockLayout * pBlock = static_cast<fl_BlockLayout *>(pNewCL);
//
// Set the sectionlayout of this table to that of the block since it is that scope
//
	pBlock->setSectionLayout(static_cast<fl_SectionLayout *>(myContainingLayout()));
	pNewCL->setContainingLayout(myContainingLayout());

		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).
		
	fl_ContainerLayout* sfhNew = pNewCL;
	pfnBindHandles(sdh,lid,sfhNew);
//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);

	return true;
}


bool fl_TableLayout::bl_doclistener_insertTable( const PX_ChangeRecord_Strux * pcrx,
											   SectionType iType,
											   pf_Frag_Strux* sdh,
											   PL_ListenerId lid,
											   void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	   PL_ListenerId lid,
																	   fl_ContainerLayout* sfhNew))
{
	UT_UNUSED(iType);
	UT_ASSERT(iType == FL_SECTION_TABLE);
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_SectionTable);
	PT_DocPosition pos1;
//
// This is to clean the fragments
//
	m_pDoc->getBounds(true,pos1);

	fl_SectionLayout* pSL = NULL;
	fl_ContainerLayout * pMyCL = static_cast<fl_ContainerLayout *>(myContainingLayout());
	if(pMyCL == NULL)
	{
		pMyCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	}
	pSL = static_cast<fl_SectionLayout *>(pMyCL->insert(sdh,this,pcrx->getIndexAP(), FL_CONTAINER_TABLE));

		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).

	fl_ContainerLayout* sfhNew = pSL;
	pfnBindHandles(sdh,lid,sfhNew);

//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() + fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
//
// OK that's it!
//
	return true;
}


bool fl_TableLayout::bl_doclistener_insertCell(fl_ContainerLayout* pCell,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{
	fl_ContainerLayout * pNewCL = NULL;
	pNewCL = insert(sdh,pCell,pcrx->getIndexAP(), FL_CONTAINER_CELL);

	
		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).
	fl_CellLayout * pCL = static_cast<fl_CellLayout *>(pNewCL);
	attachCell(pCL);
		
	if(pfnBindHandles)
	{
		fl_ContainerLayout* sfhNew = pNewCL;
		pfnBindHandles(sdh,lid,sfhNew);
	}

//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
	//
	// Look to see if we're in a HfrFtr section
	//
	fl_ContainerLayout * pMyCL = myContainingLayout();
	if(pMyCL && pMyCL->getContainerType() == FL_CONTAINER_HDRFTR)
	{
		fl_HdrFtrSectionLayout * pHFSL = static_cast<fl_HdrFtrSectionLayout *>(pMyCL);
		pHFSL->bl_doclistener_insertCell(pCell,pcrx,sdh,lid,this);
	}
	return true;
}



bool fl_TableLayout::bl_doclistener_insertEndTable(fl_ContainerLayout*,
												   const PX_ChangeRecord_Strux * pcrx,
												   pf_Frag_Strux* sdh,
												   PL_ListenerId lid,
												   void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{
	// The endTable strux actually has a format handle to to the this table layout.
	// so we bind to this layout. We also set a pointer to keep track of the endTable strux.

	if(pfnBindHandles)
	{	
		fl_ContainerLayout* sfhNew = this;
		pfnBindHandles(sdh,lid,sfhNew);
	}

	setEndStruxDocHandle(sdh);

//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() +  fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
	setNeedsReformat(this,0);
	m_bIsEndTableIn = true;
	//
	// Look to see if we're in a HfrFtr section
	//
	fl_ContainerLayout * pMyCL = myContainingLayout();
	if(pMyCL && pMyCL->getContainerType() == FL_CONTAINER_HDRFTR)
	{
		fl_HdrFtrSectionLayout * pHFSL = static_cast<fl_HdrFtrSectionLayout *>(pMyCL);
		pHFSL->bl_doclistener_insertEndTable(this,pcrx,sdh,lid);
	}
	return true;
}

bool fl_TableLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		pBL->recalculateFields(iUpdateCount);
		pBL = pBL->getNext();
	}
	return true;
}


void fl_TableLayout::setMaxExtraMargin(double margin)
{
	if (margin < 0)
	{
		m_dMaxExtraMargin = 0.;
	}
	else if (margin > 1)
	{
		m_dMaxExtraMargin = 1.;
	}
	else
	{
		m_dMaxExtraMargin = margin;
	}
}

/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_TableLayout::_lookupProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail( pSectionAP );
	
	/*
	  TODO shouldn't we be using PP_evalProperty like
	  the blockLayout does?

	  Yes, since PP_evalProperty does a fallback to the
	  last-chance defaults, whereas the code below is
	  hard-coding its own defaults.  Bad idea.
	*/

	const char* pszHomogeneous = NULL;
	pSectionAP->getProperty("homogeneous", (const gchar *&)pszHomogeneous);
	if (pszHomogeneous && pszHomogeneous[0])
	{
		if(atoi(pszHomogeneous) == 1)
		{
			m_bIsHomogeneous = true;
		}
	}
	else
	{
		m_bIsHomogeneous = false;
	}
	const char* pszTableWidth = NULL;
	const char* pszRelTableWidth = NULL;
	pSectionAP->getProperty("table-width", (const gchar *&)pszTableWidth);
	pSectionAP->getProperty("table-rel-width", (const gchar *&)pszRelTableWidth);
	if(pszTableWidth && pszTableWidth[0])
	{
		m_iTableWidth = UT_convertToLogicalUnits(pszTableWidth);
	}
	else
	{
		m_iTableWidth = getDocSectionLayout()->getActualColumnWidth();
	}
	if(pszRelTableWidth && pszRelTableWidth[0])
	{
		m_iTableWidth = getDocSectionLayout()->getActualColumnWidth();
		//
		// Assume the relative table width is in percent
		//
		double rel = UT_convertDimensionless(pszRelTableWidth);
		m_dTableRelWidth = static_cast<double>(m_iTableWidth)*rel/100.;
		m_iTableWidth = m_dTableRelWidth;
	}

	// This property defines the maximum extra margin that may be left at
	// the bottom of the page when breaking a table along cell boundaries.
	// The margin is defined as a fraction of the maximum column height.
	const char* pszMaxExtraMargin = NULL;
	pSectionAP->getProperty("table-max-extra-margin", (const gchar *&)pszMaxExtraMargin);
	if(pszMaxExtraMargin && pszMaxExtraMargin[0])
	{
		m_dMaxExtraMargin = atof(pszMaxExtraMargin);
	}
	else
	{
		m_dMaxExtraMargin = 0.05;
	}



	const char* pszLeftOffset = NULL;
	const char* pszTopOffset = NULL;
	const char* pszRightOffset = NULL;
	const char* pszBottomOffset = NULL;
	pSectionAP->getProperty("table-margin-left", (const gchar *&)pszLeftOffset);
	pSectionAP->getProperty("table-margin-top", (const gchar *&)pszTopOffset);
	pSectionAP->getProperty("table-margin-right", (const gchar *&)pszRightOffset);
	pSectionAP->getProperty("table-margin-bottom", (const gchar *&)pszBottomOffset);

	const gchar * szRulerUnits;
	UT_Dimension dim;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		dim = UT_determineDimension(szRulerUnits);
	else
		dim = DIM_IN;

	UT_String defaultOffset;
	switch(dim)
	{
	case DIM_IN:
		defaultOffset = "0.0in"; //was 0.02in
		break;

	case DIM_CM:
		defaultOffset = "0.0cm";
		break;

	case DIM_PI:
		defaultOffset = "0.0pi";
		break;

	case DIM_PT:
		defaultOffset= "0.0pt";
		break;

	case DIM_MM:
		defaultOffset= "0.0mm"; //was 0.508
		break;

		// TODO: PX, and PERCENT
		// let them fall through to the default now
		// and we don't use them anyway
#if 0
	case DIM_PX:
	case DIM_PERCENT:
#endif
	case DIM_none:
	default:
		defaultOffset = "0.0in";	// TODO: what to do with this. was 0.01in
		break;

	}
	defaultOffset = "0.01in";	// TODO: what to do with this. was 0.01in
	if(pszLeftOffset && pszLeftOffset[0])
	{
		m_iLeftOffset = UT_convertToLogicalUnits(pszLeftOffset);
		m_dLeftOffsetUserUnits = UT_convertDimensionless(pszLeftOffset);
	}
	else
	{
		m_iLeftOffset = UT_convertToLogicalUnits(defaultOffset.c_str());
		m_dLeftOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszTopOffset && pszTopOffset[0])
	{
		m_iTopOffset = UT_convertToLogicalUnits(pszTopOffset);
		m_dTopOffsetUserUnits = UT_convertDimensionless(pszTopOffset);
	}
	else
	{
		m_iTopOffset = UT_convertToLogicalUnits(defaultOffset.c_str());
		m_dTopOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszRightOffset && pszRightOffset[0])
	{
		m_iRightOffset = UT_convertToLogicalUnits(pszRightOffset);
		m_dRightOffsetUserUnits = UT_convertDimensionless(pszRightOffset);
	}
	else
	{
		m_iRightOffset = UT_convertToLogicalUnits(defaultOffset.c_str());
		m_dRightOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszBottomOffset && pszBottomOffset[0])
	{
		m_iBottomOffset = UT_convertToLogicalUnits(pszBottomOffset);
		m_dBottomOffsetUserUnits = UT_convertDimensionless(pszBottomOffset);
	}
	else
	{
		m_iBottomOffset = UT_convertToLogicalUnits(defaultOffset.c_str());
		m_dBottomOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}
	const char * pszLineThick = NULL;
	pSectionAP->getProperty("table-line-thickness", (const gchar *&)pszLineThick);
	if(pszLineThick && *pszLineThick)
	{
		m_iLineThickness = UT_convertToLogicalUnits(pszLineThick);
	}
	else
	{
		m_iLineThickness = UT_convertToLogicalUnits("0.8pt");
		if(m_iLineThickness < 1)
		{
			m_iLineThickness = 1;
		}
	}
	xxx_UT_DEBUGMSG(("SEVIOR: TableLayout::_lookup lineThickness %d \n",m_iLineThickness));
	const char * pszTableColSpacing = NULL;
	const char * pszTableRowSpacing = NULL;
	pSectionAP->getProperty("table-col-spacing", (const gchar *&)pszTableColSpacing);
	pSectionAP->getProperty("table-row-spacing", (const gchar *&)pszTableRowSpacing);
	if(pszTableColSpacing && *pszTableColSpacing)
	{
//
// Note to TF from MS. It was too hard to propagate 3 different sets of layout units
// into the Table Layout code so what I did instead was the worst of all worlds.
//
// Structures are layed horizontally in LAYOUT units and vertically in screen units.
// That is what all those SCALE_TO_SCREEN macros are about. We should think through
// how to transition to pango layout.
//
// 02/11/03: You can't get screen units anymore! HAHAHHAHAA - PL
//
// Anyway column spacing being horizontal is layout units.
//
		m_iColSpacing = UT_convertToLogicalUnits(pszTableColSpacing);
	}
	else
	{
		m_iColSpacing = UT_convertToLogicalUnits("0.03in");
	}
	if(pszTableRowSpacing && *pszTableRowSpacing)
	{
//
// Row spacing being vertical is screen units
//
		m_iRowSpacing = UT_convertToLogicalUnits(pszTableRowSpacing);
	}
	else
	{
		m_iRowSpacing = UT_convertToLogicalUnits("0.01in");
	}
//
// Positioned columns controls
//
	const char * pszLeftColPos = NULL;
	const char * pszColumnProps = NULL;
	const char * pszRelColumnProps = NULL;
	pSectionAP->getProperty("table-column-leftpos", (const gchar *&)pszLeftColPos);
	pSectionAP->getProperty("table-column-props", (const gchar *&)pszColumnProps);
	pSectionAP->getProperty("table-rel-column-props", (const gchar *&)pszRelColumnProps);

	if(pszLeftColPos && *pszLeftColPos)
	{
//
// Anyway column positioning being horizontal is layout units.
//
		m_iLeftColPos = UT_convertToLogicalUnits(pszLeftColPos);
		xxx_UT_DEBUGMSG(("Left colpos is %s \n",pszLeftColPos));

		FV_View * pView = m_pLayout->getView();
		GR_Graphics * pG = getDocLayout()->getGraphics();
		UT_return_if_fail( pView && pG );
		
		if(((pView->getViewMode() == VIEW_NORMAL) || (pView->getViewMode() == VIEW_WEB)) && 
		   m_iLeftColPos < 0 &&
		   !pG->queryProperties(GR_Graphics::DGP_PAPER))
		{
			m_iLeftColPos = 0;
		}
	}
	else
	{
		m_iLeftColPos =  0;
	}


	if(pszColumnProps && *pszColumnProps)
	{
/*
   These will be properties applied to all columns. To start with, just the 
    widths of each column are specifed. These are translated to layout units.
 
   The format of the string of properties is:

   table-column-props:1.2in/3.0in/1.3in/;
   table-rel-column-props are ignored here

   So we read back in pszColumnProps
   1.2in/3.0in/1.3in/

   The "/" characters will be used to delineate different column entries.
   As new properties for each column are defined these will be delineated with "_"
   characters. But we'll cross that bridge later.
*/
		xxx_UT_DEBUGMSG(("Processing Column width string %s \n",pszColumnProps));
		UT_VECTOR_PURGEALL(fl_ColProps *,m_vecColProps);
		m_vecColProps.clear();
		UT_String sProps = pszColumnProps;
		UT_sint32 sizes = sProps.size();
		UT_sint32 i =0;
		UT_sint32 j =0;
		while(i < sizes)
		{
			for (j=i; (j<sizes) && (sProps[j] != '/') ; j++) {}
			if((j+1)>i && sProps[j] == '/')
			{
				UT_String sSub = sProps.substr(i,(j-i));
				i = j + 1;
				fl_ColProps * pColP = new fl_ColProps;
				pColP->m_iColWidth = UT_convertToLogicalUnits(sSub.c_str());
				pColP->m_dColRelWidth = 0.0;
				m_vecColProps.addItem(pColP);
				xxx_UT_DEBUGMSG(("SEVIOR: width char %s width layout %d \n",sSub.c_str(),pColP->m_iColWidth));
			}
			else
			{
				// something not right here
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );

				// we need to quit the main loop
				break;
			}
		}
 	}
	else
	{
		UT_VECTOR_PURGEALL(fl_ColProps *,m_vecColProps);
		m_vecColProps.clear();
	}


	if(pszRelColumnProps && *pszRelColumnProps)
	{
/*
   These will be properties applied to all columns. To start with, just the 
    widths of each column are specifed. These are translated to layout units.
 
   The format of the string of properties is:

   table-column-props are ignored here
   table-rel-column-props 

   So we read back in pszRelColumnProps
   23* /25* /28* / (except that theere is no space between the * and the /!)

   The "/" characters will be used to delineate different column entries.

   The formula used to set the actual column widths is
   width_i = rel_i*m_axiTableWidth/tot_rel

   Where tot_rel is the sum of all the relative column widths.

*/
		xxx_UT_DEBUGMSG(("Processing Column width string %s \n",pszRelColumnProps));
		UT_VECTOR_PURGEALL(fl_ColProps *,m_vecColProps);
		m_vecColProps.clear();
		UT_String sProps = pszRelColumnProps;
		UT_sint32 sizes = sProps.size();
		UT_sint32 i =0;
		UT_sint32 j =0;
		double tot = 0.0;
		fl_ColProps * pColP = NULL;
		while(i < sizes)
		{
			for (j=i; (j<sizes) && (sProps[j] != '/') ; j++) {}
			if((j+1)>i && sProps[j] == '/')
			{
				UT_String sSub = sProps.substr(i,(j-i));
				i = j + 1;
				pColP = new fl_ColProps;
				pColP->m_iColWidth = 0;
				pColP->m_dColRelWidth = UT_convertDimensionless(sSub.c_str());
				m_vecColProps.addItem(pColP);
				xxx_UT_DEBUGMSG(("SEVIOR: width char %s width layout %f \n",sSub.c_str(),pColP->m_dColRelWidth));
				tot += pColP->m_dColRelWidth;
			}
			else
			{
				// something not right here
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );

				// we need to quit the main loop
				break;
			}
		}
		//
		// Now set the actual column width from the relative width
		//
		for(i=0; i<	m_vecColProps.getItemCount();i++)
		{
			pColP = m_vecColProps.getNthItem(i);
			pColP->m_iColWidth = static_cast<double>(m_iTableWidth)*pColP->m_dColRelWidth/tot;
		}
 	}

//
// global row height type
//
	const char * pszRowHeightType = NULL;
	const char * pszRowHeight = NULL;
	pSectionAP->getProperty("table-row-height-type",(const gchar *&) pszRowHeightType);
	if(pszRowHeightType && *pszRowHeightType)
	{
		if(strcmp(pszRowHeightType,"undefined") == 0)
		{
			m_iRowHeightType = 	FL_ROW_HEIGHT_NOT_DEFINED;
		}
		else if(strcmp(pszRowHeightType,"auto") == 0)
		{
			m_iRowHeightType = 	FL_ROW_HEIGHT_AUTO;
		}
		else if(strcmp(pszRowHeightType,"at-least") == 0)
		{
			m_iRowHeightType = 	FL_ROW_HEIGHT_AT_LEAST;
		}
		else if(strcmp(pszRowHeightType,"exactly") == 0)
		{
			m_iRowHeightType = 	FL_ROW_HEIGHT_EXACTLY;
		}
		else
		{
			m_iRowHeightType = 	FL_ROW_HEIGHT_NOT_DEFINED;
		}
	}
	else
	{
		m_iRowHeightType = 	FL_ROW_HEIGHT_NOT_DEFINED;
	}
	pSectionAP->getProperty("table-row-height",(const gchar *&) pszRowHeight);
	if(pszRowHeight && *pszRowHeight)
	{
		m_iRowHeight = atoi(pszRowHeight);
	}
	else
	{
		m_iRowHeight = 0;
	}
//
// Positioned row controls
//
	const char * pszRowHeights = NULL;
	pSectionAP->getProperty("table-row-heights", (const gchar *&)pszRowHeights);
	if(pszRowHeights && *pszRowHeights)
	{
/*
   These will be heights applied to all rows.
 
   The format of the string of Heights is:

   table-row-heights:1.2in/3.0in/1.3in/;

   So we read back in pszRowHeights
   1.2in/3.0in/1.3in/

   The "/" characters will be used to delineate different row entries.
   if there are not enough heights defined for the entire table then the 
   rows after the last defined height do not a fixed height.
*/
		xxx_UT_DEBUGMSG(("Processing Row Height string %s \n",pszRowHeights));
		UT_String sProps = pszRowHeights;
		UT_sint32 sizes = sProps.size();
		UT_sint32 i =0;
		UT_sint32 j =0;
		UT_sint32 iProp = 0;
		fl_RowProps * pRowP = NULL;
		while(i < sizes)
		{
			for (j=i; (j<sizes) && (sProps[j] != '/') ; j++) {}
			if((j+1)>i && sProps[j] == '/')
			{
				UT_String sSub = sProps.substr(i,(j-i));
				i = j + 1;
				bool bNew = false;
				if(iProp >= m_vecRowProps.getItemCount())
				{
					bNew = true;
					pRowP = new fl_RowProps;
				}
				else
				{
					pRowP = m_vecRowProps.getNthItem(iProp);
				}
				pRowP->m_iRowHeight = UT_convertToLogicalUnits(sSub.c_str());
				if(bNew)
				{
					m_vecRowProps.addItem(pRowP);
				}
				xxx_UT_DEBUGMSG(("SEVIOR: width char %s width layout %d \n",sSub.c_str(),pRowP->m_iRowHeight));
				iProp++;
			}
			else
			{
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				break;
			}
			
		}
 	}
	else
	{
		UT_sint32 i = 0;
		for(i=0; i< m_vecRowProps.getItemCount(); i++)
		{
			fl_RowProps * pRowP = m_vecRowProps.getNthItem(i);
			pRowP->m_iRowHeight = 0;
		}
	}

	/* table-border properties:
	 */
	const gchar * pszColor = NULL;
	pSectionAP->getProperty ("color", pszColor);
	if (pszColor)
		UT_parseColor (pszColor, m_colorDefault);
	else
		m_colorDefault = UT_RGBColor(0,0,0);

	const gchar * pszBorderColor = NULL;
	const gchar * pszBorderStyle = NULL;
	const gchar * pszBorderWidth = NULL;

	pSectionAP->getProperty ("bot-color",       pszBorderColor);
	pSectionAP->getProperty ("bot-style",       pszBorderStyle);
	pSectionAP->getProperty ("bot-thickness",   pszBorderWidth);


	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineBottom);

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("left-color",      pszBorderColor);
	pSectionAP->getProperty ("left-style",      pszBorderStyle);
	pSectionAP->getProperty ("left-thickness",  pszBorderWidth);

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineLeft);

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("right-color",     pszBorderColor);
	pSectionAP->getProperty ("right-style",     pszBorderStyle);
	pSectionAP->getProperty ("right-thickness", pszBorderWidth);

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineRight);

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("top-color",       pszBorderColor);
	pSectionAP->getProperty ("top-style",       pszBorderStyle);
	pSectionAP->getProperty ("top-thickness",   pszBorderWidth);

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineTop);

	/* table fill
	 */
	m_background.reset ();

	const gchar * pszBgStyle = NULL;
	const gchar * pszBgColor = NULL;
	const gchar * pszBackgroundColor = NULL;

	pSectionAP->getProperty ("bg-style",         pszBgStyle);
	pSectionAP->getProperty ("bgcolor",          pszBgColor);
	pSectionAP->getProperty ("background-color", pszBackgroundColor);
	
	s_background_properties (pszBgStyle, pszBgColor, pszBackgroundColor, m_background);

	// table-wait-index is set by FV_View functions to a value different than zero to prevent 
	// table initialization before the changes are completed.
	const char * pszWaitIndex = NULL;
	pSectionAP->getProperty("table-wait-index", (const gchar *&)pszWaitIndex);
	if(pszWaitIndex && *pszWaitIndex)
	{
		m_iTableWaitIndex = atoi(pszWaitIndex);		
	}
	else
	{
		m_iTableWaitIndex = 0;
	}
}

void fl_TableLayout::_lookupMarginProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail( pSectionAP );
#if 0 // I think these are relative to the position of the table, so we do not need to
	  // bother with them
	const char* pszLeftOffset = NULL;
	const char* pszTopOffset = NULL;
	const char* pszRightOffset = NULL;
	const char* pszBottomOffset = NULL;
	pSectionAP->getProperty("table-margin-left", (const gchar *&)pszLeftOffset);
	pSectionAP->getProperty("table-margin-top", (const gchar *&)pszTopOffset);
	pSectionAP->getProperty("table-margin-right", (const gchar *&)pszRightOffset);
	pSectionAP->getProperty("table-margin-bottom", (const gchar *&)pszBottomOffset);

	UT_String defaultOffset("0.01in");	// TODO: what to do with this. was 0.01in
	if(pszLeftOffset && pszLeftOffset[0])
	{
		m_iLeftOffset = UT_convertToLogicalUnits(pszLeftOffset);
		m_dLeftOffsetUserUnits = UT_convertDimensionless(pszLeftOffset);
	}
	else
	{
		m_iLeftOffset = UT_convertToLogicalUnits(defaultOffset.c_str());
		m_dLeftOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszTopOffset && pszTopOffset[0])
	{
		m_iTopOffset = UT_convertToLogicalUnits(pszTopOffset);
		m_dTopOffsetUserUnits = UT_convertDimensionless(pszTopOffset);
	}
	else
	{
		m_iTopOffset = UT_convertToLogicalUnits(defaultOffset.c_str());
		m_dTopOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszRightOffset && pszRightOffset[0])
	{
		m_iRightOffset = UT_convertToLogicalUnits(pszRightOffset);
		m_dRightOffsetUserUnits = UT_convertDimensionless(pszRightOffset);
	}
	else
	{
		m_iRightOffset = UT_convertToLogicalUnits(defaultOffset.c_str());
		m_dRightOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}

	if(pszBottomOffset && pszBottomOffset[0])
	{
		m_iBottomOffset = UT_convertToLogicalUnits(pszBottomOffset);
		m_dBottomOffsetUserUnits = UT_convertDimensionless(pszBottomOffset);
	}
	else
	{
		m_iBottomOffset = UT_convertToLogicalUnits(defaultOffset.c_str());
		m_dBottomOffsetUserUnits = UT_convertDimensionless(defaultOffset.c_str());
	}
#endif

//
// Positioned columns controls
//
	const char * pszLeftColPos = NULL;
	pSectionAP->getProperty("table-column-leftpos", (const gchar *&)pszLeftColPos);
	UT_sint32 iLeftColPos = m_iLeftColPos;
	if(pszLeftColPos && *pszLeftColPos)
	{
		m_iLeftColPos = UT_convertToLogicalUnits(pszLeftColPos);
		xxx_UT_DEBUGMSG(("Left colpos is %s \n",pszLeftColPos));

		FV_View * pView = m_pLayout->getView();
		GR_Graphics * pG = getDocLayout()->getGraphics();
		UT_return_if_fail( pView && pG );

		if(((pView->getViewMode() == VIEW_NORMAL) || (pView->getViewMode() == VIEW_WEB)) && 
		   m_iLeftColPos < 0 &&
		   !pG->queryProperties(GR_Graphics::DGP_PAPER))
		{
			m_iLeftColPos = 0;
		}
	}

	if(iLeftColPos != m_iLeftColPos)
	{
		collapse();
	}
}

UT_sint32 fl_TableLayout::getColSpacing(void) const
{
	return m_iColSpacing;
}


UT_sint32 fl_TableLayout::getRowSpacing(void) const
{
	return m_iRowSpacing;
}

UT_sint32 fl_TableLayout::getLineThickness(void) const
{
	return m_iLineThickness;
}

UT_sint32 fl_TableLayout::getTopOffset(void) const
{
	return m_iTopOffset;
}

UT_sint32 fl_TableLayout::getBottomOffset(void) const
{
	return m_iBottomOffset;
}

UT_sint32   fl_TableLayout::getLeftOffset(void) const
{
	return m_iLeftOffset;
}

UT_sint32   fl_TableLayout::getRightOffset(void) const
{
	return m_iRightOffset;
}

void fl_TableLayout::collapse(void)
{
	// Clear all our Tables
	fp_TableContainer *pTab = static_cast<fp_TableContainer *>(getFirstContainer());
	if (pTab)
	{
		pTab->clearScreen();
	}

	// get rid of all the layout information for every containerLayout
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->collapse();
		pCL = pCL->getNext();
	}
	m_iHeightChanged = 0;
	m_pNewHeightCell = NULL;
	if(pTab)
	{
//
// Remove from the container it comes from
//
		fp_VerticalContainer * pUpCon = static_cast<fp_VerticalContainer *>(pTab->getContainer());
		pUpCon->removeContainer(pTab);
		delete pTab;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
	setNeedsReformat(this);
	m_bInitialLayoutCompleted = false;
}

bool fl_TableLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()== PTX_SectionTable);
	fl_ContainerLayout * pCL = myContainingLayout();
	if(pCL->getContainerType() == FL_CONTAINER_CELL)
	{
		fl_CellLayout * pCell = static_cast<fl_CellLayout *>(pCL);
		pCell->decNumNestedTables();
		fl_TableLayout * pTab = static_cast<fl_TableLayout *>(pCell->myContainingLayout());
		pTab->decNumNestedTables();
	}
	xxx_UT_DEBUGMSG(("SEVIOR: !!!!!!!! Doing table delete strux!! \n"));
	collapse();
	//
	// Look to see if we're in a HdrFtr
	//
	fl_ContainerLayout * pMyCL = myContainingLayout();
	if(pMyCL && pMyCL->getContainerType() == FL_CONTAINER_HDRFTR)
	{
		fl_HdrFtrSectionLayout * pHFSL = static_cast<fl_HdrFtrSectionLayout *>(pMyCL);
		pHFSL->bl_doclistener_deleteTableStrux(this,pcrx);
	}
	myContainingLayout()->remove(this);

	delete this;			// TODO whoa!  this construct is VERY dangerous.

	return true;
}

/*!
 * Return the position of the table strux. 
 */
PT_DocPosition fl_TableLayout::getPosition(bool bActualBlockPosition /* = false */) const
{
	return fl_ContainerLayout::getPosition(bActualBlockPosition);
}

/*!
 * Return the total length of the table including nested tables.
 * This length includes the table and endtable struxs so if you add it
 * to the position of the table strux you will get the first position 
 * past the endtable strux
 */
UT_uint32 fl_TableLayout::getLength(void)
{
	pf_Frag_Strux* sdhTab = getStruxDocHandle();
	pf_Frag_Strux* sdhEnd = m_pDoc-> getEndTableStruxFromTableSDH(sdhTab);
	PT_DocPosition posEnd = 0;
	PT_DocPosition posStart = 0;
	UT_uint32 len = 0;
	if(sdhTab && (sdhEnd == NULL)) // handle case of endStrux not in yet
	{
		posStart = m_pDoc->getStruxPosition(sdhTab);
		m_pDoc->getBounds(true,posEnd);
		len = posEnd - posStart + 1;
	}
	else if(sdhTab == NULL)
	{
		return 0;
	}
	else
	{
		posEnd = m_pDoc->getStruxPosition(sdhEnd);
		len = posEnd -  m_pDoc->getStruxPosition(sdhTab) + 1;
	}
	return len;
}

/*!
 * This method attaches pCell to the current tablecontainer.
 */
void fl_TableLayout::attachCell(fl_ContainerLayout * pCell)
{
	//
	// Verify the cell layout is in the table.
    //
	fl_ContainerLayout * pCur = getLastLayout();
	while(pCur && pCur !=  pCell)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Looking for %x found %x \n",pCell,pCur));
		pCur = pCur->getPrev();
	}
	if(pCur == NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(getLastContainer());
	//	UT_ASSERT(pTab);
	if(pCell->getLastContainer() == NULL)
	{
		setDirty();
		return;
	}
	if(pTab)
	{
		pTab->tableAttach(static_cast<fp_CellContainer *>(pCell->getLastContainer()));
	}
	setDirty();
}

/*!
 * This method removes all layout structures contained by this layout
 * structure.
 */
void fl_TableLayout::_purgeLayout(void)
{
	fl_ContainerLayout * pCL = getFirstLayout();
	while(pCL)
	{
		fl_ContainerLayout * pNext = pCL->getNext();
		delete pCL;
		pCL = pNext;
	}
}

//------------------------------------------------------------------

fl_CellLayout::fl_CellLayout(FL_DocLayout* pLayout, pf_Frag_Strux* sdh, PT_AttrPropIndex indexAP, fl_ContainerLayout * pMyContainerLayout)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_CELL,FL_CONTAINER_CELL,PTX_SectionCell,pMyContainerLayout),
	  m_bNeedsRebuild(false),
	  m_iLeftOffset(0),
	  m_dLeftOffsetUserUnits(0.0),
	  m_iRightOffset(0),
	  m_dRightOffsetUserUnits(0.0),
	  m_iTopOffset(0),
	  m_dTopOffsetUserUnits(0.0),
	  m_iBottomOffset(0),
	  m_dBottomOffsetUserUnits(0.0),
	  m_iLeftAttach(0),
	  m_iRightAttach(1),
	  m_iTopAttach(0),
	  m_iBottomAttach(1),
	  m_bCellPositionedOnPage(false),
	  m_iCellHeight(0),
	  m_iCellWidth(0),
	  m_iNumNestedTables(0),
	  m_iVertAlign(0)
{
	createCellContainer();
}

fl_CellLayout::~fl_CellLayout()
{
	// NB: be careful about the order of these
	xxx_UT_DEBUGMSG(("Delete cell %x \n",this));
	_purgeLayout();
	fp_CellContainer * pTC = static_cast<fp_CellContainer *>(getFirstContainer());
	while(pTC)
	{
		fp_CellContainer * pNext = static_cast<fp_CellContainer *>(pTC->getNext());
		if(pTC == static_cast<fp_CellContainer *>(getLastContainer()))
		{
			pNext = NULL;
		}
		delete pTC;
		pTC = pNext;
	}
	DELETEP(m_pImageImage);
	DELETEP(m_pGraphicImage);
	setFirstContainer(NULL);
	setLastContainer(NULL);
}

/*!
 * This method creates a new cell with it's properties initially set
 * from the Attributes/properties of this Layout
 */
void fl_CellLayout::createCellContainer(void)
{
	lookupProperties();
	if(isHidden() >= FP_HIDDEN_FOLDED)
	{
		xxx_UT_DEBUGMSG(("Don't format coz I'm hidden! \n"));
		return;
	}

	fp_CellContainer * pCellContainer = new fp_CellContainer(static_cast<fl_SectionLayout *>(this));
	setFirstContainer(pCellContainer);
	setLastContainer(pCellContainer);
	fl_ContainerLayout * pCL = myContainingLayout();
	while(pCL!= NULL && ((pCL->getContainerType() != FL_CONTAINER_DOCSECTION) && (pCL->getContainerType() != FL_CONTAINER_HDRFTR)))
	{
		pCL = pCL->myContainingLayout();
	}
	fl_DocSectionLayout * pDSL = NULL;
	if(pCL->getContainerType() == FL_CONTAINER_HDRFTR)
	{
		pDSL = static_cast<fl_HdrFtrSectionLayout *>(pCL)->getDocSectionLayout();
	}
	else
	{
		pDSL = static_cast<fl_DocSectionLayout *>(pCL);
	}
	UT_ASSERT(pDSL != NULL);
	UT_sint32 iWidth = pDSL->getWidth();
	pCellContainer->setWidth(iWidth);
	// Now do cell image

	const PP_AttrProp* pSectionAP = NULL;
	// This is a real NO-NO: must *always* call getAP()
	// m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);
	getAP(pSectionAP);

	const gchar * pszDataID = NULL;
	pSectionAP->getAttribute(PT_STRUX_IMAGE_DATAID, (const gchar *&)pszDataID);
	DELETEP(m_pGraphicImage);
	DELETEP(m_pImageImage);
	if(pszDataID && *pszDataID)
	{
		UT_DEBUGMSG(("!!!Found image of file %s \n",pszDataID));
		UT_DEBUGMSG(("LeftAttach %d \n",m_iLeftAttach));
		m_pGraphicImage = FG_Graphic::createFromStrux(this);
	}
	setCellContainerProperties(pCellContainer);

}

UT_sint32 fl_CellLayout::getNumNestedTables(void) const
{
	return m_iNumNestedTables;
}


void fl_CellLayout::incNumNestedTables(void)
{
	m_iNumNestedTables++;
}

void fl_CellLayout::decNumNestedTables(void)
{
	m_iNumNestedTables--;
}

/*!
 * This method sets all the parameters of the cell container from
 * properties of this section 
 */
void fl_CellLayout::setCellContainerProperties(fp_CellContainer * pCell)
{
	if(pCell == NULL)
	{
		return;
	}
	pCell->setLeftAttach(m_iLeftAttach);
	pCell->setRightAttach(m_iRightAttach);
	pCell->setTopAttach(m_iTopAttach);
	pCell->setBottomAttach(m_iBottomAttach);
	pCell->setLeftPad(m_iLeftOffset);
	pCell->setRightPad(m_iRightOffset);
	pCell->setTopPad(m_iTopOffset);
	pCell->setBotPad(m_iBottomOffset);

	pCell->setBackground(m_background);

	pCell->setBottomStyle(m_lineBottom);
	pCell->setLeftStyle(m_lineLeft);
	pCell->setRightStyle(m_lineRight);
	pCell->setTopStyle(m_lineTop);
	pCell->setVertAlign(m_iVertAlign);
	if(m_pGraphicImage)
	{
		if(m_pImageImage == NULL)
		{
			const PP_AttrProp * pAP = NULL;
			getAP(pAP);
			GR_Graphics * pG = getDocLayout()->getGraphics();
			UT_sint32 iWidth = pG->tlu(100);
			UT_sint32 iHeight = pG->tlu(100);
			if(m_pGraphicImage->getType() == FGT_Raster)
			{
				iWidth = pG->tlu(m_pGraphicImage->getWidth());
				iHeight = pG->tlu(m_pGraphicImage->getHeight());
			}
			GR_Image * pImage = m_pGraphicImage->generateImage(pG,pAP,iWidth,iHeight);
			m_iDocImageWidth = iWidth;
			m_iDocImageHeight = iHeight;
			m_iGraphicTick = getDocLayout()->getGraphicTick();
			UT_Rect rec(0,0,iWidth,iHeight);
			pImage->scaleImageTo(pG,rec);
			m_pImageImage = pImage;
		}
		pCell->getFillType().setImagePointer(&m_pGraphicImage,&m_pImageImage);
	}
}

/*!
 * Returns true if the cell is selected.
 */
bool fl_CellLayout::isCellSelected(void)
{
	FV_View* pView = m_pLayout->getView();
	PT_DocPosition posStartCell = 0;
	PT_DocPosition posEndCell =0;
	pf_Frag_Strux* sdhEnd, *sdhStart;

	sdhStart = getStruxDocHandle();
	posStartCell = m_pDoc->getStruxPosition(sdhStart) +1;
	bool bRes = m_pDoc->getNextStruxOfType(sdhStart, PTX_EndCell, &sdhEnd);
	UT_return_val_if_fail(bRes, false);

	posEndCell = m_pDoc->getStruxPosition(sdhEnd) -1;
	if(pView->isPosSelected(posStartCell) && pView->isPosSelected(posEndCell))
	{
		xxx_UT_DEBUGMSG(("Cell at top %d left %d selected \n",m_iTopAttach,m_iLeftAttach));
		return true;
	}
	return false;
}

/*!
 * This method measures the cell height and compares it to the previously
 * measured height. If they disagree update the layout of the table.
 */
void fl_CellLayout::checkAndAdjustCellSize(void)
{
	fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getFirstContainer());
	if(pCell == NULL)
	{
		return;
	}
	fp_Requisition Req;
	pCell->sizeRequest(&Req);
	if(Req.height == m_iCellHeight)
	{
		xxx_UT_DEBUGMSG(("checkandadjustcellheight: Heights identical return %d \n",m_iCellHeight));
		return;
	}
	m_iCellHeight = Req.height;
	xxx_UT_DEBUGMSG(("checkandadjustcellheight: Heights differ format %d %d \n",m_iCellHeight,Req.height));
	pCell->setHeight(m_iCellHeight);
	m_iCellWidth = Req.width;
	static_cast<fl_TableLayout *>(myContainingLayout())->setDirty();
	static_cast<fl_TableLayout *>(myContainingLayout())->setHeightChanged(pCell);
	myContainingLayout()->format();
}
	
bool fl_CellLayout::bl_doclistener_insertCell(fl_ContainerLayout* pCell,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{
	fl_ContainerLayout * pNewCL = NULL;
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(myContainingLayout());
	pNewCL = pTL->insert(sdh,pCell,pcrx->getIndexAP(), FL_CONTAINER_CELL);
	
		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).
	if(	pfnBindHandles)
	{
		fl_ContainerLayout* sfhNew = pNewCL;
		pfnBindHandles(sdh,lid,sfhNew);
	}
	fl_CellLayout * pCL = static_cast<fl_CellLayout *>(pNewCL);
	pTL->attachCell(pCL);

//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() +  fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
	return true;
}

	
bool fl_CellLayout::bl_doclistener_insertEndCell(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{
	// The endCell strux actually needs a format handle to to this cell layout.
	// so we bind to this layout. We also set a pointer to keep track of the endCell strux.

		
	fl_ContainerLayout* sfhNew = this;
	pfnBindHandles(sdh,lid,sfhNew);
	setEndStruxDocHandle(sdh);

//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() +  fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
	return true;
}

fl_SectionLayout * fl_CellLayout::getSectionLayout(void) const
{
	fl_ContainerLayout * pDSL = myContainingLayout();
	while(pDSL)
	{
		if(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			return static_cast<fl_SectionLayout *>(pDSL);
		}
		pDSL = pDSL->myContainingLayout();
	}
	return NULL;
}

/*!
  Create a new Cell containers and plug it into the linked list of Cell 
  containers.
  \param If pPrevCell is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Cell container
*/
fp_Container* fl_CellLayout::getNewContainer(fp_Container * pPrev)
{
//
// One cell container per cell layout
//
	UT_UNUSED(pPrev);
	UT_ASSERT(pPrev == NULL);
	UT_ASSERT((getFirstContainer() == NULL) && (getLastContainer()==NULL));
	createCellContainer();
	setCellContainerProperties(static_cast<fp_CellContainer *>(getLastContainer()));
	return static_cast<fp_Container *>(getLastContainer());
}

bool fl_CellLayout::isDoingFormat(void) const
{
	return m_bDoingFormat;
}

void fl_CellLayout::format(void)
{
	if(isHidden() >= FP_HIDDEN_FOLDED)
	{
		xxx_UT_DEBUGMSG(("Don't format CELL coz I'm hidden! \n"));
		return;
	}

	if(getFirstContainer() == NULL)
	{
		getNewContainer(NULL);
	}
	m_bDoingFormat = true;
	UT_sint32 iOldHeight = getFirstContainer()->getHeight();
	fl_ContainerLayout * pPrevCL = myContainingLayout()->getPrev();
	fp_Page * pPrevP = NULL;
	m_vecFormatLayout.clear(); // Later we'll use this.
	if(pPrevCL)
	{
		fp_Container * pPrevCon = pPrevCL->getFirstContainer();
		if(pPrevCon)
		{
			pPrevP = pPrevCon->getPage();
		}
	}
	fl_ContainerLayout*	pBL = getFirstLayout();
	//	UT_ASSERT(pBL);
	while (pBL)
	{
		xxx_UT_DEBUGMSG(("Formatting Block in Cell %x \n",pBL));
		if(iOldHeight <= 0)
		{
			pBL->setNeedsReformat(pBL,0);
		}
		pBL->format();
		UT_sint32 count = 0;
		while(pBL->getLastContainer() == NULL || pBL->getFirstContainer()==NULL)
		{
			UT_DEBUGMSG(("Error formatting a block try again \n"));
			count = count + 1;
			pBL->format();
			if(count > 3)
		  	{
				UT_DEBUGMSG(("Give up trying to format. Hope for the best :-( \n"));
				break;
			}
		}
		pBL = pBL->getNext();
	}
	static_cast<fp_CellContainer *>(getFirstContainer())->layout();
	
	UT_sint32 iNewHeight = getFirstContainer()->getHeight();
	fl_ContainerLayout * myL = myContainingLayout();
	while (myL && (myL->getContainerType() != FL_CONTAINER_SHADOW &&
				   myL->getContainerType() != FL_CONTAINER_HDRFTR &&
				   myL->getContainerType() != FL_CONTAINER_DOCSECTION))
	{
		myL = myL->myContainingLayout();
	}
	if(myL && (myL->getContainerType() != FL_CONTAINER_SHADOW) &&
	   (myL->getContainerType() != FL_CONTAINER_HDRFTR))
	{
		if(iNewHeight != iOldHeight)
		{
			getDocSectionLayout()->setNeedsSectionBreak(true,pPrevP);
		}
	}
	m_bNeedsReformat = (m_vecFormatLayout.getItemCount() > 0);
	checkAndAdjustCellSize();
	m_bDoingFormat = false;
}

void fl_CellLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->markAllRunsDirty();
		pCL = pCL->getNext();
	}
}

bool fl_CellLayout::needsReformat(void) const
{
	if(fl_SectionLayout::needsReformat())
	{
		return true;
	}
	return !isLayedOut();
}

bool fl_CellLayout::isLayedOut(void) const
{
	fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getFirstContainer());
	if(pCell == NULL)
	{
		return false;
	}
	UT_return_val_if_fail(pCell->getContainerType() == FP_CONTAINER_CELL,false);
	//
	// Cell Not set
	//
	if(pCell->getStartY() < -10000000)
	{
		return false;
	}
	return true;
}

void fl_CellLayout::updateLayout(bool /*bDoAll*/)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	bool bNeedsFormat = false;
    xxx_UT_DEBUGMSG(("updateCellLayout \n"));
	m_vecFormatLayout.clear();
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			pBL->format();
			bNeedsFormat = true;
			xxx_UT_DEBUGMSG(("updateCellLayout formatting sub block \n"));
		}

		pBL = pBL->getNext();
	}
	if(bNeedsFormat)
	{
		xxx_UT_DEBUGMSG(("updateCellLayout formatting whole cell \n"));
		format();
	}
}

void fl_CellLayout::redrawUpdate(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	if(!m_bNeedsRedraw)
	{
		return;
	}
	
	while (pBL)
	{
		if (pBL->needsRedraw())
		{
			pBL->redrawUpdate();
		}

		pBL = pBL->getNext();
	}
	//
	//	fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getFirstContainer());
	//pCell->drawLines();
	//pCell->drawLinesAdjacent();
	m_bNeedsRedraw = false;
}

bool fl_CellLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	if(pcrxc->getStruxType() == PTX_SectionCell)
	{
		setAttrPropIndex(pcrxc->getIndexAP());
	}
	collapse();
	_updateCell();
	//
	// Look to see if we're in a HfrFtr section
	//
	fl_ContainerLayout * pMyCL = myContainingLayout();
	if(pMyCL)
	{
		pMyCL = pMyCL->myContainingLayout();
	}
	if(pMyCL && pMyCL->getContainerType() == FL_CONTAINER_HDRFTR)
	{
		fl_HdrFtrSectionLayout * pHFSL = static_cast<fl_HdrFtrSectionLayout *>(pMyCL);
		pHFSL->bl_doclistener_changeStrux(this,pcrxc);
	}
	return true;
}


void fl_CellLayout::_updateCell(void)
{

	const PP_AttrProp* pAP = NULL;
	// This is a real NO-NO: must *always* call getAP()
	// bool bres = m_pDoc->getAttrProp(m_apIndex, &pAP);
	getAP(pAP);
	
	lookupProperties();

	// clear all the columns
    // Assume that all formatting have already been removed via a 
    // collapse()
    //

	/*
	  TODO to more closely mirror the architecture we're using for BlockLayout, this code
	  should probably just set a flag, indicating the need to reformat this section.  Then,
	  when it's time to update everything, we'll actually do the format.
	*/

	FV_View * pView = m_pLayout->getView();
	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(false);
	}

	format();
	markAllRunsDirty();

	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(true);
	}

	return;
}

bool fl_CellLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		pBL->recalculateFields(iUpdateCount);
		pBL = pBL->getNext();
	}
	return true;
}

/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_CellLayout::_lookupProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail(pSectionAP);
	bool bFolded = (isHidden() == FP_HIDDEN_FOLDED);
	if(bFolded && (isHidden() != FP_HIDDEN_FOLDED))
	{
		UT_DEBUGMSG(("!!!!!!!!!!!!!!!!!!------------------!!!!!!!!!!!\n"));
		UT_DEBUGMSG(("!!!!!!!!!!!!!!!!!!------------------!!!!!!!!!!!\n"));
		UT_DEBUGMSG(("!!!!!!!!!!!!!!!!!! Unfold CELL Now !!!!!!!!!!!\n"));
		UT_DEBUGMSG(("!!!!!!!!!!!!!!!!!!------------------!!!!!!!!!!!\n"));
		UT_DEBUGMSG(("!!!!!!!!!!!!!!!!!!------------------!!!!!!!!!!!\n"));
	}

	xxx_UT_DEBUGMSG(("SEVIOR: indexAp in Cell Layout %d \n",m_apIndex));
	/*
	  TODO shouldn't we be using PP_evalProperty like
	  the blockLayout does?

	  Yes, since PP_evalProperty does a fallback to the
	  last-chance defaults, whereas the code below is
	  hard-coding its own defaults.  Bad idea.
	*/

	const char* pszLeftOffset = NULL;
	const char* pszTopOffset = NULL;
	const char* pszRightOffset = NULL;
	const char* pszBottomOffset = NULL;
	pSectionAP->getProperty("cell-margin-left", (const gchar *&)pszLeftOffset);
	pSectionAP->getProperty("cell-margin-top", (const gchar *&)pszTopOffset);
	pSectionAP->getProperty("cell-margin-right", (const gchar *&)pszRightOffset);
	pSectionAP->getProperty("cell-margin-bottom", (const gchar *&)pszBottomOffset);
	const gchar * szRulerUnits;
	UT_Dimension dim;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		dim = UT_determineDimension(szRulerUnits);
	else
		dim = DIM_IN;

	UT_String defaultOffset;
	switch(dim)
	{
	case DIM_IN:
		defaultOffset = "0.0in"; //was 0.05in
		break;

	case DIM_CM:
		defaultOffset = "0.0cm";
		break;

	case DIM_PI:
		defaultOffset = "0.0pi";
		break;

	case DIM_PT:
		defaultOffset= "0.0pt";
		break;

	case DIM_MM:
		defaultOffset= "0.0mm"; //was 1.27
		break;

		// TODO: PX, and PERCENT
		// let them fall through to the default now
		// and we don't use them anyway
#if 0
	case DIM_PX:
	case DIM_PERCENT:
#endif
	case DIM_none:
	default:
		defaultOffset = "0.0in";	// TODO: what to do with this. was 0.05in
		break;

	}
	defaultOffset = "0.03in";

	static UT_sint32 idefaultOffsetLogicalUnits =  UT_convertToLogicalUnits("0.01in");
	static double idefaultOffsetDimensionless =  UT_convertDimensionless("0.01in");

	if(pszLeftOffset && pszLeftOffset[0])
	{
		m_iLeftOffset = UT_convertToLogicalUnits(pszLeftOffset);
		m_dLeftOffsetUserUnits = UT_convertDimensionless(pszLeftOffset);
	}
	else
	{
		m_iLeftOffset = idefaultOffsetLogicalUnits;
		m_dLeftOffsetUserUnits = idefaultOffsetDimensionless;
	}

	if(pszTopOffset && pszTopOffset[0])
	{
		m_iTopOffset = UT_convertToLogicalUnits(pszTopOffset);
		m_dTopOffsetUserUnits = UT_convertDimensionless(pszTopOffset);
	}
	else
	{
		m_iTopOffset = idefaultOffsetLogicalUnits;
		m_dTopOffsetUserUnits = idefaultOffsetDimensionless;
	}

	if(pszRightOffset && pszRightOffset[0])
	{
		m_iRightOffset = UT_convertToLogicalUnits(pszRightOffset);
		m_dRightOffsetUserUnits = UT_convertDimensionless(pszRightOffset);
	}
	else
	{
		m_iRightOffset = idefaultOffsetLogicalUnits;
		m_dRightOffsetUserUnits = idefaultOffsetDimensionless;
	}

	if(pszBottomOffset && pszBottomOffset[0])
	{
		m_iBottomOffset = UT_convertToLogicalUnits(pszBottomOffset);
		m_dBottomOffsetUserUnits = UT_convertDimensionless(pszBottomOffset);
	}
	else
	{
		m_iBottomOffset = idefaultOffsetLogicalUnits;
		m_dBottomOffsetUserUnits = idefaultOffsetDimensionless;
	}
	const char* pszLeftAttach = NULL;
	const char* pszRightAttach = NULL;
	const char* pszTopAttach = NULL;
	const char* pszBottomAttach = NULL;
	pSectionAP->getProperty("left-attach", (const gchar *&)pszLeftAttach);
	pSectionAP->getProperty("right-attach", (const gchar *&)pszRightAttach);
	pSectionAP->getProperty("top-attach", (const gchar *&)pszTopAttach);
	pSectionAP->getProperty("bot-attach", (const gchar *&)pszBottomAttach);
	xxx_UT_DEBUGMSG(("CellLayout _lookupProps top %s bot %s left %s right %s \n",pszTopAttach,pszBottomAttach,pszLeftAttach,pszRightAttach)); 
	if(pszLeftAttach && pszLeftAttach[0])
	{
		m_iLeftAttach = atoi(pszLeftAttach);
	}
	else
	{
		m_iLeftAttach = 0;
	}
	if(pszRightAttach && pszRightAttach[0])
	{
		m_iRightAttach = atoi(pszRightAttach);
	}
	else
	{
		m_iRightAttach = m_iLeftAttach + 1;
	}
	if(pszTopAttach && pszTopAttach[0])
	{
		m_iTopAttach = atoi(pszTopAttach);
	}
	else
	{
		m_iTopAttach = 0;
	}
	if(pszBottomAttach && pszBottomAttach[0])
	{
		m_iBottomAttach = atoi(pszBottomAttach);
	}
	else
	{
		m_iBottomAttach = m_iTopAttach+1;
	}

	/* cell-border properties:
	 */
	const gchar * pszColor = NULL;
	pSectionAP->getProperty ("color", pszColor);
	
	const gchar * pszBorderColor = NULL;
	const gchar * pszBorderStyle = NULL;
	const gchar * pszBorderWidth = NULL;

	pSectionAP->getProperty ("bot-color",       pszBorderColor);
	pSectionAP->getProperty ("bot-style",       pszBorderStyle);
	pSectionAP->getProperty ("bot-thickness",   pszBorderWidth);

	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(myContainingLayout());

	s_border_properties_cell (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineBottom,pTL->getBottomStyle());

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("left-color",      pszBorderColor);
	pSectionAP->getProperty ("left-style",      pszBorderStyle);
	pSectionAP->getProperty ("left-thickness",  pszBorderWidth);


	s_border_properties_cell (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineLeft,pTL->getLeftStyle());

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("right-color",     pszBorderColor);
	pSectionAP->getProperty ("right-style",     pszBorderStyle);
	pSectionAP->getProperty ("right-thickness", pszBorderWidth);

	s_border_properties_cell (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineRight,pTL->getRightStyle());

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("top-color",       pszBorderColor);
	pSectionAP->getProperty ("top-style",       pszBorderStyle);
	pSectionAP->getProperty ("top-thickness",   pszBorderWidth);

	s_border_properties_cell (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineTop,pTL->getTopStyle());

	const char* pszVertAlign = NULL;
	pSectionAP->getProperty("vert-align", (const gchar *&)pszVertAlign);
	if(pszVertAlign && pszVertAlign[0])
	{
		m_iVertAlign = atoi(pszVertAlign);
	}
	else
	{
		m_iVertAlign = 0;
	}


	/* cell fill
	 */
	m_background.reset ();

	const gchar * pszBgStyle = NULL;
	const gchar * pszBgColor = NULL;
	const gchar * pszBackgroundColor = NULL;

	pSectionAP->getProperty ("bg-style",         pszBgStyle);
	pSectionAP->getProperty ("bgcolor",          pszBgColor);
	pSectionAP->getProperty ("background-color", pszBackgroundColor);

	s_background_properties (pszBgStyle, pszBgColor, pszBackgroundColor, m_background);
	if(pTL)
	{
		const UT_GenericVector<fl_ColProps*> * pVecCols = pTL->getVecColProps();
		const UT_GenericVector<fl_RowProps*> * pVecRows = pTL->getVecRowProps();
		if(pVecCols->getItemCount() > 0)
		{
			UT_sint32 i = 0;
			UT_sint32 cellW = 0; 
			for(i=getLeftAttach(); i<getRightAttach() && i<pVecCols->getItemCount();i++)
			{
				fl_ColProps* pCol = pVecCols->getNthItem(i);
				cellW += pCol->m_iColWidth;
			}
			m_iCellWidth = cellW;
		}
		else
		{
			m_iCellWidth = 0;
		}
		if(pVecRows->getItemCount() > 0)
		{
			UT_sint32 i = 0;
			UT_sint32 cellH = 0; 
			for(i=getTopAttach(); i<getBottomAttach() && i<pVecRows->getItemCount();i++)
			{
				fl_RowProps* pRow = pVecRows->getNthItem(i);
				cellH += pRow->m_iRowHeight;
			}
			m_iCellHeight = cellH;
		}
		else
		{
			m_iCellHeight = 0;
		}
	}
}

UT_sint32   fl_CellLayout::getLeftOffset(void) const
{
	return m_iLeftOffset;
}

UT_sint32   fl_CellLayout::getRightOffset(void) const
{
	return m_iRightOffset;
}

UT_sint32 fl_CellLayout::getTopOffset(void) const
{
	return m_iTopOffset;
}


UT_sint32 fl_CellLayout::getBottomOffset(void) const
{
	return m_iBottomOffset;
}

void fl_CellLayout::_localCollapse(void)
{

	// ClearScreen on our Cell. One Cell per layout.

	fp_CellContainer *pCell = static_cast<fp_CellContainer *>(getFirstContainer());
	xxx_UT_DEBUGMSG(("SEVIOR: Local collapse of CellLayout %x CellContainer %x \n",this,pCell));
	if (pCell)
	{
		pCell->clearScreen();
	}

	// get rid of all the layout information for every containerLayout
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL != NULL)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Local collapse of CellLayout %x Contained Layout %x \n",this,pCL));
		pCL->collapse();
		pCL = pCL->getNext();
	}
}
/*!
 * Return the total length of the cell including nested tables.
 * This length includes the cell and endcell struxs so if you add it
 * to the position of the cell strux you will get the first position 
 * past the endcell strux
 */
UT_uint32 fl_CellLayout::getLength(void)
{
	pf_Frag_Strux* sdhCell = getStruxDocHandle();
	pf_Frag_Strux* sdhEnd = m_pDoc->getEndCellStruxFromCellSDH(sdhCell);
	PT_DocPosition posEnd = 0;
	PT_DocPosition posStart = 0;
	UT_uint32 len = 0;
	if(sdhCell && (sdhEnd == NULL)) // handle case of endStrux not in yet
	{
		posStart = m_pDoc->getStruxPosition(sdhCell);
		m_pDoc->getBounds(true,posEnd);
		len = posEnd - posStart + 1;
	}
	else if(sdhCell == NULL)
	{
		return 0;
	}
	else
	{
		posEnd = m_pDoc->getStruxPosition(sdhEnd);
		len = posEnd -  m_pDoc->getStruxPosition(sdhCell) + 1;
	}
	return len;
}


void fl_CellLayout::collapse(void)
{
	_localCollapse();

	// Delete our Cell. One Cell per layout.

	fp_CellContainer *pCell = static_cast<fp_CellContainer *>(getFirstContainer());
//
// Remove it from the table container
//
	if (pCell)
	{
		fp_TableContainer * pTabCon = static_cast<fp_TableContainer *>(pCell->getContainer());
		if(pTabCon)
		{
			pTabCon->removeContainer(pCell);
		}
//
// remove it from the linked list.
//
		fp_CellContainer * pPrev = static_cast<fp_CellContainer *>(pCell->getPrev());
		if(pPrev)
		{
			pPrev->setNext(pCell->getNext());
		}
		if(pCell->getNext())
		{
			pCell->getNext()->setPrev(pPrev);
		}
		delete pCell;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
	setNeedsReformat(this);
}

bool fl_CellLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()== PTX_SectionCell);

	collapse();
	//
	// Look to see if we're in a HdrFtr
	//
	fl_ContainerLayout * pMyCL = myContainingLayout();
	if(pMyCL)
	{
		pMyCL = pMyCL->myContainingLayout();
	}
	if(pMyCL && pMyCL->getContainerType() == FL_CONTAINER_HDRFTR)
	{
		fl_HdrFtrSectionLayout * pHFSL = static_cast<fl_HdrFtrSectionLayout *>(pMyCL);
		pHFSL->bl_doclistener_deleteCellStrux(this,pcrx);
	}
	myContainingLayout()->remove(this);

	delete this;			// TODO whoa!  this construct is VERY dangerous.

	return true;
}

/*!
 * This method removes all layout structures contained by this layout
 * structure.
 */
void fl_CellLayout::_purgeLayout(void)
{
	fl_ContainerLayout * pCL = getFirstLayout();
	while(pCL)
	{
		fl_ContainerLayout * pNext = pCL->getNext();
		delete pCL;
		pCL = pNext;
	}
}

static void s_background_properties (const char * pszBgStyle, const char * pszBgColor,
									 const char * pszBackgroundColor,
									 PP_PropertyMap::Background & background)
{
	if (pszBgStyle)
	{
		if (strcmp (pszBgStyle, "0") == 0)
		{
			background.m_t_background = PP_PropertyMap::background_none;
		}
		else if (strcmp (pszBgStyle, "1") == 0)
		{
			if (pszBgColor)
			{
				background.m_t_background = PP_PropertyMap::background_type (pszBgColor);
				if (background.m_t_background == PP_PropertyMap::background_solid)
				{
					UT_parseColor (pszBgColor, background.m_color);
				}
			}
		}
	}

	if (pszBackgroundColor)
	{
		background.m_t_background = PP_PropertyMap::background_type (pszBackgroundColor);
		if (background.m_t_background == PP_PropertyMap::background_solid)
			UT_parseColor (pszBackgroundColor, background.m_color);
	}
}

static void s_border_properties (const char * border_color, const char * border_style, const char * border_width,
								 const char * color, PP_PropertyMap::Line & line)
{
	/* cell-border properties:
	 * 
	 * (1) color      - defaults to value of "color" property
	 * (2) line-style - defaults to solid (in contrast to "none" in CSS)
	 * (3) thickness  - defaults to 1 layout unit (??, vs "medium" in CSS)
	 */
	line.reset ();

	PP_PropertyMap::TypeColor t_border_color = PP_PropertyMap::color_type (border_color);
	if (t_border_color)
	{
		line.m_t_color = t_border_color;
		if (t_border_color == PP_PropertyMap::color_color)
			UT_parseColor (border_color, line.m_color);
	}
	else if (color)
	{
		PP_PropertyMap::TypeColor t_color = PP_PropertyMap::color_type (color);

		line.m_t_color = t_color;
		if (t_color == PP_PropertyMap::color_color)
			UT_parseColor (color, line.m_color);
	}

	line.m_t_linestyle = PP_PropertyMap::linestyle_type (border_style);
	if (!line.m_t_linestyle)
		line.m_t_linestyle = PP_PropertyMap::linestyle_solid;

	line.m_t_thickness = PP_PropertyMap::thickness_type (border_width);
	if (line.m_t_thickness == PP_PropertyMap::thickness_length)
	{
		if (UT_determineDimension (border_width, (UT_Dimension)-1) == DIM_PX)
		{
			double thickness = UT_LAYOUT_RESOLUTION * UT_convertDimensionless (border_width);
			line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
		}
		else
			line.m_thickness = UT_convertToLogicalUnits (border_width);

		if (!line.m_thickness)
		{
			// default to 0.72pt
			double thickness = UT_LAYOUT_RESOLUTION;
			line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
		}
	}
	else // ??
	{
		// default to 0.72pt
		double thickness = UT_LAYOUT_RESOLUTION;
		line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
	}
}

/*!
 * Like above except if the property is not defined from the const char's
 * It's inherited from the lineTable class.
 */
static void s_border_properties_cell (const char * border_color, 
									  const char * border_style, 
									  const char * border_width,
									  const char * color, 
									  PP_PropertyMap::Line & line,
									  const PP_PropertyMap::Line lineTable
									  )
{
	/* cell-border properties:
	 * 
	 * (1) color      - defaults to value of "color" property
	 * (2) line-style - defaults to solid (in contrast to "none" in CSS)
	 * (3) thickness  - defaults to 1 layout unit (??, vs "medium" in CSS)
	 */
	line.reset ();
	
	PP_PropertyMap::TypeColor t_border_color = PP_PropertyMap::color_type (border_color);
	if (t_border_color)
	{
		line.m_t_color = t_border_color;
		if (t_border_color == PP_PropertyMap::color_color)
			UT_parseColor (border_color, line.m_color);
	}
	else if (color)
	{
		PP_PropertyMap::TypeColor t_color = PP_PropertyMap::color_type (color);

		line.m_t_color = t_color;
		if (t_color == PP_PropertyMap::color_color)
			UT_parseColor (color, line.m_color);
	}
	else if(lineTable.m_t_color)
	{
		line.m_t_color = lineTable.m_t_color;
		line.m_color = lineTable.m_color;
	}
	line.m_t_linestyle = PP_PropertyMap::linestyle_type (border_style);
	if (!line.m_t_linestyle)
	{ 
		if(lineTable.m_t_linestyle)
		{
			line.m_t_linestyle = lineTable.m_t_linestyle;
		}
		else
		{
			line.m_t_linestyle = PP_PropertyMap::linestyle_solid;
		}
	}
	line.m_t_thickness = PP_PropertyMap::thickness_type (border_width);
	if (line.m_t_thickness == PP_PropertyMap::thickness_length)
	{
		if (UT_determineDimension (border_width, (UT_Dimension)-1) == DIM_PX)
   		{
			double thickness = UT_LAYOUT_RESOLUTION * UT_convertDimensionless (border_width);
			line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
		}
		else
			line.m_thickness = UT_convertToLogicalUnits (border_width);
	
		if (!line.m_thickness)
		{
			double thickness = UT_LAYOUT_RESOLUTION;
			line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
		}
	}
	else if(lineTable.m_t_thickness ==  PP_PropertyMap::thickness_length)
   	{
		line.m_thickness = lineTable.m_thickness;
		line.m_t_thickness = lineTable.m_t_thickness;
	}
	else //
	{
		// default to 0.72pt
		line.m_t_thickness = PP_PropertyMap::thickness_length;
		double thickness = UT_LAYOUT_RESOLUTION;
		line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
	}
}
