/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
#include <stdio.h>
#include <string.h>

#include "fl_AutoNum.h"
#include "fl_Layout.h"
#include "fl_BlockLayout.h"
#include "fp_Run.h"
#include "fp_Line.h"
#include "pd_Document.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "ut_string_class.h"

#define CONV_TO_UCS (UT_UCSChar) (unsigned char)

class pf_Frag;

fl_AutoNum::fl_AutoNum(	UT_uint32 id,
						UT_uint32 start,
						PL_StruxDocHandle pFirst,
						fl_AutoNum * pParent,
						const XML_Char * lDelim,
						const XML_Char * lDecimal,
						List_Type lType,
						PD_Document * pDoc)
	:	m_pParent(pParent),
		m_pDoc(pDoc),
		m_List_Type(lType),
		m_iID(id),
		m_iParentID(0),
		m_iLevel(pParent ? pParent->getLevel() + 1 : 1),
		m_iStartValue(start),
		m_iAsciiOffset(0),
		m_bUpdatingItems(false),
		m_bDirty(false),
		m_ioffset(0),
		m_bWordMultiStyle(true),
		m_pParentItem(0)
{
	UT_uint32 i;
	i =  UT_XML_strncpy( m_pszDelim, 80, lDelim);
	i =  UT_XML_strncpy( m_pszDecimal, 80, lDecimal);

 	addItem(pFirst);	

	m_pDoc->addList(this);
	// New 6/11/2000. m_pParentItem is the item in the parent list
	// that the new list points
}

fl_AutoNum::fl_AutoNum(	UT_uint32 id,
						UT_uint32 parent_id,
						List_Type lType,
						UT_uint32 start,
						const XML_Char * lDelim,
						const XML_Char * lDecimal,
						PD_Document * pDoc)
	:	m_pParent(0),
		m_pDoc(pDoc),
		m_List_Type(lType),
		m_iID(id),
		m_iParentID(parent_id),
		m_iLevel(1),
		m_iStartValue(start),
		m_iAsciiOffset(0),
		m_bUpdatingItems(false),
		m_bDirty(false),
		m_ioffset(0),
		m_bWordMultiStyle(true),
		m_pParentItem(0)
{
	// Set in Block???
	UT_XML_strncpy( m_pszDelim, 80, lDelim);
	UT_XML_strncpy( m_pszDecimal, 80, lDecimal);
	if(m_iParentID != 0)
	{
		m_pParent = m_pDoc->getListByID(parent_id);
		if(m_pParent != NULL)
			m_iLevel = m_pParent->getLevel() + 1;
	}
}


void fl_AutoNum::addItem(PL_StruxDocHandle pItem)
{
    UT_sint32 i = m_pItems.findItem(const_cast<void *>(pItem));
    if(i < 0 )
    {
		m_pItems.addItem(const_cast<void *>(pItem));
    }
    m_bDirty = true;
}

void fl_AutoNum::fixHierarchy(PD_Document * pDoc)
{
	fl_AutoNum * pParent;
	
	if (m_iParentID != 0)
		pParent = pDoc->getListByID(m_iParentID);
	// TODO Add error checking?
	else
		pParent = NULL;

	m_pParent = pParent;

	if (m_pParent)
		m_iLevel = m_pParent->getLevel() + 1;
	else
		m_iLevel = 1;
	m_bDirty = true;
}

fl_AutoNum::~fl_AutoNum()
{
	if (m_pParent && m_pParent->isEmpty())
		DELETEP(m_pParent);
}

static PD_Document * pCurDoc;

