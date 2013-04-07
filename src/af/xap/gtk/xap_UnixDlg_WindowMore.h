/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifndef XAP_UNIXDIALOG_WINDOWMORE_H
#define XAP_UNIXDIALOG_WINDOWMORE_H

#include <gtk/gtk.h>
#include "xap_Dlg_WindowMore.h"
class XAP_Frame;

/*****************************************************************/

class XAP_UnixDialog_WindowMore: public XAP_Dialog_WindowMore
{
public:
	XAP_UnixDialog_WindowMore(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_WindowMore(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

private:

	void			event_View(void);
	void			event_Cancel(void);

	static void s_list_dblclicked(GtkTreeView *treeview,
								  GtkTreePath *arg1,
								  GtkTreeViewColumn *arg2,
								  XAP_UnixDialog_WindowMore * me);

	GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);

	GtkWidget * m_windowMain;
	GtkWidget * m_listWindows;
};

#endif /* XAP_UNIXDIALOG_WINDOWMORE_H */
