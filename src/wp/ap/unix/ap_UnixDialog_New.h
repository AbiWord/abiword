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
	void event_ClistClicked ( gint row, gint col )
	  {
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(m_radioNew), TRUE);
	    mRow = row; mCol = col;
	  }
	void event_ToggleOpenExisting ();
	
protected:

	typedef enum
		{
			BUTTON_OK,
			BUTTON_CANCEL
		} ResponseId ;
	
	virtual GtkWidget * _constructWindow ();
	virtual void _constructWindowContents (GtkWidget * container);

	/* private ... */
	GtkWidget * m_mainWindow;

private:
	XAP_Frame * m_pFrame;

	GtkWidget * m_entryFilename;
	GtkWidget * m_radioNew;
	GtkWidget * m_radioExisting;
	GtkWidget * m_radioEmpty;
	GtkWidget * m_choicesList;

	gint mRow, mCol;

	UT_Vector mTemplates ;
};

#endif /* AP_UNIXDIALOG_NEW_H */
