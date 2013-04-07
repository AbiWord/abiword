/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiSource
 *
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
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

#ifndef _OXML_ELEMENT_CELL_H_
#define _OXML_ELEMENT_CELL_H_

// Internal includes
#include <OXML_Element.h>
#include <OXML_Element_Table.h>
#include <OXML_Element_Row.h>
#include <ie_exp_OpenXML.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

class OXML_Element_Table;
class OXML_Element_Row;

class OXML_Element_Cell : public OXML_Element
{
public:
	OXML_Element_Cell(const std::string & id, OXML_Element_Table* table, OXML_Element_Row* row,
					  UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bottom);
	virtual ~OXML_Element_Cell();

	virtual UT_Error serialize(IE_Exp_OpenXML* exporter);
	virtual UT_Error addToPT(PD_Document * pDocument);
	UT_sint32 getLeft() const
		{
			return m_iLeft;
		}
	UT_sint32 getRight() const
		{
			return m_iRight;
		}
	UT_sint32 getTop() const
		{
			return m_iTop;
		}
	UT_sint32 getBottom() const
		{
			return m_iBottom;
		}

	void setLeft(UT_sint32 left);
	void setRight(UT_sint32 right);
	void setTop(UT_sint32 top);
	void setBottom(UT_sint32 bottom);

	void setRow(OXML_Element_Row* row)
		{
			m_row = row;
		}

	void setVerticalMergeStart(bool start);
	void setHorizontalMergeStart(bool start);
	bool startsVerticalMerge() const
        {
            return m_startVerticalMerge;
        }
	bool startsHorizontalMerge() const
        {
            return m_startHorizontalMerge;
        }

	void setLastHorizontalContinuationCell(OXML_Element_Cell* cell);
	void setLastVerticalContinuationCell(OXML_Element_Cell* cell);

private:
	virtual UT_Error serializeProperties(IE_Exp_OpenXML* exporter);
	UT_sint32 m_iLeft, m_iRight, m_iTop, m_iBottom;
	bool m_startVerticalMerge;
	bool m_startHorizontalMerge;
	OXML_Element_Table* m_table;
	OXML_Element_Row*   m_row;

	OXML_Element_Cell* m_horizontalTail;
	OXML_Element_Cell* m_verticalTail;
};

#endif //_OXML_ELEMENT_CELL_H_

