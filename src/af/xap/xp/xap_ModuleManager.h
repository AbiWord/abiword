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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef XAP_MODULE_MANAGER_H
#define XAP_MODULE_MANAGER_H

// Singleton class that will load/unload modules for us

#include "ut_vector.h"

class XAP_Module;

class XAP_ModuleManager
{
private:

	XAP_ModuleManager ();
public:
	~XAP_ModuleManager ();

public:

	static XAP_ModuleManager & instance ();

	XAP_Module * loadModule (const char * szFilename);
	bool         unloadModule (XAP_Module * module);

	const UT_Vector *  enumModules () const;

private:

	UT_Vector m_modules;

};

#endif /* XAP_MODULE_MANAGER_H */
