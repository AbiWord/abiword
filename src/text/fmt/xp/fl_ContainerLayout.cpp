/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "fl_TOCLayout.h"
#include "fl_TableLayout.h"
#include "fp_TableContainer.h"
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
#include "fl_FrameLayout.h"
#include "fp_FrameContainer.h"
#include "fl_AutoNum.h"


fl_ContainerLayout::fl_ContainerLayout(fl_ContainerLayout* pMyLayout, pf_Frag_Strux* sdh, PT_AttrPropIndex indexAP, PTStruxType iStrux, fl_ContainerType iType)
	: fl_Layout(iStrux, sdh),
	  m_iConType(iType),
	  m_pMyLayout(pMyLayout),
	  m_pPrev(NULL),
	  m_pNext(NULL),
	  m_pFirstL(NULL),
	  m_pLastL(NULL),
	  m_pFirstContainer(NULL),
	  m_pLastContainer(NULL),
	  m_eHidden(FP_VISIBLE),
	  m_iFoldedLevel(0)
{
//	UT_ASSERT(pMyLayout != NULL);
	setAttrPropIndex(indexAP);
	if(pMyLayout)
	{
		m_pDoc = pMyLayout->getDocument();
	}
}

fl_ContainerLayout::~fl_ContainerLayout()
{
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

bool fl_ContainerLayout::_getPropertiesAP(const PP_AttrProp*& pAP)
{
	pAP = NULL;
	FPVisibility eVisibility = getAP(pAP);
	UT_return_val_if_fail(pAP, false);

	setVisibility(eVisibility);
	
	//  Find the folded Level of the strux
	lookupFoldedLevel();
	if((isHidden() == FP_VISIBLE) && (getFoldedLevel() > 0) && (getLevelInList() > getFoldedLevel()) )
	{
		xxx_UT_DEBUGMSG(("Table set to hidden folded \n"));
		setVisibility(FP_HIDDEN_FOLDED);
	}
	// evaluate "display" property
	// display property
	const char* pszDisplay = NULL;
	pAP->getProperty("display", (const gchar *&)pszDisplay);
	if(isHidden() == FP_VISIBLE && pszDisplay && !strcmp(pszDisplay, "none"))
	{
		setVisibility(FP_HIDDEN_TEXT);
	}

	return true;
}

void fl_ContainerLayout::lookupProperties(void)
{
	// first of all, call getAP() which will set default visibility
	// for us (either visible, or hidden revision)

	// other common properties should come here

	// this should only implement class-specific properties ...
	const PP_AttrProp* pAP;

	// assert not needed, since _getPropertiesAP() asserts on failure
	if(!_getPropertiesAP(pAP))
		return;
	
	_lookupProperties(pAP);
}

/*!
    This function looks up only a limited set of properties such as margins.
    It's purpose is to allow to reformat the document when the view mode changes (in
    normal view we cannot allow negative margins; the derrived _lookupMarginProperties()
    needs to handled that in a meaningful way.
*/
void fl_ContainerLayout::lookupMarginProperties(void)
{
	// first of all, call getAP() which will set default visibility
	// for us (either visible, or hidden revision)

	// other common properties should come here

	// this should only implement class-specific properties ...
	const PP_AttrProp* pAP;

	// assert not needed, since _getPropertiesAP() asserts on failure
	if(!_getPropertiesAP(pAP))
		return;
	
	_lookupMarginProperties(pAP);
}


/*!
    retrieves AP associated with this layout, corretly processing any
    revision information;

    /return return value indicates whether the layout is hidden due to
    current revision settings or not
*/
FPVisibility fl_ContainerLayout::getAP(const PP_AttrProp *& pAP)const
{
	FL_DocLayout* pDL =	getDocLayout();
	UT_return_val_if_fail(pDL,FP_VISIBLE);

	FV_View* pView = pDL->getView();
	UT_return_val_if_fail(pView,FP_VISIBLE);

	UT_uint32 iId  = pView->getRevisionLevel();
	bool bShow     = pView->isShowRevisions();
	bool bHiddenRevision = false;

	getAttrProp(&pAP,NULL,bShow,iId,bHiddenRevision);

	if(bHiddenRevision)
	{
		return FP_HIDDEN_REVISION;
	}
	else
	{
		return FP_VISIBLE;
	}
}

void fl_ContainerLayout::getSpanAP(UT_uint32 blockPos, bool bLeft, const PP_AttrProp * &pSpanAP) const
{
	//first we need to ascertain if this revision is visible
	FL_DocLayout* pDL =	getDocLayout();
	UT_return_if_fail(pDL);

	FV_View* pView = pDL->getView();
	UT_return_if_fail(pView);

	UT_uint32 iId  = pView->getRevisionLevel();
	bool bShow     = pView->isShowRevisions();
	bool bHiddenRevision = false;

	getSpanAttrProp(blockPos, bLeft, &pSpanAP,NULL,bShow,iId,bHiddenRevision);
}


const char * fl_ContainerLayout::getContainerString(void)
{
	switch(getContainerType())
	{
	case FL_CONTAINER_BLOCK:
		return "FL_CONTAINER_BLOCK";
	case  FL_CONTAINER_DOCSECTION:
		return "FL_CONTAINER_DOCSECTION";
	case FL_CONTAINER_HDRFTR:
		return "FL_CONTAINER_HDRFTR";
	case FL_CONTAINER_SHADOW:
		return "FL_CONTAINER_SHADOW";
	case FL_CONTAINER_FOOTNOTE:
		return "FL_CONTAINER_FOOTNOTE";
	case FL_CONTAINER_ENDNOTE:
		return "FL_CONTAINER_ENDNOTE";
	case FL_CONTAINER_MARGINNOTE:
		return "FL_CONTAINER_MARGINNOTE";
	case FL_CONTAINER_TABLE:
		return "FL_CONTAINER_TABLE";
	case FL_CONTAINER_CELL:
		return "FL_CONTAINER_CELL";
	case FL_CONTAINER_FRAME:
		return "FL_CONTAINER_FRAME";
	case FL_CONTAINER_TOC:
		return "FL_CONTAINER_TOC";
	case FL_CONTAINER_ANNOTATION:
		return "FL_CONTAINER_ANNOTATION";
	case FL_CONTAINER_RDFANCHOR:
		return "FL_CONTAINER_RDFANCHOR";
	default:
		return "NOT_IMPLEMENTED";
	}
	return "NOT IMPLEMENTED";
}



/*!
 * Return the value of the attribute keyed by pszName
 */
const char*	fl_ContainerLayout::getAttribute(const char * pszName) const
{
	const PP_AttrProp * pAP = NULL;
	getAP(pAP);
	UT_return_val_if_fail(pAP, NULL);
	
	const gchar* pszAtt = NULL;
	pAP->getAttribute(static_cast<const gchar*>(pszName), pszAtt);

	return pszAtt;
}

/*!
 * Return the nested List level of this structure.
 */
UT_sint32 fl_ContainerLayout::getLevelInList(void)
{
      fl_BlockLayout * pBList = NULL;
      if(getContainerType() == FL_CONTAINER_BLOCK)
      {
	   pBList = static_cast<fl_BlockLayout * >(this);
      }
      else
      {
	   pBList = getPrevBlockInDocument();
      }
      UT_sint32 iLevel = 0;
      bool bLoop = true;
      while(pBList && bLoop)
      {
	  while(pBList && !pBList->isListItem())
	  {
	       pBList = pBList->getPrevBlockInDocument();
	  }
	  if(pBList == NULL)
	  {
	       bLoop = false;
	       break;
	  }
	  const PP_AttrProp * pAP = NULL;
	  pBList->getAP(pAP);
	  const gchar * szLid=NULL;
	  UT_uint32 id=0;

	  if (!pAP || !pAP->getAttribute(PT_LISTID_ATTRIBUTE_NAME, szLid))
	       szLid = NULL;
	  if (szLid)
	  {
	       id = atoi(szLid);
		
	  }
	  else
	  {
		id = 0;
	  }
	  if(id == 0)
	  {
	        bLoop = false;
	        break;
	  }
	  PD_Document * pDoc = getDocLayout()->getDocument();
	  fl_AutoNum * pAuto = pDoc->getListByID( id);
	  if(pAuto->getLastItem() == pBList->getStruxDocHandle())
	  {
	        if(pAuto->getLastItem() == getStruxDocHandle())
		{
		     iLevel = pAuto->getLevel();
		     bLoop = false;
		     break;
		}
	        iLevel = pAuto->getLevel() -1;
		if(iLevel < 0)
		{
		      iLevel = 0;
		}
	  }
	  else
	  {
	        if(pBList == this)
	        { 
		      iLevel = pAuto->getLevel();
		}
		else
		{
		      iLevel = pAuto->getLevel() + 1;
		}
	  }
	  bLoop = false;
	  break;
      }
      return iLevel;
}
/*!
 * This method returns the folded level of the text.
 */
UT_sint32 fl_ContainerLayout::getFoldedLevel(void)
{
	return m_iFoldedLevel;
}


/*!
 * This method returns the ID of the list that is folded.
 */
UT_uint32 fl_ContainerLayout::getFoldedID(void)
{
	return m_iFoldedID;
}

/*!
 * This Method looks up the folded level of the strux.
 */
void fl_ContainerLayout::lookupFoldedLevel(void)
{
 	const PP_AttrProp* pSectionAP = NULL;

	getAP(pSectionAP);

	const gchar *pszTEXTFOLDED = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("text-folded",pszTEXTFOLDED))
	{
		m_iFoldedLevel = 0;
	}
	else
	{
		m_iFoldedLevel = atoi(pszTEXTFOLDED);
	}
	xxx_UT_DEBUGMSG(("FOlded Level is %d \n",m_iFoldedLevel));
    pszTEXTFOLDED = NULL;
	if(!pSectionAP || !pSectionAP->getProperty("text-folded-id",pszTEXTFOLDED))
	{
		m_iFoldedID = 0;
	}
	else
	{
		m_iFoldedID = atoi(pszTEXTFOLDED);
	}
}

