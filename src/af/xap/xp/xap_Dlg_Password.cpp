/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ut_types.h"
#include "ut_string.h"
#include "xap_Dlg_Password.h"

XAP_Dialog_Password::XAP_Dialog_Password(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent (pDlgFactory, id), m_answer(a_Cancel)
{
}

XAP_Dialog_Password::~XAP_Dialog_Password ()
{
}

void XAP_Dialog_Password::setPassword (const char * pass)
{
	m_passwd = pass;
}

void XAP_Dialog_Password::setPassword (const UT_UTF8String& pass)
{
	m_passwd = pass;
}