static int compareListItems(const void* p1, const void* p2)
{
	//
	// Fun with (void *) pointers!
	// 
	//	pf_Frag * pf1 = (pf_Frag *) p1;
	//	PD_Document * pDoc = pf1->getPieceTable()->getDocument();
	PL_StruxDocHandle sdh1 = (PL_StruxDocHandle) p1;
	PL_StruxDocHandle sdh2 = (PL_StruxDocHandle) p2;
	PT_DocPosition pos1 = pCurDoc->getStruxPosition(sdh1);
	PT_DocPosition pos2 = pCurDoc->getStruxPosition(sdh2);
	if(pos1 < pos2)
	{
		return -1;
	}
	if(pos1 > pos2)
	{
		return 1;
	}
	return 0;
}

void    fl_AutoNum::fixListOrder(void)
{
	pCurDoc = m_pDoc;
	m_pItems.qsort(compareListItems);
	m_bDirty = true;
}

void    fl_AutoNum::markAsDirty(void)
{
	m_bDirty = true;
}

void    fl_AutoNum::findAndSetParentItem(void)
{
	if(m_iParentID == 0)
		return;
	else if( m_pParent == NULL)
	{
		m_pParent = m_pDoc->getListByID(m_iParentID);
	}
	if(m_pParent == NULL)
		return;
	//	fixListOrder();
	//	m_pParent->fixListOrder();
	//	m_pParent->update(0);

	if (m_pItems.getItemCount() == 0)
	{
		return;
	}
	PL_StruxDocHandle pCurFirst =  (PL_StruxDocHandle) m_pItems.getFirstItem();
	if(pCurFirst == NULL)
		return;
	PT_DocPosition posCur = m_pDoc->getStruxPosition(pCurFirst);

	UT_uint32 cnt = m_pDoc->getListsCount();
	UT_ASSERT(cnt);
	UT_uint32 iList;
	fl_AutoNum * pClosestAuto = NULL;
	PT_DocPosition posClosest = 0;
	PL_StruxDocHandle pClosestItem = NULL;
	for(iList = 0; iList < cnt; iList++) 
	{
		fl_AutoNum * pParent = m_pDoc->getNthList(iList);
		UT_uint32 i=0;
		PL_StruxDocHandle pParentItem = pParent->getNthBlock(i);
		PT_DocPosition posParent=0;
		if(pParentItem != NULL)
		{
			posParent = m_pDoc->getStruxPosition(pParentItem);
		}
		while(pParentItem != NULL && (posParent < posCur))
		{
			i++;
			pParentItem = pParent->getNthBlock(i);
			if(pParentItem != NULL)
			{
				posParent = m_pDoc->getStruxPosition(pParentItem);
			}
		}
		if( i > 0)
		{
			i--;
			pParentItem = pParent->getNthBlock(i);;
			posParent = m_pDoc->getStruxPosition(pParentItem);
			if( posParent > posClosest)
			{
				posClosest = posParent;
				pClosestAuto = pParent;
				pClosestItem = pParentItem;
			}
		}
	}
	if(m_pParentItem != pClosestItem)
		m_bDirty = true;
	m_pParentItem = pClosestItem;
	if(m_pParent != pClosestAuto)
		m_bDirty = true;
	m_pParent = pClosestAuto;
	if(m_pParent != NULL)
	{
		m_iParentID = m_pParent->getID();
		m_iLevel = m_pParent->getLevel()+ 1;
		//
		// TODO: change all the para attributes in the list to reflect
		// this change of Parent ID and Level.
	}
	else
	{
		//	        m_iParentID = 0;
		m_iLevel = 1;
	}
	if(m_bDirty == true)
		update(0);
}

