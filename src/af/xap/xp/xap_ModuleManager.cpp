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

#include "ut_string_class.h"
#include "ut_exception.h"
#include "ut_vector.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Spider.h"
#include "xap_Module.h"
#include "xap_ModuleManager.h"

// the loader manages instances of one of these target classes

#if defined (WIN32)
  #include "xap_Win32Module.h"
  #define MODULE_CLASS XAP_Win32Module

#elif defined (__BEOS__)
  #include "xap_BeOSModule.h"
  #define MODULE_CLASS XAP_BeOSModule

#elif defined (__QNXNTO__)
  #include "xap_QNXModule.h"
  #define MODULE_CLASS XAP_QNXModule

#elif defined (__APPLE__) && defined (XP_MAC_TARGET_MACOSX)
	#if defined (XP_MAC_TARGET_CARBON)
		#include <ConditionalMacros.h>
		#if defined(TARGET_RT_MAC_CFM) && (TARGET_RT_MAC_CFM == 1)
			#include "xap_MacCFMModule.h"
			#define MODULE_CLASS XAP_MacModule
		#elif defined (TARGET_RT_MAC_MACHO) && (TARGET_RT_MAC_MACHO == 1)
			#include "xap_MacModule.h"
			#define MODULE_CLASS XAP_MacModule
		#else
			#error Unknown Apple architecture
		#endif
	#elif defined (XP_TARGET_COCOA)
		#include "xap_CocoaModule.h"
		#define MODULE_CLASS XAP_CocoaModule	
	#endif
#else
  // *NIX
  #include "xap_UnixModule.h"
  #define MODULE_CLASS XAP_UnixModule

#endif

/*!
 * Protected destructor creates an instance of this module class
 */
XAP_ModuleManager::XAP_ModuleManager ()	:
  m_spider(new XAP_Spider)
{
	m_modules = new UT_Vector (11);
}

/*!
 * Private destructor
 */
XAP_ModuleManager::~XAP_ModuleManager ()
{
	DELETEP (m_spider);
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
 * Note that if the plugin is a SPI then registration will be postponed until
 * XAP_ModuleManager::registerPending () is called.
 *
 * \return true if loaded (and, if not a SPI, registered) successfully, false otherwise
 */
bool XAP_ModuleManager::loadModule (const char * szFilename)
{
	UT_ASSERT (szFilename);

	if ( szFilename == 0) return false;
	if (*szFilename == 0) return false;

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
		
		char * errorMsg = 0;
		if (pModule->getErrorMsg (&errorMsg))
		{	
			UT_DEBUGMSG (("Reason: %s\n", errorMsg));
			FREEP (errorMsg);
		}
		delete pModule;
		return false;
	}

	/* assign the module's creator to be us, etc.
	 */
	pModule->setSpider (m_spider);
	pModule->setLoaded (true);
	pModule->setCreator (this);

	if (!pModule->registerThySelf ())
	{
		UT_DEBUGMSG (("Failed to register module %s\n", szFilename));
		
		char * errorMsg = 0;
		if (pModule->getErrorMsg (&errorMsg))
		{	
			UT_DEBUGMSG (("Reason: %s\n", errorMsg));
			FREEP (errorMsg);
		}
		pModule->unload ();
		delete pModule;
		return false;
	}
	if (m_modules->addItem (pModule)) // an error occurred...
	{
		pModule->unregisterThySelf ();
		pModule->unload ();
		delete pModule;
		return false;
	}

	/* we (somehow :^) got here. count our blessings and return
	 */
	return true;
}

/*!
 * Register any SPI plugins/modules pending registration.
 * 
 * This procedure calls XAP_ModuleManager::unloadUnregistered
 * 
 * \return number of plugins registered
 */
UT_uint32 XAP_ModuleManager::registerPending ()
{
	UT_uint32 count = 0;

	if (m_spider) count = m_spider->register_spies ();

	/* The Spider does the plugin registration (see above)
	 * but the module info has to be updated:
	 */
	for (UT_uint32 i = 0; i < m_modules->size (); i++)
	{
		XAP_Module * pModule = reinterpret_cast<XAP_Module *>(m_modules->getNthItem(i));
		if (pModule) pModule->registerPending ();
	}
	unloadUnregistered ();

	return count;
}

/*!
 * Unload a module
 *
 * \param pModule - a valid module that you want to unload
 *
 * WARNING: zero or more plugins/modules may be unloaded as a result of
 * calling XAP_ModuleManager::unloadModule since the plugin may be unloaded
 * already or other plugins may depend on this one and be unloaded
 * automatically (this procedure calls XAP_ModuleManager::unloadUnregistered).
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
	unloadUnregistered ();
}

/* for use by unloadAllPlugins, unloadModule & unloadUnregistered only!
 */
void XAP_ModuleManager::unloadModule (UT_sint32 ndx)
{
	UT_return_if_fail(m_modules != NULL);

	XAP_Module * pModule = reinterpret_cast<XAP_Module *>(m_modules->getNthItem (ndx));

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
	UT_ASSERT (module_unloaded == true);

	delete pModule;
}

/*!
 * Unload any SPI plugins/modules that haven't been registered
 */
void XAP_ModuleManager::unloadUnregistered ()
{
	UT_return_if_fail(m_spider  != NULL);
	UT_return_if_fail(m_modules != NULL);

	UT_sint32 bad_module;
	do
	{
		bad_module = -1;
		for (UT_uint32 i = 0; i < m_modules->size (); i++)
		{
			XAP_Module * pModule = reinterpret_cast<XAP_Module *>(m_modules->getNthItem (i));
			if (!pModule->registered ())
			{
				bad_module = static_cast<UT_sint32>(i);
				break;
			}
		}
		if (bad_module != -1) unloadModule (bad_module);
	} while (bad_module != -1);
}

/*!
 * Enumerate the modules loaded thus far
 *
 * \return a vector containing XAP_Module*'s
 */
const UT_Vector * XAP_ModuleManager::enumModules () const
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
	while (UT_uint32 count = m_modules->getItemCount ())
	{
		unloadModule (count - 1);

		if (m_modules->getItemCount () == count) // huh?
		{
			UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
			break;
		}
	}
}
