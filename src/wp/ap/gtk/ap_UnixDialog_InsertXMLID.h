/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2011 Ben Martin
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

#ifndef AP_UNIXDIALOG_INSERTXMLID_H
#define AP_UNIXDIALOG_INSERTXMLID_H

#include "ap_Dialog_InsertXMLID.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_InsertXMLID: public AP_Dialog_InsertXMLID
{
public:
	AP_UnixDialog_InsertXMLID(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_InsertXMLID(void);

	virtual void runModal(XAP_Frame * pFrame);
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	void event_OK();
	void event_Cancel();
	void event_Delete();

protected:
	virtual GtkWidget* _constructWindow(void);
	void _constructWindowContents (GtkWidget * container);
	void _setList();

	enum
	  {
	    BUTTON_CANCEL = GTK_RESPONSE_CANCEL,
	    BUTTON_OK = GTK_RESPONSE_OK,
	    BUTTON_DELETE
	  } ResponseId ;

	GtkWidget*   m_window;
	GtkWidget*   m_combo;
	GtkWidget*   m_btInsert;
};

#endif