void    fl_AutoNum::_getLabelstr( UT_UCSChar labelStr[], UT_uint32 * insPoint, 
								  UT_uint32 depth, PL_StruxDocHandle pLayout)  
{
	// This method recursively calculates a label based on the type of label
	// of the AutoNum Class. This is output to the label string labelStr.
	//
	// insPoint is the position in the string where the new text goes. It starts
	// Pointing to the byte == 0
	// depth is the level of recursion
	// pLayout is a pointer to the Layout item containing the current list item
	//
    char p[100], leftDelim[10], rightDelim[10];
	UT_uint32 i,psz;
	//
	// Don't get the next level if we don't have a list
	//
	if(m_List_Type == NOT_A_LIST)
	{
		*insPoint = 0;
		return;
	}
//	if(depth > 0 && m_List_Type >= BULLETED_LIST)
//	{
//		*insPoint = 0;
//		return;
//	}

	// TODO This is a bit of a hack to split the delim string. It would be 
	// TODO nice to clear it up.

	sprintf(p, "%s", m_pszDelim);
	UT_uint32 rTmp;
	
	i = 0;

	while (p[i] && p[i] != '%' && p[i+1] != 'L')
	{
		leftDelim[i] = p[i];
		i++;
	}
	leftDelim[i] = '\0';
	i += 2;
	rTmp = i;
	while (p[i] || p[i] != '\0')
	{
		rightDelim[i - rTmp] = p[i];
		i++;
	}
	rightDelim[i - rTmp] = '\0';
	//UT_DEBUGMSG(("Left Delim: %s, Right Delim: %s\n", leftDelim, rightDelim));
	
	if(m_pParent != NULL  && m_List_Type < BULLETED_LIST)
	{
		m_pParent->_getLabelstr( labelStr, insPoint, depth+1,getParentItem());
		if(*insPoint != 0)
		{
			psz = UT_XML_strlen(m_pszDecimal);
			for(i=0; i<=psz;i++)
			{
				labelStr[(*insPoint)++] = CONV_TO_UCS m_pszDecimal[i];
			}
			(*insPoint)--;
		}	       
	}
	
	UT_sint32 place = getPositionInList(pLayout,depth);
	if(place == -1)
	{
		//	       UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		labelStr[0] = NULL;
		(*insPoint) = 0;
		return;
	}
	place = place + m_iStartValue;
 
	//	if (depth == 0 )
	if( m_List_Type < BULLETED_LIST)
	{
		psz = UT_XML_strlen(leftDelim);
		for (i = 0; i < psz; i++)
		{
			labelStr[(*insPoint)++] = CONV_TO_UCS leftDelim[i];
		}
	}
		
	switch( m_List_Type)
	{ 
	case NUMBERED_LIST:
		sprintf(p,"%i",place);
		psz = UT_XML_strlen( p);
		for(i=0; i<psz; i++)
		{
			labelStr[(*insPoint)++] =  CONV_TO_UCS p[i];
		}
		break;

	case UPPERCASE_LIST:
		sprintf(p,"%s",dec2ascii(place - 1, 65));
		psz = UT_XML_strlen( p);
		for(i=0; i<psz; i++)
		{
			labelStr[(*insPoint)++] =  CONV_TO_UCS p[i];
		}
		break;

	case LOWERCASE_LIST:
		sprintf(p,"%s",dec2ascii(place - 1, 97));
		psz = UT_XML_strlen( p);
		for(i=0; i<psz; i++)
		{
			labelStr[(*insPoint)++] =  CONV_TO_UCS p[i];
		}
		break;

	case UPPERROMAN_LIST:
		sprintf(p,"%s",dec2roman(place,false));
		psz = UT_XML_strlen( p);
		for(i=0; i<psz; i++)
		{
			labelStr[(*insPoint)++] =  CONV_TO_UCS p[i];
		}
		break;

	case LOWERROMAN_LIST:
		sprintf(p,"%s",dec2roman(place,true));
		psz = UT_XML_strlen( p);
		for(i=0; i<psz; i++)
		{
			labelStr[(*insPoint)++] =  CONV_TO_UCS p[i];
		}
		break;

	case BULLETED_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0xb7; // was UCS_BULLET;
		break;

	case DASHED_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS '-';
		break;

	case SQUARE_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0x6E;
		break;

	case TRIANGLE_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0x73;
		break;

	case DIAMOND_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0xA9;
		break;

	case STAR_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0x53;
		break;

	case IMPLIES_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0xDE;
		break;

	case TICK_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0x33;
		break;

	case BOX_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0x72;
		break;

	case HAND_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0x2B;
		break;

	case HEART_LIST:
		labelStr[(*insPoint)++] =  CONV_TO_UCS 0xAA;
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	
	if( m_List_Type < BULLETED_LIST && 
	    (UT_XML_strnicmp(m_pszDecimal,rightDelim,4) != 0 || depth == 0) )
	{
		psz = UT_XML_strlen(rightDelim);
		for (i = 0; i < psz; i++)
		{
			labelStr[(*insPoint)++] =  CONV_TO_UCS rightDelim[i];
		}
	}
	labelStr[(*insPoint)] = NULL;
	return;
}

const UT_UCSChar * fl_AutoNum::getLabel(PL_StruxDocHandle pItem) 
{
	static UT_UCSChar label[100];
	UT_uint32  insPoint=0;
	UT_uint32 depth;
	depth = 0;
	_getLabelstr( label, &insPoint, depth , pItem);
	if(insPoint == 0 )
	{
		return (const UT_UCSChar *) NULL;
	}
	else
	{
		return (const UT_UCSChar *) label;
	}
}

UT_uint32 fl_AutoNum::getValue(PL_StruxDocHandle pItem) 
{
	return getPositionInList(pItem,0) + m_iStartValue;
}


void fl_AutoNum::setListType(List_Type lType)
{
	m_List_Type = lType;
}

bool fl_AutoNum::isDirty(void)
{
	return m_bDirty;
}

void fl_AutoNum::setDelim(const XML_Char * lDelim)
{
	UT_uint32 i;
	i =  UT_XML_strncpy( m_pszDelim, 80, lDelim);
	m_bDirty = true;
}

const XML_Char * fl_AutoNum::getDelim( void)
{
	return m_pszDelim;
}


const XML_Char * fl_AutoNum::getDecimal( void)
{
	return m_pszDecimal;
}

void fl_AutoNum::setDecimal(const XML_Char * lDecimal)
{
	UT_uint32 i;
	i =  UT_XML_strncpy( m_pszDecimal, 80, lDecimal);
	m_bDirty = true;
}

List_Type fl_AutoNum::getType(void)
{
	return m_List_Type;
}

void fl_AutoNum::setStartValue(UT_uint32 start)
{
	m_iStartValue = start;
	m_bDirty = true;
	_updateItems(0,NULL);
}

void fl_AutoNum::setAsciiOffset(UT_uint32 new_asciioffset)
{
	m_iAsciiOffset = (UT_uint16) new_asciioffset;
	m_bDirty = true;
}

UT_uint32 fl_AutoNum::getStartValue32(void)
{
	return m_iStartValue;
}

void fl_AutoNum::insertFirstItem(PL_StruxDocHandle pItem, PL_StruxDocHandle pLast, UT_uint32 depth)
{
	UT_sint32 i = -1; 
	if(m_pItems.getItemCount() > 0)
		i = m_pItems.findItem((void *) pItem);
	if(i < 0)
	{
		m_pItems.insertItemAt((void *) pItem, 0);
		m_bDirty = true;
	}


	if (m_pParent)
	{
		m_pParentItem = pLast;
		m_bDirty = true;
	}
	if ( getAutoNumFromSdh(pItem) == this)
		_updateItems(0,NULL);
}

PL_StruxDocHandle fl_AutoNum::getParentItem(void)
{
	return m_pParentItem;
}


void fl_AutoNum::setParentItem(PL_StruxDocHandle pItem)
{
	m_pParentItem = pItem;
	m_bDirty = true;
}

void fl_AutoNum::insertItem(PL_StruxDocHandle pItem, PL_StruxDocHandle pPrev)
{
	UT_sint32 ndx,i;
	UT_ASSERT(pItem);
	ndx = m_pItems.findItem((void *) pItem);
	if(ndx >= 0)
		return;
	m_bDirty = true;
	ndx = m_pItems.findItem((void *) pPrev) + 1;
	m_pItems.insertItemAt((void *) pItem, ndx);
	if(m_pDoc->areListUpdatesAllowed() == false)
		return;

	// scan through all the lists and update parent pointers
        
	UT_sint32 numlists = m_pDoc->getListsCount();
	for(i=0; i<numlists; i++)
	{
		fl_AutoNum * pAuto = m_pDoc->getNthList(i);
		if( pPrev == pAuto->getParentItem())
		{
			pAuto->setParentItem(pItem);
			pAuto->m_bDirty = true;
			pAuto->_updateItems(0,NULL);
		}
	}

	_updateItems(ndx+1,NULL);
}


void fl_AutoNum::prependItem(PL_StruxDocHandle pItem, PL_StruxDocHandle pNext)
{
	UT_sint32 ndx;
	UT_sint32 i;
	UT_ASSERT(pItem);
	PL_StruxDocHandle pPrev = NULL;
	ndx = m_pItems.findItem((void *) pItem);
	if(ndx >= 0)
		return;
	m_bDirty = true;
	ndx = m_pItems.findItem((void *) pNext);
	if(ndx > 0)
	{
		pPrev = m_pItems.getNthItem(ndx-1);
	}
	m_pItems.insertItemAt((void *) pItem, ndx);
	if(m_pDoc->areListUpdatesAllowed() == false)
		return;
	if(pPrev != NULL)
	{
		// scan through all the lists and update parent pointers
        
		UT_sint32 numlists = m_pDoc->getListsCount();
		for(i=0; i<numlists; i++)
		{
			fl_AutoNum * pAuto = m_pDoc->getNthList(i);
			if( pPrev == pAuto->getParentItem())
			{
				pAuto->setParentItem(pItem);
				pAuto->m_bDirty = true;
				pAuto->_updateItems(0,NULL);
			}
		}
	}
	_updateItems(ndx,NULL);
}

void fl_AutoNum::removeItem(PL_StruxDocHandle pItem)
{
	UT_sint32 ndx = m_pItems.findItem((void *)pItem);
	UT_sint32 i;
	//
	// For multi-views we might have already deleted pItem from the
	// fl_AutoNum 
	//
	//
	UT_ASSERT(ndx != -1);
	if(ndx < 0 )
	{
		m_bDirty = true;
		_updateItems(0,NULL);
	}        
	PL_StruxDocHandle ppItem = NULL;
	if(ndx > 0)
	{
		ppItem =  ( PL_StruxDocHandle) m_pItems.getNthItem(ndx - 1);
	}
	
	m_pItems.deleteNthItem(ndx);
	m_bDirty = true;

	// scan through all the lists and update parent pointers
	
	UT_sint32 numlists = m_pDoc->getListsCount();
	for(i=0; i<numlists; i++)
	{
		fl_AutoNum * pAuto = m_pDoc->getNthList(i);
		if( pItem == pAuto->getParentItem())
		{
			pAuto->setParentItem(ppItem);
			if(ppItem == NULL)
			{
				UT_uint32 level = pAuto->getLevel();
				if(level > 0)
				{
					level = level - 1;
				}
				else
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}
				pAuto->setLevel(level);
				pAuto->setParent(getParent());
				pAuto->m_bDirty = true;
				pAuto->setParentItem(getParentItem());
			}
			if(m_pDoc->areListUpdatesAllowed() == true)
				pAuto->_updateItems(0,NULL);
		}
	}
	_updateItems(ndx,NULL);
}

