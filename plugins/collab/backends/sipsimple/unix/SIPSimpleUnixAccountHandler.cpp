/* Copyright (C) 2010 AbiSource Corporation B.V.
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

#include "SIPSimpleUnixAccountHandler.h"

AccountHandlerConstructor SIPSimpleAccountHandlerConstructor = &SIPSimpleUnixAccountHandler::static_constructor;

SIPSimpleUnixAccountHandler::SIPSimpleUnixAccountHandler()
	: SIPSimpleAccountHandler(),
	table(NULL),
	address_entry(NULL),
	password_entry(NULL),
	proxy_entry(NULL),
	autoconnect_button(NULL)
{
}

AccountHandler * SIPSimpleUnixAccountHandler::static_constructor()
{
	return static_cast<AccountHandler *>(new SIPSimpleUnixAccountHandler());
}

void SIPSimpleUnixAccountHandler::embedDialogWidgets(void* pEmbeddingParent)
{
	UT_return_if_fail(pEmbeddingParent);

	table = gtk_grid_new();
	g_object_set(G_OBJECT(table),
	             "row-spacing", 6,
	             "column-spacing", 12,
	             "hexpand", true,
	             NULL);
	GtkBox* parent = (GtkBox*)pEmbeddingParent;

	// username
	GtkWidget* address_label = gtk_widget_new(GTK_TYPE_LABEL,
                                                  "label", "SIP address:",
                                                  "xalign", 0.0, "yalign", 0.5,
                                                  NULL);
	gtk_grid_attach(GTK_GRID(table), address_label, 0, 0, 1, 1);
	address_entry = gtk_entry_new();
	gtk_grid_attach(GTK_GRID(table), address_entry, 1, 0, 1, 1);
	gtk_entry_set_activates_default(GTK_ENTRY(address_entry), true);

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

	// outbound proxy
	GtkWidget* proxy_label = gtk_widget_new(GTK_TYPE_LABEL,
                                                "label", "Outbound proxy:",
                                                "xalign", 0.0, "yalign", 0.5,
                                                NULL);
	gtk_grid_attach(GTK_GRID(table), proxy_label, 0, 2, 1, 1);
	proxy_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(proxy_entry), false);
	gtk_grid_attach(GTK_GRID(table), proxy_entry, 1, 2, 1, 1);
	gtk_entry_set_activates_default(GTK_ENTRY(proxy_entry), true);

	// autoconnect
	autoconnect_button = gtk_check_button_new_with_label ("Connect on application startup");
	gtk_widget_set_hexpand(autoconnect_button, true);
	gtk_grid_attach(GTK_GRID(table), autoconnect_button, 0, 3, 2, 1);

	gtk_box_pack_start(GTK_BOX(parent), table, false, TRUE, 0);
	gtk_widget_show_all(GTK_WIDGET(parent));
}

void SIPSimpleUnixAccountHandler::removeDialogWidgets(void* /*pEmbeddingParent*/)
{
	UT_DEBUGMSG(("SIPSimpleUnixAccountHandler::removeDialogWidgets\n"));

	// this will conveniently destroy all contained widgets as well
	if (table && GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);
}

void SIPSimpleUnixAccountHandler::loadProperties()
{
	if (address_entry && GTK_IS_ENTRY(address_entry))
		gtk_entry_set_text(GTK_ENTRY(address_entry), getProperty("address").c_str());

	if (password_entry && GTK_IS_ENTRY(password_entry))
		gtk_entry_set_text(GTK_ENTRY(password_entry), getProperty("password").c_str());

	if (proxy_entry && GTK_IS_ENTRY(proxy_entry))
		gtk_entry_set_text(GTK_ENTRY(proxy_entry), getProperty("outbound-proxy").c_str());

	bool autoconnect = hasProperty("autoconnect") ? getProperty("autoconnect") == "true" : true;
	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoconnect_button), autoconnect);
}

void SIPSimpleUnixAccountHandler::storeProperties()
{
	if (address_entry && GTK_IS_ENTRY(address_entry))
		addProperty("address", gtk_entry_get_text(GTK_ENTRY(address_entry)));

	if (password_entry && GTK_IS_ENTRY(password_entry))
		addProperty("password", gtk_entry_get_text(GTK_ENTRY(password_entry)));

	if (proxy_entry && GTK_IS_ENTRY(proxy_entry))
		addProperty("outbound-proxy", gtk_entry_get_text(GTK_ENTRY(proxy_entry)));

	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		addProperty("autoconnect", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoconnect_button)) ? "true" : "false" );
}
