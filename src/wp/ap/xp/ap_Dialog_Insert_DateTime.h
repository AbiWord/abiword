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

#ifndef AP_DIALOG_INSERT_DATETIME_H
#define AP_DIALOG_INSERT_DATETIME_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

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

#define CURRENT_DATE_TIME_SIZE 256

static const char *InsertDateTimeFmts[] = {
    "%m/%d/%y",
	"%A, %B %d, %Y",
	"%B %d, %Y",
	"%m/%d/%Y",
	"%Y-%m-%d",
	"%d-%b-%y",
    "%m.%d.%y",
	"%b. %d, %y",
	"%d %B, %Y",
	"%B, %Y",
	"%b-%y",
	"%m/%d/%y %H:%M",
    "%m/%d/%y %H:%M:%S",
	"%H:%M",
	"%H:%M:%S",
	NULL
};

class XAP_Frame;

class AP_Dialog_Insert_DateTime : public XAP_Dialog_NonPersistent
{
public:
    typedef enum { a_OK, a_CANCEL } tAnswer;

    AP_Dialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
    AP_Dialog_Insert_DateTime::tAnswer    getAnswer(void) const;
    const char *GetDateTimeFormat(void) const;
protected:
    AP_Dialog_Insert_DateTime::tAnswer    m_answer;
    int m_iFormatIndex;
};

#endif /* AP_DIALOG_INSERT_DATETIME_H */
