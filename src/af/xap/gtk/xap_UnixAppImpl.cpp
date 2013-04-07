/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuiere
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

#include <glib.h>

#include "xap_UnixAppImpl.h"
#include "ut_string_class.h"
#include "ut_files.h"
#include "ut_go_file.h"

UT_String XAP_UnixAppImpl::localizeHelpUrl (const char * pathBefore, 
											const char * pathAfter,
											const char * remoteURLbase)
{
	return XAP_AppImpl::localizeHelpUrl (pathBefore, pathAfter, remoteURLbase);
}

bool XAP_UnixAppImpl::openHelpURL(const char * url)
{
	return openURL(url);
}

bool XAP_UnixAppImpl::openURL(const char * url)
{
	// Need this to make AbiGimp Load!!!!!
	if (progExists("foo")) {}

	GError * err = NULL;
	err = UT_go_url_show (url);
	if (err) {
		g_warning ("%s", err->message);
		g_error_free (err);
		return FALSE;
	}
	return TRUE;
}
