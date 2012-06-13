/* AbiSource Application Framework
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef XAP_WIN32DIALOGFACTORY_H
#define XAP_WIN32DIALOGFACTORY_H

#include "xap_DialogFactory.h"

class ABI_EXPORT AP_Win32DialogFactory : public XAP_DialogFactory
{
public:	
	AP_Win32DialogFactory(XAP_App * pApp, XAP_Frame * pFrame = NULL);
	virtual ~AP_Win32DialogFactory(void);

protected:
};

#endif /* XAP_WIN32DIALOGFACTORY_H */
