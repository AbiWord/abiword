/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
 * Copyright (C) 2004 Francis James Franklin
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

#ifndef XAP_COCOAMODULE_H
#define XAP_COCOAMODULE_H

#include <string>
#include "xap_Module.h"

class XAP_CocoaModule : public XAP_Module 
{

	friend class XAP_ModuleManager;

protected:

	XAP_CocoaModule () ;

	virtual ~XAP_CocoaModule (void);

	virtual bool	load (const char * name);
	virtual bool	unload (void);

public:
	virtual bool	resolveSymbol (const char * symbol_name, void ** symbol);
	virtual bool	getModuleName (char ** dest) const;
	virtual bool	getErrorMsg (char ** dest) const;

	static void		loadAllPlugins (); // called on start-up, and *only* then

	/* used by CocoaAppController to auto-load plugins selected through the Finder
	 */
	static bool		loadPlugin (const std::string & path);
	static bool		hasPluginExtension (const std::string & path);

private:
	std::string		m_szname;
	std::string		m_module_path;

	void *			m_module;

	bool			m_bLoaded;
};

#endif /* XAP_COCOAMODULE_H */
