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

#ifndef AP_UNIXDIALOG_FIELD_H
#define AP_UNIXDIALOG_FIELD_H

#include "ap_Dialog_Field.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Field: public AP_Dialog_Field
{
public:
	AP_UnixDialog_Field(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *
pFactory , XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Field(void);

	virtual void runModal(XAP_Frame * pFrame);

	void event_Insert(void);
	void types_changed(GtkTreeView *treeview);
	void setTypesList(void);
	void setFieldsList(void);

protected:
	virtual GtkWidget *		_constructWindow(void);
	void					_populateCatogries(void);

	static void s_field_dblclicked(GtkTreeView *treeview,
								   GtkTreePath *arg1,
								   GtkTreeViewColumn *arg2,
								   AP_UnixDialog_Field * me);

	GtkWidget * m_windowMain;

	GtkWidget * m_listTypes;
	GtkWidget * m_listFields;
	GtkWidget * m_entryParam;
        
        gulong m_cursorChangedHandlerId;
        gulong m_rowActivatedHandlerId;   

};

#endif /* AP_UNIXDIALOG_FIELD_H */