UT_uint32 fl_AutoNum::getNumLabels(void)
{
	return m_pItems.getItemCount();
}

UT_sint32 fl_AutoNum::getPositionInList(PL_StruxDocHandle pItem, UT_uint32 depth) 
{
	UT_ASSERT(m_pItems.getItemCount() > 0);
	
	PL_StruxDocHandle pTmp;
	UT_uint32 ndx = 0;
	UT_uint32 count = m_pItems.getItemCount();
	bool bOnLevel = true;
	bool bFirstItem = false;
	
	for (UT_uint32 i = 0; i < count; i++)
	{
		pTmp = (PL_StruxDocHandle) m_pItems.getNthItem(i);
		//		bOnLevel = (depth == 0);
		fl_AutoNum * pAuto = getAutoNumFromSdh(pItem);
		bOnLevel = (bool)( pAuto == this);
		bFirstItem = (bool)(pTmp == m_pItems.getFirstItem());
		if (pTmp == pItem)
		{
			if (m_bWordMultiStyle && !bOnLevel && !bFirstItem)
				ndx--;
			return ndx;
		}
		else if (!m_bWordMultiStyle || bOnLevel || bFirstItem)
		{
			ndx++;
		}
	}
	
	return -1;
	// return m_pItems.findItem(pItem);
}

