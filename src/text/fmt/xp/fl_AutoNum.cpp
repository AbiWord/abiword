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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"



fl_AutoNum::fl_AutoNum(UT_uint32 id, UT_uint32 start,  fl_Layout * pFirst, fl_AutoNum * pParent, const XML_Char * lDelim, const XML_Char * lDecimal, List_Type lType)
{
        UT_uint32 i;
	m_iID = id;
	m_iStartValue = start;
	m_iAsciiOffset = 0;
	m_bUpdatingItems = UT_FALSE;
	m_ioffset = 0;
        i =  UT_XML_strncpy( m_pszDelim, 80, lDelim);
	i =  UT_XML_strncpy( m_pszDecimal, 80, lDecimal);
	m_pParent = pParent;
	if (m_pParent) 
	{
		m_iLevel = m_pParent->getLevel() + 1;
	} 
	else 
	{
		m_iLevel = 1;
	}

	m_pItems.addItem(pFirst);	
        m_List_Type = lType;
}

fl_AutoNum::~fl_AutoNum()
{
	if (m_pParent && m_pParent->isEmpty())
		DELETEP(m_pParent);
}


void    fl_AutoNum::_getLabelstr( XML_Char labelStr[], UT_uint32 * insPoint, 
				  UT_uint32 depth, fl_Layout * pLayout) const 
{
  // This method recursively calculates a label based on the type of label
  // of the AutoNum Class. This is output to the label string labelStr.
  //
  // insPoint is the position in the string where the new text goes. It starts
  // Pointing to the byte == 0
  // depth is the level of recursion
  // pLayout is a pointer to the Layout item containing the current list item
  //
        XML_Char p[100];
	UT_uint32 i,psz;
	//
	// Only get the next level if the list type is not bullet or similar
	//
        if(m_List_Type == NOT_A_LIST)
	{
	       *insPoint = 0;
	       return;
	}
        if(m_pParent != NULL  && m_List_Type < BULLETED_LIST)
	{
	       m_pParent->_getLabelstr( labelStr, insPoint, depth+1,getFirstItem());
	       if(*insPoint != 0)
	       {
		        psz = UT_XML_strlen(m_pszDecimal);
			for(i=0; i<=psz;i++)
			{
		               labelStr[(*insPoint)++] = m_pszDecimal[i];
			}
			(*insPoint)--;
	       }	       
	}
	UT_sint32 place = m_pItems.findItem(pLayout);
	if(place == -1)
	{
	       labelStr[0] = NULL;
	       (*insPoint) = 0;
	       return;
	}

	place = place + m_iStartValue;
	switch( m_List_Type)
	{ 
	        case NUMBERED_LIST:
		        sprintf(p,"%i",place);
			psz = UT_XML_strlen( p);
			for(i=0; i<psz; i++)
			{
			         labelStr[(*insPoint)++] = p[i];
			}
			labelStr[(*insPoint)] = NULL;
	                break;
	        case UPPERCASE_LIST:
		        sprintf(p,"%c",place+64);
			psz = UT_XML_strlen( p);
			for(i=0; i<psz; i++)
			{
			         labelStr[(*insPoint)++] = p[i];
			}
			labelStr[(*insPoint)] = NULL;
	                break;
	        case LOWERCASE_LIST:
		        sprintf(p,"%c",place+96);
			psz = UT_XML_strlen( p);
			for(i=0; i<psz; i++)
			{
			         labelStr[(*insPoint)++] = p[i];
			}
			labelStr[(*insPoint)] = NULL;
	                break;
	        case UPPERROMAN_LIST:
		        sprintf(p,"%s",dec2roman(place,UT_FALSE));
			psz = UT_XML_strlen( p);
			for(i=0; i<psz; i++)
			{
			         labelStr[(*insPoint)++] = p[i];
			}
			labelStr[(*insPoint)] = NULL;
	                break;
	        case LOWERROMAN_LIST:
		        sprintf(p,"%s",dec2roman(place,UT_TRUE));
			psz = UT_XML_strlen( p);
			for(i=0; i<psz; i++)
			{
			         labelStr[(*insPoint)++] = p[i];
			}
			labelStr[(*insPoint)] = NULL;
	                break;
	        case BULLETED_LIST:
	    	        labelStr[(*insPoint)++] = (XML_Char) 0xb7;
			labelStr[(*insPoint)] = NULL;
			break;
	        case DASHED_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) '-';
			labelStr[(*insPoint)] = NULL;
			break;
	        case SQUARE_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) 0x6E;
			labelStr[(*insPoint)] = NULL;
			break;
	        case TRIANGLE_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) 0x73;
			labelStr[(*insPoint)] = NULL;
			break;
	        case DIAMOND_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) 0xA9;
			labelStr[(*insPoint)] = NULL;
			break;
	        case STAR_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) 0x53;
			labelStr[(*insPoint)] = NULL;
			break;
	        case IMPLIES_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) 0xDE;
			labelStr[(*insPoint)] = NULL;
			break;
	        case TICK_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) 0x33;
			labelStr[(*insPoint)] = NULL;
			break;
	        case BOX_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) 0x72;
			labelStr[(*insPoint)] = NULL;
			break;
	        case HAND_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) 0x2B;
			labelStr[(*insPoint)] = NULL;
			break;
	        case HEART_LIST:
		        labelStr[(*insPoint)++] = (XML_Char) 0xAA;
			labelStr[(*insPoint)] = NULL;
			break;
	        default:
		        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
	}
	return;
}

