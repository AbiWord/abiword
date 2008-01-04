/* Copyright (C) 2006,2007 Marc Maurer <uwog@uwog.net>
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
	autoconnect_button(NULL)
{
}

void ServiceUnixAccountHandler::embedDialogWidgets(void* pEmbeddingParent)
{
	UT_DEBUGMSG(("ServiceUnixAccountHandler::embedDialogWidgets()\n"));
	UT_return_if_fail(pEmbeddingParent);

	table = gtk_table_new(2, 2, FALSE);
	GtkVBox* parent = (GtkVBox*)pEmbeddingParent;
	
	// username	
	GtkWidget* username_label = gtk_label_new("Username:");
	gtk_misc_set_alignment(GTK_MISC(username_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), username_label, 0, 1, 0, 1);
	username_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table), username_entry, 1, 2, 0, 1);

	// password
	GtkWidget* password_label = gtk_label_new("Password:");
	gtk_misc_set_alignment(GTK_MISC(password_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), password_label, 0, 1, 1, 2);
	password_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(password_entry), false);
	gtk_table_attach_defaults(GTK_TABLE(table), password_entry, 1, 2, 1, 2);
	
	// autoconnect
	autoconnect_button = gtk_check_button_new_with_label ("Connect on application startup");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoconnect_button), true);
	gtk_table_attach_defaults(GTK_TABLE(table), autoconnect_button, 0, 2, 4, 5);
	
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

void ServiceUnixAccountHandler::storeProperties()
{
	UT_DEBUGMSG(("ServiceUnixAccountHandler::storeProperties()\n"));	

	if (username_entry && GTK_IS_ENTRY(username_entry))
		addProperty("username", gtk_entry_get_text(GTK_ENTRY(username_entry)));

	if (password_entry && GTK_IS_ENTRY(password_entry))
		addProperty("password", gtk_entry_get_text(GTK_ENTRY(password_entry)));
	
	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		addProperty("autoconnect", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoconnect_button)) ? "true" : "false" );	
}