fl_AutoNum * fl_AutoNum::getAutoNumFromSdh(PL_StruxDocHandle sdh)
{
	UT_sint32 i;
	fl_AutoNum * pAuto;
	if(m_pDoc->areListUpdatesAllowed() == false)
	{
		if(isItem(sdh) == false)
		{
			return (fl_AutoNum *) NULL;
		}
		return this;
	}
	UT_sint32 numLists = m_pDoc->getListsCount();
	for(i=0; i<numLists; i++)
	{
		pAuto = m_pDoc->getNthList(i);
		if(pAuto->isItem(sdh))
			break;
	}
	if(i>= numLists)
	{
		return (fl_AutoNum * ) NULL;
	}
	return pAuto;
}

	   

const bool fl_AutoNum::isItem(PL_StruxDocHandle pItem) 
{
	if (m_pItems.findItem((void *)(pItem)) == -1)
		return false;
	else
		return true;
}

const bool fl_AutoNum::isEmpty() 
{
	if (m_pItems.getItemCount() > 0)
		return false;
	else
		return true;
}

PL_StruxDocHandle fl_AutoNum::getFirstItem()
{
	return (PL_StruxDocHandle)
		(m_pItems.getItemCount() ? m_pItems.getFirstItem() : 0);
}


PL_StruxDocHandle fl_AutoNum::getLastItem()
{
	UT_uint32 i = m_pItems.getItemCount();
	if(i == 0 )
		return NULL;
	else
	{
		return (PL_StruxDocHandle) m_pItems.getNthItem(i-1);
	}
}

