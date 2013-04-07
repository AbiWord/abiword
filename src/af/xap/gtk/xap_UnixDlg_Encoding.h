/* AbiSource Application Framework
 * Copyright (C) 2001-2002 AbiSource, Inc.
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

#ifndef XAP_UNIXDIALOG_ENCODING_H
#define XAP_UNIXDIALOG_ENCODING_H

#include "ut_types.h"
#include "ut_xml.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_UnixDialogHelper.h"
#include "xap_Dialog.h"
#include "ut_Encoding.h"
#include "xap_Dlg_Encoding.h"

class XAP_Frame;

class XAP_UnixDialog_Encoding : public XAP_Dialog_Encoding
{
public:
	XAP_UnixDialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Encoding(void);

	virtual void			runModal(XAP_Frame * pFrame);
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

 private:

	void event_Ok (void);
	void event_Cancel (void);

	static void s_encoding_dblclicked(GtkTreeView *treeview,
									  GtkTreePath *arg1,
									  GtkTreeViewColumn *arg2,
									  XAP_UnixDialog_Encoding * me);

	GtkWidget     * _constructWindow(void);
	void		_populateWindowData(void);

	GtkWidget * m_windowMain;
	GtkWidget * m_listEncodings;
};
#endif /* XAP_UNIXDIALOG_ENCODING_H */





