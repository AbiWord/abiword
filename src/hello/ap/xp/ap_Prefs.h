/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#ifndef AP_PREFS_H
#define AP_PREFS_H

#include "xap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"

class AP_Prefs : public XAP_Prefs
{
 public:
	AP_Prefs(XAP_App* pApp);
	virtual ~AP_Prefs(void);

	virtual void					fullInit(void);
	virtual bool                loadBuiltinPrefs(void);
	virtual const XML_Char*        getBuiltinSchemeName(void) const;
	virtual const char*            getPrefsPathname(void) const = 0;
};

#endif // AP_PREFS_H
