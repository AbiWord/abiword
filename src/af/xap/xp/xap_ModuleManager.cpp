/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ut_exception.h"
#include "ut_vector.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_ModuleManager.h"
#include "xap_App.h"
#include "xap_Prefs.h"
#include "ut_string_class.h"
#include "ut_path.h"

// the loader manages instances of one of these target classes

#if defined (TOOLKIT_WIN)
  #include "xap_Win32Module.h"
  #define MODULE_CLASS XAP_Win32Module
#elif defined (TOOLKIT_COCOA)
  #include "xap_CocoaModule.h"
  #define MODULE_CLASS XAP_CocoaModule	
#else
  #include "xap_UnixModule.h"
  #define MODULE_CLASS XAP_UnixModule
#endif

// log information about plugin loading into the <log> section of AbiWord.profile
// (we save the prefs file after each call, so as to maximise the information we have in
// case a plugin crashes and the crash handler does not get a chance to save it for us
#define XAP_MODULE_MANAGER_LOAD_LOG(msg1, msg2)                                      \
if(XAP_App::getApp() && XAP_App::getApp()->getPrefs())                               \
{                                                                                    \
    UT_String __s;                                                                   \
    UT_String_sprintf(__s, "(L%d): %s %s", __LINE__, msg1, msg2);                    \
	UT_DEBUGMSG(("%s\n",__s.c_str()));                                                 \
    XAP_App::getApp()->getPrefs()->log("XAP_ModuleManager::loadModule", __s.c_str());\
	XAP_App::getApp()->getPrefs()->savePrefsFile();                                  \
}

/*!
 * Protected destructor creates an instance of this module class
 */
XAP_ModuleManager::XAP_ModuleManager ()
{
	m_modules = new UT_GenericVector<XAP_Module*> (11);
}

/*!
 * Private destructor
 */
XAP_ModuleManager::~XAP_ModuleManager ()
{
	UT_VECTOR_PURGEALL (MODULE_CLASS *, (*m_modules));
	delete m_modules;
}

/*!
 * Acquire a handle to an instance of this module loader class
 */
XAP_ModuleManager & XAP_ModuleManager::instance ()
{
	static XAP_ModuleManager me;
	return me;
}

/*!
 * Request that the ModuleManager load the module represented by szFilename.
 *
 * \param szFilename - the .dll or .so on your system that you wish to load
 *
 * \return true if loaded successfully, false otherwise
 */
bool XAP_ModuleManager::loadModule (const char * szFilename)
{
	UT_ASSERT (szFilename);

	if ( szFilename == 0) return false;
	if (*szFilename == 0) return false;

	XAP_MODULE_MANAGER_LOAD_LOG("loading", szFilename)

	// check to see if plugin is already loaded
	
	XAP_Module* pModuleLoop = 0;
	const UT_GenericVector<class XAP_Module *> *pVec = enumModules();
	
	for (UT_sint32 i = 0; i < pVec->size(); i++)
	{
		pModuleLoop = (XAP_Module *)pVec->getNthItem (i);

		char * moduleName = 0;
		if(pModuleLoop && pModuleLoop->getModuleName(&moduleName))
		{
			if (!strcmp(UT_basename(szFilename), UT_basename(moduleName)))
			{
				// already loaded, don't attempt to load again and exit quietly
				FREEP(moduleName);
				return true;
			}
			FREEP(moduleName);
		}
	}


	XAP_Module * pModule = 0;
	UT_TRY
	{
		pModule = new MODULE_CLASS;
	}
	UT_CATCH (...)
	{
		pModule = 0;
	}
	if (pModule == 0) return false;

	if (!pModule->load (szFilename))
	{		
		UT_DEBUGMSG (("Failed to load module %s\n", szFilename));
		XAP_MODULE_MANAGER_LOAD_LOG("failed to load", szFilename)
		
		char * errorMsg = 0;
		if (pModule->getErrorMsg (&errorMsg))
		{	
			UT_DEBUGMSG (("Reason: %s\n", errorMsg));
			XAP_MODULE_MANAGER_LOAD_LOG("error msg", errorMsg)
			FREEP (errorMsg);
		}
		delete pModule;
		return false;
	}

	/* assign the module's creator to be us, etc.
	 */
	pModule->setLoaded (true);
	pModule->setCreator (this);

	if (!pModule->registerThySelf ())
	{
		UT_DEBUGMSG (("Failed to register module %s\n", szFilename));
		XAP_MODULE_MANAGER_LOAD_LOG("failed to register", szFilename)
		
		char * errorMsg = 0;
		if (pModule->getErrorMsg (&errorMsg))
		{	
			UT_DEBUGMSG (("Reason: %s\n", errorMsg?errorMsg:"unknown"));
			XAP_MODULE_MANAGER_LOAD_LOG("error msg",
errorMsg?errorMsg:"Unknown")
			FREEP (errorMsg);
		}
		pModule->unload ();
		delete pModule;
		return false;
	}
	if (m_modules->addItem (pModule)) // an error occurred...
	{
		XAP_MODULE_MANAGER_LOAD_LOG("could not add", szFilename)
		pModule->unregisterThySelf ();
		pModule->unload ();
		delete pModule;
		return false;
	}

	/* we (somehow :^) got here. count our blessings and return
	 */
	XAP_MODULE_MANAGER_LOAD_LOG("success", szFilename)
	return true;
}

