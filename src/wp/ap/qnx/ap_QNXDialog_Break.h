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

#ifndef AP_QNXDIALOG_BREAK_H
#define AP_QNXDIALOG_BREAK_H

#include "ap_Dialog_Break.h"

class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_Break: public AP_Dialog_Break
{
public:
	AP_QNXDialog_Break(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_Break(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_Cancel(void);
	virtual void			event_WindowDelete(void);

protected:

#if 0
	// private construction functions
	GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	void 		_storeWindowData(void);
#endif

	void * _findRadioByID(AP_Dialog_Break::breakType b);
	AP_Dialog_Break::breakType _getActiveRadioItem(void);
	
	// pointers to widgets we need to query/set
	void * m_windowMain;

	// group of radio buttons for easy traversal
	void *	m_radioGroup;

	void * m_buttonOK;
	void * m_buttonCancel;

};

#endif /* AP_QNXDIALOG_BREAK_H */
