/* Copyright etc. */
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
		 
#ifndef FL_AUTONUM_H
#define FL_AUTONUM_H

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_vector.h"

class fl_BlockLayout;
class fl_Layout;

class fl_AutoNum
{
public:
	fl_AutoNum(UT_uint32 id, UT_uint32 start, const XML_Char * format, fl_Layout * pItem, fl_AutoNum * pParent);
	~fl_AutoNum();

	XML_Char *				getLabel(fl_Layout *) const;
	XML_Char *				getType(void);
	UT_uint32				getValue(fl_Layout *) const;
	UT_uint32				getLevel(void) const { return m_iLevel; }
	UT_sint32				getPositionInList( fl_Layout * pItem);
	void					setFormat(const XML_Char * format);

	UT_uint16				getStartValue(void) const { return m_iStartValue; }

	UT_uint32				getStartValue32(void);
	void					setStartValue(UT_uint32 start);

	void					insertItem(fl_Layout * pItem, fl_Layout * pBefore);
	void					removeItem(fl_Layout * pItem);
	fl_Layout *				getNthBlock(UT_uint32 i);
	fl_Layout *				getPrevInList(fl_Layout * pItem);

	UT_Bool					isItem(fl_Layout * pItem) const;
	UT_Bool					doesItemHaveLabel( fl_BlockLayout * pItem);
	UT_Bool					isEmpty(void) const;
	inline fl_Layout *		getFirstItem(void) const { return (fl_Layout *)m_pItems.getFirstItem(); }	
	UT_Bool					isLastOnLevel(fl_Layout * pItem) const;

	fl_AutoNum *			getParent(void) const { return m_pParent; }
	void					setParent(fl_AutoNum *);
	void					setAsciiOffset(UT_uint32 new_asciioffset);

	void					update(UT_uint32 start);
	inline UT_Bool			isUpdating(void) const { return m_bUpdatingItems; }

	inline UT_uint32		getID(void) const { return m_iID; }

protected:
	void					_calculateLabelStr(UT_uint32 depth);
	inline void				_updateItems(UT_uint32 start);
	inline UT_uint32		_getLevelValue(fl_AutoNum * pAutoNum); 

	fl_AutoNum *			m_pParent;

	UT_Vector				m_pItems;

	UT_uint32				m_iID;
	UT_uint32				m_iLevel;
	const XML_Char *		m_pszFormat;
	XML_Char *				m_pszLabelStr;
	UT_uint32				m_iStartValue;
	UT_uint16				m_iAsciiOffset;
	UT_Bool					m_bUpdatingItems;
	UT_sint32				m_ioffset;
};

#endif
