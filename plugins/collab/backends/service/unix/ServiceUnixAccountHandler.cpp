/* Copyright (C) 2006,2007 Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2008 by AbiSource Corporation B.V.
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

#include "ServiceUnixAccountHandler.h"

AccountHandlerConstructor ServiceAccountHandlerConstructor = &ServiceUnixAccountHandler::static_constructor;

AccountHandler * ServiceUnixAccountHandler::static_constructor()
{
	return static_cast<AccountHandler *>(new ServiceUnixAccountHandler());
}

ServiceUnixAccountHandler::ServiceUnixAccountHandler()
	: ServiceAccountHandler(),
	table(NULL),
	username_entry(NULL),
	password_entry(NULL),
	autoconnect_button(NULL),
	register_button(NULL)
#if DEBUG
	, uri_entry(NULL),
	verify_webapp_host_button(NULL),
	verify_realm_host_button(NULL)
#endif
{
}

void ServiceUnixAccountHandler::embedDialogWidgets(void* pEmbeddingParent)
{
	UT_DEBUGMSG(("ServiceUnixAccountHandler::embedDialogWidgets()\n"));
	UT_return_if_fail(pEmbeddingParent);

	table = gtk_table_new(2, 2, FALSE);
	GtkVBox* parent = (GtkVBox*)pEmbeddingParent;

	// username
	GtkWidget* username_label
          = gtk_widget_new(GTK_TYPE_LABEL, "label", "E-mail address:",
                           "xalign", 0.0, "yalign", 0.5,
                           NULL);
	gtk_table_attach_defaults(GTK_TABLE(table), username_label, 0, 1, 0, 1);
	username_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table), username_entry, 1, 2, 0, 1);
	gtk_entry_set_activates_default(GTK_ENTRY(username_entry), true);

	// password
	GtkWidget* password_label
          = gtk_widget_new(GTK_TYPE_LABEL, "label", "Password:",
                           "xalign", 0.0, "yalign", 0.5,
                           NULL);
	gtk_table_attach_defaults(GTK_TABLE(table), password_label, 0, 1, 1, 2);
	password_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(password_entry), false);
	gtk_table_attach_defaults(GTK_TABLE(table), password_entry, 1, 2, 1, 2);
	gtk_entry_set_activates_default(GTK_ENTRY(password_entry), true);

	// autoconnect
	autoconnect_button = gtk_check_button_new_with_label ("Connect on application startup");
	gtk_table_attach_defaults(GTK_TABLE(table), autoconnect_button, 0, 2, 2, 3);

	// register
	register_button = gtk_link_button_new_with_label (SERVICE_REGISTRATION_URL, "Get a free abicollab.net account");
	gtk_table_attach_defaults(GTK_TABLE(table), register_button, 0, 2, 3, 4);

#ifdef DEBUG
	// uri
	GtkWidget* uri_label
          = gtk_widget_new(GTK_TYPE_LABEL, "label", "WebApp SOAP url:",
                           "xalign", 0.0, "yalign", 0.5,
                           NULL);
	gtk_table_attach_defaults(GTK_TABLE(table), uri_label, 0, 1, 4, 5);
	uri_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table), uri_entry, 1, 2, 4, 5);

	// check webapp hostname
	verify_webapp_host_button = gtk_check_button_new_with_label ("Verify WebApp hostname");
	gtk_table_attach_defaults(GTK_TABLE(table), verify_webapp_host_button, 0, 2, 5, 6);

	// check realm hostname
	verify_realm_host_button = gtk_check_button_new_with_label ("Verify Realm hostname");
	gtk_table_attach_defaults(GTK_TABLE(table), verify_realm_host_button, 0, 2, 6, 7);
#endif

	gtk_box_pack_start(GTK_BOX(parent), table, FALSE, TRUE, 0);
	gtk_widget_show_all(GTK_WIDGET(parent));
}

void ServiceUnixAccountHandler::removeDialogWidgets(void* pEmbeddingParent)
{
	UT_DEBUGMSG(("ServiceUnixAccountHandler::removeDialogWidgets()\n"));
	UT_return_if_fail(pEmbeddingParent);
	
	// this will conveniently destroy all contained widgets as well
	if (table && GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);
}

void ServiceUnixAccountHandler::loadProperties()
{
	UT_DEBUGMSG(("ServiceUnixAccountHandler::loadProperties()\n"));

	if (username_entry && GTK_IS_ENTRY(username_entry))
		gtk_entry_set_text(GTK_ENTRY(username_entry), getProperty("email").c_str());

	if (password_entry && GTK_IS_ENTRY(password_entry))
		gtk_entry_set_text(GTK_ENTRY(password_entry), getProperty("password").c_str());

	bool autoconnect = hasProperty("autoconnect") ? getProperty("autoconnect") == "true" : true;
	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoconnect_button), autoconnect);

#ifdef DEBUG
	std::string uri = hasProperty("uri") ? getProperty("uri").c_str() : "https://abicollab.net/soap/";
	if (uri_entry && GTK_IS_ENTRY(uri_entry))
		gtk_entry_set_text(GTK_ENTRY(uri_entry), uri.c_str());

	bool verify_webapp_host = hasProperty("verify-webapp-host") ? getProperty("verify-webapp-host") == "true" : true;
	if (verify_webapp_host_button && GTK_IS_TOGGLE_BUTTON(verify_webapp_host_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(verify_webapp_host_button), verify_webapp_host);

	bool verify_realm_host = hasProperty("verify-realm-host") ? getProperty("verify-realm-host") == "true" : false;
	if (verify_realm_host_button && GTK_IS_TOGGLE_BUTTON(verify_realm_host_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(verify_realm_host_button), verify_realm_host);
#endif
}

void ServiceUnixAccountHandler::storeProperties()
{
	UT_DEBUGMSG(("ServiceUnixAccountHandler::storeProperties()\n"));

	if (username_entry && GTK_IS_ENTRY(username_entry))
		addProperty("email", gtk_entry_get_text(GTK_ENTRY(username_entry)));

	if (password_entry && GTK_IS_ENTRY(password_entry))
		addProperty("password", gtk_entry_get_text(GTK_ENTRY(password_entry)));
	
	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
	{
		addProperty("autoconnect", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoconnect_button)) ? "true" : "false" );
		printf(">>> AUTOCONNECT SET TO: %s\n", getProperty("autoconnect").c_str());
	}

#ifdef DEBUG
	if (uri_entry && GTK_IS_ENTRY(uri_entry))
		addProperty("uri", gtk_entry_get_text(GTK_ENTRY(uri_entry)));

	if (verify_webapp_host_button && GTK_IS_TOGGLE_BUTTON(verify_webapp_host_button))
		addProperty("verify-webapp-host", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(verify_webapp_host_button)) ? "true" : "false" );

	if (verify_realm_host_button && GTK_IS_TOGGLE_BUTTON(verify_realm_host_button))
		addProperty("verify-realm-host", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(verify_realm_host_button)) ? "true" : "false" );
#else
	addProperty("uri", "https://abicollab.net/soap/");
	addProperty("verify-webapp-host", "true");
	addProperty("verify-realm-host", "false");
#endif
}
