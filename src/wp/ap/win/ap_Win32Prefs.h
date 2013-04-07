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

#ifndef AP_WIN32PREFS_H
#define AP_WIN32PREFS_H

#include "ap_Prefs.h"

class ABI_EXPORT AP_Win32Prefs : public AP_Prefs
{
public:
	AP_Win32Prefs();

	virtual bool			loadBuiltinPrefs(void);
	virtual void			overlayEnvironmentPrefs(void);

protected:
	virtual const char *	_getPrefsPathname(void) const;
};

#endif /* AP_WIN32PREFS_H */
