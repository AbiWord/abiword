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
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "ap_Dialog_Insert_DateTime.h"

/*
M/d/yy
dddd, MMMM dd, yyyy
MMMM d, yyyy
M/d/yyyy
yyyy-MM-dd
d-MMM-yy
M.d.yy
MMM. d, yy
d MMMM, yyyy
MMMM, yyyy
MMM-yy
M/d/yy h:mm
M/d/yy h:mm:ss
h:mm
h:mm:ss
HH:mm
HH:mm:ss
*/

#if 0
#error TF CHANGE: The # key is not POSIX/ANSI
const char *InsertDateTimeFmts[] = {
    "%#m/%#d/%y",
	"%A, %B %d, %Y",
	"%B %#d, %Y",
	"%#m/%#d/%Y",
	"%Y-%m-%d",
	"%#d-%b-%y",
    "%#m.%#d.%y",
	"%b. %#d, %y",
	"%#d %B, %Y",
	"%B, %Y",
	"%b-%y",
	"%#m/%#d/%y %#I:%M %p",
    "%#m/%#d/%y %#I:%M:%S %p",
	"%#I:%M %p",
	"%#I:%M:%S %p",
	"%H:%M",
	"%H:%M:%S",
	NULL
};
#else
const char *InsertDateTimeFmts[] = {
	"%Y-%m-%d",
	"%m/%d/%Y",
	"%m/%d/%y",
	"%m.%d.%y",
	"%d.%m.%Y",
	"%B %Y",
	"%B, %Y",
	"%b-%y",
	"%B %d, %Y",
	"%b. %d, %y",
	"%d. %B %Y",
	"%d %B, %Y",
	"%d-%b-%y",
	"%A, %B %d, %Y",
	"%A, %d. %B %Y",
	"%H:%M",
	"%H:%M:%S",
	"%I:%M %p",
	"%I:%M:%S %p",
	"%Y-%m-%d %H:%M:%S",
	"%m/%d/%y %I:%M %p",
	"%m/%d/%y %I:%M:%S %p",
	"%d.%m.%Y %H:%M",
	"%d.%m.%Y %H:%M:%S",
	NULL
};
#endif

AP_Dialog_Insert_DateTime::AP_Dialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogdateandtime")
{
    m_answer = a_OK;
    m_iFormatIndex = 0;
}

AP_Dialog_Insert_DateTime::tAnswer AP_Dialog_Insert_DateTime::getAnswer(void) const
{
    return m_answer;
}

const char *AP_Dialog_Insert_DateTime::GetDateTimeFormat(void) const
{
    return (const char *)InsertDateTimeFmts[m_iFormatIndex];
}
