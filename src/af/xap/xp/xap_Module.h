/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <cinamod@hotmail.com>
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

#ifndef XAP_MODULE_H
#define XAP_MODULE_H

// This is an abstract class meant for dynamically
// Loading and unloading modules. To load a module,
// Pass the intended file name (dll, so, whatever)
// To a valid child instance of this class.

#include "ut_types.h"

// we want to have C linkage for both 
// this and for all of our required functions
extern "C" {
	typedef struct {
		char * name;
		char * desc;
		char * version;
		char * author;
		char * usage;
	} XAP_ModuleInfo;
}

class XAP_ModuleManager;

class XAP_Module {

	friend class XAP_ModuleManager;

protected:
	XAP_Module ();
	virtual ~XAP_Module (void);
	
	// load this module into memory. true on success
	virtual bool load (const char * name) = 0;
	
	// unload this module from memory. true on success
	virtual bool unload (void) = 0;	
	
private:

	// silly names. fscking c/c++ has the keyword 'register' taken #:^)
	bool registerThySelf ();
	bool unregisterThySelf ();
	bool supportsAbiVersion (UT_uint32 major, UT_uint32 minor, 
							 UT_uint32 release);
	inline void setCreator (XAP_ModuleManager * creator) {m_creator = creator;}
	inline void setLoaded (bool bLoaded) {m_bLoaded = bLoaded;}

public:
	
	// passed a symbol name and a void ** symbol, 
	// *symbol refers to the actual representation of @symbol_name
	//
	// void (*func) (XAP_ModuleInfo *);
	// resolveSymbol ("abi_plugin_init", &(void *)func);
	// int result = func (&m_info);
	virtual bool resolveSymbol (const char * symbol_name, void ** symbol) = 0;
	
	// returns the name of this module, if it has one
	// if return is true, you must FREEP dest
	virtual bool getModuleName (char ** dest) const = 0;
	
	// returns the most recent error message from one of these
	// calls failing. If return is true, you must FREEP dest
	virtual bool getErrorMsg (char ** dest) const = 0;

	inline XAP_ModuleManager * getCreator () const {return m_creator;}

	inline const XAP_ModuleInfo *getModuleInfo (void) const {return &m_info;}
	
private:

	XAP_ModuleManager   * m_creator;
	bool                  m_bLoaded;
	bool                  m_bRegistered;
	XAP_ModuleInfo        m_info;
};

#endif /* XAP_MODULE_H */
