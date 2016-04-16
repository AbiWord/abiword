/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Application Framework
 * Copyright (C) 1998, 1999 AbiSource, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <memory>

#include <glib.h>
#include <gsf/gsf-utils.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_path.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_Language.h"
#include "ut_uuid.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Actions.h"
#include "ev_Toolbar_Actions.h"
#include "xap_App.h"
#include "xap_AppImpl.h"
#include "xap_InputModes.h"
#include "gr_Image.h"
#include "xap_Frame.h"
#include "xap_EditMethods.h"
#include "xap_ModuleManager.h"
#include "xap_Module.h"

#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_LoadBindings.h"
#include "xap_Dictionary.h"
#include "xap_Prefs.h"
#include "xap_EncodingManager.h"
#include "xap_Menu_Layouts.h"
#include "xap_Toolbar_Layouts.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "ut_rand.h"
#include "gr_CharWidthsCache.h"
#include "xav_Listener.h"
#include "gr_EmbedManager.h"
#include "ut_Script.h"


/*****************************************************************/

XAP_StateData::XAP_StateData()
	: iFileCount(0)
{
	memset(&filenames, 0, sizeof(filenames));
	memset(&iDocPos, 0, sizeof(iDocPos));
	memset(&iXScroll, 0, sizeof(iXScroll));
	memset(&iYScroll, 0, sizeof(iYScroll));
}


XAP_App * XAP_App::m_pApp = NULL;

XAP_App * XAP_App::getApp() {return m_pApp;}

XAP_App::XAP_App(const char * szAppName)
	: m_szAppName(szAppName),
	  m_szAbiSuiteLibDir(NULL),
	  m_pEMC(NULL),
	  m_pBindingSet(NULL),
	  m_pMenuActionSet(NULL),
	  m_pToolbarActionSet(NULL),
	  m_pDict(NULL),
	  m_prefs(NULL),
	  m_lastFocussedFrame(NULL),
	  m_pMenuFactory(NULL),
	  m_pToolbarFactory(NULL),
	  m_bAllowCustomizing(true),
	  m_bAreCustomized(true),
	  m_bDebugBool(false),
	  m_bBonoboRunning(false),
	  m_bEnableSmoothScrolling(true),
      m_bDisableDoubleBuffering(false),
      m_bNoGUI(false),
	  m_pKbdLang(NULL), // must not be deleted by destructor !!!
 	  m_pUUIDGenerator(NULL),
	  m_pGraphicsFactory(NULL),
	  m_iDefaultGraphicsId(0),
	  m_pInputModes(NULL),
	  m_pImpl(NULL),
	  m_pScriptLibrary(NULL)
{
#ifdef DEBUG
	_fundamentalAsserts(); // see the comments in the function itself
	UT_DEBUGMSG(("ZZZZZZZZZZZZZZZZZZZZZZZaaaaaaaaaaaaaaAbiSuite Home |%s|\n",XAP_App::s_szAbiSuite_Home ));

#endif

	m_pGraphicsFactory = new GR_GraphicsFactory;
	UT_ASSERT( m_pGraphicsFactory );
	
	m_pImpl = XAP_AppImpl::static_constructor();
	
	UT_ASSERT(szAppName && *szAppName);
	m_pApp = this;

	// TODO don't build GUI when only doing commandline
	//      file conversion
	UT_DEBUGMSG(("SEVIOR: Building menus and toolbars \n"));
	m_pMenuFactory = new XAP_Menu_Factory(this);
	m_pToolbarFactory = new XAP_Toolbar_Factory(this);
	clearIdTable();
	m_vecPluginListeners.clear(); // Just to be safe
}

XAP_App::~XAP_App()
{
	// HACK: for now, this works from XAP code
	// TODO: where should this really go?
	if (m_pDict)
		m_pDict->save();

	// run thru and destroy all frames on our window list.
	UT_VECTOR_PURGEALL(XAP_Frame *, m_vecFrames);
	// when can have NULL pointers....

	FREEP(m_szAbiSuiteLibDir);
	DELETEP(m_pEMC);
	DELETEP(m_pBindingSet);
	DELETEP(m_pMenuActionSet);
	DELETEP(m_pToolbarActionSet);
	DELETEP(m_pDict);
	DELETEP(m_prefs);
	DELETEP(m_pMenuFactory);
	DELETEP(m_pToolbarFactory);

	// Delete the instance of the Encoding Manager.
	XAP_EncodingManager::get_instance()->Delete_instance();

	GR_CharWidthsCache::destroyCharWidthsCache();

	DELETEP(m_pUUIDGenerator);
	DELETEP(m_pGraphicsFactory);
	DELETEP(m_pInputModes);
	DELETEP(m_pImpl);
	DELETEP(m_pScriptLibrary);

	/* reset the static pointer, since it is no longer valid */
	m_pApp = NULL;
}

const char* XAP_App::getBuildId ()
{
	return s_szBuild_ID;
}

const char* XAP_App::getBuildVersion ()
{
	return s_szBuild_Version;
}

const char* XAP_App::getBuildOptions ()
{
	return s_szBuild_Options;
}

const char* XAP_App::getBuildTarget ()
{
	return s_szBuild_Target;
}

const char* XAP_App::getBuildCompileTime ()
{
	return s_szBuild_CompileTime;
}

const char* XAP_App::getBuildCompileDate ()
{
	return s_szBuild_CompileDate;
}

const char* XAP_App::getAbiSuiteHome ()
{
	return s_szAbiSuite_Home;
}

/*! this function is silly */
const XAP_EncodingManager* XAP_App::getEncodingManager() const 
{ 
	return XAP_EncodingManager::get_instance();
}

EV_Menu_ActionSet *XAP_App::getMenuActionSet()
{
	return m_pMenuActionSet;
}

EV_Toolbar_ActionSet *XAP_App::getToolbarActionSet()
{
	return m_pToolbarActionSet;
}

/*!
 * Returns a pointer to the requested plugin if it is loaded.
 * Return NULL otherwise.
 */
