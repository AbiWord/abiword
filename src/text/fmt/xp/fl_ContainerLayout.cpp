/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior
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

#include <string.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_string.h"

#include "ap_Prefs.h"
#include "fl_ContainerLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_SectionLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fl_TableLayout.h"
#include "fp_TableContainer.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "pt_Types.h"
#include "gr_Graphics.h"
#include "fv_View.h"
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"

fl_ContainerLayout::fl_ContainerLayout(fl_ContainerLayout* pMyLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP, PTStruxType iStrux, fl_ContainerType iType)
	: fl_Layout(iStrux, sdh),
	  m_iConType(iType),
	  m_pMyLayout(pMyLayout),
	  m_pPrev(NULL),
	  m_pNext(NULL),
	  m_pFirstL(NULL),
	  m_pLastL(NULL),
	  m_pFirstContainer(NULL),
	  m_pLastContainer(NULL),
	  m_pLB(NULL),
	  m_eHidden(FP_VISIBLE)
{
	setAttrPropIndex(indexAP);
	if(pMyLayout)
	{
		m_pDoc = pMyLayout->getDocument();
	}
}

fl_ContainerLayout::~fl_ContainerLayout()
{
	if(m_pLB)
	{
		delete m_pLB;
	}
#if 1
	m_pMyLayout = NULL;
	m_pFirstL = NULL;
	m_pLastL = NULL;
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pFirstContainer = NULL;
	m_pLastContainer = NULL;
#endif
}

/*!
 * Return the value of the attribute keyed by pszName
 */
const char*	fl_ContainerLayout::getAttribute(const char * pszName) const
{
	const PP_AttrProp * pAP = NULL;
	getAttrProp(&pAP);

	const XML_Char* pszAtt = NULL;
	pAP->getAttribute(static_cast<const XML_Char*>(pszName), pszAtt);

	return pszAtt;
}

/*!
 * Set the pointer to the next containerLayout given by pL
 */
void fl_ContainerLayout::setNext(fl_ContainerLayout* pL)
{
	m_pNext = pL;
}

/*!
 * Set the pointer to the previous containerLayout in the linked list
 * given by pL
 */
void fl_ContainerLayout::setPrev(fl_ContainerLayout* pL)
{
	m_pPrev = pL;
}

/*!
 * Return the next fl_ContainerLayout in the linked list.
 */
fl_ContainerLayout * fl_ContainerLayout::getNext(void) const
{
	return m_pNext;
}

/*!
 * Return the previous fl_ContainerLayout in the linked list
 */
fl_ContainerLayout * fl_ContainerLayout::getPrev(void) const
{
	return m_pPrev;
}

