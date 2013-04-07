/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_UNIXDIALOG_NEW_H
#define AP_UNIXDIALOG_NEW_H

#include <gtk/gtk.h>
#include "ap_Dialog_New.h"
#include "ut_vector.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_New: public AP_Dialog_New
{
public:
	AP_UnixDialog_New(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_New(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *,
											   XAP_Dialog_Id id);

	void event_Ok ();
	void event_Cancel ();
	void event_ToggleOpenExisting ();
	void event_RadioButtonSensitivity ();

	void event_ListClicked();

private:

	GtkWidget * _constructWindow ();

	static void s_template_dblclicked(GtkTreeView *treeview,
									  GtkTreePath *arg1,
									  GtkTreeViewColumn *arg2,
									  AP_UnixDialog_New * me);

	/* private ... */
	GtkWidget * m_mainWindow;

	XAP_Frame * m_pFrame;

	GtkWidget * m_buttonFilename;
	GtkWidget * m_radioNew;
	GtkWidget * m_radioExisting;
	GtkWidget * m_choicesList;

	UT_Vector mTemplates ;
};

#endif /* AP_UNIXDIALOG_NEW_H */