XAP_Module * XAP_App::getPlugin(const char * szPluginName)
{
     XAP_Module * pModule = NULL;
     const UT_GenericVector<XAP_Module*> * pVec = XAP_ModuleManager::instance().enumModules ();
     bool bFound = false;
     for (UT_sint32 i = 0; (i < pVec->size()) && !bFound; i++)
     {
          pModule = pVec->getNthItem (i);
	  const char * szName = pModule->getModuleInfo()->name;
	  if(g_ascii_strcasecmp(szName,szPluginName) == 0)
	  {
	        bFound = true;
	  }
     }
     if(!bFound)
     {
           return NULL;
     }
     return pModule;
}

/*!
 * Register an embeddable plugin with XAP_App
 */
bool XAP_App::registerEmbeddable(GR_EmbedManager * pEmbed, const char *uid)
{
	 UT_return_val_if_fail( pEmbed, false );
	 
	 if (uid == NULL)
		uid = pEmbed->getObjectType();
	 if (uid && *uid && m_mapEmbedManagers.find(uid) == m_mapEmbedManagers.end())
     {
		 m_mapEmbedManagers[uid] = pEmbed;
		 return true;
     }
	return false;
}


/*!
 * UnRegister an embeddable plugin with XAP_App. The plugin itself is 
 * responsible for actually deleting the object.
 */
bool XAP_App::unRegisterEmbeddable(const char *uid)
{
  if (uid == NULL || *uid == 0)
    return false;
  std::map<std::string, GR_EmbedManager *>::iterator i = m_mapEmbedManagers.find(uid);
  if(i != m_mapEmbedManagers.end())
    {
      m_mapEmbedManagers.erase(i);
      return true;
    }
  return false;
}

/*!
 * Return a copy of the requested embedable plugin or a default manager.
 * The calling routine must delete this.
 */
GR_EmbedManager * XAP_App:: getEmbeddableManager(GR_Graphics * pG, const char * szObjectType)
{
     GR_EmbedManager * pCur = NULL;
	 if (szObjectType && szObjectType != 0)
       pCur =  m_mapEmbedManagers[szObjectType];
     if(pCur != NULL)
     {
       UT_DEBUGMSG(("Found a plugin of type %s \n",pCur->getObjectType()));
       return pCur->create(pG);
     }
     return new GR_EmbedManager(pG);
}
 
bool XAP_App::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue)
{
	gsf_init();

	// create application-wide resources that
	// are shared by everything.

	// init keyboard language (cannot be in the constructor as it
	// requires the platform code already initialised
	setKbdLanguage(_getKbdLanguage());

	// HACK: for now, this works from XAP code
	// TODO: where should this really go?
	char * szPathname = g_build_filename(getUserPrivateDirectory(), "custom.dic", NULL);
	UT_ASSERT(szPathname);
	m_pDict = new XAP_Dictionary(szPathname);
	FREEP(szPathname);
	UT_return_val_if_fail(m_pDict,false);
	m_pDict->load();
	clearIdTable();
//
// Set Smooth Scrolling
//
	bool bEnableSmooth = true;
	getPrefsValueBool(XAP_PREF_KEY_EnableSmoothScrolling, &bEnableSmooth);
	if(bEnableSmooth)
		setEnableSmoothScrolling(true);
	else
		setEnableSmoothScrolling(false);

	//
	// Need to initialize the random number generator. 
	//
	UT_uint32 t = static_cast<UT_uint32>(time(NULL));
	UT_srandom(t);

	// Input mode initilization, taken out of the XAP_Frame
	const char * szBindings = NULL;
	EV_EditBindingMap * pBindingMap = NULL;

	if ((getPrefsValue(szKeyBindingsKey,
				 static_cast<const gchar**>(&szBindings))) && 
	    (szBindings) && (*szBindings))
		pBindingMap = m_pApp->getBindingMap(szBindings);
	if (!pBindingMap)
		pBindingMap = m_pApp->getBindingMap(szKeyBindingsDefaultValue);
	UT_ASSERT(pBindingMap);

	if (!m_pInputModes)
	{
		m_pInputModes = new XAP_InputModes();
		UT_ASSERT(m_pInputModes);
	}
	bool bResult;
	bResult = m_pInputModes->createInputMode(szBindings,pBindingMap);
	UT_ASSERT(bResult);
	bResult = m_pInputModes->setCurrentMap(szBindings);
	UT_ASSERT(bResult);
	UT_UNUSED(bResult); // TODO deal with the error

	// check if the prefs are set to use specific graphics class
	const char * pszGraphics = NULL;
	if(getPrefsValue(XAP_PREF_KEY_DefaultGraphics, &pszGraphics))
	{
		UT_uint32 iID = 0;

		// please leave this in hex format (the constants are defined in gr_Graphics.h as hex)
		sscanf(pszGraphics,"%x", &iID);
		if(iID != 0)
		{
			UT_DEBUGMSG(("Graphics %d requested as default\n", iID));

			// first of all, check that it is registered
			GR_GraphicsFactory * pGF = getGraphicsFactory();
			UT_return_val_if_fail(pGF,false);

			if(pGF->isRegistered(iID))
			{
				pGF->registerAsDefault(iID, true);
				pGF->registerAsDefault(iID, false);
			}
			else
			{
				UT_DEBUGMSG(("Graphics not loaded\n"));
			}
		}
	}
	m_pScriptLibrary = new UT_ScriptLibrary();
	return true;
}

/*!
 * Plugins can register themselves here if they want to receive
 * notifications of document changes.
 */
bool XAP_App::addListener(AV_Listener * pListener, 
							 AV_ListenerId * pListenerId)
{
	UT_sint32 kLimit = m_vecPluginListeners.getItemCount();
	UT_sint32 k;

	// see if we can recycle a cell in the vector.
	UT_DEBUGMSG(("Asked to register pListener %p \n",pListener));
	
	for (k=0; k<kLimit; k++)
		if (m_vecPluginListeners.getNthItem(k) == 0)
		{
			static_cast<void>(m_vecPluginListeners.setNthItem(k,pListener,NULL));
			goto ClaimThisK;
		}

	// otherwise, extend the vector for it.
	
	if (m_vecPluginListeners.addItem(pListener,&k) != 0)
	{
		UT_DEBUGMSG(("Failed! id %d \n",k));
		return false;				// could not add item to vector
	}

  ClaimThisK:

	// give our vector index back to the caller as a "Listener Id".
	
	*pListenerId = k;
	return true;
}


