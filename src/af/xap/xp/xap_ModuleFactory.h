/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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

#ifndef XAP_MODULE_FACTORY_H
#define XAP_MODULE_FACTORY_H

#include "xap_Module.h"

class ABI_EXPORT XAP_ModuleFactory
{
	friend class XAP_Module;

public:

	/*!
	* Returns the name of the factory for logging and debugging purposes.
	*/
	virtual const char * getName (void) = NULL;

	/*!
	* Load a module.
	* \param szFilename Plugin file to load.
	* \return Ptr to module instance if success, otherwise NULL;
	*/
	virtual XAP_Module * loadModule (const char * szFilename) = NULL;
	/*!
	* Unload a module.
	* Plugin factories can implement this method if they want to handle unloading of
	* plugins themselves.
	* \param module Ptr to module instance.
	* \return TRUE on success, FALSE on failure.
	*/
	virtual bool unloadModule (XAP_Module * module) { return false; }
};

#endif /* XAP_MODULE_FACTORY_H */