/*!
* Load builtin plugins.
*/
bool XAP_ModuleManager::loadPreloaded (XAP_Plugin_Registration fnRegister,
									   XAP_Plugin_Registration fnDeregister,
									   XAP_Plugin_VersionCheck fnSupportsVersion)
{
	UT_ASSERT (fnRegister && fnDeregister && fnSupportsVersion);

	if (!(fnRegister && fnDeregister && fnSupportsVersion)) return false;

	XAP_Module * pModule = 0;
	UT_TRY
		{
			pModule = new MODULE_CLASS;
		}
	UT_CATCH (...)
		{
			pModule = 0;
		}
	if (pModule == 0) return false;

	if (!pModule->setSymbols (fnRegister, fnDeregister, fnSupportsVersion)) // huh?
		{		
			delete pModule;
			return false;
		}

	/* assign the module's creator to be us, etc.
	 */
	pModule->setLoaded (true);
	pModule->setCreator (this);

	if (!pModule->registerThySelf ())
		{
			UT_DEBUGMSG (("Failed to register preloaded module\n"));
			delete pModule;
			return false;
		}
	if (m_modules->addItem (pModule)) // an error occurred...
		{
			UT_DEBUGMSG (("oops! out of memory?\n"));
			pModule->unregisterThySelf ();
			delete pModule;
			return false;
		}

	/* we (somehow :^) got here. count our blessings and return
	 */
	return true;
}

/*!
 * Unload a module
 *
 * \param pModule - a valid module that you want to unload
 *
 * WARNING: zero or more plugins/modules may be unloaded as a result of
 * calling XAP_ModuleManager::unloadModule since the plugin may be unloaded
 * already or other plugins may depend on this one and be unloaded
 * automatically.
 * 
 * Don't assume anything, therefore, about the UT_Vector returned by
 * XAP_ModuleManager::enumModules
 */
void XAP_ModuleManager::unloadModule (XAP_Module * pModule)
{
	UT_ASSERT (pModule);
	if (pModule == 0) return;

	UT_ASSERT (pModule->getCreator () == this);
	if (pModule->getCreator () != this) return;

	UT_sint32 ndx = m_modules->findItem (pModule);
	if (ndx == -1)
	{
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
		UT_DEBUGMSG (("Could not unload module\n"));
		return;
	}
	unloadModule (ndx);
}

/* for use by unloadAllPlugins, unloadModule only!
 */
void XAP_ModuleManager::unloadModule (UT_sint32 ndx)
{
	UT_return_if_fail(m_modules != NULL);

	XAP_Module * pModule = m_modules->getNthItem (ndx);

	m_modules->deleteNthItem (ndx);

	// we're less picky when unloading than we are when loading
	// the (necessarily true) assumptions are that
	//
	// 1) we were the one who loaded the module
	// 2) registerThySelf() worked
	//
	// so it had better damn well work in the opposite direction!

	pModule->unregisterThySelf ();
	pModule->setLoaded (false);

	bool module_unloaded = pModule->unload ();
	UT_UNUSED (module_unloaded);
	UT_ASSERT (module_unloaded);

	delete pModule;
}

/*!
 * Enumerate the modules loaded thus far
 *
 * \return a vector containing XAP_Module*'s
 */
const UT_GenericVector<XAP_Module*> * XAP_ModuleManager::enumModules () const
{
	// TODO: perhaps we should clone this
	return m_modules;
}

/*!
 * Unload all currently loaded plugins, in order of last added to first
 *
 * \return nothing
 */
void XAP_ModuleManager::unloadAllPlugins ()
{
	UT_return_if_fail(m_modules != NULL);

	/* make sure all the plugins are unloaded (reverse order loaded)
	 * 
	 * Note: can no longer assume that XAP_ModuleManager::unloadModule()
	 *       will unload only one module.
	 */
	while (UT_sint32 count = m_modules->getItemCount ())
	{
		unloadModule (count - 1);

		if (m_modules->getItemCount () == count) // huh?
		{
			UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
			break;
		}
	}
}
