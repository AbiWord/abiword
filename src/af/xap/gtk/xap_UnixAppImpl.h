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

#ifndef _XAP_UNIXAPPIMPL_H_
#define _XAP_UNIXAPPIMPL_H_

#include "xap_AppImpl.h"

class XAP_UnixAppImpl
	: public XAP_AppImpl
{
protected:
	virtual std::string 	localizeHelpUrl (const char * pathBeforeLang,
						 const char * pathAfterLang,
						 const char * remoteURLbase);
	virtual bool openURL(const char * url);
	virtual bool openHelpURL(const char * url);
};

#endif
