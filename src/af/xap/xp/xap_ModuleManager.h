/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <cinamod@hotmail.com>
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

#ifndef XAP_MODULE_MANAGER_H
#define XAP_MODULE_MANAGER_H

// Singleton class that will load/unload modules for us

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "ut_vector.h"

#include "xap_Module.h"


class ABI_EXPORT XAP_ModuleManager
{
	friend class XAP_Module;

private:
	XAP_ModuleManager ();

public:
	~XAP_ModuleManager (); // grrr

	static XAP_ModuleManager & instance ();

	bool         loadModule (const char * szFilename);

	bool         loadPreloaded (XAP_Plugin_Registration fnRegister,
								XAP_Plugin_Registration fnDeregister,
								XAP_Plugin_VersionCheck fnSupportsVersion);

	void         unloadModule (XAP_Module * module);
private:
	void         unloadModule (UT_sint32 ndx);
public:
	void         unloadAllPlugins ();

	const UT_GenericVector<XAP_Module*> * enumModules () const;

private:

	XAP_ModuleManager(const XAP_ModuleManager &);		// no impl
	void operator=(const XAP_ModuleManager &);	        // no impl

	UT_GenericVector<XAP_Module*> * m_modules;
};

#endif /* XAP_MODULE_MANAGER_H */
