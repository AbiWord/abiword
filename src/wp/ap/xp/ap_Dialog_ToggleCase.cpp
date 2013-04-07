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

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "ap_Dialog_ToggleCase.h"

AP_Dialog_ToggleCase::AP_Dialog_ToggleCase(XAP_DialogFactory * pDlgFactory, 
					   XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogchangecase"), m_answer(a_CANCEL), 
  m_case(CASE_SENTENCE)
{
}

AP_Dialog_ToggleCase::~AP_Dialog_ToggleCase(void)
{
}

void AP_Dialog_ToggleCase::setAnswer(AP_Dialog_ToggleCase::tAnswer a)
{
  m_answer = a;
}

AP_Dialog_ToggleCase::tAnswer AP_Dialog_ToggleCase::getAnswer(void) const
{
  return m_answer;
}

void AP_Dialog_ToggleCase::setCase(ToggleCase tc)
{
  m_case = tc;
}

ToggleCase AP_Dialog_ToggleCase::getCase(void) const
{
  return m_case;
}
