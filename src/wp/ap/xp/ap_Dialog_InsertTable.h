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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_DIALOG_INSERTTABLE_H
#define AP_DIALOG_INSERTTABLE_H

#include "ut_types.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_units.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_InsertTable : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_InsertTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_InsertTable(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;
	typedef enum { b_AUTOSIZE = 0, b_FIXEDSIZE } columnType;

	AP_Dialog_InsertTable::tAnswer		getAnswer(void) const;
	AP_Dialog_InsertTable::columnType	getColumnType(void) const;
	UT_uint32							getNumRows(void);
	UT_uint32							getNumCols(void);
	float								getColumnWidth(void);
	void								setColumnWidth(float columnWidth);
	double								getSpinIncr (void);
	double								getSpinMin (void);

protected:

	void 					_doSpin(UT_sint32 amt, double& dValue);

	AP_Dialog_InsertTable::tAnswer		m_answer;
	AP_Dialog_InsertTable::columnType	m_columnType;
	UT_uint32							m_numRows;
	UT_uint32							m_numCols;
	float					m_columnWidth;
	UT_Dimension			m_dim;
};

#endif /* AP_DIALOG_INSERTTABLE_H */
