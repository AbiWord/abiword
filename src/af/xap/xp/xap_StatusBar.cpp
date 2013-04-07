/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * 
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
 * Copyright (C) 2002 AbiSource, Inc.
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

#include "ut_sleep.h"

#include "xap_StatusBar.h"

/* This allows for up to two concurrently active status bars, since
 * the splash screen may still be active when the application's
 * principal status bar registers...
 * - FIXME: this can be removed, as I killed off the splash screen - MARCM
 */
static XAP_StatusBar * s_SB1 = 0;
static XAP_StatusBar * s_SB2 = 0;

void XAP_StatusBar::setStatusBar (XAP_StatusBar * statusbar)
{
	if (s_SB1 == 0)
		{
			s_SB1 = statusbar;
			return;
		}
	if (s_SB2 == 0)
		{
			s_SB2 = statusbar;
			return;
		}
	message ("Too many status bars!!!", true); // :-)
}

void XAP_StatusBar::unsetStatusBar (XAP_StatusBar * statusbar)
{
	if (s_SB1 == statusbar) s_SB1 = 0;
	if (s_SB2 == statusbar) s_SB2 = 0;
}

void XAP_StatusBar::message (const char * pbuf, bool urgent)
{
	if (s_SB1 || s_SB2)
		{
			if (s_SB1) s_SB1->statusMessage (pbuf, urgent);
			if (s_SB2) s_SB2->statusMessage (pbuf, urgent);

			if (urgent) UT_usleep (1000000);
		}
}

void XAP_StatusBar::debugmsg (const char * pbuf, bool urgent)
{
	if (s_SB2)
		{
			s_SB2->statusMessage (pbuf, urgent);

			if (urgent) UT_usleep (1000000);
		}
}
