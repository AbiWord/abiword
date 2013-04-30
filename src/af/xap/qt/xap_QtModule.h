/* AbiSource Application Framework
 * Copyright (C) 2012 Hubert Figuiere
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

#ifndef XAP_QT_MODULE_H
#define XAP_QT_MODULE_H

#include "xap_Module.h"

// The Qt impl. of XAP_Module interface

class XAP_QtModule : public XAP_Module 
{
	friend class XAP_ModuleManager;

protected:
	
	XAP_QtModule();
	virtual ~XAP_QtModule(void);
	
	virtual bool   load (const char * name);
	virtual bool   unload (void);
	
public:
	
	virtual bool   resolveSymbol (const char * symbol_name, void ** symbol);
	virtual bool   getModuleName (char ** dest) const;
	virtual bool   getErrorMsg (char ** dest) const;
	
private:

	bool m_bLoaded;
	char * m_szname;
};

#endif /* XAP_QT_MODULE_H */
