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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "ap_Dialog_Break.h"

AP_Dialog_Break::AP_Dialog_Break(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer = a_OK;
	m_break = b_PAGE;
}

AP_Dialog_Break::~AP_Dialog_Break(void)
{
}

AP_Dialog_Break::tAnswer AP_Dialog_Break::getAnswer(void) const
{
	return m_answer;
}

AP_Dialog_Break::breakType AP_Dialog_Break::getBreakType(void) const
{
	return m_break;
}
