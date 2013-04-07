/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#ifndef _OXML_ELEMENT_TABLE_H_
#define _OXML_ELEMENT_TABLE_H_

// Internal includes
#include <OXML_Element.h>
#include <OXML_Element_Row.h>
#include <ie_exp_OpenXML.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>
#include <vector>

class OXML_Element_Table : public OXML_Element
{
public:
	OXML_Element_Table(const std::string & id);
	virtual ~OXML_Element_Table();

	virtual UT_Error serialize(IE_Exp_OpenXML* exporter);
	virtual UT_Error serializeChildren(IE_Exp_OpenXML* exporter);
	virtual UT_Error addToPT(PD_Document * pDocument);
	virtual std::string getColumnWidth(int colIndex) const;
	virtual std::string getRowHeight(int rowIndex) const;
	UT_Error addChildrenToPT(PD_Document * pDocument);

	void addRow(OXML_Element_Row* row);

	int getCurrentRowNumber() const;
	int getCurrentColNumber() const;

	void setCurrentRowNumber(int row);
	void setCurrentColNumber(int col);

	void incrementCurrentRowNumber();
	void incrementCurrentColNumber();

	//this method increments the vertical merge start cell's bottom by one.
	//It traverses up the cells in the table and finds the vertical merge starting cell
	//and increments its bottom value by one. Should be called for the vertMerge=continue cells.
	//return true if successful
	bool incrementBottomVerticalMergeStart(OXML_Element_Cell* cell);

	//this method increments the horizontal merge start cell's right by one.
	//It traverses up the cells in the table and finds the horizontal merge starting cell
	//and increments its right value by one. Should be called for the hMerge=continue cells.
	//return true if successful
	bool incrementRightHorizontalMergeStart(OXML_Element_Cell* cell);

	void addMissingCell(unsigned int rowNumber, OXML_Element_Cell* cell);

	void applyStyle(OXML_SharedStyle style);

	int getNumberOfRows() const
		{
			return m_rows.size();
		}

private:
	virtual UT_Error serializeProperties(IE_Exp_OpenXML* exporter);
	std::vector<std::string> columnWidth;
	std::vector<std::string> rowHeight;
	std::vector<OXML_Element_Row*> m_rows;
	int m_currentRowNumber;
	int m_currentColNumber;
};

#endif //_OXML_ELEMENT_TABLE_H_

