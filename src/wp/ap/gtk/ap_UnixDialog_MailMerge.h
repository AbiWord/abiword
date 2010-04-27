/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#ifndef AP_UNIXDIALOG_MAILMERGE_H
#define AP_UNIXDIALOG_MAILMERGE_H

#include "ap_Dialog_MailMerge.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_MailMerge: public AP_Dialog_MailMerge
{
public:
	AP_UnixDialog_MailMerge(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_MailMerge(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void activate(void) {gdk_window_raise (m_windowMain->window);}
	virtual void destroy(void) {modeless_cleanup();}

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	void fieldClicked(UT_uint32 index);

	void event_AddClicked ();
	void event_Close();

protected:
	virtual void setFieldList();

private:

	// private construction functions
	GtkWidget *  _constructWindow(void);

	GtkWidget * m_windowMain;
	GtkWidget * m_entry;
	GtkWidget * m_treeview;
};

#endif /* AP_UNIXDIALOG_BREAK_H */
