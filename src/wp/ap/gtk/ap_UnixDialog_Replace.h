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

#ifndef AP_UNIXDIALOG_REPLACE_H
#define AP_UNIXDIALOG_REPLACE_H

#include "ap_Dialog_Replace.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Replace: public AP_Dialog_Replace
{
public:
	AP_UnixDialog_Replace(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Replace(void);


	virtual void			runModal(XAP_Frame * /*pFrame*/){};
	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);
	virtual void			notifyCloseFrame(XAP_Frame * /*pFrame*/){};
	virtual void			destroy(void);
	virtual void			activate(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	void			event_Find(void);
	void			event_FindEntryChange(void);
	void			event_Replace(void);
	void			event_ReplaceAll(void);
	void			event_MatchCaseToggled(void);
	void			event_WholeWordToggled(void);
	void			event_ReverseFindToggled(void);
	void			event_Cancel(void);

	enum
	  {
	    BUTTON_CANCEL = GTK_RESPONSE_CANCEL,
	    // enum GtkResponseType seems to only use negative integers, so we'll use positive ones to
	    // prevent potential conflicts (the cause of Bug 11583)
	    BUTTON_FIND = 0,
	    BUTTON_REPLACE = 1,
	    BUTTON_REPLACE_ALL = 2
	  } ResponseId;

protected:

	virtual void			_updateLists();

private:

	// private construction functions
	GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	void 		_storeWindowData(void);

	static void s_response_triggered(GtkWidget * widget, gint resp, AP_UnixDialog_Replace * dlg);

	void			_updateList(GtkWidget* combo, UT_GenericVector<UT_UCS4Char*>* list);

	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;

	GtkWidget *	m_buttonFind;
	GtkWidget *	m_buttonFindReplace;
	GtkWidget *	m_buttonReplaceAll;

	GtkWidget * m_comboFind;
	GtkWidget * m_comboReplace;

	GtkWidget * m_checkbuttonMatchCase;
	GtkWidget * m_checkbuttonWholeWord;
	GtkWidget * m_checkbuttonReverseFind;
};

#endif /* AP_UNIXDIALOG_REPLACE_H */
