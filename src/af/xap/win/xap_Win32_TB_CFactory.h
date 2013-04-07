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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_WIN32TOOLBAR_CONTROLFACTORY_H
#define XAP_WIN32TOOLBAR_CONTROLFACTORY_H

#include "xap_Toolbar_ControlFactory.h"

class ABI_EXPORT AP_Win32Toolbar_ControlFactory : public XAP_Toolbar_ControlFactory
{
public:
	AP_Win32Toolbar_ControlFactory();
	virtual ~AP_Win32Toolbar_ControlFactory(void);

protected:
};

#endif /* XAP_WIN32TOOLBAR_CONTROLFACTORY_H */