XML_Char * fl_AutoNum::getLabel(fl_Layout * pItem) const
{

	XML_Char label[100];
	UT_uint32  insPoint=0;
	UT_uint32 depth;
	depth = 0;

	_getLabelstr( label, &insPoint, depth , pItem);
	if(insPoint == 0 )
	{
	        return (XML_Char *) NULL;
	}
	else
	{
	        return (XML_Char *) label;
	}
}

UT_uint32 fl_AutoNum::getValue(fl_Layout * pItem) const
{
	return m_pItems.findItem(pItem) + m_iStartValue;
}


void fl_AutoNum::setListType(List_Type lType)
{
        m_List_Type = lType;
}


void fl_AutoNum::setDelim(const XML_Char * lDelim)
{
	UT_uint32 i;
	i =  UT_XML_strncpy( m_pszDelim, 80, lDelim);
}

void fl_AutoNum::setDecimal(const XML_Char * lDecimal)
{
	UT_uint32 i;
	i =  UT_XML_strncpy( m_pszDecimal, 80, lDecimal);
}

List_Type fl_AutoNum::getType(void)
{
        return m_List_Type;
}

void fl_AutoNum::setStartValue(UT_uint32 start)
{
	m_iStartValue = start;
	_updateItems(0);
}

void fl_AutoNum::setAsciiOffset(UT_uint32 new_asciioffset)
{
        m_iAsciiOffset = (UT_uint16) new_asciioffset;
}

UT_uint32 fl_AutoNum::getStartValue32(void)
{
	return m_iStartValue;
}

void fl_AutoNum::insertItem(fl_Layout * pItem, fl_Layout * pPrev)
{
	UT_sint32 ndx;
	UT_ASSERT(pItem);
	
	ndx = m_pItems.findItem(pPrev) + 1;
	m_pItems.insertItemAt(pItem, ndx);
	
	_updateItems(ndx + 1);
}


void fl_AutoNum::prependItem(fl_Layout * pItem, fl_Layout * pNext)
{
	UT_sint32 ndx;
	UT_ASSERT(pItem);
	
	ndx = m_pItems.findItem(pNext);
	m_pItems.insertItemAt(pItem, ndx);
	
	_updateItems(ndx + 1);
}

void fl_AutoNum::removeItem(fl_Layout * pItem)
{
	UT_sint32 ndx = m_pItems.findItem(pItem);
	UT_ASSERT(ndx != -1);
	
	m_pItems.deleteNthItem(ndx);
	
	if ((ndx == 0) && (m_pParent))
	{
		UT_ASSERT(m_pParent->isItem(pItem));
		if (m_pItems.getItemCount() > 0)
			m_pParent->insertItem(getFirstItem(), pItem);
		m_pParent->removeItem(pItem);
	}
		
	_updateItems(ndx);
}

UT_sint32 fl_AutoNum::getPositionInList(fl_Layout * pItem)
{
        return m_pItems.findItem(pItem);
}

UT_Bool fl_AutoNum::isItem(fl_Layout * pItem) const
{
	if (m_pItems.findItem(pItem) == -1)
		return UT_FALSE;
	else
		return UT_TRUE;
}

UT_Bool fl_AutoNum::isEmpty() const
{
	if (m_pItems.getItemCount() > 0)
		return UT_FALSE;
	else
		return UT_TRUE;
}

