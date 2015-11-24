/* 
 * Copyright (C) 2006 by Marc Maurer <uwog@uwog.net>
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

#include "XMPPUnixAccountHandler.h"

AccountHandlerConstructor XMPPAccountHandlerConstructor = &XMPPUnixAccountHandler::static_constructor;

XMPPUnixAccountHandler::XMPPUnixAccountHandler()
	: XMPPAccountHandler(),
	table(NULL),
	username_entry(NULL),
	password_entry(NULL),
	server_entry(NULL),
	port_entry(NULL),
	starttls_button(NULL),
	autoconnect_button(NULL)
{
}

AccountHandler * XMPPUnixAccountHandler::static_constructor()
{
	return static_cast<AccountHandler *>(new XMPPUnixAccountHandler());
}

void XMPPUnixAccountHandler::embedDialogWidgets(void* pEmbeddingParent)
{
	UT_return_if_fail(pEmbeddingParent);

	table = gtk_grid_new();
	g_object_set(G_OBJECT(table),
	             "row-spacing", 6,
	             "column-spacing", 12,
	             "hexpand", true,
	             NULL);
	GtkVBox* parent = (GtkVBox*)pEmbeddingParent;

	// username
	GtkWidget* username_label = gtk_widget_new(GTK_TYPE_LABEL,
                                                   "label", "Username:",
                                                   "xalign", 0.0, "yalign", 0.5,
                                                   NULL);
	gtk_grid_attach(GTK_GRID(table), username_label, 0, 0, 1, 1);
	username_entry = gtk_entry_new();
	gtk_widget_set_hexpand(username_entry, true);
	gtk_grid_attach(GTK_GRID(table), username_entry, 1, 0, 1, 1);
	gtk_entry_set_activates_default(GTK_ENTRY(username_entry), true);

	// password
	GtkWidget* password_label = gtk_widget_new(GTK_TYPE_LABEL,
                                                   "label", "Password:",
                                                   "xalign", 0.0, "yalign", 0.5,
                                                   NULL);
	gtk_grid_attach(GTK_GRID(table), password_label, 0, 1, 1, 1);
	password_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(password_entry), false);
	gtk_grid_attach(GTK_GRID(table), password_entry, 1, 1, 1, 1);
	gtk_entry_set_activates_default(GTK_ENTRY(password_entry), true);

	// server
	GtkWidget* server_label = gtk_widget_new(GTK_TYPE_LABEL,
                                                "label", "Server:",
                                                 "xalign", 0.0, "yalign", 0.5,
                                                 NULL);
	gtk_grid_attach(GTK_GRID(table), server_label, 0, 2, 1, 1);
	server_entry = gtk_entry_new();
	gtk_grid_attach(GTK_GRID(table), server_entry, 1, 2, 1, 1);
	gtk_entry_set_activates_default(GTK_ENTRY(server_entry), true);

	// port
	GtkWidget* port_label = gtk_widget_new(GTK_TYPE_LABEL,
                                               "label", "Port:",
                                               "xalign", 0.0, "yalign", 0.5,
                                               NULL);
	gtk_grid_attach(GTK_GRID(table), port_label, 0, 3, 1, 1);
	port_entry = gtk_entry_new(); // TODO: should be a numerical entry
	gtk_grid_attach(GTK_GRID(table), port_entry, 1, 3, 1, 1);
	gtk_entry_set_activates_default(GTK_ENTRY(port_entry), true);

	// Encryption
	starttls_button = gtk_check_button_new_with_label("Use StartTLS Encryption");
	gtk_grid_attach(GTK_GRID(table), starttls_button, 0, 4, 2, 1);
	if (!lm_ssl_is_supported())
		gtk_widget_set_sensitive(starttls_button, FALSE);

	// autoconnect
	autoconnect_button = gtk_check_button_new_with_label ("Connect on application startup");
	gtk_grid_attach(GTK_GRID(table), autoconnect_button, 0, 5, 2, 1);

	gtk_box_pack_start(GTK_BOX(parent), table, false, TRUE, 0);
	gtk_widget_show_all(GTK_WIDGET(parent));

	// some convenient default values
	gtk_entry_set_text(GTK_ENTRY(port_entry), "5222");
}

void XMPPUnixAccountHandler::removeDialogWidgets(void* /*pEmbeddingParent*/)
{
	UT_DEBUGMSG(("XMPPUnixAccountHandler::removeDialogWidgets\n"));
	
	// this will conveniently destroy all contained widgets as well
	if (table && GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);
}

void XMPPUnixAccountHandler::loadProperties()
{
	if (username_entry && GTK_IS_ENTRY(username_entry))
		gtk_entry_set_text(GTK_ENTRY(username_entry), getProperty("username").c_str());

	if (password_entry && GTK_IS_ENTRY(password_entry))
		gtk_entry_set_text(GTK_ENTRY(password_entry), getProperty("password").c_str());

	if (server_entry && GTK_IS_ENTRY(server_entry))
		gtk_entry_set_text(GTK_ENTRY(server_entry), getProperty("server").c_str());

	if (port_entry && GTK_IS_ENTRY(server_entry))
		gtk_entry_set_text(GTK_ENTRY(port_entry), getProperty("port").c_str());

	bool tls = hasProperty("encryption") ? getProperty("encryption") == "true" : false;
	if (lm_ssl_is_supported() && starttls_button && GTK_IS_TOGGLE_BUTTON(starttls_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(starttls_button), tls);

	bool autoconnect = hasProperty("autoconnect") ? getProperty("autoconnect") == "true" : true;
	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoconnect_button), autoconnect);
}

void XMPPUnixAccountHandler::storeProperties()
{
	if (username_entry && GTK_IS_ENTRY(username_entry))
		addProperty("username", gtk_entry_get_text(GTK_ENTRY(username_entry)));

	if (password_entry && GTK_IS_ENTRY(password_entry))
		addProperty("password", gtk_entry_get_text(GTK_ENTRY(password_entry)));

	if (server_entry && GTK_IS_ENTRY(server_entry))
		addProperty("server", gtk_entry_get_text(GTK_ENTRY(server_entry)));
		
	if (port_entry && GTK_IS_ENTRY(server_entry))
		addProperty("port", gtk_entry_get_text(GTK_ENTRY(port_entry)));	
	
	if (lm_ssl_is_supported() && starttls_button && GTK_IS_TOGGLE_BUTTON(starttls_button))
		addProperty("encryption", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(starttls_button)) ? "true" : "false" );
		
	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		addProperty("autoconnect", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoconnect_button)) ? "true" : "false" );
		
	// TODO: make this a global define
	addProperty("resource", "abicollab_protocol");
}
