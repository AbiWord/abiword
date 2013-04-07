/* Copyright (C) 2010 AbiSource Corporation B.V.
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

#ifndef AP_UNIXDIALOG_COLLABORATIONEDITACCOUNT_H
#define AP_UNIXDIALOG_COLLABORATIONEDITACCOUNT_H

#include <gtk/gtk.h>
#include <xp/ap_Dialog_CollaborationEditAccount.h>

class XAP_Frame;

class AP_UnixDialog_CollaborationEditAccount : public AP_Dialog_CollaborationEditAccount
{
public:
	AP_UnixDialog_CollaborationEditAccount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	static XAP_Dialog * static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id);
	void						runModal(XAP_Frame * pFrame);

protected:
	virtual void*				_getEmbeddingParent()
		{ return m_wEmbeddingParent; }

private:
	GtkWidget*	 				_constructWindow(void);
	void						_populateWindowData(void);


	GtkWidget*					m_wWindowMain;
	GtkBox*						m_wEmbeddingParent;
	GtkWidget*					m_wOk;
};

#endif /* AP_UNIXDIALOG_COLLABORATIONEDITACCOUNT_H */
