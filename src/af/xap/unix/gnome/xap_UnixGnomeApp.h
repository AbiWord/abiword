/* AbiSource Application Framework
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


#ifndef XAP_UNIXGNOMEAPP_H
#define XAP_UNIXGNOMEAPP_H

extern "C" {
#include <gnome.h>
#ifdef HAVE_BONOBO
#include <libgnorba/gnorba.h>
#include <bonobo/gnome-bonobo.h>
#endif
}

#include "xap_App.h"
#include "xap_UnixApp.h"

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class XAP_UnixGnomeApp : public XAP_UnixApp
{
public:
	XAP_UnixGnomeApp(XAP_Args * pArgs, const char * szAppName);
	virtual ~XAP_UnixGnomeApp(void);
	
	virtual UT_Bool							initialize(void);
	
protected:
	void							_setAbiSuiteLibDir(void);
};

#endif /* XAP_UNIXGNOMEAPP_H */

