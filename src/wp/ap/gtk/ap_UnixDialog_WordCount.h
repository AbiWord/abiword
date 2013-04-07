/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2005,2011 Hubert Figuiere
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

#ifndef AP_UNIXDIALOG_WORDCOUNT_H
#define AP_UNIXDIALOG_WORDCOUNT_H

#include <string>

#include "ap_Dialog_WordCount.h"
#include "ut_timer.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_WordCount: public AP_Dialog_WordCount
{
public:
	AP_UnixDialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_WordCount(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events

	void			event_OK(void);
	void			event_WindowDelete(void);

protected:
	virtual XAP_Widget *getWidget(xap_widget_id wid);
	virtual void constructDialog(void);

	static void s_response(GtkWidget * wid, gint id, AP_UnixDialog_WordCount * me ) ;

	// private construction functions
//	GtkWidget * _constructWindow(void);

	static void                     autoupdateWC(UT_Worker * pTimer);

	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;

	// Labels for the Word Count data
	GtkWidget * m_labelWCount;
	GtkWidget * m_labelWNoFootnotesCount;
	GtkWidget * m_labelPCount;
	GtkWidget * m_labelCCount;
	GtkWidget * m_labelCNCount;
	GtkWidget * m_labelLCount;
	GtkWidget * m_labelPgCount;
	GtkWidget * m_labelLabelWCount;
	GtkWidget * m_labelWNFCount;
	GtkWidget * m_labelLabelPCount;
	GtkWidget * m_labelLabelCCount;
	GtkWidget * m_labelLabelCNCount;
	GtkWidget * m_labelLabelLCount;
	GtkWidget * m_labelLabelPgCount;
	GtkWidget * m_labelTitle;
	std::string m_labelTitleMarkupFormat;

	UT_Timer * m_pAutoUpdateWC;

	// Handshake variables
	bool m_bDestroy_says_stopupdating;
	bool m_bAutoUpdate_happening_now;
};

#endif /* AP_UNIXDIALOG_WORDCOUNT_H */








