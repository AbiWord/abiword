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

#ifndef XAP_QNXDIALOG_PLUGIN_MANAGER_H
#define XAP_QNXDIALOG_PLUGIN_MANAGER_H

#include "xap_App.h"
#include "xap_Dlg_PluginManager.h"

class XAP_Frame;

class XAP_QNXDialog_PluginManager : public XAP_Dialog_PluginManager
{
public:
	XAP_QNXDialog_PluginManager(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_QNXDialog_PluginManager(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	void event_DeactivateAll ();
	void event_Deactivate ();
	void event_Load ();
	void event_Close ();
	void event_Select1 ();

protected:
	PtWidget_t * m_windowMain;

	void _constructWindowContents (PtWidget_t * container);
	virtual PtWidget_t * _constructWindow ();

private:
	void _refreshAll ();
	void _refreshTab1 ();
	void _refreshTab2 ();

	PtWidget_t * m_clist;
	PtWidget_t * m_name;
	PtWidget_t * m_author;
	PtWidget_t * m_version;
	PtWidget_t * m_desc;

	XAP_Frame * m_pFrame;
	int 		done;
};

#endif /* PLUGIN_MANAGER_H */
