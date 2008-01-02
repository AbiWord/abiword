/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
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

#ifndef AP_UNIXSTATUSBAR_H
#define AP_UNIXSTATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame window.

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ap_StatusBar.h"
class XAP_Frame;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_UnixStatusBar : public AP_StatusBar
{
public:
	AP_UnixStatusBar(XAP_Frame * pFrame);
	virtual ~AP_UnixStatusBar(void);

	virtual void		setView(AV_View * pView);
	GtkWidget *			createWidget(void);

	virtual void		show(void);
	virtual void		hide(void);

protected:
	GtkWidget *			m_wStatusBar;
};

#endif /* AP_UNIXSTATUSBAR_H */
