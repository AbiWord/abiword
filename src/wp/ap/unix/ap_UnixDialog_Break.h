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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_UNIXDIALOG_BREAK_H
#define AP_UNIXDIALOG_BREAK_H

#include <gtk/gtktogglebutton.h>
#include <glade/glade.h>
#include "ap_Dialog_Break.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Break: public AP_Dialog_Break
{
public:
	AP_UnixDialog_Break(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Break(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void			event_OK(void);
	virtual void			event_Cancel(void);

protected:
	virtual void			_init(void);

	// private construction functions
	virtual const char *			_getGladeName(void)
	  {
	    return "break.glade";
	  }

	virtual bool _isActive(const char *widget_name)
	  {
	    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(m_pXML, widget_name)));
	  }

	virtual GtkWidget * 	_constructWindow(void);

	void		_populateWindowData(void);
	void 		_storeWindowData(void);

	GtkWidget * _findRadioByID(AP_Dialog_Break::breakType b);
	AP_Dialog_Break::breakType _getActiveRadioItem(void);
	
	// pointers to widgets we need to query/set
	GtkWidget * m_wMainWindow;

private:
	GladeXML *  m_pXML;
};

#endif /* AP_UNIXDIALOG_BREAK_H */
