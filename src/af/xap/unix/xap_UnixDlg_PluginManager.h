/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef XAP_UNIXDIALOG_PLUGIN_MANAGER_H
#define XAP_UNIXDIALOG_PLUGIN_MANAGER_H

#include <gtk/gtk.h>

#include "xap_App.h"
#include "xap_Dlg_PluginManager.h"

class XAP_Frame;

class XAP_UnixDialog_PluginManager : public XAP_Dialog_PluginManager
{
public:
	XAP_UnixDialog_PluginManager(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_PluginManager(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	void event_DeactivateAll ();
	void event_Deactivate ();
	void event_Load();

protected:
	GtkWidget * m_windowMain;

	void _constructWindowContents (GtkWidget * container);
	virtual GtkWidget * _constructWindow ();

private:
	void _refreshAll ();

	GtkWidget * m_clist;
	GtkWidget * m_name;
	GtkWidget * m_author;
	GtkWidget * m_version;
	GtkWidget * m_desc;

	XAP_Frame * m_pFrame;
};

#endif /* PLUGIN_MANAGER_H */