bool fl_AutoNum::doesItemHaveLabel( fl_BlockLayout * pItem)
{
	fp_Run * pRun = pItem->getFirstRun();
	bool bStop = false;
	while(bStop == false)
	{
		if(pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
			if(pFRun->getFieldType() == FPFIELD_list_label)
			{
				bStop = true;
				return true;
			}
		}
		pRun = pRun->getNext();
		if(pRun == NULL)
		{
			bStop = true;
			return false;
		}
	}
	return false;
}


bool fl_AutoNum::isLastOnLevel(PL_StruxDocHandle pItem) 
{
	UT_sint32 itemloc = m_pItems.findItem((void *) pItem);
	if (itemloc == -1)
		return false;
	if(itemloc == (UT_sint32) (m_pItems.getItemCount() - 1))
		return true;
	else
		return false;
}

fl_AutoNum * fl_AutoNum::getActiveParent(void) 
{
	fl_AutoNum * pAutoNum = m_pParent;

	while (pAutoNum && pAutoNum->isEmpty())
		pAutoNum = pAutoNum->getParent();

	return pAutoNum;
}

void fl_AutoNum::setParent(fl_AutoNum * pParent)
{
	m_bDirty = true;
	m_pParent = pParent;
}

void fl_AutoNum::update(UT_uint32 start)
{
	//UT_DEBUGMSG(("Updating List %d  There are %d items here \n",m_iID,m_pItems.getItemCount()));
	if (isUpdating())
		return;
	//_calculateLabelStr(0);
	_updateItems(start, NULL);
	void * sdh = const_cast<void *>( getFirstItem());
	if (m_pParent && !m_pParent->isUpdating())
	{
		UT_uint32 ndx = m_pParent->m_pItems.findItem(sdh);
		m_pParent->update(ndx + 1);
	}
}

