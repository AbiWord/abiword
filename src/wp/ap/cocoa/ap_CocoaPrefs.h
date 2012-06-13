/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#ifndef AP_COCOAPREFS_H
#define AP_COCOAPREFS_H

#include "ap_Prefs.h"

class AP_CocoaPrefs : public AP_Prefs
{
public:
	AP_CocoaPrefs();
	
	virtual void			overlayEnvironmentPrefs(void);
	
protected:
	virtual const char *	_getPrefsPathname(void) const;
};

#endif /* AP_COCOAPREFS_H */
