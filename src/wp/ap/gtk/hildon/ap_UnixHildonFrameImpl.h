/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 William Lachance
 * Copyright (C) 2005 INdT
 * Author: Renato Araujo <renato.filho@indt.org.br>
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
 *
 */


#ifndef AP_UNIXHILDONFRAMEIMPL_H
#define AP_UNIXHILDONFRAMEIMPL_H

#include <gtk/gtkwidget.h>
#include <gtk/gtkadjustment.h>
#include <gdk/gdktypes.h>
#include <gtk/gtk.h>
#include "ap_UnixFrameImpl.h"
#include "ap_UnixFrame.h"
#include "xap_UnixApp.h"


/********************************************************************
*********************************************************************
** This file defines the unixhildon-platform-specific class for the
** cross-platform application frame helper.  This is used to hold all
** unixhildon-specific data.  One of these is created for each top-level
** document window.
*********************************************************************
********************************************************************/

class AP_UnixHildonFrameImpl : public AP_UnixFrameImpl
{
public:
	AP_UnixHildonFrameImpl(AP_UnixFrame *pUnixFrame);
	virtual ~AP_UnixHildonFrameImpl();

	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame);

protected:
	GtkWidget *  _createInternalWindow(void);
};
#endif /* AP_UNIXHILDONFRAMEIMPL_H */



