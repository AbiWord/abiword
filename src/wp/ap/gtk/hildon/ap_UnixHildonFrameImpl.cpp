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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_files.h"
#include "ut_sleep.h"
#include "ev_UnixMenuBar.h"
#include "ev_EditMethod.h"
#include "xap_ViewListener.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "hildon/xap_UnixHildonApp.h"

#include "fv_View.h"

#include "ap_UnixHildonFrameImpl.h"

#include <hildon/hildon-program.h>
#include <hildon/hildon-window.h>

/**
 * A Constructor
 * @param pUnixFrame the pointer of frame
 * @param pUnixApp the pointer of App
 */
AP_UnixHildonFrameImpl::AP_UnixHildonFrameImpl(AP_UnixFrame *pUnixFrame) 
:AP_UnixFrameImpl(pUnixFrame)
{
	UT_DEBUGMSG(("Created AP_UnixHildonFrameImpl %p \n",this));
}

/**
 * A Destructor
 */
AP_UnixHildonFrameImpl::~AP_UnixHildonFrameImpl() 
{
}

/**
 * Create a new frame instance
 * @return the pointer of new frame
 */
XAP_FrameImpl * AP_UnixHildonFrameImpl::createInstance(XAP_Frame *pFrame)
{
	XAP_FrameImpl *pFrameImpl = new AP_UnixHildonFrameImpl(static_cast<AP_UnixFrame *>(pFrame));

	return pFrameImpl;
}

GtkWidget * AP_UnixHildonFrameImpl::_createInternalWindow(void)
{
	XAP_UnixHildonApp * pHApp = static_cast<XAP_UnixHildonApp*>(XAP_App::getApp());
	GObject * pHildonProgram = (pHApp)->getHildonProgram();
	GtkWidget *window;

    window = hildon_window_new ();
	hildon_program_add_window (HILDON_PROGRAM (pHildonProgram),
							   HILDON_WINDOW (window));

	return window;
}
