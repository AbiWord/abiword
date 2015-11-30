/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz and others
 * Copyright (C) 2004, 2009 Hubert Figuiere
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

#ifndef AP_APP_H
#define AP_APP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class AP_Args;
class AP_Frame;
class XAP_Frame;

// this ugliness is needed for proper inheritence

#if defined(TOOLKIT_WIN)
  #include "xap_Win32App.h"
  #define XAP_App_BaseClass XAP_Win32App
#elif defined(TOOLKIT_COCOA)
  #include "xap_CocoaApp.h"
  #define XAP_App_BaseClass XAP_CocoaApp
#elif defined(TOOLKIT_QT)
  #include "xap_QtApp.h"
  #define XAP_App_BaseClass XAP_QtApp
#else
  #include "xap_UnixApp.h"
  #define XAP_App_BaseClass XAP_UnixApp
#endif

/*!
 * Generic application base class
 */
class ABI_EXPORT AP_App : public XAP_App_BaseClass
{
 public:

#if defined(TOOLKIT_WIN)
	AP_App (HINSTANCE hInstance, const char * szAppName);
#else
	AP_App (const char * szAppName);
#endif
	virtual ~AP_App ();
	virtual bool	initialize(void);

	/* Command line stuff. */

	/* Print an error message.  eg printf on UNIX, MessageBox on Windows. */
	virtual void errorMsgBadArg (AP_Args *, int);
	virtual void errorMsgBadFile(XAP_Frame *, const char *, UT_Error);

	/* Allow additional platform-specific windowless args. */
	virtual bool doWindowlessArgs (const AP_Args *, bool & bSuccess);

	bool openCmdLineFiles(const AP_Args * args);
	bool openCmdLinePlugins(const AP_Args * args, bool & bSuccess);
protected:
	void saveRecoveryFiles();
 private:

};

#endif // AP_APP_H
