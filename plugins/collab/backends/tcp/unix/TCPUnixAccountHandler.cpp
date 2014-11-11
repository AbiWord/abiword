/* Copyright (C) 2007 by Marc Maurer <uwog@uwog.net>
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

#include "boost/lexical_cast.hpp"
#include "TCPUnixAccountHandler.h"

#include "xap_Gtk2Compat.h"

AccountHandlerConstructor TCPAccountHandlerConstructor = &TCPUnixAccountHandler::static_constructor;

AccountHandler * TCPUnixAccountHandler::static_constructor()
{
	return static_cast<AccountHandler *>(new TCPUnixAccountHandler());
}

void s_group_changed(GtkToggleButton* /*button*/, TCPUnixAccountHandler* pHandler)
{
	pHandler->eventGroupChanged();
}

TCPUnixAccountHandler::TCPUnixAccountHandler()
	: TCPAccountHandler(),
	vbox(NULL),
	server_button(NULL),
	client_button(NULL),
	server_entry(NULL),
	port_button(NULL),
	allow_all_button(NULL),
	autoconnect_button(NULL)
{
}

void TCPUnixAccountHandler::embedDialogWidgets(void* pEmbeddingParent)
{
	UT_DEBUGMSG(("TCPUnixAccountHandler::embedDialogWidgets()\n"));
	UT_return_if_fail(pEmbeddingParent);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	GtkVBox* parent = (GtkVBox*)pEmbeddingParent;

	// host a session (we should really use a GtkAction for this)
	server_button = gtk_radio_button_new_with_label(NULL, "Accept incoming connections");
	gtk_box_pack_start(GTK_BOX(vbox), server_button, TRUE, TRUE, 0);

	// join a session
	client_button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(server_button), "Connect to a server");
	gtk_box_pack_start(GTK_BOX(vbox), client_button, TRUE, TRUE, 0);

	// add a table to hold the server and port options
	GtkWidget* table = gtk_table_new(1, 3, FALSE);

	// spacer
	GtkWidget* spacer = gtk_label_new("");
	gtk_widget_set_size_request(spacer, 12, -1);
	gtk_table_attach_defaults(GTK_TABLE(table), spacer, 0, 1, 0, 1);

	// host
	GtkWidget* server_label = gtk_widget_new(GTK_TYPE_LABEL,
                                                 "label", "Address:",
                                                 "xalign", 0.0, "yalign", 0.5,
                                                 NULL);
	gtk_table_attach_defaults(GTK_TABLE(table), server_label, 1, 2, 0, 1);
	server_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table), server_entry, 2, 3, 0, 1);
	gtk_widget_set_sensitive(server_entry, false);
	gtk_entry_set_activates_default(GTK_ENTRY(server_entry), true);

	gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);

	// port
	GtkWidget* portHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	GtkWidget* port_label = gtk_widget_new(GTK_TYPE_LABEL, "label", "Port:",
                                               "xalign", 0.0, "yalign", 0.5,
                                               NULL);
	gtk_box_pack_start(GTK_BOX(portHBox), port_label, false, false, 0);
	port_button = gtk_spin_button_new_with_range(1, 65536, 1);
	gtk_box_pack_start(GTK_BOX(portHBox), port_button, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), portHBox, false, false, 0);

	// allow-all
	allow_all_button = gtk_check_button_new_with_label("Automatically grant buddies access to shared documents");
	gtk_box_pack_start(GTK_BOX(vbox), allow_all_button, TRUE, TRUE, 0);

	// autoconnect
	autoconnect_button = gtk_check_button_new_with_label("Connect on application startup");
	gtk_box_pack_start(GTK_BOX(vbox), autoconnect_button, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(parent), vbox, FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(parent));

	// attach some signals
	g_signal_connect(G_OBJECT(server_button),
							"toggled",
							G_CALLBACK(s_group_changed),
							static_cast<gpointer>(this));
}

void TCPUnixAccountHandler::removeDialogWidgets(void* pEmbeddingParent)
{
	UT_DEBUGMSG(("TCPAccountHandler::removeDialogWidgets()\n"));
	UT_return_if_fail(pEmbeddingParent);

	// this will conveniently destroy all contained widgets as well
	if (vbox && GTK_IS_WIDGET(vbox))
		gtk_widget_destroy(vbox);
}

void TCPUnixAccountHandler::loadProperties()
{
	UT_DEBUGMSG(("TCPUnixAccountHandler::loadProperties()\n"));

	bool serve = getProperty("server") == "";

	if (server_button && GTK_IS_TOGGLE_BUTTON(server_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(server_button), serve);

	if (client_button && GTK_IS_TOGGLE_BUTTON(client_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(client_button), !serve);

	if (server_entry && GTK_IS_ENTRY(server_entry))
		gtk_entry_set_text(GTK_ENTRY(server_entry), getProperty("server").c_str());

	int port = DEFAULT_TCP_PORT;
	try {
		if (hasProperty("port"))
			port = boost::lexical_cast<int>(getProperty("port"));
	} catch (boost::bad_lexical_cast &) {
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
	if (port_button && GTK_IS_ENTRY(port_button))
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(port_button), port);

	if (allow_all_button && GTK_IS_TOGGLE_BUTTON(allow_all_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(allow_all_button), hasProperty("allow-all") ? getProperty("allow-all") == "true" : false);

	bool autoconnect = hasProperty("autoconnect") ? getProperty("autoconnect") == "true" : true;
	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoconnect_button), autoconnect);
}

void TCPUnixAccountHandler::storeProperties()
{
	UT_DEBUGMSG(("TCPUnixAccountHandler::storeProperties()\n"));
	
	bool serve = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(server_button));
	if (server_entry && GTK_IS_ENTRY(server_entry))
	{
		// simply clear the server field if we are are hosting this session
		addProperty("server", serve ? "" : gtk_entry_get_text(GTK_ENTRY(server_entry)));
	}
	
	if (port_button && GTK_IS_ENTRY(port_button))
		addProperty("port", gtk_entry_get_text(GTK_ENTRY(port_button)));
		
	if (allow_all_button && GTK_IS_TOGGLE_BUTTON(allow_all_button))
		addProperty("allow-all", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(allow_all_button)) ? "true" : "false" );

	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		addProperty("autoconnect", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoconnect_button)) ? "true" : "false" );
}

void TCPUnixAccountHandler::eventGroupChanged()
{
	UT_DEBUGMSG(("TCPUnixAccountHandler::eventGroupChanged()\n"));
	
	bool serve = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(server_button));
	gtk_widget_set_sensitive(server_entry, !serve);
}
