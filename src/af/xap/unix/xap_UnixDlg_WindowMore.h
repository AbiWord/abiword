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

#ifndef XAP_UNIXDIALOG_WINDOWMORE_H
#define XAP_UNIXDIALOG_WINDOWMORE_H

#include "xap_Dlg_WindowMore.h"
class XAP_UnixFrame;

/*****************************************************************/

class XAP_UnixDialog_WindowMore: public XAP_Dialog_WindowMore
{
public:
	XAP_UnixDialog_WindowMore(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~XAP_UnixDialog_WindowMore(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static AP_Dialog *		static_constructor(AP_DialogFactory *, AP_Dialog_Id id);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_Cancel(void);
	virtual void			event_DoubleClick(void);
	virtual void			event_WindowDelete(void);
	
protected:

	gint 		_GetFromList(void);
	GtkWidget * _constructWindow(void);
	void		_populateWindow(void);
	
	GtkWidget * m_windowMain;
	GtkWidget * m_clistWindows;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;
	
};

#endif /* XAP_UNIXDIALOG_WINDOWMORE_H */
