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
 
extern "C" {
#include <gnome.h>
#ifdef HAVE_BONOBO
#include <libgnorba/gnorba.h>
#include <bonobo/gnome-bonobo.h>
#endif
}

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "ut_types.h"
#include "ut_stack.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Types.h"
#include "ev_UnixGnomeMenu.h"
#include "xap_UnixGnomeApp.h"
#include "xap_UnixGnomeFrame.h"
#include "ev_UnixKeyboard.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"

/*****************************************************************/

EV_UnixGnomeMenuBar::EV_UnixGnomeMenuBar(XAP_UnixApp * pUnixApp,
										 XAP_UnixFrame * pUnixFrame,
										 const char * szMenuLayoutName,
										 const char * szMenuLabelSetName)
	: EV_UnixMenuBar(/*static_cast<XAP_UnixApp *>*/ pUnixApp, /*static_cast<XAP_UnixFrame *>*/ pUnixFrame,szMenuLayoutName,szMenuLabelSetName)
{
}

EV_UnixGnomeMenuBar::~EV_UnixGnomeMenuBar(void)
{
}

UT_Bool EV_UnixGnomeMenuBar::synthesizeMenuBar(void)
{
	// Just create, don't show the menu bar yet.  It is later added
	// to a 3D handle box and shown
	m_wMenuBar = gtk_menu_bar_new();

	synthesizeMenu(m_wMenuBar);
	
	// show up the properly connected menu structure
	gtk_widget_show(m_wMenuBar);

	// pack it in a handle box
	gnome_app_set_menus(GNOME_APP(m_pUnixFrame->getTopLevelWindow()), GTK_MENU_BAR(m_wMenuBar));

	return UT_TRUE;
}
