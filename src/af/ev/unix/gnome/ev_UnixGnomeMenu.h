/* AbiSource Program Utilities
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef EV_UNIXGNOMEMENU_H
#define EV_UNIXGNOMEMENU_H

#include <gnome.h>

#include "ev_UnixMenu.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Menu.h"

class XAP_UnixGnomeApp;
class XAP_UnixGnomeFrame;

/*****************************************************************/

class EV_UnixGnomeMenu : public EV_UnixMenu
{
public:
	EV_UnixGnomeMenu(XAP_UnixApp * pUnixApp,
						XAP_UnixFrame * pUnixFrame,
						const char * szMenuLayoutName,
						const char * szMenuLabelSetName);
	virtual ~EV_UnixGnomeMenu(void);

	virtual UT_Bool		refreshMenu(AV_View * pView) = 0;
	UT_Bool				synthesizeMenu(GtkWidget * wMenuRoot);
	static void			menuEvent(GtkWidget *w, gpointer);

protected:
	GnomeUIInfo *       _convertMenu2UIInfo (int &pos);
	void                _destroyUIInfo (GnomeUIInfo *uiinfo);
	void                _attachWidgetsAndSignals(GtkWidget * wMenuRoot, GnomeUIInfo * uiinfo);
	UT_Bool             _refreshMenu(AV_View * pView, GtkWidget * wMenuRoot);
	void                _convertString2Accel(const char *s, guint &accel_key, GdkModifierType &ac_mods);

	// static functions
	static void         s_getStockPixmapFromName (const char *name, char *pixmap_name, int n);
	static void         s_onMenuItemSelect(GtkWidget * widget, gpointer data);
	static void         s_onMenuItemDeselect(GtkWidget * widget, gpointer data);
	static void         s_onInitMenu(GtkMenuItem * menuItem, gpointer data);
	static void         s_onDestroyMenu(GtkMenuItem * menuItem, gpointer data);
	//	static void         s_onDestroyPopupMenu(GtkMenuItem * menuItem, gpointer callback_data);

	GnomeUIInfo *       m_pUIInfo;
};

#endif /* EV_UNIXGNOMEMENU_H */