/*!
 * This method appends all the text in the current layout to the supplied 
 * GrowBuf.
 */
void fl_ContainerLayout::appendTextToBuf(UT_GrowBuf & buf) const
{
	if(getContainerType() == FL_CONTAINER_BLOCK)
	{
		const fl_BlockLayout * pBL = static_cast<const fl_BlockLayout *>(this);
		pBL->appendTextToBuf(buf);
		return;
	}
	const fl_ContainerLayout * pCL = getFirstLayout();
	while(pCL)
	{
		pCL->appendTextToBuf(buf);
		pCL = pCL->getNext();
	}
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

fl_DocSectionLayout * fl_ContainerLayout::getDocSectionLayout(void) const
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

/*!
 * If this container is contained by a HdrFtrSectionLayout return it. 
 * Otherwise return
 * NULL.
 */
fl_HdrFtrSectionLayout * fl_ContainerLayout::getHdrFtrLayout(void)
{
	fl_ContainerLayout * pCL = this;
	while(pCL && (pCL->getContainerType() != FL_CONTAINER_HDRFTR) && 
		  (pCL->getContainerType() != FL_CONTAINER_DOCSECTION))
	{
		pCL = pCL->myContainingLayout();
	}
	if(pCL && (pCL->getContainerType() ==  FL_CONTAINER_HDRFTR))
	{
		return static_cast<fl_HdrFtrSectionLayout *>(pCL);
	}
	return NULL;
}

FL_DocLayout* fl_ContainerLayout::getDocLayout(void) const
{
	const fl_ContainerLayout * pMyContainer = static_cast<const fl_ContainerLayout *>(this);
	while(pMyContainer->getContainerType() != FL_CONTAINER_DOCSECTION && pMyContainer->myContainingLayout())
	{
		pMyContainer = pMyContainer->myContainingLayout();
	}
	return static_cast<const fl_DocSectionLayout *>(pMyContainer)->getDocLayout();
}

void fl_ContainerLayout::setContainingLayout(fl_ContainerLayout * pL)
{
//	UT_ASSERT(pL != NULL); // useful for debugging
	m_pMyLayout = pL;
}

fl_ContainerLayout * fl_ContainerLayout::append(pf_Frag_Strux* sdh, PT_AttrPropIndex indexAP,fl_ContainerType iType)
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


fl_BlockLayout* fl_ContainerLayout::getNextBlockInDocument(void) const
{
	fl_ContainerLayout * pNext = getNext();
	if(getContainerType() != FL_CONTAINER_BLOCK)
	{
	  pNext = getFirstLayout();
	}
	fl_ContainerLayout * pOld = NULL;
	UT_uint32 depth = 0;
	next_is_null :
	if(pNext == NULL)
	{
		while((pNext == NULL) && ((pOld != NULL) || (depth == 0)))
	    {
			fl_ContainerLayout * pPrevOld = pOld;
			if(depth > 0)
			{
				pOld = pOld->myContainingLayout();
			}
			else
			{
				pOld = myContainingLayout();
			}
			depth++;
			if(pOld != NULL) // HdrFtr's have myContainingLayout == NULL
			{
				pNext = pOld->getNext();
			}
			if(pPrevOld == pOld)
			{
				pOld = NULL;
			}
		}
	}
	while(pNext)
	{
		pOld = pNext;
		if(pNext->getContainerType() == FL_CONTAINER_BLOCK)
		{
			return static_cast<fl_BlockLayout *>(pNext);
		}
		else if(pNext->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			pNext = pNext->getFirstLayout();
		}
		else if(pNext->getContainerType() == FL_CONTAINER_TABLE)
		{
			pNext = pNext->getFirstLayout();
		}
		else if(pNext->getContainerType() == FL_CONTAINER_FRAME)
		{
			if(pNext->getFirstLayout() == NULL)
			{
			     pNext = pNext->getNext();
			}
			else
			{
			     pNext = pNext->getFirstLayout();
			}
		}
		else if(pNext->getContainerType() == FL_CONTAINER_CELL)
		{
			pNext = pNext->getFirstLayout();
		}
		else if(pNext->getContainerType() == FL_CONTAINER_TOC)
		{
			pNext = pNext->getNext();
			if(pNext == NULL)
			{
				goto next_is_null;
			}
		}
		else if(pNext->getContainerType() == FL_CONTAINER_FOOTNOTE)
		{
			pNext = pNext->getNext();
			if(pNext == NULL)
			{
				goto next_is_null;
			}
		}
		else if(pNext->getContainerType() == FL_CONTAINER_ANNOTATION)
		{
			pNext = pNext->getNext();
			if(pNext == NULL)
			{
				goto next_is_null;
			}
		}
		else if(pNext->getContainerType() == FL_CONTAINER_RDFANCHOR)
		{
			pNext = pNext->getNext();
			if(pNext == NULL)
			{
				goto next_is_null;
			}
		}
		else if(pNext->getContainerType() == FL_CONTAINER_ENDNOTE)
		{
			pNext = pNext->getNext();
			if(pNext == NULL)
			{
				goto next_is_null;
			}
		}
		else
		{
			pNext = NULL;
			break;
		}
		if(pNext == NULL)
		{
				goto next_is_null;
		}
	}
	return NULL;
}

fl_BlockLayout* fl_ContainerLayout::getPrevBlockInDocument(void) const
{
	fl_ContainerLayout * pPrev = getPrev();
	fl_ContainerLayout * pOld = NULL;
	UT_uint32 depth = 0;
	if(pPrev == NULL)
	{
		while((pPrev == NULL) && ((pOld != NULL) || (depth == 0)))
	    {
			fl_ContainerLayout * pPrevOld = pOld;
			if(depth > 0)
			{
				pOld = pOld->myContainingLayout();
			}
			else
			{
				pOld = myContainingLayout();
			}
			depth++;
			if(pOld != NULL) // HdrFtr's can have NULL myContainingLayout's
			{
				pPrev = pOld->getPrev();
			}
			if(pPrevOld == pOld )
			{
				pOld = NULL;
			}
		}
	}
	while(pPrev)
	{
		pOld = pPrev;
		if(pPrev->getContainerType() == FL_CONTAINER_BLOCK)
		{
			return static_cast<fl_BlockLayout *>(pPrev);
		}
		else if(pPrev->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			pPrev = pPrev->getLastLayout();
		}
		else if(pPrev->getContainerType() == FL_CONTAINER_FRAME)
		{
			if(pPrev->getLastLayout() == NULL)
			{
			     pPrev = pPrev->getPrev();
			}
			else
			{
			     pPrev = pPrev->getLastLayout();
			}
		}
		else if(pPrev->getContainerType() == FL_CONTAINER_TABLE)
		{
			pPrev = pPrev->getLastLayout();
		}
		else if(pPrev->getContainerType() == FL_CONTAINER_CELL)
		{
			pPrev = pPrev->getLastLayout();
		}
		else if(pPrev->getContainerType() == FL_CONTAINER_FOOTNOTE)
		{
			pPrev = pPrev->getLastLayout();
		}
		else if(pPrev->getContainerType() == FL_CONTAINER_ANNOTATION)
		{
			pPrev = pPrev->getLastLayout();
		}
		else if(pPrev->getContainerType() == FL_CONTAINER_RDFANCHOR)
		{
			pPrev = pPrev->getLastLayout();
		}
		else if(pPrev->getContainerType() == FL_CONTAINER_TOC)
		{
			pPrev = pPrev->getLastLayout();
		}
		else if(pPrev->getContainerType() == FL_CONTAINER_ENDNOTE)
		{
			pPrev = pPrev->getLastLayout();
		}
		else
		{
			pPrev = NULL;
			break;
		}
		if(pPrev == NULL)
		{
			if(pOld && pOld->myContainingLayout())
			{
				pPrev = pOld->myContainingLayout()->getPrev();
			}
		}
	}
	return NULL;
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
 * Create a new containerLayout  and insert it into the linked list of
 * layouts held by this class.
 * Returns a pointer to the generated ContainerLayout class.
 */
fl_ContainerLayout * fl_ContainerLayout::insert(pf_Frag_Strux* sdh, fl_ContainerLayout * pPrev, PT_AttrPropIndex indexAP,fl_ContainerType iType)
{
	fl_ContainerLayout* pL=NULL;
	switch (iType)
	{
	case FL_CONTAINER_BLOCK:
		// we have a problem here -- the block needs to be in the list before the consturction is completed
		if(getContainerType() ==  FL_CONTAINER_HDRFTR)
		{
			pL = static_cast<fl_ContainerLayout *>(new fl_BlockLayout(sdh, pPrev, static_cast<fl_SectionLayout *>(this), indexAP,true));
		}
		else if ((pPrev!= NULL) && (pPrev->getContainerType() == FL_CONTAINER_TABLE))
		{
			pL = static_cast<fl_ContainerLayout *>(new fl_BlockLayout(sdh,pPrev, static_cast<fl_SectionLayout *>(pPrev->myContainingLayout()), indexAP));
		}
		else if ((pPrev!= NULL) && (pPrev->getContainerType() == FL_CONTAINER_ANNOTATION))
		{
			pL = static_cast<fl_ContainerLayout *>(new fl_BlockLayout(sdh,pPrev, static_cast<fl_SectionLayout *>(this), indexAP));
			fp_Container * pFirstC = pL->getFirstContainer();
			//
			// This sets indent for a annotation label.
			//
			if(pFirstC)
			  pFirstC->recalcMaxWidth(true);
		}
		else if ((pPrev!= NULL) && (pPrev->getContainerType() == FL_CONTAINER_RDFANCHOR))
		{
			pL = static_cast<fl_ContainerLayout *>(new fl_BlockLayout(sdh,pPrev, static_cast<fl_SectionLayout *>(this), indexAP));
		}
		else
		{
			pL = static_cast<fl_ContainerLayout *>(new fl_BlockLayout(sdh, static_cast<fl_BlockLayout *>(pPrev), static_cast<fl_SectionLayout *>(this), indexAP));
		}
		break;
	case FL_CONTAINER_TABLE:
		pL = static_cast<fl_ContainerLayout *>(new fl_TableLayout(getDocLayout(),sdh, indexAP, this));
		if(pPrev && (pPrev == this))
		{
		  fl_ContainerLayout * pOldFirst = pPrev->getFirstLayout();
		  pPrev->setFirstLayout(pL);
		  pL->setNext(pOldFirst);
		  if(pOldFirst)
		  {
		    pOldFirst->setPrev(pL);
		  }
		  if(pPrev->getLastLayout() == NULL)
		  {
		    pPrev->setLastLayout(pL);
		  }
		}
		else if (pPrev)
		{
			pPrev->_insertIntoList(pL);
		}
//
// Now put the Physical Container into the vertical container that contains it.
//
		{
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(static_cast<fl_TableLayout *>(pL)->getLastContainer());
			static_cast<fl_TableLayout *>(pL)->insertTableContainer(static_cast<fp_TableContainer *>(pTab));
		}
		if(getContainerType() == FL_CONTAINER_CELL)
		{
			fl_CellLayout * pCell = static_cast<fl_CellLayout *>(this);
			pCell->incNumNestedTables();
			fl_TableLayout * pTab = static_cast<fl_TableLayout *>(pCell->myContainingLayout());
			pTab->incNumNestedTables();
		}
		break;
	case FL_CONTAINER_CELL:
		pL = static_cast<fl_ContainerLayout *>(new fl_CellLayout(getDocLayout(),sdh, indexAP, this));
		if (pPrev)
		{
			pPrev->_insertIntoList(pL);
		}
		else
		{
			_insertFirst(pL);
		}
		break;
	case FL_CONTAINER_FRAME:
	{
		pL = static_cast<fl_ContainerLayout *>
		  (new fl_FrameLayout(getDocLayout(), 
				      sdh, indexAP, this));
		if (pPrev)
		{
			while(pPrev && pPrev->getContainerType() != FL_CONTAINER_BLOCK)
			{
				pPrev = pPrev->getPrev();
			}
//
// Add the frame to the list in te previous block.
//
			if(pPrev)
			{
				pPrev->_insertIntoList(pL);
				pPrev->addFrame(static_cast<fl_FrameLayout *>(pL));
			}
		}
		break;
	}
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
	case FL_CONTAINER_ANNOTATION:
	{
		fl_DocSectionLayout * pDSL = getDocSectionLayout();
		pL = static_cast<fl_ContainerLayout *>(new fl_AnnotationLayout(getDocLayout(), 
					  pDSL, 
					  sdh, indexAP, this));
		if (pPrev)
			pPrev->_insertIntoList(pL);
		break;
	}
	case FL_CONTAINER_TOC:
	{
		fl_DocSectionLayout * pDSL = getDocSectionLayout();
		pL = static_cast<fl_ContainerLayout *>(new fl_TOCLayout(getDocLayout(), 
					  pDSL, 
					  sdh, indexAP, this));
		if (pPrev)
			pPrev->_insertIntoList(pL);
		static_cast<fl_TOCLayout *>(pL)->getNewContainer(NULL);
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
   Inserts pL into the containment hierarchy at First Location;
*/
void fl_ContainerLayout::_insertFirst(fl_ContainerLayout * pL)
{
	if(m_pFirstL == NULL)
	{
		m_pFirstL = pL;
		pL->setPrev(NULL);
		pL->setNext(NULL);
		m_pLastL = pL;
		return;
	}
	fl_ContainerLayout * pOldFirst = m_pFirstL;
	m_pFirstL = pL;
	pL->setNext(pOldFirst);
	pL->setPrev(NULL);
	pOldFirst->setPrev(pL);
}

/*!
 * Remove a containerLayout class from the linked list held here.
 */
void fl_ContainerLayout::remove(fl_ContainerLayout * pL)
{
	UT_ASSERT(pL);
	UT_ASSERT(m_pFirstL);
	fl_ContainerLayout* prev = pL->getPrev(); // can be NULL

	if (prev)
	{
		prev->setNext(pL->getNext());
	}

	if (pL->getNext())
	{
		pL->getNext()->setPrev(prev);
		if(pL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			UT_ASSERT(getContainerType() != FL_CONTAINER_BLOCK);
			static_cast<fl_BlockLayout *>(pL)->transferListFlags();
		}
		if (pL->getNext()->getContainerType() == FL_CONTAINER_BLOCK)
		{
			fl_BlockLayout* pBNext = static_cast<fl_BlockLayout *>(pL->getNext());
			if (pBNext->hasBorders())
			{
				pBNext->setLineHeightBlockWithBorders(1);
			}
		}
		if (prev && prev->getContainerType() == FL_CONTAINER_BLOCK)
		{
			fl_BlockLayout* pBPrev = static_cast<fl_BlockLayout *>(prev);
			if (pBPrev->hasBorders())
			{
				pBPrev->setLineHeightBlockWithBorders(-1);
			}
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
	if(getContainerType() != FL_CONTAINER_BLOCK)
	{
	  fl_SectionLayout * pSL = static_cast<fl_SectionLayout *>(this);
	  pSL->removeFromUpdate(pL);
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
	if(pCon && getContainerType() == FL_CONTAINER_BLOCK)
	{
		UT_ASSERT(pCon->getContainerType() == FP_CONTAINER_LINE);
	}
	xxx_UT_DEBUGMSG(("Set FirstContainer of %x to %x \n",this,pCon));
	m_pFirstContainer = pCon;
}

void fl_ContainerLayout::setLastContainer(fp_Container * pCon)
{
	xxx_UT_DEBUGMSG(("Set LastContainer of %x to %x \n",this,pCon));
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
	if(!bActualBlockPos && (getContainerType() != FL_CONTAINER_TOC))
	{
		pL = static_cast<fl_ContainerLayout *>(getNextBlockInDocument());
		if(pL == NULL)
		{
		  PT_DocPosition pos = getDocLayout()->getDocument()->getStruxPosition(getStruxDocHandle());
		  return pos;
		}
		if(pL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			const fl_BlockLayout * pBL = static_cast<const fl_BlockLayout *>(pL);
			return pBL->getPosition(bActualBlockPos);
		}
		return 0;
	}
	PT_DocPosition pos = getDocLayout()->getDocument()->getStruxPosition(getStruxDocHandle());
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
	
	if(!_canContainPoint())
		return false;

	// see if we are not inside a containing layout that cannot contain point
	fl_ContainerLayout * pMyLayout = myContainingLayout();

	if(!pMyLayout || pMyLayout->getContainerType() == FL_CONTAINER_DOCSECTION)
		return true;
	
	return pMyLayout->canContainPoint();
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
	
	UT_GenericVector<UT_Rect*> vRect;
	UT_GenericVector<fp_Page*> vPages;

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
				fp_Page * pPage = vPages.getNthItem(i);

				if(pPage == pMyPage)
				{
					UT_Rect r;
					UT_Rect *pR = vRect.getNthItem(i);

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

// Frames stuff

void fl_ContainerLayout::addFrame(fl_FrameLayout * pFrame)
{
	UT_DEBUGMSG(("Adding frame %p to list in container %p \n",pFrame,this));
	UT_sint32 i = m_vecFrames.findItem(pFrame);
	if(i>= 0)
	{
		UT_DEBUGMSG(("Adding already existing frame \n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	m_vecFrames.addItem(pFrame);
	if (!pFrame->getParentContainer())
	{
		pFrame->setParentContainer(this);
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
}

UT_sint32 fl_ContainerLayout::getNumFrames(void) const
{
	return m_vecFrames.getItemCount();
}

fl_FrameLayout * fl_ContainerLayout::getNthFrameLayout(UT_sint32 i) const
{
	if(i> getNumFrames())
	{
		return NULL;
	}
	return m_vecFrames.getNthItem(i);
}


fp_FrameContainer * fl_ContainerLayout::getNthFrameContainer(UT_sint32 i) const
{
	if(i> getNumFrames())
	{
		return NULL;
	}
	fl_FrameLayout * pFrame= m_vecFrames.getNthItem(i);
	fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(pFrame->getFirstContainer());
	return pFC;
}

bool fl_ContainerLayout::removeFrame(fl_FrameLayout * pFrame)
{
	UT_DEBUGMSG(("Remove Frame %p from this container %p \n",pFrame,this));
	UT_sint32 i = m_vecFrames.findItem(pFrame);
	if(i >= 0)
	{
		m_vecFrames.deleteNthItem(i);
		if (pFrame->getParentContainer() == this)
		{
			pFrame->setParentContainer(NULL);
		}
		return true;
	}
	else
	{
		UT_DEBUGMSG((" Requested Frame not found \n"));
		return false;
	}
}


/* This function returns true if the layout contains a footnote layout and false otherwise.
   The function returns false if the layout is contained inside a footnote layout.
   TODO TODO TODO: Move embedded layouts out of the main fl_Container lists
*/

bool fl_ContainerLayout::containsFootnoteLayouts(void) const
{
	if (getEndStruxDocHandle())
	{
		PT_DocPosition posStart = getDocument()->getStruxPosition(getStruxDocHandle());
		PT_DocPosition posEnd = getDocument()->getStruxPosition(getEndStruxDocHandle());
		return getDocument()->hasEmbedStruxOfTypeInRange(posStart,posEnd,PTX_SectionFootnote);
	}
	// This function has not yet been implemented for layouts that do not have a end strux (blocks, sections)
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return false;
}


/* This function returns true if the layout contains a footnote layout and false otherwise.
   The function returns false if the layout is contained inside a footnote layout.
   TODO TODO TODO: Move embedded layouts out of the main fl_Container lists
*/

bool fl_ContainerLayout::containsAnnotationLayouts(void) const
{
	if (getEndStruxDocHandle())
	{
		PT_DocPosition posStart = getDocument()->getStruxPosition(getStruxDocHandle());
		PT_DocPosition posEnd = getDocument()->getStruxPosition(getEndStruxDocHandle());
		return getDocument()->hasEmbedStruxOfTypeInRange(posStart,posEnd,PTX_SectionAnnotation);
	}
	// This function has not yet been implemented for layouts that do not have a end strux (blocks, sections)
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return false;
}

