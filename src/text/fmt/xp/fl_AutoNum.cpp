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



fl_AutoNum::fl_AutoNum(UT_uint32 id, UT_uint32 start, const XML_Char * format, fl_Layout * pFirst, fl_AutoNum * pParent)
{
	m_iID = id;
	m_iStartValue = start;
	m_pszFormat = format;
	m_iAsciiOffset = 0;
	m_bUpdatingItems = UT_FALSE;
	m_ioffset = 0;

	m_pParent = pParent;
	if (m_pParent) {
		m_iLevel = m_pParent->getLevel() + 1;
	} else {
		m_iLevel = 1;
	}

	m_pItems.addItem(pFirst);	

	_calculateLabelStr( 0 );

}

void fl_AutoNum::_calculateLabelStr(UT_uint32 depth)
{
	UT_ASSERT(m_pszFormat);
	
	UT_uint32 num_fchars, i;
	num_fchars = 0;

	if (m_pParent)
	{
		m_pParent->_calculateLabelStr(depth + 1);
	}

	for (i = 0; i < UT_XML_strlen(m_pszFormat); i++)
		if (m_pszFormat[i] == '%')
			num_fchars++;
	XML_Char * p;
	// this writes m_pszFormat into p and hence into curr_str
	if (!UT_cloneString((char *&)p, m_pszFormat))
	{
		// TODO out of mem
	} 
	UT_ASSERT(p);
	
	fl_AutoNum * pCurr = this;
	
	i = 0;
	XML_Char ** f_strs = new XML_Char * [num_fchars];
	XML_Char * curr_str = strtok(p, "%");
	
	while (curr_str)
	{
		f_strs[i] = curr_str;
		curr_str = strtok(NULL, "%");
		i++;
	}
	UT_ASSERT(i == num_fchars);
	
	curr_str = new XML_Char[30];
	while (pCurr)
	{
		i--;
		sprintf(curr_str, "%%%s", f_strs[i]);
		switch(curr_str[1])
		{
		case 'd':
			curr_str[1] = 'i';
			if (pCurr != this)
				sprintf(curr_str, curr_str, _getLevelValue(pCurr));
			break;
		case 'b':
			XML_Char * tmp;
			UT_XML_cloneString(*&tmp, curr_str);
			tmp[1] = 'c';
			//			sprintf(curr_str, tmp, UCS_BULLET);
			sprintf(curr_str,tmp,0xb7);
			free(tmp);
			break;
		case 'A':
			curr_str[1] = 'c';
			if (pCurr != this)
				sprintf(curr_str, curr_str, _getLevelValue(pCurr) + 64);
			else
				m_iAsciiOffset = 64;
			break;
		case 'a':
			curr_str[1] = 'c';
			if (pCurr != this)
				sprintf(curr_str, curr_str, _getLevelValue(pCurr) + 96);
			else
				m_iAsciiOffset = 96;
			break;
		case '*':
			curr_str[1] = 's';
			UT_ASSERT(pCurr == getParent());
			curr_str = pCurr->getLabel(getFirstItem());
			m_ioffset = 0;
			pCurr = NULL;
			break;
		}
		if (pCurr)
			pCurr = pCurr->getParent();
		UT_ASSERT(curr_str);
		UT_XML_cloneString(*&f_strs[i], curr_str);
	}
	free(p); 
	
	XML_Char buf[100] = { "" };

	if (!getParent())
	{
	        sprintf(buf, "%s", f_strs[num_fchars - 1]);
	}
	else
	{
		for (i = 0; i < num_fchars; i++)
		{
			sprintf(buf, "%s%s", buf, f_strs[i]);
		}
	}
	
	UT_XML_cloneString(*&m_pszLabelStr, buf);
	UT_DEBUGMSG(("[fl_AutoNum::_calculateLabelStr] List Label: %s\n", m_pszLabelStr));
}

fl_AutoNum::~fl_AutoNum()
{
	if (m_pParent && m_pParent->isEmpty())
		DELETEP(m_pParent);
}

XML_Char * fl_AutoNum::getLabel(fl_Layout * pItem) const
{

	XML_Char * format = new XML_Char [100];
	XML_Char * label = new XML_Char [100];
	UT_ASSERT(m_pszLabelStr);
	UT_XML_cloneString(*&format, m_pszLabelStr);
	
	UT_sint32 place = m_pItems.findItem(pItem);
	//UT_ASSERT(place != -1);
	if(place != -1)
	{
		sprintf(label, format, place + m_iStartValue + m_iAsciiOffset + m_ioffset);
		return label;
	}
	else
	{
		return NULL;
	}
}

UT_uint32 fl_AutoNum::getValue(fl_Layout * pItem) const
{
	return m_pItems.findItem(pItem) + m_iStartValue;
}


void fl_AutoNum::setFormat(const XML_Char * format)
{
	UT_ASSERT(format);
	m_pszFormat = format;
	_calculateLabelStr(0);
	_updateItems(0);
}

XML_Char * fl_AutoNum::getType(void)
{
		//TF NOTE: These casts can't be good!
        return (XML_Char *)m_pszFormat;
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
	_calculateLabelStr(0);
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





