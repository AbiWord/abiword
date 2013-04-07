/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_MODULE_H
#define XAP_MODULE_H

// This is an abstract class meant for dynamically
// Loading and unloading modules. To load a module,
// Pass the intended file name (dll, so, whatever)
// To a valid child instance of this class.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#ifdef TOOLKIT_WIN
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #define ABI_PLUGIN_DECLARE(name) static HANDLE s_hModule = static_cast<HANDLE>(NULL); extern "C" {BOOL APIENTRY DllMain( HANDLE hModule, DWORD /*ul_reason_for_call*/, LPVOID /*lpReserved*/ ) { s_hModule = hModule; return TRUE; } }
#else
  #define ABI_PLUGIN_DECLARE(name)
#endif

// we want to have C linkage for both
// this and for all of our required functions
extern "C" {
	typedef struct {
		const char * name;
		const char * desc;
		const char * version;
		const char * author;
		const char * usage;
	} XAP_ModuleInfo;
}

class XAP_ModuleManager;

typedef int (*XAP_Plugin_Registration) (XAP_ModuleInfo * mi);
typedef int (*XAP_Plugin_VersionCheck) (UT_uint32 major, UT_uint32 minor, UT_uint32 release);

class ABI_EXPORT XAP_Module {

	friend class XAP_ModuleManager;

protected:
	XAP_Module ();

	virtual ~XAP_Module (void);

	// load this module into memory. true on success
	virtual bool load (const char * name) = 0;

	// unload this module from memory. true on success
	virtual bool unload (void) = 0;

public:
	// marks the module as loaded; returns false if module is already loaded
	bool setSymbols (XAP_Plugin_Registration fnRegister,
					 XAP_Plugin_Registration fnDeregister,
					 XAP_Plugin_VersionCheck fnSupportsVersion);

	bool registered ();
private:
	bool registerPending ();
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
	// resolveSymbol ("abi_plugin_init", static_cast<void **>(&func));
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

	XAP_Plugin_Registration		m_fnRegister;
	XAP_Plugin_Registration		m_fnDeregister;
	XAP_Plugin_VersionCheck		m_fnSupportsVersion;

	XAP_ModuleManager   * m_creator;
	bool                  m_bLoaded;
	bool                  m_bRegistered;
	int                   m_iStatus;
	const char          * m_szSPI;
	XAP_ModuleInfo        m_info;
};

#if 0 // TODO: make me 100% and turn me on again one day
#ifndef ABI_PLUGIN_SOURCE
static inline bool
_isCurrentAbiVersion(int a, int b, int c)
{
       int _a, _b, _c, _n;

       _n = sscanf( ABI_BUILD_VERSION , "%d.%d.%d", &_a, &_b, &_c);

       if (_n <= 1 || _a != a || _b != b)
               return false;

       if (3 == _n && _c != c)
               return false;

       return true;
}
#endif /* ! ABI_PLUGIN_SOURCE */

#define isCurrentAbiVersion(a,b,c) _isCurrentAbiVersion(a, b, c)

#else
#define isCurrentAbiVersion(a,b,c) true
#endif

#ifdef ABI_PLUGIN_SOURCE
#define ABI_VERSION_STRING ABI_PLUGIN_VERSION
#else
#define ABI_VERSION_STRING ABI_BUILD_VERSION
#endif

#endif /* XAP_MODULE_H */
