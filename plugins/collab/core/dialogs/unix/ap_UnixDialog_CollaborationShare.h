/* AbiCollab- Code to enable the modification of remote documents.
 * Copyright (C) 2006 by Marc Maurer <uwog@uwog.net>
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

#ifndef AP_UNIXDIALOG_COLLABORATIONSHARE_H
#define AP_UNIXDIALOG_COLLABORATIONSHARE_H

#include <gtk/gtk.h>
#include <xp/ap_Dialog_CollaborationShare.h>

class XAP_Frame;

class AP_UnixDialog_CollaborationShare : public AP_Dialog_CollaborationShare
{
public:
	AP_UnixDialog_CollaborationShare(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	static XAP_Dialog * static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id);
	void						runModal(XAP_Frame * pFrame);

	void						eventOk();
	void						eventToggle(gchar* path_str);

private:
	GtkWidget*	 				_constructWindow(void);
	void						_populateWindowData(void);
	void						_populateBuddyModel(bool refresh);
	AccountHandler*				_getActiveAccountHandler();
	void						_setAccountHint(const UT_UTF8String& sHint);
	void						_refreshWindow();
	void						_getSelectedBuddies(std::vector<std::string>& vACL);
	void						_freeBuddyList();

	GtkWidget*					m_wWindowMain;
	GtkWidget*					m_wAccount;
	GtkWidget*					m_wAccountHint;

	GtkWidget*					m_wAccountHintSpacer;
	GtkWidget*					m_wAccountHintHbox;

	GtkWidget*					m_wBuddyTree;
	GtkTreeModel*				m_pAccountModel;
	GtkListStore*				m_pBuddyModel;
	GObject*					m_crToggle;
	GtkWidget*					m_wOk;
};

#endif /* AP_UNIXDIALOG_COLLABORATIONSHARE_H */