void fl_AutoNum::_updateItems(UT_uint32 start, PL_StruxDocHandle notMe)
{
	//	UT_DEBUGMSG(("Entering _updateItems\n"));
	UT_sint32 j;
	if(m_pDoc->areListUpdatesAllowed() == true)
	{
		UT_sint32 numlists = m_pDoc->getListsCount();
		m_bUpdatingItems = true;
		for (UT_uint32 i = start; i < m_pItems.getItemCount(); i++)
		{
			//       	 	UT_DEBUGMSG(("Entering _updateItems for loop\n"));
			PL_StruxDocHandle pTmp = (PL_StruxDocHandle) m_pItems.getNthItem(i);
			UT_ASSERT(pTmp);
			m_pDoc->listUpdate(pTmp);
			
			// scan through all the lists and update child lists if connected to this item
	
			PL_StruxDocHandle pItem =  (PL_StruxDocHandle) m_pItems.getNthItem(i);  
			for(j=0; j<numlists; j++)
			{
				fl_AutoNum * pAuto = m_pDoc->getNthList(j);
				UT_ASSERT(pAuto);
				if( pItem == pAuto->getParentItem() && pItem != notMe)
				{
					pAuto->_updateItems(0,pItem);
				}
			}
		}
		m_bUpdatingItems = false;
		m_bDirty = false;
	}
}

///
/// Returns true if item is contained or immediately adjacent to the list
///
bool fl_AutoNum::isContainedByList(PL_StruxDocHandle pItem)
{
	PL_StruxDocHandle sdh, sdh_prev,sdh_next;
	PT_DocPosition pos_prev,pos_next,pos;
	bool bret;
	UT_uint32 no_items = m_pItems.getItemCount();
	if(no_items == 0)
		return false;
	sdh = ( PL_StruxDocHandle) m_pItems.getFirstItem();
	bret = m_pDoc->getPrevStruxOfType(sdh,PTX_Block, &sdh_prev);
	if(bret == false)
		sdh_prev = sdh;
	pos_prev = m_pDoc->getStruxPosition(sdh_prev);
	sdh = ( PL_StruxDocHandle) m_pItems.getNthItem(no_items-1);
	bret = m_pDoc->getNextStruxOfType(sdh,PTX_Block, &sdh_next);
	if(bret == false)
		sdh_next = sdh;
	pos_next = m_pDoc->getStruxPosition(sdh_next);
	pos =  m_pDoc->getStruxPosition(pItem);
	if((pos >= pos_prev) && (pos <= pos_next))
		return true;
	return false;
}


PL_StruxDocHandle fl_AutoNum::getNthBlock( UT_uint32 list_num)
{
	if(list_num <0 || list_num >= m_pItems.getItemCount())
		return (PL_StruxDocHandle) NULL;
	else
		return (PL_StruxDocHandle) m_pItems.getNthItem(list_num);
}

PL_StruxDocHandle fl_AutoNum::getPrevInList( PL_StruxDocHandle pItem)
{
	UT_sint32 itemloc = m_pItems.findItem((void *) pItem);
	if (itemloc == -1 || itemloc == 0)
		return NULL;
	return (PL_StruxDocHandle) m_pItems.getNthItem( (UT_uint32) itemloc - 1);
}

