/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_INSERT_DATETIME_H
#define AP_COCOADIALOG_INSERT_DATETIME_H

#include "ap_Dialog_Insert_DateTime.h"
#import "xap_Cocoa_NSTableUtils.h"
#import "xap_GenericListChooser_Controller.h"

class XAP_CocoaFrame;

/*****************************************************************/

class AP_CocoaDialog_Insert_DateTime: public AP_Dialog_Insert_DateTime
{
public:
	AP_CocoaDialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Insert_DateTime(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_Cancel(void);

protected:
#if 0
	// private construction functions
	virtual GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	void            _connectSignals(void);
	GtkWidget *     _constructWindowContents(void);

	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;

	// group of radio buttons for easy traversal
	GtkWidget * m_listFormats;

	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;
#endif
private:
	void		_populateWindowData(void);
	XAP_StringListDataSource*	m_dataSource;
	XAP_GenericListChooser_Controller*	m_dlg;
};

#endif /* AP_COCOADIALOG_INSERT_DATETIME_H */
