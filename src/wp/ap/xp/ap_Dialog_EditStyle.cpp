/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2008 Ryan Pavlik <abiryan@ryand.net>
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

#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "ap_Dialog_EditStyle.h"

AP_Dialog_EditStyle::AP_Dialog_EditStyle(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogeditstyle"),
	m_answer(a_OK)
{
}

AP_Dialog_EditStyle::~AP_Dialog_EditStyle(void)
{
}

void AP_Dialog_EditStyle::setStyleToEdit(UT_UTF8String sName, PD_Style * pStyle)
{
	m_sName=sName;
	m_pStyle=pStyle;
	
}

AP_Dialog_EditStyle::tAnswer AP_Dialog_EditStyle::getAnswer(void) const
{
	return m_answer;
}
