/* AbiSource Application Framework
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

#ifndef XAP_UNIXDIALOG_BREAK_H
#define XAP_UNIXDIALOG_BREAK_H

#include "xap_Dlg_Break.h"
class XAP_UnixFrame;

/*****************************************************************/

class XAP_UnixDialog_Break: public XAP_Dialog_Break
{
public:
	XAP_UnixDialog_Break(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Break(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static AP_Dialog *		static_constructor(AP_DialogFactory *, AP_Dialog_Id id);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_Cancel(void);
	virtual void			event_WindowDelete(void);

protected:

	// private construction functions
	GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	void 		_storeWindowData(void);

	GtkWidget * _findRadioByID(XAP_Dialog_Break::breakType b);
	XAP_Dialog_Break::breakType _getActiveRadioItem(void);
	
	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;

	// group of radio buttons for easy traversal
	GSList *	m_radioGroup;

	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;

};

#endif /* XAP_UNIXDIALOG_BREAK_H */
