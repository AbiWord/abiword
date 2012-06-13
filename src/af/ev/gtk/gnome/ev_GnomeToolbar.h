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

#ifndef EV_GNOMETOOLBAR_H
#define EV_GNOMETOOLBAR_H

#include <gtk/gtk.h>
#include "ev_UnixToolbar.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

class EV_GnomeToolbar : public EV_UnixToolbar
{
public:
	EV_GnomeToolbar(XAP_UnixApp * pUnixApp, 
		       XAP_Frame *pFrame, 
		       const char * szToolbarLayoutName,
		       const char * szToolbarLabelSetName);

	void setStyle(GtkToolbarStyle style);
	bool getDetachable(void);
	void setDetachable(gboolean detachable);

protected:

	GtkToolbarStyle getStyle(void);
};

#endif /* EV_GNOMETOOLBAR_H */
