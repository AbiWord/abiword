/* AbiSource Application Framework
 * Copyright (C) 2001 Dom Lachowicz <doml@appligent.com>
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

#ifndef XAP_UNIXMODULE_H
#define XAP_UNIXMODULE_H

#include "xap_Module.h"
#include <glib.h>
#include <gmodule.h>

// The Unix/GLib impl. of XAP_Module interface

class XAP_UnixModule : public XAP_Module 
{
	friend class XAP_ModuleManager;

protected:
	
	XAP_UnixModule () ;
	virtual ~XAP_UnixModule (void);
	
	virtual bool   load (const char * name);
	virtual bool   unload (void);
	
public:
	
	virtual bool   resolveSymbol (const char * symbol_name, void ** symbol);
	virtual bool   getModuleName (char ** dest) const;
	virtual bool   getErrorMsg (char ** dest) const;
	
private:

	bool m_bLoaded;
	char * m_szname;
	GModule * m_module;
};

#endif /* XAP_UNIXMODULE_H */
