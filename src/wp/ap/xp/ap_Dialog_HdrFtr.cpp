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

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"

#include "ap_Dialog_HdrFtr.h"

AP_Dialog_HdrFtr::AP_Dialog_HdrFtr(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogformatheaderfooters"), m_answer(a_OK)
{
	UT_sint32 i = 0;
	for(i=0; i< 6; i++)
	{
		m_bHdrFtrValues[i] = false;
		m_bHdrFtrChanged[i] = false;
	}
	m_bDoRestart = false;
	m_bRestartChanged = false;
	m_iStartAt = 0;
}

AP_Dialog_HdrFtr::~AP_Dialog_HdrFtr(void)
{
}

AP_Dialog_HdrFtr::tAnswer AP_Dialog_HdrFtr::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_HdrFtr::setAnswer (AP_Dialog_HdrFtr::tAnswer answer)
{
	m_answer = answer;
}

void AP_Dialog_HdrFtr::setValue(AP_Dialog_HdrFtr::HdrFtr_Control which, bool value, bool changed)
{
	m_bHdrFtrValues[which] = value;
	if(changed)
	{
		m_bHdrFtrChanged[which] = true;
	}
}


bool AP_Dialog_HdrFtr::getValue(AP_Dialog_HdrFtr::HdrFtr_Control which)
{
	return m_bHdrFtrValues[which];
}


bool AP_Dialog_HdrFtr::isChanged(AP_Dialog_HdrFtr::HdrFtr_Control which)
{
	return m_bHdrFtrChanged[which];
}

bool AP_Dialog_HdrFtr::isRestartChanged(void) const
{
	return m_bRestartChanged;
}

bool AP_Dialog_HdrFtr::isRestart(void) const
{
	return m_bDoRestart;
}

UT_sint32 AP_Dialog_HdrFtr::getRestartValue(void) const
{
	return m_iStartAt;
}

void AP_Dialog_HdrFtr::setRestart( bool bRestart, UT_sint32 RestartValue, bool bRestartChanged)
{
	m_bDoRestart = bRestart;
	m_iStartAt = RestartValue;
	m_bRestartChanged = bRestartChanged;
}








