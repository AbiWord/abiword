/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz and others
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

#ifndef AP_APP_H
#define AP_APP_H

class AP_Args;

// this ugliness is needed for proper inheritence

#if defined(WIN32)
#include "xap_Win32App.h"
#define XAP_App_BaseClass XAP_Win32App
#elif defined(__BEOS__)
#include "xap_BeOSApp.h"
#define XAP_App_BaseClass XAP_BeOSApp
#elif defined(__QNX__)
#include "xap_QNXApp.h"
#define XAP_App_BaseClass XAP_QNXApp
#elif (defined(XP_MAC_TARGET_CARBON) && XP_MAC_TARGET_CARBON) && (!defined(CARBON_ON_MACH_O) || (CARBON_ON_MACH_O == 0)) // Carbon on Mach-O as UNIX
#include "xap_MacApp.h"
#define XAP_App_BaseClass XAP_MacApp
#elif defined(XP_TARGET_COCOA)
#include "xap_CocoaApp.h"
#define XAP_App_BaseClass XAP_CocoaApp
#elif defined(HAVE_GNOME)
#include "xap_UnixGnomeApp.h"
#define XAP_App_BaseClass XAP_UnixGnomeApp
#else
#include "xap_UnixApp.h"
#define XAP_App_BaseClass XAP_UnixApp
#endif

/*!
 * Generic application base class
 */
class AP_App : public XAP_App_BaseClass
{
 public:

	AP_App (XAP_Args * pArgs, const char * szAppName);
	virtual ~AP_App ();
	virtual void initPopt (AP_Args *);
	virtual bool doWindowlessArgs (const AP_Args *);

 private:

};

#endif // AP_APP_H
