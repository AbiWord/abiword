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

#ifndef AP_UNIXDIALOG_GOTO_H
#define AP_UNIXDIALOG_GOTO_H

#include "ap_Dialog_Goto.h"
class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Goto: public AP_Dialog_Goto
{
public:
	AP_UnixDialog_Goto(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Goto(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void                    notifyActiveFrame(XAP_Frame *pFrame);
	void					setSelectedRow(int row);
	int						getSelectedRow(void);

	/* CALLBACKS */
	static void				s_targetChanged(GtkWidget *clist, gint row, gint column,
											GdkEventButton *event, AP_UnixDialog_Goto *me);
	static void				s_dataChanged (GtkWidget *widget, AP_UnixDialog_Goto * me);
	static void				s_goto (const char *number, AP_UnixDialog_Goto * me);
	static void				s_gotoClicked (GtkWidget * widget, AP_UnixDialog_Goto * me);
	static void				s_nextClicked (GtkWidget * widget, AP_UnixDialog_Goto * me);
	static void				s_prevClicked (GtkWidget * widget, AP_UnixDialog_Goto * me);
	static void				s_closeClicked (GtkWidget * widget, AP_UnixDialog_Goto * me);
	static void				s_deleteClicked (GtkWidget * widget, gpointer /* data */ , AP_UnixDialog_Goto * me);

	/* Widgets members.  Publics to make them accesible to the callbacks */
	/* TODO: Convert them to private members, and add an inline accesor/mutator per member */
	GtkWidget *				m_wMainWindow;
	GtkWidget *				m_wEntry;
	GtkWidget *				m_wPrev;
	GtkWidget *				m_wNext;
	GtkWidget *				m_wGoto;
	GtkWidget *				m_wClose;
	GtkAccelGroup *			m_accelGroup;
	int						m_iRow;
	
protected:
	virtual GtkWidget *		_constructWindow(void);
	GtkWidget *				_constructWindowContents(void);
	void					_populateWindowData(void);
	void					_connectSignals(void);

	static char *			s_convert(const char * st);
};

#endif /* AP_UNIXDIALOG_GOTO_H */



