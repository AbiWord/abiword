/* AbiSource Application Framework
 * Copyright (C) 2001 Mike Nordell. Portions
 * Copyright (C) 1998 AbiSource, Inc. Other Portions
 * Copyright (C) 2001 Dom
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

#ifndef XAP_WIN32MODULE_H
#define XAP_WIN32MODULE_H

#include "xap_Module.h"

/*!
	XAP_Win32Module
	Implements the XAP_Module interface.
*/

class ABI_EXPORT XAP_Win32Module : public XAP_Module
{

	friend class XAP_ModuleManager;

protected:

	XAP_Win32Module();
	virtual ~XAP_Win32Module();

	virtual bool load(const char* name) override;
	virtual bool unload() override;

public:
	virtual bool resolveSymbol(const char* symbol_name, void** symbol) override;
	virtual bool getModuleName(char** dest) const override;
	virtual bool getErrorMsg(char** dest) const override;

	class XAP_Win32ModuleImpl* pimpl;
};

#endif /* XAP_WIN32MODULE_H */
