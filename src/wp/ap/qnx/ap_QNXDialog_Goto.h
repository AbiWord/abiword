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

#ifndef AP_QNXDIALOG_GOTO_H
#define AP_QNXDIALOG_GOTO_H

#include "ap_Dialog_Goto.h"
class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_Goto: public AP_Dialog_Goto
{
public:
	AP_QNXDialog_Goto(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_Goto(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			runModeless(XAP_Frame * pFrame);

	// public so callbacks can access them
   	GtkWidget *				targetList;
	GtkWidget *				targetEntry;

protected:

};

#endif /* AP_QNXDIALOG_GOTO_H */
