/* AbiSource Program Utilities
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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


#include "ev_GnomeToolbar.h"
#include <gconf/gconf-client.h>

#define PATH "/desktop/gnome/interface"
#define KEY_DETACHABLE "/desktop/gnome/interface/toolbar_detachable"
#define KEY_STYLE "/desktop/gnome/interface/toolbar_style"

/*!
* "detachable toolbars" value changed in gconf
*/
static void detachable_changed_cb(GConfClient *client,
								  guint        cnxn_id,
								  GConfEntry  *entry,
								  gpointer     data)
{
	EV_GnomeToolbar *self = reinterpret_cast<EV_GnomeToolbar*>(data);
	GConfValue *value;
	gboolean    detachable;
	
	value = gconf_entry_get_value (entry);
	detachable = gconf_value_get_bool (value);
	self->setDetachable(detachable);
}

#if HONOR_GNOME_TOOLBAR_SETTINGS
/*!
* "toolbar style" value changed in gconf
*/
static void style_changed_cb(GConfClient *client,
							 guint        cnxn_id,
							 GConfEntry  *entry,
							 gpointer     data)
{
	EV_GnomeToolbar *self = reinterpret_cast<EV_GnomeToolbar*>(data);
	GConfValue *value;
	const gchar *s;
	GtkToolbarStyle style;
	
	value = gconf_entry_get_value (entry);
	s = gconf_value_get_string (value);

	style = GTK_TOOLBAR_ICONS;
	if (g_utf8_collate(s,"text")==0)
		style = GTK_TOOLBAR_TEXT;
	else if (g_utf8_collate(s,"both")==0)
		style = GTK_TOOLBAR_BOTH;

	self->setStyle(style);
}
#endif


/*!
* Init parent and hook gconf.
*/
EV_GnomeToolbar::EV_GnomeToolbar(XAP_UnixApp * pUnixApp, 
								 XAP_Frame *pFrame, 
								 const char * szToolbarLayoutName,
								 const char * szToolbarLabelSetName)
 :  EV_UnixToolbar(pUnixApp, pFrame, szToolbarLayoutName, szToolbarLabelSetName)
{

	GConfClient *client;

	client = gconf_client_get_default();

	gconf_client_add_dir (client,
						  PATH,
						  GCONF_CLIENT_PRELOAD_NONE,
						  NULL);

	gconf_client_notify_add (client,
							 KEY_DETACHABLE,
							 detachable_changed_cb,
							 (gpointer)this,
							 NULL,
							 NULL);

#if HONOR_GNOME_TOOLBAR_SETTINGS
	gconf_client_notify_add (client,
							 KEY_STYLE,
							 style_changed_cb,
							 (gpointer)this,
							 NULL,
							 NULL);
#endif

	g_object_unref(G_OBJECT(client));
}

/*!
* Get toolbar style setting.
*/
GtkToolbarStyle EV_GnomeToolbar::getStyle(void)
{
#if HONOR_GNOME_TOOLBAR_SETTINGS
	GConfClient 	*client;
	const gchar 	*style;
	
	client = gconf_client_get_default();
	style = gconf_client_get_string(client, KEY_STYLE, NULL);
	g_object_unref(G_OBJECT(client));

	if (g_utf8_collate(style, "text") == 0)
		return GTK_TOOLBAR_TEXT;
	
	if (g_utf8_collate(style, "both") == 0)
		return GTK_TOOLBAR_BOTH;
	
	return GTK_TOOLBAR_ICONS;
#else
	return EV_UnixToolbar::getStyle();
#endif
}

/*!
* Set toolbar style setting.
*/
void EV_GnomeToolbar::setStyle(GtkToolbarStyle style)
{
	gtk_toolbar_set_style(GTK_TOOLBAR(m_wToolbar), style);
}

/*!
* Get "toolbars detachable" setting.
*/
bool EV_GnomeToolbar::getDetachable(void)
{
	GConfClient 	*client;
	bool			 detachable;
	
	client = gconf_client_get_default();
	detachable = gconf_client_get_bool(client, KEY_DETACHABLE, NULL);
	g_object_unref(G_OBJECT(client));
	return detachable;
}

/*!
* Set "toolbars detachable" setting.
*/
void EV_GnomeToolbar::setDetachable(gboolean detachable)
{
	if (detachable && GTK_IS_TOOLBAR(gtk_bin_get_child(GTK_BIN(m_wHandleBox)))) {
		// not detachable -> detachable
		GtkWidget *box = gtk_handle_box_new();
		gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(box), GTK_SHADOW_NONE);
		gtk_widget_reparent(m_wToolbar, box);
		gtk_container_add(GTK_CONTAINER(m_wHandleBox), box);
		if (!isHidden()) {
			gtk_widget_show(box);
		}
	}
	else if (!detachable && GTK_IS_HANDLE_BOX(gtk_bin_get_child(GTK_BIN(m_wHandleBox)))) {
		// detachable -> not detachable
		GtkWidget *box = gtk_bin_get_child(GTK_BIN(m_wHandleBox));
		g_object_ref(G_OBJECT(box));
		gtk_container_remove(GTK_CONTAINER(m_wHandleBox), box);
		gtk_widget_reparent(m_wToolbar, m_wHandleBox);
		g_object_unref(G_OBJECT(box));
	}
}