UT_Bool fl_AutoNum::doesItemHaveLabel( fl_BlockLayout * pItem)
{
        fp_Run * pRun = pItem->getFirstRun();
	UT_Bool bStop = UT_FALSE;
	while(bStop == UT_FALSE)
	{
		if(pRun->getType() == FPRUN_FIELD)
		{
	                 fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
	                 if(pFRun->getFieldType() == FPFIELD_list_label)
	                 {
			          bStop = UT_TRUE;
	                          return UT_TRUE;
			 }
		}
	        pRun = pRun->getNext();
		if(pRun == NULL)
		{
		         bStop = UT_TRUE;
		         return UT_FALSE;
		}
	}
	return UT_FALSE;
}

UT_Bool fl_AutoNum::isLastOnLevel(fl_Layout * pItem) const
{
        UT_sint32 itemloc = m_pItems.findItem(pItem);
	if (itemloc == -1)
		return UT_FALSE;
	if(itemloc == (UT_sint32) (m_pItems.getItemCount() - 1))
		return UT_TRUE;
	else
		return UT_FALSE;
}

void fl_AutoNum::setParent(fl_AutoNum * pParent)
{
	m_pParent = pParent;
}

void fl_AutoNum::update(UT_uint32 start)
{
	if(isUpdating())
	        return;
	//_calculateLabelStr(0);
	_updateItems(start);
	if (m_pParent && !m_pParent->isUpdating())
	{
		UT_uint32 ndx = m_pParent->m_pItems.findItem(getFirstItem());
		m_pParent->update(ndx + 1);
	}
}

inline void fl_AutoNum::_updateItems(UT_uint32 start)
{
	m_bUpdatingItems = UT_TRUE;
	for (UT_uint32 i = start; i < m_pItems.getItemCount(); i++)
	{
		fl_Layout * pTmp = (fl_Layout *)m_pItems.getNthItem(i);
		pTmp->listUpdate();
	}
	m_bUpdatingItems = UT_FALSE;
}

fl_Layout * fl_AutoNum::getNthBlock( UT_uint32 list_num)
{
        if(list_num <0 || list_num >= m_pItems.getItemCount())
	        return (fl_Layout *) NULL;
	else
	        return (fl_Layout *) m_pItems.getNthItem(list_num);
}

fl_Layout * fl_AutoNum::getPrevInList( fl_Layout * pItem)
{
        UT_sint32 itemloc = m_pItems.findItem(pItem);
	if (itemloc == -1 || itemloc == 0)
		return NULL;
        return (fl_Layout *) m_pItems.getNthItem( (UT_uint32) itemloc - 1);
}

inline UT_uint32 fl_AutoNum::_getLevelValue(fl_AutoNum * pAutoNum)
{
	fl_Layout * pBlock = getFirstItem();
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

char *  fl_AutoNum::dec2roman(UT_sint32 value, UT_Bool lower) const
{
	char roman[80];		//Pretty big number if you ask me

	roman[0] = '\0';

	while( value >= 1000 )
	{
                strcat( roman, "M" );
		value -= 1000;
	}
	if( value >= 900 )
	{
                strcat( roman, "CM" );
		value -= 900;
	}
	while( value >= 500 )
        {
                strcat( roman, "D" );
		value -= 500;
	}
	if( value >= 400 )
	{
                strcat( roman, "CD" );
		value -= 400;
	}
	while( value >= 100 )
	{
	        strcat( roman, "C" );
                value -= 100;
	}
	if( value >= 90 )
        {
                strcat( roman, "XC" );
		value -= 90;
	}
	while( value >= 50 )
	{
                 strcat( roman, "L" );
		 value -= 50;
	}
	if( value >= 40 )
        {
	         strcat( roman, "XL" );
                 value -= 40;
        }
	while( value >= 10 )
        {
                 strcat( roman, "X" );
                 value -= 10;
	}
	if( value >= 9 )
	{
	         strcat( roman, "IX" );
		 value -= 9;
	}
	while( value >= 5 )
	{
                 strcat( roman, "V" );
		 value -= 5;
	}
        if( value >= 4 )
	{
	         strcat( roman, "IV" );
		 value -= 4;
	}
	while( value > 0 )
        {
	         strcat( roman, "I" );
		 value--;
        }

	if (lower == UT_TRUE) 
	{
		int len;
		len = strlen(roman);
		while (--len >= 0) 
		{
		        UT_sint32 r = (UT_sint32) roman[len];
		        if( (r >= (UT_sint32) 'A') && (r <= (UT_sint32) 'Z'))
			       r = r + 32;
			roman[len] = (char) r;
		}
	}

	return UT_strdup(roman);
}
