/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2019 Hubert Figuière
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

#ifndef AP_UNIXDIALOG_MAILMERGE_H
#define AP_UNIXDIALOG_MAILMERGE_H

#include "xap_UnixDialog.h"
#include "ap_Dialog_MailMerge.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_MailMerge
  : public AP_Dialog_MailMerge
  , public XAP_UnixDialog
{
public:
	AP_UnixDialog_MailMerge(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_MailMerge(void);

	virtual void runModeless(XAP_Frame * pFrame) override;
	virtual void activate(void) override {XAP_gtk_window_raise(m_windowMain);}
	virtual void destroy(void) override {modeless_cleanup();}

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	void fieldClicked(UT_uint32 index);

	void event_AddClicked ();
	void event_Close();

protected:
	virtual void setFieldList() override;

private:

	// private construction functions
	GtkWidget *  _constructWindow(void);

	GtkWidget * m_entry;
	GtkWidget * m_treeview;
};

#endif /* AP_UNIXDIALOG_BREAK_H */
