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

#include "xap_ModuleManager.h"

#include "xap_Module.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// the loader manages instances of one of these target classes

#if defined (WIN32)
  #include "xap_Win32Module.h"
  #define MODULE_CLASS XAP_Win32Module

#elif defined (__BEOS__)
  #include "xap_BeOSModule.h"
  #define MODULE_CLASS XAP_BeOSModule

#elif defined (__QNXTO__)
  #include "xap_QNXModule.h"
  #define MODULE_CLASS XAP_QNXModule

#elif defined (TARGET_OS_MAC)
  #include "xap_MacModule.h"
  #define MODULE_CLASS XAP_MacModule

#else
  // *NIX
  #include "xap_UnixModule.h"
  #define MODULE_CLASS XAP_UnixModule

#endif

// TODO: get this from some auto-generated place
const UT_uint32 major   = 0;
const UT_uint32 minor   = 7;
const UT_uint32 release = 15;

/*!
 * Protected destructor creates an instance of this module class
 */
XAP_ModuleManager::XAP_ModuleManager ()
	: m_modules (11)
{
}

/*!
 * Private destructor
 */
XAP_ModuleManager::~XAP_ModuleManager ()
{
	UT_VECTOR_PURGEALL (MODULE_CLASS *, m_modules);
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
 * Request that the ModuleManager load the module represented by
 * szFilename. Returns a valid XAP_Module on success, 0 on failure
 *
 * \param szFilename - the .dll or .so on your system that you wish to load
 * \return a valid XAP_Module or 0
 */
XAP_Module * XAP_ModuleManager::loadModule (const char * szFilename)
{
	UT_ASSERT (szFilename);

	XAP_Module * pModule = new MODULE_CLASS;
	UT_ASSERT (pModule);

	char * errorMsg;

	if (!pModule->load (szFilename))
	{		
		UT_DEBUGMSG (("Failed to load module %s\n", szFilename));
		
		if (pModule->getErrorMsg (&errorMsg))
		{	
			UT_DEBUGMSG (("Reason: %s\n", errorMsg));
			FREEP (errorMsg);
		}
		
		delete pModule;
		return 0;
	}

	// assign the module's creator to be us
	pModule->setLoaded (true);
	pModule->setCreator (this);

	if (!pModule->registerThySelf ())
	{
		UT_DEBUGMSG (("Failed to register module %s\n", szFilename));
		
		if (pModule->getErrorMsg (&errorMsg))
		{	
			UT_DEBUGMSG (("Reason: %s\n", errorMsg));
			FREEP (errorMsg);
		}

		pModule->unload ();
		delete pModule;
		return 0;
	}

#if 0
	// TODO: we possibly (probably?) should turn this code on
	if (!pModule->supportsAbiVersion (major, minor, release))
	{
		UT_DEBUSGMSG (("Plugin does not support AbiWord version %d.%d.%d\n",
					   major, minor, release));

		pModule->unregisterThySelf ();
		pModule->unload ();
		delete pModule;
		return 0;
	}
#endif

	// we (somehow :^) got here. count our blessings and return the module
	m_modules.addItem (pModule);
	return pModule;
}

/*!
 * When passed a valid XAP_Module that this class loaded, this method
 * will unload the module from memory
 *
 * \param pModule - a valid module that you want to unload
 * \return true if we could register the module, false if not
 */
bool XAP_ModuleManager::unloadModule (XAP_Module * pModule)
{
	UT_ASSERT (pModule);
	UT_ASSERT (pModule->getCreator () == this);

	int ndx = m_modules.findItem (pModule);

	if (ndx != -1)
	{
		m_modules.deleteNthItem (ndx);

		// we're less picky when unloading than we are when loading
		// the (necessarily true) assumptions are that
		//
		// 1) we were the one who loaded the module
		// 2) registerThySelf() worked
		//
		// so it had better damn well work in the opposite direction!

		UT_ASSERT (pModule->unregisterThySelf ());
		pModule->setLoaded (false);
		UT_ASSERT (pModule->unload ());
		delete pModule;

		return true;
	}
	else
	{
		// how the heck did this happen?? something is *clearly* wrong
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
		UT_DEBUGMSG (("Could not unload module\n"));
		return false;
	}
}

/*!
 * Enumerate the modules loaded thus far
 *
 * \return a vector containing XAP_Module*'s
 */
const UT_Vector * XAP_ModuleManager::enumModules () const
{
	// TODO: perhaps we should clone this
	return &m_modules;
}