/*!
 * Plugins must remove themselves if they've registered.
 */
bool XAP_App::removeListener(AV_ListenerId listenerId)
{
	if (listenerId == (AV_ListenerId) -1)
		return false;
	
	if (m_vecPluginListeners.getNthItem(listenerId)) {
		m_vecPluginListeners.deleteNthItem(listenerId);
		return true;
	}
	return false;
}

/*!
 * Send notifications to all the registered plugins
 */
bool XAP_App::notifyListeners(AV_View * pView, const AV_ChangeMask hint, void * pPrivateData)
{
	/*
		App-specific logic calls this virtual method when relevant portions of 
		the view state *may* have changed.  (That's why it's called a hint.)

		This base class implementation doesn't do any filtering of those 
		hints, it just broadcasts those hints to all listeners.  

		Subclasses are encouraged to improve the quality of those hints by 
		filtering out mask bits which haven't *actually* changed since the 
		last notification.  To do so, they would 

			- copy the hint (it's passed as a const),
			- clear any irrelevant bits, and 
			- call this implementation on the way out

		Good hinting logic is app-specific, and non-trivial to tune, but it's 
		worth doing, because it helps minimizes flicker for things like 
		toolbar button state.  
	*/

	// make sure there's something left

	if (hint == AV_CHG_NONE)
	{
		return false;
	}
	
	// notify listeners of a change.
		
	AV_ListenerId lid;
	AV_ListenerId lidCount = m_vecPluginListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listners (for listeners which have been
	// removed (views that went away)).

	for (lid=0; lid<lidCount; lid++)
	{
		AV_Listener * pListener = static_cast<AV_Listener *>(m_vecPluginListeners.getNthItem(lid));
		if(pListener->getType()!= AV_LISTENER_PLUGIN_EXTRA )
		{
				pListener->notify(pView,hint);
		}
		else
		{
		  AV_ListenerExtra * pExtra = static_cast<AV_ListenerExtra *>(pListener);
		  pExtra->notify(pView,hint,pPrivateData);
		}
	}

	return true;
}


void XAP_App::setEnableSmoothScrolling(bool b)
{
	m_bEnableSmoothScrolling = b;
}

const char * XAP_App::getApplicationTitleForTitleBar() const
{
	static char _title[512];

	// return a string that the platform-specific code
	// can copy to the title bar of a window.

	//sprintf(_title, "%s (www.abisource.com)", m_szAppName);
	sprintf(_title, "%s", m_szAppName);

	return _title;
}

const char * XAP_App::getApplicationName() const
{
	// return a string that the platform-specific code
	// can use as a class name for various window-manager-like
	// operations.
	return m_szAppName;
}

void XAP_App::rebuildMenus(void)
{
	UT_uint32 frameCount = getFrameCount();

	for (UT_uint32 i = 0; i < frameCount; i++)
		if (XAP_Frame * pFrame = getFrame(i))
		{
			pFrame->rebuildMenus();
		}
}

EV_EditMethodContainer * XAP_App::getEditMethodContainer() const
{
	return m_pEMC;
}

EV_EditBindingMap * XAP_App::getBindingMap(const char * szName)
{
	UT_return_val_if_fail(m_pBindingSet,NULL);
	return m_pBindingSet->getMap(szName);
}

const EV_Menu_ActionSet * XAP_App::getMenuActionSet() const
{
	return m_pMenuActionSet;
}

const EV_Toolbar_ActionSet * XAP_App::getToolbarActionSet() const
{
	return m_pToolbarActionSet;
}

bool XAP_App::rememberFrame(XAP_Frame * pFrame, XAP_Frame * pCloneOf)
{
	UT_ASSERT(pFrame);

	// add this frame to our window list
	m_vecFrames.addItem(pFrame);

	if(! m_lastFocussedFrame)
	    rememberFocussedFrame(pFrame);

	// TODO: error-check the following for mem failures
	if (pCloneOf)
	{
		// locate vector of this frame's clones
		CloneMap::const_iterator iter = m_hashClones.find(pCloneOf->getViewKey());
		
		UT_GenericVector<XAP_Frame*> * pvClones = NULL;

		if (iter != m_hashClones.end())
		{
			UT_GenericVector<XAP_Frame*> * pEntry = iter->second;

			// hash table entry already exists
			pvClones = pEntry;

			if (!pvClones)
			{
				// nothing there, so create a new one
				pvClones = new UT_GenericVector<XAP_Frame*>();
				UT_return_val_if_fail(pvClones,false);

				pvClones->addItem(pCloneOf);

				// reuse this slot
				m_hashClones[pCloneOf->getViewKey()] = pvClones;
			}
		}
		else
		{
			// create a new one
			pvClones = new UT_GenericVector<XAP_Frame*>();
			UT_return_val_if_fail(pvClones,false);

			pvClones->addItem(pCloneOf);

			// add it to the hash table
			m_hashClones.insert(std::make_pair(pCloneOf->getViewKey(), pvClones));
		}

		pvClones->addItem(pFrame);

		// notify all clones of their new view numbers
		for (UT_sint32 j=0; j<pvClones->getItemCount(); j++)
		{
			XAP_Frame * f = pvClones->getNthItem(j);
			UT_continue_if_fail(f);

			f->setViewNumber(j+1);

			if (f != pFrame)
				f->updateTitle();
		}
	}
	
	// TODO do something here...
	notifyFrameCountChange();
	return true;
}

bool XAP_App::forgetFrame(XAP_Frame * pFrame)
{
	UT_return_val_if_fail(pFrame,false);

	// If this frame is the currently focussed frame write in NULL
	// until another frame appears

	if(pFrame == m_lastFocussedFrame )
	{
		m_lastFocussedFrame = static_cast<XAP_Frame *>(NULL);
	}

	if (pFrame->getViewNumber() > 0)
	{
		// locate vector of this frame's clones
		CloneMap::const_iterator iter = m_hashClones.find(pFrame->getViewKey());
		UT_ASSERT(iter != m_hashClones.end());

		if (iter != m_hashClones.end())
		{
			UT_GenericVector<XAP_Frame*> * pvClones = iter->second;
			UT_return_val_if_fail(pvClones,false);

			// remove this frame from the vector
			UT_sint32 i = pvClones->findItem(pFrame);
			UT_ASSERT(i >= 0);

			if (i >= 0)
			{
				pvClones->deleteNthItem(i);
			}

			// see how many clones are left
			UT_uint32 count = pvClones->getItemCount();
			UT_ASSERT(count > 0);
			XAP_Frame * f = NULL;

			if (count == 1)
			{
				// remaining clone is now a singleton
				f = pvClones->getNthItem(count-1);
				UT_return_val_if_fail(f,false);

				f->setViewNumber(0);
				f->updateTitle();

				// remove this entry from hashtable
				m_hashClones.erase(f->getViewKey());
				delete pvClones;
			}
			else
			{
				// notify remaining clones of their new view numbers
				for (UT_uint32 j=0; j<count; j++)
				{
					f = static_cast<XAP_Frame *>(pvClones->getNthItem(j));
					UT_continue_if_fail(f);

					f->setViewNumber(j+1);
					f->updateTitle();
				}
			}
		}
	}

	// remove this frame from our window list
	UT_sint32 ndx = m_vecFrames.findItem(pFrame);
	UT_ASSERT_HARMLESS(ndx >= 0);

	if (ndx >= 0)
	{
		m_vecFrames.deleteNthItem(ndx);
		notifyFrameCountChange();
	}

	notifyModelessDlgsCloseFrame(pFrame);

	// TODO do something here...

	return true;
}

bool XAP_App::forgetClones(XAP_Frame * pFrame)
{
	UT_return_val_if_fail(pFrame,false);

	if (pFrame->getViewNumber() == 0)
	{
		return forgetFrame(pFrame);
	}

	UT_GenericVector<XAP_Frame*> vClones;
	getClones(&vClones, pFrame);
	
	for (UT_sint32 i = 0; i < vClones.getItemCount(); i++)
	{
		XAP_Frame * f = static_cast<XAP_Frame *>(vClones.getNthItem(i));
		forgetFrame(f);
	}

	return true;
}

bool XAP_App::getClones(UT_GenericVector<XAP_Frame*> *pvClonesCopy, XAP_Frame * pFrame)
{
	UT_ASSERT(pvClonesCopy);
	UT_return_val_if_fail(pFrame,false);
	UT_ASSERT(pFrame->getViewNumber() > 0);

	// locate vector of this frame's clones
	CloneMap::const_iterator iter = m_hashClones.find(pFrame->getViewKey());
	UT_GenericVector<XAP_Frame*> * pvClones = NULL;
	if (iter != m_hashClones.end()) {
		pvClones = iter->second;
	}
	UT_ASSERT(pvClones);

	return pvClonesCopy->copy(pvClones);
}

bool XAP_App::updateClones(XAP_Frame * pFrame)
{
	UT_return_val_if_fail(pFrame,false);
	UT_ASSERT(pFrame->getViewNumber() > 0);

	// locate vector of this frame's clones
	CloneMap::const_iterator iter = m_hashClones.find(pFrame->getViewKey());
	UT_ASSERT(iter != m_hashClones.end());

	if (iter != m_hashClones.end())
	{
		UT_GenericVector<XAP_Frame*>* pvClones = iter->second;
		UT_return_val_if_fail(pvClones,false);

		UT_uint32 count = pvClones->getItemCount();
		UT_ASSERT(count > 0);
		XAP_Frame * f = NULL;

		for (UT_uint32 j=0; j<count; j++)
		{
			f = pvClones->getNthItem(j);
			UT_continue_if_fail(f);

			f->updateTitle();
		}
	}

	return true;
}

void XAP_App::notifyFrameCountChange() // default is empty method
{
	UT_DEBUGMSG(("XAP_App::notifyFrameCountChange(): count=%lu\n", static_cast<unsigned long>(getFrameCount())));
}

UT_sint32 XAP_App::getFrameCount() const
{
	return m_vecFrames.getItemCount();
}

XAP_Frame * XAP_App::getFrame(UT_sint32 ndx) const
{
	XAP_Frame * pFrame = NULL;
	
	if (ndx < m_vecFrames.getItemCount())
	{
		pFrame = m_vecFrames.getNthItem(ndx);
		UT_ASSERT(pFrame);
	}
	return pFrame;
}
	
UT_sint32 XAP_App::findFrame(XAP_Frame * pFrame) const
{
	return m_vecFrames.findItem(pFrame);
}
	
UT_sint32 XAP_App::findFrame(const char * szFilename) const
{
	if (!szFilename || !*szFilename)
		return -1;

	for (UT_sint32 i=0; i<getFrameCount(); i++)
	{
		XAP_Frame * f = getFrame(i);
		UT_continue_if_fail(f);
		const char * s = f->getFilename();

		if (s && *s && (0 == g_ascii_strcasecmp(szFilename, s)))
		{
			return i;
		}
	}

	return -1;
}

void XAP_App::_setAbiSuiteLibDir(const char * sz)
{
	FREEP(m_szAbiSuiteLibDir);
	m_szAbiSuiteLibDir = g_strdup(sz);
}

// This should be removed at some time.
void XAP_App::migrate (const char *oldName, const char *newName, const char *path) const
{
	UT_UNUSED(oldName);
	UT_UNUSED(newName);
	UT_UNUSED(path);
}

const char * XAP_App::getAbiSuiteLibDir() const
{
	return m_szAbiSuiteLibDir;
}

bool XAP_App::findAbiSuiteLibFile(std::string & path, const char * filename, const char * subdir)
{
	if (!filename)
	{ 
		return false;
	}

	bool bFound = false;

	const char * dir[2] = {
		getUserPrivateDirectory(),
		getAbiSuiteLibDir()
	};
	for (int i=0; !bFound && i<2; i++)
	{
		path = dir[i];
		if (subdir)
		{
			path += G_DIR_SEPARATOR;
			path += subdir;
		}
		path += G_DIR_SEPARATOR;
		path += filename;
		bFound = UT_isRegularFile (path.c_str ());
	}
	return bFound;
}

bool XAP_App::findAbiSuiteAppFile(std::string & path, const char * filename, const char * subdir) 
{
	if (!filename) 
	{
		return false;
	}

	bool bFound = false;

	const char * dir = getAbiSuiteAppDir();
	if (dir)
	{
		path = dir;
		if (subdir)
		{
			path += G_DIR_SEPARATOR;
			path += subdir;
		}
		path += G_DIR_SEPARATOR;
		path += filename;
		bFound = UT_isRegularFile (path.c_str ());
	}
	return bFound;
}

bool XAP_App::addWordToDict(const UT_UCSChar * pWord, UT_uint32 len)
{
	if (!m_pDict)
		return false;

	return m_pDict->addWord(pWord, len);
}

bool XAP_App::isWordInDict(const UT_UCSChar * pWord, UT_uint32 len) const
{
	if (!m_pDict)
		return false;

	return m_pDict->isWord(pWord, len);
}

/*!
 * Look up the custom dictionary for suggested words
 */
void XAP_App::suggestWord(UT_GenericVector<UT_UCSChar*> * pVecSuggestions, const UT_UCSChar * pWord, UT_uint32 lenWord)
{
	if(m_pDict)
	{
		m_pDict->suggestWord(pVecSuggestions, pWord, lenWord);
	}
}

XAP_Prefs * XAP_App::getPrefs() const
{
	return m_prefs;
}

bool XAP_App::getPrefsValue(const gchar * szKey, const gchar ** pszValue) const
{
	if (!m_prefs)
		return false;

	return m_prefs->getPrefsValue(szKey,pszValue);
}

bool XAP_App::getPrefsValue(const UT_String &stKey, UT_String &stValue) const
{
	if (!m_prefs)
		return false;

	return m_prefs->getPrefsValue(stKey, stValue);
}

bool XAP_App::getPrefsValueBool(const gchar * szKey, bool * pbValue) const
{
	if (!m_prefs)
		return false;

	return m_prefs->getPrefsValueBool(szKey,pbValue);
}

void XAP_App::rememberFocussedFrame( void * pJustFocussedFrame)
{
	m_lastFocussedFrame = static_cast<XAP_Frame *>(pJustFocussedFrame);

	UT_sint32 i = safefindFrame( m_lastFocussedFrame);
	if(i < 0 ) 
	{   
		m_lastFocussedFrame = static_cast<XAP_Frame *>(NULL);
	}
	notifyModelessDlgsOfActiveFrame(m_lastFocussedFrame);
}

UT_sint32 XAP_App::safefindFrame( XAP_Frame * f) const
{
	size_t ff = reinterpret_cast<size_t>(f);
	UT_sint32 num_frames = m_vecFrames.getItemCount();
	UT_sint32 i;
	for( i = 0; i< num_frames; i++)
	{
		size_t lf = reinterpret_cast<size_t>( m_vecFrames.getNthItem(i));
		if( lf == ff) break;
	}
	if( i == num_frames ) i = -1;
	return i;
}

void XAP_App::clearLastFocussedFrame()
{
	m_lastFocussedFrame = static_cast<XAP_Frame *>(NULL);
}

XAP_Frame* XAP_App::getLastFocussedFrame() const
{
		if(m_lastFocussedFrame == static_cast<XAP_Frame *>(NULL))
			return static_cast<XAP_Frame *>(NULL);
	UT_sint32 i = safefindFrame(m_lastFocussedFrame);
	if( i>= 0)
		return m_lastFocussedFrame;
	return static_cast<XAP_Frame *>(NULL);
}

XAP_Frame * XAP_App::findValidFrame() const
{
	XAP_Frame * validFrame =  getFrame(0);
	return validFrame;
}

void XAP_App::clearIdTable()
{
	for(UT_sint32 i =0; i <= NUM_MODELESSID; i++)
	{
		m_IdTable[i].id =  -1;
		m_IdTable[i].pDialog = static_cast<XAP_Dialog_Modeless *>(NULL);
	}
}

void XAP_App::rememberModelessId(UT_sint32 id, XAP_Dialog_Modeless * pDialog)
{
	// find a free slot in the m_IdTable
 
	UT_sint32 i;
	for(i=0; (i<= NUM_MODELESSID) && (m_IdTable[i].id !=  -1); i++) {

	}
	UT_ASSERT( i <= NUM_MODELESSID );
// This assert cause a warning because array subscript is out of bounds.
//	UT_ASSERT( m_IdTable[i].id == -1 );
	UT_ASSERT( pDialog);
	m_IdTable[i].id =  id;
	m_IdTable[i].pDialog =  pDialog;
}

void XAP_App::forgetModelessId( UT_sint32 id )
{
	// remove the id, pDialog pair from the m_IdTable

	UT_sint32 i;
	for(i=0; i <= NUM_MODELESSID && m_IdTable[i].id != id; i++) {

	} 
	if(i >  NUM_MODELESSID)
	{
//
// forgotten already
//
		return;
	}
	m_IdTable[i].id =  -1;
	m_IdTable[i].pDialog = static_cast<XAP_Dialog_Modeless *>(NULL);
}

bool XAP_App::isModelessRunning(UT_sint32 id)
{
	// returns true if the modeless dialog given by id is running
	
	UT_sint32 i;
	for(i=0; i <= NUM_MODELESSID && m_IdTable[i].id != id; i++) {

	}
	if( i> NUM_MODELESSID)
	{
		return false;
	}
	UT_ASSERT( m_IdTable[i].id == id );
	return true;
}
        
XAP_Dialog_Modeless * XAP_App::getModelessDialog( UT_sint32 i)
{
	// Retrieve pDialog from the table based on its location in the table
	return m_IdTable[i].pDialog;
}

void XAP_App::closeModelessDlgs()
{
	// Delete all the open modeless dialogs

	for(UT_sint32 i=0; i <= NUM_MODELESSID; i++)
	{
		if(m_IdTable[i].id >= 0)
		{
			if(getModelessDialog(i) != static_cast<XAP_Dialog_Modeless *>(NULL))
			{
				getModelessDialog(i)->destroy();
			}
			m_IdTable[i].id = -1;
			m_IdTable[i].pDialog = NULL;
		}
	}
}


void XAP_App::notifyModelessDlgsOfActiveFrame(XAP_Frame *p_Frame)
{
	for(UT_sint32 i=0; i <= NUM_MODELESSID; i++)
	{
		if(getModelessDialog(i) != static_cast<XAP_Dialog_Modeless *>(NULL))
		{
			getModelessDialog(i)->setActiveFrame(p_Frame);
		}
	}
}

void XAP_App::notifyModelessDlgsCloseFrame(XAP_Frame *p_Frame)
{
	for(UT_sint32 i=0; i <= NUM_MODELESSID; i++)
	{
		if(getModelessDialog(i) != static_cast<XAP_Dialog_Modeless *>(NULL))
		{
			getModelessDialog(i)->notifyCloseFrame(p_Frame);
		}
	}
}

/* Window Geometry Preferences */
bool XAP_App::setGeometry(UT_sint32 x, UT_sint32 y, UT_uint32 width, UT_uint32 height , UT_uint32 flags) 
{
	XAP_Prefs *prefs = getPrefs();
	return prefs->setGeometry(x, y, width, height, flags);
}

bool XAP_App::getGeometry(UT_sint32 *x, UT_sint32 *y, UT_uint32 *width, UT_uint32 *height, UT_uint32 *flags) 
{
	XAP_Prefs *prefs = getPrefs();
	return prefs->getGeometry(x, y, width, height, flags);
}

void XAP_App::parseAndSetGeometry(const char *string)
{
	UT_uint32 nw, nh, nflags;
    UT_sint32 nx, ny;
    char *next;

	nw = nh = nflags = 0;
	nx = ny = 0;

    next = const_cast<char *>(string);
    if (*next != '+' && *next != '-')
	{
        nw = strtoul(next, &next, 10);
        if(*next == 'x' || *next == 'X')
		{
			next++;
            nh = strtoul(next, &next, 10);
            nflags |= PREF_FLAG_GEOMETRY_SIZE;
        }
    }
    if (*next == '+' || *next == '-')
	{
        nx = strtoul(next, &next, 10);
        if(*next == '+' || *next == '-')
		{
            ny = strtoul(next, &next, 10);
            nflags |= PREF_FLAG_GEOMETRY_POS;
        }
    }

	//Don't update the geometry from the file
	if(nflags)
	{
		nflags |= PREF_FLAG_GEOMETRY_NOUPDATE;
		setGeometry(nx, ny, nw, nh, nflags);
	}
} 

/*!
    translate given language tag into static UT_LangRecord stored in
    UT_Language class and set m_pKbdLang to it; do addtional
    processing to ensure the change propagates into the status bar
*/
void XAP_App::setKbdLanguage(const char * pszLang)
{
	if(!pszLang)
	{
		m_pKbdLang = NULL;
	}
	else
	{
		UT_Language Lang;
		m_pKbdLang = Lang.getLangRecordFromCode(pszLang);

		// ensure that the change is shown in our status bar
	    bool bChangeLang = false;
		getPrefsValueBool(XAP_PREF_KEY_ChangeLanguageWithKeyboard, &bChangeLang);

		if(bChangeLang && m_pKbdLang)
		{
			UT_return_if_fail(m_pKbdLang->m_szLangCode);
			
			// invoke appropriate formatting method if it exists
			const EV_EditMethodContainer * pEMC = getEditMethodContainer();

			if(pEMC)
			{
				EV_EditMethod * pEM = pEMC->findEditMethodByName("language");
			
				if (pEM)
				{
					XAP_Frame * pFrame = getLastFocussedFrame();
					
					if(pFrame)
					{
						AV_View * pView = pFrame->getCurrentView();

						if(pView)
						{
							EV_EditMethodCallData CallData(m_pKbdLang->m_szLangCode,
														   strlen(m_pKbdLang->m_szLangCode));

							pEM->Fn(pView,&CallData);
						}
					}
				}
			}
		}
	}
}

void XAP_App::enumerateFrames(UT_Vector & v) const
{
	for(UT_sint32 i = 0; i < getFrameCount(); ++i)
	{
		XAP_Frame * pF = getFrame(i);
		if(pF)
		{
			if (v.findItem((void*)pF) < 0)
			{
				v.addItem((void*)pF);
			}
		}
	}
}

std::list< AD_Document* >
XAP_App::getDocuments( const AD_Document * pExclude ) const
{
    UT_Vector t;
    enumerateDocuments( t, pExclude );
    std::list< AD_Document* > ret;
    for( int i=0; i < t.size(); ++i )
        ret.push_back( (AD_Document*)t[i] );
    return ret;
}


/*!
    Enumerates currently open document associated with the
    application, excluding document pointed to by pExclude

    \param v: UT_Vector into which to store the document pointers
    
    \para pExclude: pointer to a document to exclude from enumeration,
                    can be NULL (e.g., if this function is called from
                    inside a document, it might be desirable to
                    exclude that document)
*/
void XAP_App::enumerateDocuments(UT_Vector & v, const AD_Document * pExclude) const
{
	UT_sint32 iIndx;

	for(UT_sint32 i = 0; i < getFrameCount(); ++i)
	{
		XAP_Frame * pF = getFrame(i);

		if(pF)
		{
			AD_Document * pD = pF->getCurrentDoc();

			if(pD && pD != pExclude)
			{
				iIndx = v.findItem((void*)pD);

				if(iIndx < 0)
				{
					v.addItem((void*)pD);
				}
			}
		}
	}
}

EV_EditEventMapper * XAP_App::getEditEventMapper(void) const
{
	UT_return_val_if_fail(m_pInputModes,NULL);
	return m_pInputModes->getCurrentMap();
}

UT_sint32 XAP_App::setInputMode(const char * szName, bool bForce)
{
	UT_sint32 i;
	
	UT_DEBUGMSG(("XAP_App::setInputMode: %s %d\n", szName, bForce));
	
	UT_return_val_if_fail(m_pInputModes,-1);
	const char * szCurrentName = m_pInputModes->getCurrentMapName();
	if (!bForce && (g_ascii_strcasecmp(szName,szCurrentName) == 0))
		return 0;					// already set, no change required

	EV_EditEventMapper * p = m_pInputModes->getMapByName(szName);
	if (!p)
	{
		// map not previously loaded -- we need to install it first

		EV_EditBindingMap * pBindingMap = m_pApp->getBindingMap(szName);
		UT_return_val_if_fail(pBindingMap,-1);
		bool bResult;
		bResult = m_pInputModes->createInputMode(szName,pBindingMap);
		UT_return_val_if_fail(bResult,-1);
	}
	
	// note: derrived classes will need to update keyboard
	// note: and mouse after we return.

	UT_DEBUGMSG(("Setting InputMode to [%s] for the current window.\n",szName));
	
	bool bStatus = m_pInputModes->setCurrentMap(szName);
	
	// notify all the frames about the INPUTMODE change
	for (i = 0; i < getFrameCount(); i++) {
		getFrame(i)->getCurrentView()->notifyListeners(AV_CHG_INPUTMODE);
	}
	
	// rebuild menu's
	UT_DEBUGMSG(("XAP_App::setInputMode:: rebuilding menu's!"));
	rebuildMenus();
	
	return (bStatus);
}

const char * XAP_App::getInputMode(void) const
{
	return m_pInputModes->getCurrentMapName();
}

/*!
    This is the primary function for allocating graphics classes. It
    determines if screen or priter graphics is required and then
    allocates appropriate graphics respecting current setting.
*/
GR_Graphics * XAP_App::newGraphics(GR_AllocInfo &param) const
{
	UT_return_val_if_fail(m_pGraphicsFactory, NULL);

	if(param.isPrinterGraphics())
	{
		return m_pGraphicsFactory->newGraphics(GRID_DEFAULT_PRINT, param);
	}
	else
	{
		return m_pGraphicsFactory->newGraphics(GRID_DEFAULT, param);
	}
}

/*!
    Use only if you require specific graphics, otherwise use newGraphics(GR_AllocInfo*)
*/
GR_Graphics * XAP_App::newGraphics(UT_uint32 iClassId, GR_AllocInfo &param) const
{
	UT_return_val_if_fail(m_pGraphicsFactory, NULL);

	return m_pGraphicsFactory->newGraphics(iClassId, param);
}

void XAP_App::setDefaultGraphicsId(UT_uint32 i)
{
	if(i == GRID_UNKNOWN)
		return;
	
	m_iDefaultGraphicsId = i;

	if(i < GRID_LAST_BUILT_IN && i > GRID_LAST_DEFAULT)
	{
		// change the preference settings
		UT_return_if_fail(m_prefs)
			
		XAP_PrefsScheme *pPrefsScheme = m_prefs->getCurrentScheme();
		UT_return_if_fail(pPrefsScheme);

		UT_String s;
		UT_String_sprintf(s, "%d", i);
		
		pPrefsScheme->setValue(XAP_PREF_KEY_DefaultGraphics, s.c_str());
	}
}

/*!
    Find the nearest matching font based on the provided parameters

    TODO: we should use a more sophisticated description of fonts to allow for
    better matching
*/
const char* XAP_App::findNearestFont(const char* pszFontFamily,
									 const char* pszFontStyle,
									 const char* pszFontVariant,
									 const char* pszFontWeight,
									 const char* pszFontStretch,
									 const char* pszFontSize,
									 const char* pszLang)
{
	return GR_Graphics::findNearestFont(pszFontFamily, pszFontStyle,
										pszFontVariant, pszFontWeight,
										pszFontStretch, pszFontSize,
										pszLang);
}

/*!
   Puts together the data necessary for state saving, carrying out any auxiliary actions
   required (such as dealing with dirty documents).

   The actual mechanism through which the state data is stored is implemented in _saveState().
*/

#define HIBERNATED_EXT ".HIBERNATED.abw"
bool XAP_App::saveState(bool bQuit)
{
	// gather the state data for platform code to deal with
	XAP_StateData sd;

	bool bRet = true;

	// We will store data for up to XAP_SD_MAX_FILES, making sure we save it for the last
	// focussed frame in the first slot

	XAP_Frame * pLastFrame = getLastFocussedFrame();

	UT_sint32 i;
	UT_sint32 j;
	
	for(i = 0, j = 0; i < m_vecFrames.getItemCount(); ++i, ++j)
	{
		XAP_Frame * pFrame = NULL;

		if(i == 0)
			pFrame = pLastFrame;
		else
			pFrame = m_vecFrames[i];

		if(pLastFrame == pFrame && j != 0)
		{
			// we have done this frame, but need to do the one at pos 0 in its place
			pFrame = m_vecFrames[0];
		}
		

		if(!pFrame)
		{
			--j;
			continue;
		}
		
		AD_Document * pDoc = pFrame->getCurrentDoc();

		if(!pDoc)
		{
			--j;
			continue;
		}

		UT_Error e = UT_OK;
		
		if(pDoc->isDirty())
		{
			// need to decide what to do about dirty documents; perhaps we should keep a
			// copy of the unsaved state under a different name?
			// for now just save (otherwise the user will loose the changes when app
			// hibernates)
			e = pDoc->save();
			if(e == UT_SAVE_NAMEERROR)
			{
				// this is an Untitled document
				UT_UTF8String s = pFrame->getNonDecoratedTitle();
				s += HIBERNATED_EXT;
				e = pDoc->saveAs(s.utf8_str(), 0);
			}
			
			bRet &= (UT_OK == e);
		}

		if(j >= XAP_SD_MAX_FILES || e != UT_OK)
		{
			// no storage space left -- nothing more we can do with this document, so move
			// to the next one (do not break, we need to deal with anything that is not
			// saved)
			
			--j;      // we want to preserve the j value 
			continue;
		}
		
			
		const std::string & file = pDoc->getFilename();
		if(!file.empty() && file.size() < XAP_SD_FILENAME_LENGTH)
		{
			strncpy(sd.filenames[j], file.c_str(), XAP_SD_FILENAME_LENGTH);

			AV_View * pView = pFrame->getCurrentView();
			if(pView)
			{
				sd.iDocPos[j]  = pView->getPoint();
				sd.iXScroll[j] = pView->getXScrollOffset();
				sd.iYScroll[j] = pView->getYScrollOffset();
			}
		}
		else
		{
			--j;
			continue;
		}
	}

	sd.iFileCount = j;
	
	if(!_saveState(sd))
		return false;

	if(bQuit)
	{
		// we have dealt with unsaved docs above, so just clean up any modeless dlgs and quit
		closeModelessDlgs();
		reallyExit();
	}

	return bRet;
}

/*!
    Implements the actual mechanism for storing of status data; derrived classes can
    override to provide methods suitable to their platform.

    For now, this method does nothing; it could save state data in the profile if we
    wanted to. (For a working implementation see xap_UnixHildonApp.cpp)

    returns true on success
*/
bool XAP_App::_saveState(XAP_StateData & )
{
	return false;
}

/*!
    Does the work necessary to restore the application to a previously saved state. The
    actual mechanism through which the stored state data is retrieved is implemented in
    _retrieveState().
*/


bool XAP_App::retrieveState()
{
	XAP_StateData sd;

	bool bRet = true;
	
	if(!_retrieveState(sd))
		return false;

	UT_return_val_if_fail(sd.iFileCount <= XAP_SD_MAX_FILES, false);
		
	// now do our thing with it:
	//  * open the files stored in the data
	//  * move carets and scrollbars to the saved positions
	//  * make the first saved frame to be the current frame

	// we should only be restoring state with no docs already
	// opened
	UT_return_val_if_fail(m_vecFrames.getItemCount() <= 1, false);
	XAP_Frame * pFrame = NULL;

	if(m_vecFrames.getItemCount())
		pFrame = m_vecFrames.getNthItem(0);

	// if there is a frame, it should be one with unmodified untitled document
	UT_return_val_if_fail( !pFrame || (!pFrame->getFilename() && !pFrame->isDirty()), false );
		
	UT_Error errorCode = UT_IE_IMPORTERROR;
	
	for(UT_uint32 i = 0; i < sd.iFileCount; ++i)
	{
		if(!pFrame)
			pFrame = newFrame();
		
		if (!pFrame)
			return false;
		
		// Open a complete but blank frame, then load the document into it
		errorCode = pFrame->loadDocument((const char *)NULL, 0 /*IEFT_Unknown*/);

		bRet &= (errorCode == UT_OK);
		
		if (errorCode == UT_OK)
			pFrame->show();
	    else
			continue;

		errorCode = pFrame->loadDocument(sd.filenames[i], 0 /*IEFT_Unknown*/);

		bRet &= (errorCode == UT_OK);
		
		if (errorCode != UT_OK)
			continue;

		pFrame->show();

		AV_View* pView = pFrame->getCurrentView();
		if(!pView)
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			bRet = false;
			continue;
		}
		
		pView->setPoint(sd.iDocPos[i]);
		pView->setXScrollOffset(sd.iXScroll[i]);
		pView->setYScrollOffset(sd.iYScroll[i]);

		// now we check if this doc was autosaved Untitled* doc at hibernation
		char * p = strstr(sd.filenames[i], HIBERNATED_EXT);
		if(p)
		{
			// remove extension
			p = 0;
			AD_Document * pDoc = pFrame->getCurrentDoc();

			if(pDoc)
			{
				pDoc->clearFilename();
				pDoc->forceDirty();
				pFrame->updateTitle();
			}
		}
		
		
		// frame used -- next doc needs a new one
		pFrame = NULL;
	}

	// set focus to the first frame
	pFrame = m_vecFrames.getNthItem(0);
	UT_return_val_if_fail( pFrame, false );

	AV_View* pView = pFrame->getCurrentView();
	UT_return_val_if_fail( pView, false );

	pView->focusChange(AV_FOCUS_HERE);

	return bRet;
}