fl_DocSectionLayout * fl_ContainerLayout::getDocSectionLayout(void)
{
	fl_ContainerLayout * pCL = myContainingLayout();
	while(pCL!= NULL && ((pCL->getContainerType() != FL_CONTAINER_DOCSECTION) && (pCL->getContainerType() != FL_CONTAINER_HDRFTR)))
	{
		pCL = pCL->myContainingLayout();
	}
	if(pCL== NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return NULL;
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
	return pDSL;
}

/*!
 * Return the fl_ContainerLayout that "owns" this. Set to NULL for
 * fl_DocSectionLayout
 */
fl_ContainerLayout * fl_ContainerLayout::myContainingLayout(void) const
{
	return m_pMyLayout;
}

FL_DocLayout* fl_ContainerLayout::getDocLayout(void) const
{
	const fl_ContainerLayout * pMyContainer = static_cast<const fl_ContainerLayout *>(this);
	while(pMyContainer->getContainerType() != FL_CONTAINER_DOCSECTION && pMyContainer->myContainingLayout())
	{
		pMyContainer = pMyContainer->myContainingLayout();
	}
	return const_cast<fl_DocSectionLayout *>(static_cast<const fl_DocSectionLayout *>(pMyContainer))->getDocLayout();
}

void fl_ContainerLayout::setContainingLayout(fl_ContainerLayout * pL)
{
	m_pMyLayout = pL;
}

fl_ContainerLayout * fl_ContainerLayout::append(PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP,fl_ContainerType iType)
{
	return insert(sdh, m_pLastL, indexAP,iType);
}

void fl_ContainerLayout::add(fl_ContainerLayout* pL)
{
	if (m_pLastL)
	{
		UT_ASSERT(m_pLastL->getNext() == NULL);

		pL->setNext(NULL);
		pL->setPrev(m_pLastL);
		m_pLastL->setNext(pL);
		m_pLastL = pL;
	}
	else
	{
		UT_ASSERT(!m_pFirstL);
		UT_DEBUGMSG(("add: doing First = Last = NULL \n"));
		pL->setNext(NULL);
		pL->setPrev(NULL);
		m_pFirstL = pL;
		m_pLastL = m_pFirstL;
	}
	pL->setContainingLayout(this);
	if(pL->getContainerType() == FL_CONTAINER_BLOCK)
	{
		UT_ASSERT(getContainerType() != FL_CONTAINER_BLOCK);
		static_cast<fl_BlockLayout *>(pL)->setSectionLayout(static_cast<fl_SectionLayout *>(this));
	}
}

/*!
 * Set the pointer to the first Layout in the linked list.
 */
void fl_ContainerLayout::setFirstLayout(fl_ContainerLayout * pL)
{
	m_pFirstL = pL;
}

/*!
 * Set the pointer to the last Layout in the linked list.
 */
void fl_ContainerLayout::setLastLayout(fl_ContainerLayout * pL)
{
	m_pLastL = pL;
}

/*!
 * Return the pointer to the first layout in the structure.
 */
fl_ContainerLayout * fl_ContainerLayout::getFirstLayout(void) const
{
	return m_pFirstL;
}

/*!
 * Return the pointer to the last layout in the structure.
 */
fl_ContainerLayout * fl_ContainerLayout::getLastLayout(void) const
{
	return m_pLastL;
}

/*!
 * Return a pointer to a line breaker class. Gnerate a new one
 * if the class doesn't yet exist.
 */
fb_LineBreaker * fl_ContainerLayout::getLineBreaker(void)
{
	if (!m_pLB)
	{
		fb_LineBreaker* slb = new fb_LineBreaker();

		m_pLB = slb;
	}

	UT_ASSERT(m_pLB);

	return m_pLB;
}

/*!
 * Create a new containerLayout  and insert it into the linked list of
 * layouts held by this class.
 * Returns a pointer to the generated ContainerLayout class.
 */
fl_ContainerLayout * fl_ContainerLayout::insert(PL_StruxDocHandle sdh, fl_ContainerLayout * pPrev, PT_AttrPropIndex indexAP,fl_ContainerType iType)
{
	fl_ContainerLayout* pL=NULL;
	switch (iType)
	{
	case FL_CONTAINER_BLOCK:
		if(getContainerType() ==  FL_CONTAINER_HDRFTR)
		{
			pL = static_cast<fl_ContainerLayout *>(new fl_BlockLayout(sdh, getLineBreaker(), static_cast<fl_BlockLayout *>(pPrev), static_cast<fl_SectionLayout *>(this), indexAP,true));
			if (pPrev)
				pPrev->_insertIntoList(pL);
			else
			{ 
				pL->setNext(getFirstLayout()); 
				if (getFirstLayout()) getFirstLayout()->setPrev(pL); 
			}
		}
		else if ((pPrev!= NULL) && (pPrev->getContainerType() == FL_CONTAINER_TABLE))
		{
			pL = static_cast<fl_ContainerLayout *>(new fl_BlockLayout(sdh, getLineBreaker(), static_cast<fl_BlockLayout *>(pPrev), static_cast<fl_SectionLayout *>(pPrev->myContainingLayout()), indexAP));
			pPrev->_insertIntoList(pL);
		}
		else
		{
			pL = static_cast<fl_ContainerLayout *>(new fl_BlockLayout(sdh, getLineBreaker(), static_cast<fl_BlockLayout *>(pPrev), static_cast<fl_SectionLayout *>(this), indexAP));
			if (pPrev)
				pPrev->_insertIntoList(pL);
			else
			{ 
				pL->setNext(getFirstLayout()); 
				if (getFirstLayout()) getFirstLayout()->setPrev(pL); 
			}
		}
		break;
	case FL_CONTAINER_TABLE:
		pL = static_cast<fl_ContainerLayout *>(new fl_TableLayout(getDocLayout(),sdh, indexAP, this));
		if (pPrev)
			pPrev->_insertIntoList(pL);
//
// Now put the Physical Container into the vertical container that contains it.
//
		{
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(static_cast<fl_TableLayout *>(pL)->getLastContainer());
			static_cast<fl_TableLayout *>(pL)->insertTableContainer(static_cast<fp_TableContainer *>(pTab));
		}
		break;
	case FL_CONTAINER_CELL:
		pL = static_cast<fl_ContainerLayout *>(new fl_CellLayout(getDocLayout(),sdh, indexAP, this));
		if (pPrev)
			pPrev->_insertIntoList(pL);
		break;
	case FL_CONTAINER_FOOTNOTE:
	{
		fl_DocSectionLayout * pDSL = getDocSectionLayout();
		pL = static_cast<fl_ContainerLayout *>(new fl_FootnoteLayout(getDocLayout(), 
					  pDSL, 
					  sdh, indexAP, this));
		if (pPrev)
			pPrev->_insertIntoList(pL);
		break;
	}
	case FL_CONTAINER_ENDNOTE:
	{
		fl_DocSectionLayout * pDSL = getDocSectionLayout();
		pL = static_cast<fl_ContainerLayout *>(new fl_EndnoteLayout(getDocLayout(), 
					  pDSL, 
					  sdh, indexAP, this));
		if (pPrev)
			pPrev->_insertIntoList(pL);
		break;
	}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (pL == NULL)
	{
		return pL;
	}

	if (!m_pLastL)
	{
		UT_ASSERT(!m_pFirstL);
		m_pFirstL = pL;
		m_pLastL = pL;
	}
	else if (m_pLastL == pPrev)
	{
		m_pLastL = pL;
	}
	else if (!pPrev)
	{
		m_pFirstL = pL;
	}
	if(getContainerType() == FL_CONTAINER_CELL)
	{
		static_cast<fl_TableLayout *>(myContainingLayout())->setDirty();
	}
	return pL;
}

/*! 
   Inserts pL into the containment hierarchy after 'this'.
   This is actually a general linked list insertion routine.
*/
void fl_ContainerLayout::_insertIntoList(fl_ContainerLayout * pL)
{
	fl_ContainerLayout * pNext = getNext();
	setNext(pL);

	pL->setPrev(this);
	pL->setNext(pNext);

	if(pNext)
		pNext->setPrev(pL);
}

/*!
 * Remove a containerLayout class from the linked list held here.
 */
void fl_ContainerLayout::remove(fl_ContainerLayout * pL)
{
	UT_ASSERT(pL);
	UT_ASSERT(m_pFirstL);

	if (pL->getPrev())
	{
		pL->getPrev()->setNext(pL->getNext());
	}

	if (pL->getNext())
	{
		pL->getNext()->setPrev(pL->getPrev());
		if(pL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			UT_ASSERT(getContainerType() != FL_CONTAINER_BLOCK);
			static_cast<fl_BlockLayout *>(pL)->transferListFlags();
		}
	}

	if (pL == m_pFirstL)
	{
		m_pFirstL = m_pFirstL->getNext();
		if (!m_pFirstL)
		{
			m_pLastL = NULL;
		}
	}

	if (pL == m_pLastL)
	{
		m_pLastL = m_pLastL->getPrev();
		if (!m_pLastL)
		{
			m_pFirstL = NULL;
		}
	}

	pL->setNext(NULL);
	pL->setPrev(NULL);
	pL->setContainingLayout(NULL);
	if(pL->getContainerType() == FL_CONTAINER_BLOCK)
	{
		UT_ASSERT(getContainerType() != FL_CONTAINER_BLOCK);
		static_cast<fl_BlockLayout *>(pL)->setSectionLayout(NULL);
	}
}

fp_Container* fl_ContainerLayout::getFirstContainer() const
{
	return m_pFirstContainer;
}

fp_Container* fl_ContainerLayout::getLastContainer() const
{
	return m_pLastContainer;
}

void fl_ContainerLayout::setFirstContainer(fp_Container * pCon)
{
	m_pFirstContainer = pCon;
}

void fl_ContainerLayout::setLastContainer(fp_Container * pCon)
{
	m_pLastContainer = pCon;
}

fp_Run * fl_ContainerLayout::getFirstRun(void) const
{
	if(getContainerType() == FL_CONTAINER_BLOCK)
	{
		const fl_BlockLayout * pBL = static_cast<const fl_BlockLayout *>(this);
		return pBL->getFirstRun();
	}
	else if(getFirstLayout() == NULL)
	{
		return NULL;
	}
	return getFirstLayout()->getFirstRun();
}

/*!
 Get Container's position in document
 \param bActualContainerPos When true return block's position. When false
						return position of first run in block
 \return Position of Container (or first run in block)
 \fixme Split in two functions if called most often with FALSE
*/
UT_uint32 fl_ContainerLayout::getPosition(bool bActualBlockPos) const
{
	const fl_ContainerLayout * pL = this;
    if(!bActualBlockPos)
	{
		while(pL->getContainerType() != FL_CONTAINER_BLOCK && pL->getFirstLayout())
		{
			pL = pL->getFirstLayout();
		}
		if(pL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			const fl_BlockLayout * pBL = static_cast<const fl_BlockLayout *>(pL);
			return pBL->getPosition(bActualBlockPos);
		}
		return 0;
	}
	PT_DocPosition pos = getDocLayout()->getDocument()->getStruxPosition(m_sdh);
	return pos;
}

fl_HdrFtrSectionLayout*	fl_ContainerLayout::getHdrFtrSectionLayout(void) const
{
	if(getContainerType() != FL_CONTAINER_SHADOW)
	{
		return NULL;
	}
	const fl_HdrFtrShadow * pHFS = static_cast<const fl_HdrFtrShadow * >(this);
	return pHFS->getHdrFtrSectionLayout();
}

bool fl_ContainerLayout::canContainPoint() const
{
	if(isCollapsed())
		return false;

	FV_View* pView = getDocLayout()->getView();
	bool bShowHidden = pView->getShowPara();

	bool bHidden = ((m_eHidden == FP_HIDDEN_TEXT && !bShowHidden)
	              || m_eHidden == FP_HIDDEN_REVISION
		          || m_eHidden == FP_HIDDEN_REVISION_AND_TEXT);

	if(bHidden)
		return false;
	else
		return _canContainPoint();
}

bool fl_ContainerLayout::isOnScreen() const
{
	// we check if any of our containers is on screen
	// however, we will not call fp_Container::isOnScreen() to avoid
	// unnecessary overhead

	if(isCollapsed())
		return false;

	UT_return_val_if_fail(getDocLayout(),false);
	
	FV_View *pView = getDocLayout()->getView();

	bool bShowHidden = pView && pView->getShowPara();

	bool bHidden = ((m_eHidden == FP_HIDDEN_TEXT && !bShowHidden)
	              || m_eHidden == FP_HIDDEN_REVISION
		          || m_eHidden == FP_HIDDEN_REVISION_AND_TEXT);


	if(bHidden)
		return false;
	
	UT_Vector vRect;
	UT_Vector vPages;

	pView->getVisibleDocumentPagesAndRectangles(vRect, vPages);

	UT_uint32 iCount = vPages.getItemCount();

	if(!iCount)
		return false;
	
	bool bRet = false;
	fp_Container * pC = getFirstContainer();

	if(!pC)
		return false;

	fp_Container *pCEnd = getLastContainer();

	while(pC)
	{
		fp_Page * pMyPage = pC->getPage();

		if(pMyPage)
		{
			for(UT_uint32 i = 0; i < iCount; i++)
			{
				fp_Page * pPage = static_cast<fp_Page*>(vPages.getNthItem(i));

				if(pPage == pMyPage)
				{
					UT_Rect r;
					UT_Rect *pR = static_cast<UT_Rect*>(vRect.getNthItem(i));

					if(!pC->getPageRelativeOffsets(r))
						break;
				
					bRet = r.intersectsRect(pR);
					break;
				}
		
			}
		}

		if(bRet || pC == pCEnd)
			break;

		pC = static_cast<fp_Container*>(pC->getNext());
	}
	
	UT_VECTOR_PURGEALL(UT_Rect*,vRect);
	return bRet;
}

