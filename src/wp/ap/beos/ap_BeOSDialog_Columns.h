/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifndef AP_BEOSDIALOG_COLUMNS_H
#define AP_BEOSDIALOG_COLUMNS_H

#include "ap_Dialog_Columns.h"

class XAP_BeOSFrame;

class AP_BeOSDialog_Columns : public AP_Dialog_Columns
{
	friend class ColumnWin;
	
public:
	AP_BeOSDialog_Columns(XAP_DialogFactory * pDlgFactory,XAP_Dialog_Id id);
	virtual ~AP_BeOSDialog_Columns();

	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			enableLineBetweenControl(UT_Bool bState = UT_TRUE);

    static XAP_Dialog *   static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

protected:

private:
	bool enableLineBetween;
	class ColumnWin  *newwin;
};

#endif /* AP_WIN32DIALOG_FIELD_H */
