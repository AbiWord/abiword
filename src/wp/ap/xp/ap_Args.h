/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2002 Patrick Lam
 * Copyright (C) 2008 Robert Staudinger
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

#ifndef AP_ARGS_H
#define AP_ARGS_H

#include <glib.h>
#include "ut_types.h"
#include "xap_Args.h"

class AP_Args;
class AP_App;
class UT_String;
class XAP_App;

// warning: options array is static.
class ABI_EXPORT AP_Args
{
public:
	AP_Args(XAP_Args * pArgs, const char * szAppName, AP_App * pApp);
	~AP_Args(void);

	XAP_Args *XArgs;

	GOptionContext * getContext() const { return m_context; }
	void addOptions(GOptionGroup *options);

	/* Parse options. */
	void parseOptions();
	UT_String * getPluginOptions() const;

	AP_App* getApp() const { return m_pApp; }
	bool doWindowlessArgs(bool & bSuccessful);
#ifdef DEBUG
	static int    m_iDumpstrings;
#endif
	static const char * m_sGeometry;
	static const char * m_sToFormat;
	static const char * m_sPrintTo;
	static const char * m_sName;
	static int    m_iToThumb;
	static const char * m_sThumbXY;
	static int	  m_iVerbose;
	static int	  m_iShow;
	static const char ** m_sPluginArgs;
	static const char ** m_sFiles;
	static int    m_iVersion;
	static int    m_iHelp;
	static const char * m_sMerge;

	static const char * m_impProps;
	static const char * m_expProps;
	static const char * m_sUserProfile;

	static const char * m_sFileExtension;
private:
	AP_App*       m_pApp;
	GOptionContext *m_context;
};

#endif /* AP_ARGS_H */
