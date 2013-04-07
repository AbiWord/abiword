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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ap_Features.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_App.h"
#include "ap_Dialog_InsertTable.h"
#include "ap_Prefs_SchemeIds.h"

AP_Dialog_InsertTable::AP_Dialog_InsertTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialoginsertable")
{
	m_answer = a_OK;
	
	/* Default values for the dialog box*/
	m_numRows = 2;
	m_numCols = 5;
	m_columnWidth = static_cast<float>(0.7);	// In DIM_IN
	
	/* Use default units*/
	const gchar * szRulerUnits;
	if (getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		m_dim = UT_determineDimension(szRulerUnits);
	else
		m_dim = DIM_IN;

	// The default m_columnWidth is in inches, convert 
	// if the user default unit is different
	if (m_dim != DIM_IN)								
		m_columnWidth = static_cast<float>(UT_convertInchesToDimension(m_columnWidth, m_dim));
}

void AP_Dialog_InsertTable::setColumnWidth(float columnWidth)
{
	m_columnWidth = columnWidth;
}


#define SPIN_INCR_IN	0.1
#define SPIN_INCR_CM	0.1
#define SPIN_INCR_MM	1.0
#define SPIN_INCR_PI	6.0
#define SPIN_INCR_PT	1.0
#define SPIN_INCR_none	0.1
double AP_Dialog_InsertTable::getSpinIncr(void)
{
       double dSpin = SPIN_INCR_PT;
	switch (m_dim)
	{
	case DIM_IN:	
		dSpin = SPIN_INCR_IN;	
		break;

	case DIM_CM:	
		dSpin = SPIN_INCR_CM;	
		break;

	case DIM_MM:	
		dSpin = SPIN_INCR_MM;	
		break;

	case DIM_PI:	
		dSpin = SPIN_INCR_PI;
		break;

	case DIM_PT:	
		dSpin = SPIN_INCR_PT;	
		break;
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	return dSpin;
}


#define SPIN_MIN_IN	0.1
#define SPIN_MIN_CM	0.1
#define SPIN_MIN_MM	1.0
#define SPIN_MIN_PI	6.0
#define SPIN_MIN_PT	1.0
#define SPIN_MIN_none	0.1
double AP_Dialog_InsertTable::getSpinMin (void)
{
       double dSpin = SPIN_MIN_PT;
	switch (m_dim)
	{
	case DIM_IN:	
		dSpin = SPIN_MIN_IN;	
		break;

	case DIM_CM:	
		dSpin = SPIN_MIN_CM;	
		break;

	case DIM_MM:	
		dSpin = SPIN_MIN_MM;	
		break;

	case DIM_PI:	
		dSpin = SPIN_MIN_PI;
		break;

	case DIM_PT:	
		dSpin = SPIN_MIN_PT;	
		break;
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	return dSpin;
}

// Does the table size spin
void AP_Dialog_InsertTable::_doSpin(UT_sint32 amt, double& dValue)
{
	
	// figure out which dimension and units to spin in
	double dSpinUnit = getSpinIncr ();
	double dMin = getSpinMin ();
	// value is now in desired units, so change it
	dValue +=  (dSpinUnit * static_cast<double>(amt));
	if (dValue < dMin)
		dValue = dMin;
}


AP_Dialog_InsertTable::~AP_Dialog_InsertTable(void)
{
}

AP_Dialog_InsertTable::tAnswer AP_Dialog_InsertTable::getAnswer(void) const
{
	return m_answer;
}

AP_Dialog_InsertTable::columnType AP_Dialog_InsertTable::getColumnType(void) const
{
	return m_columnType;
}

UT_uint32 AP_Dialog_InsertTable::getNumRows(void)
{
	return m_numRows;
}

UT_uint32 AP_Dialog_InsertTable::getNumCols(void)
{
	return m_numCols;
}

float AP_Dialog_InsertTable::getColumnWidth(void)
{
	return static_cast<float>(UT_convertDimToInches(m_columnWidth, m_dim));
}
