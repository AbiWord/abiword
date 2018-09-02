/* AbiSource Application Framework
 * Copyright (C) 2004 Hubert Figuiere
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


#ifndef _XAP_APP_IMPL_H_
#define _XAP_APP_IMPL_H_

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include <string>

class ABI_EXPORT XAP_AppImpl
{
public:
	virtual ~XAP_AppImpl() {}

	static XAP_AppImpl* static_constructor(void);		/* must be implemented by the ap_<FE>AppImpl */

	/* XAP_App can call impl methods. */
	friend class XAP_App;

protected:
	virtual std::string localizeHelpUrl (const char * pathBeforeLang,
										   const char * pathAfterLang,
										   const char * remoteURLbase);
	virtual bool openURL(const char * url) = 0;
	virtual bool openHelpURL(const char * url);
};

#endif
