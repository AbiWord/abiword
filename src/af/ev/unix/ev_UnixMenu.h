/* AbiSource Program Utilities
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

#ifndef EV_UNIXMENU_H
#define EV_UNIXMENU_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ev_Menu.h"
#include "ap_Menu_Id.h"
class FV_View;
class AP_UnixApp;
class AP_UnixFrame;

/*****************************************************************/

class EV_UnixMenu : public EV_Menu
{
public:
	EV_UnixMenu(AP_UnixApp * pUnixApp, AP_UnixFrame * pUnixFrame,
				const char * szMenuLayoutName,
				const char * szMenuLabelSetName);
	~EV_UnixMenu(void);

	UT_Bool				synthesize(void);
	UT_Bool				menuEvent(AP_Menu_Id id);
	UT_Bool				refreshMenu(FV_View * pView);

	AP_UnixFrame * 		getFrame(void);
	
protected:
	void				_append_NormalItem(char * bufMenuPathname,
										   const char * szLabelName,
										   AP_Menu_Id id,
										   UT_Bool bCheckable);
	void				_append_SubMenu(char * bufMenuPathname,
										const char * szLabelName,
										AP_Menu_Id id);
	void				_end_SubMenu(char * bufMenuPathname);
	void				_append_Separator(char * bufMenuPathname,
										  AP_Menu_Id id);

	UT_Bool				_refreshMenu(FV_View * pView);
	const char *		_getItemPath(AP_Menu_Id id) const;
	UT_Bool				_isItemPresent(AP_Menu_Id id) const;

	AP_UnixApp *		m_pUnixApp;
	AP_UnixFrame *		m_pUnixFrame;

	GtkWidget *			m_wMenuBar;
	GtkWidget * 		m_wHandleBox;
	GtkAccelGroup *		m_wAccelGroup;
	GtkItemFactory *	m_wMenuBarItemFactory;
	GtkItemFactoryEntry * m_menuFactoryItems;
	UT_uint32			m_nrActualFactoryItems;
	UT_Vector			m_vecMenuWidgets;
};

#endif /* EV_UNIXMENU_H */
