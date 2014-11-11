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

#ifndef AP_UNIXDIALOG_LISTREVISIONS_H
#define AP_UNIXDIALOG_LISTREVISIONS_H

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ap_Dialog_ListRevisions.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_ListRevisions: public AP_Dialog_ListRevisions
{
 public:
	AP_UnixDialog_ListRevisions(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_ListRevisions(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

 protected:
	typedef enum
	{
		BUTTON_OK = GTK_RESPONSE_OK,
		BUTTON_CANCEL = GTK_RESPONSE_CANCEL
	} ResponseId ;

	virtual GtkWidget * constructWindow () ;

	static void select_row_cb(GtkTreeSelection * select,
							   AP_UnixDialog_ListRevisions * me);
	static void row_activated_cb(GtkTreeView *, GtkTreePath *,
								 GtkTreeViewColumn*,
								 AP_UnixDialog_ListRevisions * me);

	void event_OK () ;
	void event_Cancel () ;

 private:

	void constructWindowContents ( GtkWidget * container ) ;

	void select_Row (GtkTreeIter iter) ;
	void unselect_Row () ;

	GtkWidget* m_mainWindow;
	GtkListStore* m_treeModel;

    enum
    {
        COL_REVID = 0,
        COL_DATE_STRING,
        COL_COMMENT,
        COL_DATE_AS_TIMET,
        COL_MAX
    };
} ;

#endif /* AP_UNIXDIALOG_LISTREVISIONS_H */
