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

#ifndef EV_UNIXGNOMETOOLBAR_H
#define EV_UNIXGNOMETOOLBAR_H

#include <gtk/gtkwidget.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_UnixToolbar.h"
#include "xav_Listener.h"
#include "xap_UnixGnomeApp.h"
#include "xap_UnixGnomeFrame.h"

class AP_UnixToolbar_Icons;
class EV_UnixToolbar_ViewListener;

class _wd;

class EV_UnixGnomeToolbar : public EV_UnixToolbar
{
public:
	EV_UnixGnomeToolbar(XAP_UnixGnomeApp * pUnixApp, XAP_UnixGnomeFrame * pUnixFrame,
						const char * szToolbarLayoutName,
						const char * szToolbarLabelSetName);
	
	virtual ~EV_UnixGnomeToolbar(void);

	virtual UT_Bool synthesize(void);
	virtual void show(void);
	virtual void hide(void);
	virtual UT_Bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);

private:
	GtkWidget *_makeToolbar(void);
	UT_Bool _addToolbar (GtkWidget *toolbar);

	static int nbBands;
	int nbToolbarsInBand;
	UT_Vector m_vecToolbars;
};

#endif /* EV_UNIXGNOMETOOLBAR_H */
