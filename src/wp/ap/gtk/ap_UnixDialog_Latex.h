/* AbiWord
 * Copyright (C) 2005 Martin Sevior
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

#ifndef AP_UNIXDIALOG_LATEX_H
#define AP_UNIXDIALOG_LATEX_H

#include "ap_Dialog_Latex.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Latex: public AP_Dialog_Latex
{
public:
	AP_UnixDialog_Latex(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Latex(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events

	void			event_Insert(void);
	void			event_Close(void);
	void			event_WindowDelete(void);
	virtual void            setLatexInGUI(void);
	virtual bool            getLatexFromGUI(void);

protected:
	virtual void constructDialog(void);

	// pointers to widgets we need

	GtkWidget * m_wClose;
	GtkWidget * m_wInsert;
	GtkWidget * m_wText;
	GtkWidget * m_windowMain;
};

#endif /* AP_UNIXDIALOG_Latex_H */








