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

#ifndef __SIPSIMPLEUNIXACCOUNTHANDLER__
#define __SIPSIMPLEUNIXACCOUNTHANDLER__

#include "gtk/gtk.h"
#include <backends/sipsimple/xp/SIPSimpleAccountHandler.h>

class SIPSimpleUnixAccountHandler : public SIPSimpleAccountHandler
{
public:
	SIPSimpleUnixAccountHandler();
	static AccountHandler * static_constructor();

	// dialog management
	virtual void			embedDialogWidgets(void* pEmbeddingParent);
	virtual void			removeDialogWidgets(void* pEmbeddingParent);
	virtual void			loadProperties();
	virtual void			storeProperties();

private:
	// dialog management
	GtkWidget*				table;
	GtkWidget*				address_entry;
	GtkWidget*				password_entry;
	GtkWidget*				proxy_entry;
	GtkWidget*				autoconnect_button;
};

#endif /* __SIPSIMPLEUNIXACCOUNTHANDLER__ */
