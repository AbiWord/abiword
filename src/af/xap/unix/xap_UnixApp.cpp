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

#include <gtk/gtk.h>
#include "ap_UnixAp.h"

/*****************************************************************/

AP_UnixAp::AP_UnixAp(void)
{
}

AP_UnixAp::~AP_UnixAp(void)
{
}

UT_Bool AP_UnixAp::initialize(int * pArgc, char *** pArgv)
{
	// initialize GTK first.
	
	gtk_set_locale();
	gtk_init(pArgc,pArgv);

	// let our base class do it's thing.
	
	AP_Ap::initialize(pArgc,pArgv);

	// do any thing we need here...
	
	return UT_TRUE;
}