/*!
    Implements the actual mechanism for retrieving of status data; derrived classes can
    override to provide methods suitable to their platform.

    For now, this method does nothing; it could read state data in the profile if we
    wanted to. (For a working implementation see xap_UnixHildonApp.cpp)

    returns true on success
*/
bool XAP_App::_retrieveState(XAP_StateData & )
{
	return false;
}



#ifdef DEBUG
/*!
    This function contains some of the most fundamental assumptions
    about the compiler without which the code will not work; this is
    for the benefit of future ports, etc.
    Tomas, May 15, 2003

    If you get assert in this function, then you have a real problem
    -- the code will not work without substantial adjustments.
*/
void XAP_App::_fundamentalAsserts() const
{
	// our UT_Vector class is frequently used to store integers
	// directly inside of a void *
	UT_ASSERT(sizeof(void*) >= sizeof(UT_uint32));

	// we frequently assume that gchar * is only one byte in size
	UT_ASSERT(sizeof(gchar) == sizeof(char));
}
#endif

bool
XAP_App::getDisableDoubleBuffering() const
{
    return m_bDisableDoubleBuffering;
}

void
XAP_App::setDisableDoubleBuffering( bool v )
{
    m_bDisableDoubleBuffering = v;
}

bool
XAP_App::getNoGUI() const
{
    return m_bNoGUI;
}

void
XAP_App::setNoGUI( bool v )
{
    setDisableDoubleBuffering(true);
    m_bNoGUI = v;
}

std::string
XAP_App::createUUIDString() const
{
    std::unique_ptr<UT_UUID> uuido(getUUIDGenerator()->createUUID());
    std::string ret;
	uuido->toString(ret);
    return ret;
}


/*!
  Signal function
*/
void XAP_App::signalWrapper(int sig_num)
{
    XAP_App *pApp = XAP_App::getApp();
    pApp->catchSignals(sig_num);
}

