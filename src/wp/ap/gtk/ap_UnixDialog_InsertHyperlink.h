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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_UNIXDIALOG_INSERTHYPERLINK_H
#define AP_UNIXDIALOG_INSERTHYPERLINK_H

#include "ap_Dialog_InsertHyperlink.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_InsertHyperlink: public AP_Dialog_InsertHyperlink
{
public:
	AP_UnixDialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_InsertHyperlink(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	void event_OK(void);
	void event_Cancel(void);
	void setRow(gint row) {m_iRow = row;}

	GtkWidget * m_entry;
	const gchar ** m_pBookmarks;
	
 protected:
	virtual GtkWidget *		_constructWindow(void);
	void _constructWindowContents (GtkWidget * container);
	void					_connectSignals (void);

	GtkWidget * m_windowMain;

 private:

	enum
	  {
	    BUTTON_OK = GTK_RESPONSE_OK,
	    BUTTON_CANCEL = GTK_RESPONSE_CANCEL
	  } ResponseId ;

	//GtkWidget * m_comboEntry;
	//GtkWidget * m_comboHyperlink;
	GtkWidget * m_clist;
	GtkWidget * m_swindow;
	gint		m_iRow;
		
};

#endif /* AP_UNIXDIALOG_INSERTBOOKMARK_H */