inline UT_uint32 fl_AutoNum::_getLevelValue(fl_AutoNum * pAutoNum)
{
	PL_StruxDocHandle pBlock = const_cast<PL_StruxDocHandle>(getFirstItem());
	fl_AutoNum * pCurr = this;

	while (1)
	{
		if (pAutoNum->isItem(pBlock))
		{
			break;
		}
		else
		{
			pCurr = pCurr->getParent();
			pBlock = pCurr->getFirstItem();
		}
	}

	return pAutoNum->getValue(pBlock);
}

char *  fl_AutoNum::dec2roman(UT_sint32 value, bool lower) 
{
	UT_String roman;

	while( value >= 1000 )
	{
		roman += "M";
		value -= 1000;
	}
	if( value >= 900 )
	{
		roman += "CM";
		value -= 900;
	}
	while( value >= 500 )
	{
		roman += "D";
		value -= 500;
	}
	if( value >= 400 )
	{
		roman += "CD";
		value -= 400;
	}
	while( value >= 100 )
	{
		roman += "C";
		value -= 100;
	}
	if( value >= 90 )
	{
		roman += "XC";
		value -= 90;
	}
	while( value >= 50 )
	{
		roman += "L";
		value -= 50;
	}
	if( value >= 40 )
	{
		roman += "XL";
		value -= 40;
	}
	while( value >= 10 )
	{
		roman += "X";
		value -= 10;
	}
	if( value >= 9 )
	{
		roman += "IX";
		value -= 9;
	}
	while( value >= 5 )
	{
		roman += "V";
		value -= 5;
	}
	if( value >= 4 )
	{
		roman += "IV";
		value -= 4;
	}
	while( value > 0 )
	{
		roman += "I";
		value--;
	}

	char * rmn = UT_strdup (roman.c_str());

	if (lower == true) 
	{
		int len;
		len = roman.size();
		while (--len >= 0) 
		{
			UT_sint32 r = (UT_sint32) roman[len];
			if( (r >= (UT_sint32) 'A') && (r <= (UT_sint32) 'Z'))
				r = r + 32;
			rmn[len] = (char) r;
		}
	}

	return rmn;
}

char * fl_AutoNum::dec2ascii(UT_sint32 value, UT_uint32 offset) 
{
	char ascii[30];
	UT_uint32 ndx, count, i;
	
	ascii[0] = '\0';
	ndx = abs(value % 26);
	count = abs(value / 26);
	
	// For now, we do this like Word. A preference would be nice.
	for (i = 0; i <= count; i++)
	{
		ascii[i] = (char)(ndx + offset);
	}
	ascii[i] = '\0';
	
	return UT_strdup(ascii);
}

const char ** fl_AutoNum::getAttributes(void) 
{
	static char  szID[15], szPid[15], szType[5], szStart[5];
	UT_Vector va;
 
	/*       
			 szID = (char *)malloc(10);
			 szPid = (char *)malloc(10);
			 szType = (char *)malloc(5);
			 szStart = (char *)malloc(5);
	*/

	sprintf(szID, "%i", m_iID);
	va.addItem( (void *) "id"); va.addItem( (void *) szID);
        
	if (m_pParent)
		sprintf(szPid, "%i", m_pParent->getID());
	else
		sprintf(szPid, "0");
	va.addItem( (void *) "parentid"); va.addItem( (void *) szPid);
                
	sprintf(szType, "%i", m_List_Type);
	va.addItem( (void *) "type"); va.addItem(szType);
        
	sprintf(szStart, "%i", m_iStartValue);
	va.addItem( (void *) "start-value"); va.addItem(szStart);
	
	va.addItem( (void *) "list-delim"); va.addItem( (void *) m_pszDelim);
	
	va.addItem( (void *) "list-decimal"); va.addItem( (void *) m_pszDecimal);
	
	UT_uint32 counta = va.getItemCount() + 1;
	UT_uint32 i;
	const char ** attribs = (const char **) calloc(counta, sizeof(char *));
	for (i = 0; i < va.getItemCount(); i++)
	{
		attribs[i] = (const char *) va[i];
	}
	return attribs;
}
