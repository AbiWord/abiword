/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2008 Ryan Pavlik <abiryan@ryand.net>
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

#ifndef AP_UNIXDIALOG_EDITSTYLE_H
#define AP_UNIXDIALOG_EDITSTYLE_H

#include "ap_Dialog_EditStyle.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_EditStyle: public AP_Dialog_EditStyle
{
public:
	AP_UnixDialog_EditStyle(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_EditStyle(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

protected:
	// private construction functions
	virtual GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	void 		_storeWindowData(void);
											   
	// pointers to widgets we need to query/set
	GtkWidget * 		m_windowMain;
	GtkWidget * 		m_enName;
	GtkWidget * 		m_cbBasedOn;
	GtkWidget * 		m_cbFollowedBy;
	GSList *			m_radioGroup;

	GtkWidget * 		m_wPropListContainer;
	GtkWidget * 		m_wPropList;

	GtkWidget * 		m_btAdd;
	GtkWidget * 		m_btRemove;
	GtkWidget * 		m_btClose;
	
	GtkCellRenderer * 	m_wNameRenderer;
	GtkCellRenderer * 	m_wValueRenderer;
	GtkCellRenderer *	m_wComboRenderer;
	GtkListStore * 		m_wModel;
	GtkListStore * 		m_wStyleModel;

};

#endif /* AP_UNIXDIALOG_EDITSTYLE_H */
