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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_Language.h"
#include "ut_uuid.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Actions.h"
#include "ev_Toolbar_Actions.h"
#include "xap_App.h"
#include "xap_AppImpl.h"
#include "xap_Args.h"
#include "xap_InputModes.h"
#include "gr_Image.h"
#include "xap_Frame.h"
#include "xap_EditMethods.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_LoadBindings.h"
#include "xap_Dictionary.h"
#include "xap_Prefs.h"
#include "xap_EncodingManager.h"
#include "xap_Menu_Layouts.h"
#include "xap_Toolbar_Layouts.h"
#include "xav_View.h"
#include "ut_rand.h"
#include "ut_map.h"
#include "gr_CharWidthsCache.h"
#include "gr_ContextGlyph.h"

UT_Map * abi_ut_map_instance = 0;

/*****************************************************************/

XAP_App * XAP_App::m_pApp = NULL;

XAP_App * XAP_App::getApp() {return m_pApp;}

XAP_App::XAP_App(XAP_Args * pArgs, const char * szAppName)
	: m_pArgs(pArgs),
	  m_szAppName(szAppName),
	  m_szAbiSuiteLibDir(NULL),
	  m_pEMC(NULL),
	  m_pBindingSet(NULL),
	  m_pMenuActionSet(NULL),
	  m_pToolbarActionSet(NULL),
	  m_pDict(NULL),
	  m_prefs(NULL),
	  m_hashClones(5),
	  m_lastFocussedFrame(NULL),
	  m_pMenuFactory(NULL),
	  m_pToolbarFactory(NULL),
	  m_bAllowCustomizing(true),
	  m_bAreCustomized(true),
	  m_bDebugBool(false),
	  m_bBonoboRunning(false),
	  m_bEnableSmoothScrolling(true),
	  m_pKbdLang(NULL), // must not be deleted by destructor !!!
 	  m_pUUIDGenerator(NULL),
	  m_pGraphicsFactory(NULL),
	  m_iDefaultGraphicsId(0),
	  m_pInputModes(NULL),
	  m_pImpl(NULL)
{
#ifdef DEBUG
	_fundamentalAsserts(); // see the comments in the function itself
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

	/* hack to force the linker to link in UT_Map functions
	 */
	if (abi_ut_map_instance)
	{
	    delete abi_ut_map_instance;
	    abi_ut_map_instance = new UT_Map;

	    _UT_OutputMessage("Yet another fun hack from the makers of 'Fun Hacks: The Prequel'!");
	}
}

XAP_App::~XAP_App()
{
	// HACK: for now, this works from XAP code
	// TODO: where should this really go?
	if (m_pDict)
		m_pDict->save();

	// run thru and destroy all frames on our window list.
	UT_VECTOR_PURGEALL(XAP_Frame *, m_vecFrames);

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

	// This is to delete static data allocated by Gr_ContextGlyph; it
	// is strictly speaking not necessary -- this is really to shut up
	// the debugger complaining about memory leaks
	GR_ContextGlyph::static_destructor();
	GR_CharWidthsCache::destroyCharWidthsCache();

	DELETEP(m_pUUIDGenerator);
	DELETEP(m_pGraphicsFactory);
	DELETEP(m_pInputModes);
	DELETEP(m_pImpl);
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
};

EV_Menu_ActionSet *XAP_App::getMenuActionSet()
{
	return m_pMenuActionSet;
}

EV_Toolbar_ActionSet *XAP_App::getToolbarActionSet()
{
	return m_pToolbarActionSet;
}

bool XAP_App::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue)
{
	// create application-wide resources that
	// are shared by everything.

	// init keyboard language (cannot be in the constructor as it
	// requires the platform code already initialised
	setKbdLanguage(_getKbdLanguage());
	
#if 0 
	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_ASSERT(m_pBindingSet);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);
#endif

	// HACK: for now, this works from XAP code
	// TODO: where should this really go?
	char * szPathname = UT_catPathname(getUserPrivateDirectory(),"custom.dic");
	UT_ASSERT(szPathname);
	m_pDict = new XAP_Dictionary(szPathname);
	FREEP(szPathname);
	UT_ASSERT(m_pDict);
	m_pDict->load();
	clearIdTable();
//
// Reload toolbar configuration from prefs
//
	bool bAllowCustom = true;
	getPrefsValueBool(XAP_PREF_KEY_AllowCustomToolbars, &bAllowCustom);
	if(bAllowCustom)
		setToolbarsCustomizable(true);
	else
	{
		setToolbarsCustomizable(false);
		setToolbarsCustomized(false);
	}
	m_pToolbarFactory->restoreToolbarsFromCurrentScheme();
	if(!bAllowCustom)
	{
		m_pToolbarFactory->resetAllToolbarsToDefault();
	}
//
// Set Smooth Scrolling
//
	bool bEnableSmooth = true;
	getPrefsValueBool(XAP_PREF_KEY_EnableSmoothScrolling, &bEnableSmooth);
	if(bEnableSmooth)
		setEnableSmoothScrolling(true);
	else
		setEnableSmoothScrolling(false);

	// TODO use m_pArgs->{argc,argv} to process any command-line
	// TODO options that we need.
	//
	// Need to initialize the random number generator. 
	//
	UT_uint32 t = static_cast<UT_uint32>(time(NULL));
	UT_srandom(t);

	// Input mode initilization, taken out of the XAP_Frame
	const char * szBindings = NULL;
	EV_EditBindingMap * pBindingMap = NULL;

	if ((getPrefsValue(szKeyBindingsKey,
				 static_cast<const XML_Char**>(&szBindings))) && 
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
	bool bResult2;
	bResult2 = m_pInputModes->setCurrentMap(szBindings);
	UT_ASSERT(bResult2);


	return true;
}

void XAP_App::resetToolbarsToDefault(void)
{
	//
	// Set all the frames to default toolbars
	//
	m_pToolbarFactory->resetAllToolbarsToDefault();
	UT_uint32 count = m_vecFrames.getItemCount();
	UT_Vector vClones;
	UT_uint32 i = 0;
	for(i=0; i< count; i++)
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(m_vecFrames.getNthItem(i));
		if(pFrame->getViewNumber() > 0)
		{
			getClones(&vClones,pFrame);
			UT_uint32 j=0;
			for(j=0; j < vClones.getItemCount(); j++)
			{
				XAP_Frame * f = static_cast<XAP_Frame *>(vClones.getNthItem(j));
				f->rebuildAllToolbars();
			}
		}
		else
		{
			pFrame->rebuildAllToolbars();
		}
	}
	setToolbarsCustomized (true);
}

void XAP_App::setEnableSmoothScrolling(bool b)
{
	m_bEnableSmoothScrolling = b;
}

void XAP_App::setToolbarsCustomizable(bool b)
{
	if(m_bAllowCustomizing != b)
		m_bAllowCustomizing = b;
}

void XAP_App::setToolbarsCustomized(bool b)
{
	if(m_bAreCustomized != b)
		m_bAreCustomized = b;
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

EV_EditMethodContainer * XAP_App::getEditMethodContainer() const
{
	return m_pEMC;
}

EV_EditBindingMap * XAP_App::getBindingMap(const char * szName)
{
	UT_ASSERT(m_pBindingSet);
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
		void * pEntry = const_cast<void *>(m_hashClones.pick(pCloneOf->getViewKey()));
		UT_Vector * pvClones = NULL;

		if (pEntry)
		{
			// hash table entry already exists
			pvClones = static_cast<UT_Vector *>(pEntry);

			if (!pvClones)
			{
				// nothing there, so create a new one
				pvClones = new UT_Vector();
				UT_ASSERT(pvClones);

				pvClones->addItem(pCloneOf);

				// reuse this slot
				m_hashClones.set(pCloneOf->getViewKey(), 
						 static_cast<void *>(pvClones));
			}
		}
		else
		{
			// create a new one
			pvClones = new UT_Vector();
			UT_ASSERT(pvClones);

			pvClones->addItem(pCloneOf);

			// add it to the hash table
			m_hashClones.insert(pCloneOf->getViewKey(), static_cast<void *>(pvClones));
		}

		pvClones->addItem(pFrame);

		// notify all clones of their new view numbers
		for (UT_uint32 j=0; j<pvClones->getItemCount(); j++)
		{
			XAP_Frame * f = static_cast<XAP_Frame *>(pvClones->getNthItem(j));
			UT_ASSERT(f);

			f->setViewNumber(j+1);

			if (f != pFrame)
				f->updateTitle();
		}
	}
	
	// TODO do something here...
	return true;
}

bool XAP_App::forgetFrame(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	// If this frame is the currently focussed frame write in NULL
	// until another frame appears

	if(pFrame == m_lastFocussedFrame )
	{
		m_lastFocussedFrame = static_cast<XAP_Frame *>(NULL);
	}

	if (pFrame->getViewNumber() > 0)
	{
		// locate vector of this frame's clones
		void * pEntry = const_cast<void *>(m_hashClones.pick(pFrame->getViewKey()));
		UT_ASSERT(pEntry);

		if (pEntry)
		{
			UT_Vector * pvClones = static_cast<UT_Vector *>(pEntry);
			UT_ASSERT(pvClones);

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
				f = static_cast<XAP_Frame *>(pvClones->getNthItem(count-1));
				UT_ASSERT(f);

				f->setViewNumber(0);
				f->updateTitle();

				// remove this entry from hashtable
				m_hashClones.remove(f->getViewKey(), 
						    NULL);
				delete pvClones;
			}
			else
			{
				// notify remaining clones of their new view numbers
				for (UT_uint32 j=0; j<count; j++)
				{
					f = static_cast<XAP_Frame *>(pvClones->getNthItem(j));
					UT_ASSERT(f);

					f->setViewNumber(j+1);
					f->updateTitle();
				}
			}
		}
	}

	// remove this frame from our window list
	UT_sint32 ndx = m_vecFrames.findItem(pFrame);
	UT_ASSERT(ndx >= 0);

	if (ndx >= 0)
	{
		m_vecFrames.deleteNthItem(ndx);
	}

	notifyModelessDlgsCloseFrame(pFrame);

	// TODO do something here...

	return true;
}

bool XAP_App::forgetClones(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	if (pFrame->getViewNumber() == 0)
	{
		return forgetFrame(pFrame);
	}

	UT_Vector vClones;
	getClones(&vClones, pFrame);
	
	for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
	{
		XAP_Frame * f = static_cast<XAP_Frame *>(vClones.getNthItem(i));
		forgetFrame(f);
	}

	return true;
}

bool XAP_App::getClones(UT_Vector *pvClonesCopy, XAP_Frame * pFrame)
{
	UT_ASSERT(pvClonesCopy);
	UT_ASSERT(pFrame);
	UT_ASSERT(pFrame->getViewNumber() > 0);

	// locate vector of this frame's clones
	void * pEntry = const_cast<void *>(m_hashClones.pick(pFrame->getViewKey()));
	UT_ASSERT(pEntry);

	UT_Vector * pvClones = static_cast<UT_Vector *>(pEntry);
	UT_ASSERT(pvClones);

	return pvClonesCopy->copy(pvClones);
}

bool XAP_App::updateClones(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(pFrame->getViewNumber() > 0);

	// locate vector of this frame's clones
	void * pEntry = const_cast<void *>(m_hashClones.pick(pFrame->getViewKey()));
	UT_ASSERT(pEntry);

	if (pEntry)
	{
		UT_Vector * pvClones = static_cast<UT_Vector *>(pEntry);
		UT_ASSERT(pvClones);

		UT_uint32 count = pvClones->getItemCount();
		UT_ASSERT(count > 0);
		XAP_Frame * f = NULL;

		for (UT_uint32 j=0; j<count; j++)
		{
			f = static_cast<XAP_Frame *>(pvClones->getNthItem(j));
			UT_ASSERT(f);

			f->updateTitle();
		}
	}

	return true;
}

UT_uint32 XAP_App::getFrameCount() const
{
	return m_vecFrames.getItemCount();
}

XAP_Frame * XAP_App::getFrame(UT_uint32 ndx) const
{
	XAP_Frame * pFrame = NULL;
	
	if (ndx < m_vecFrames.getItemCount())
	{
		pFrame = static_cast<XAP_Frame *>(m_vecFrames.getNthItem(ndx));
		UT_ASSERT(pFrame);
	}
	return pFrame;
}
	
UT_sint32 XAP_App::findFrame(XAP_Frame * pFrame)
{
	return m_vecFrames.findItem(pFrame);
}
	
UT_sint32 XAP_App::findFrame(const char * szFilename)
{
	if (!szFilename || !*szFilename)
		return -1;

	for (UT_uint32 i=0; i<getFrameCount(); i++)
	{
		XAP_Frame * f = getFrame(i);
		UT_ASSERT(f);
		const char * s = f->getFilename();

		if (s && *s && (0 == UT_stricmp(szFilename, s)))
		{
			return static_cast<UT_sint32>(i);
		}
	}

	return -1;
}

void XAP_App::_setAbiSuiteLibDir(const char * sz)
{
	FREEP(m_szAbiSuiteLibDir);
	UT_cloneString((char *&)m_szAbiSuiteLibDir,sz);
}

const char * XAP_App::getAbiSuiteLibDir() const
{
	return m_szAbiSuiteLibDir;
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
void XAP_App::suggestWord(UT_Vector * pVecSuggestions, const UT_UCSChar * pWord, UT_uint32 lenWord)
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

bool XAP_App::getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const
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

bool XAP_App::getPrefsValueBool(const XML_Char * szKey, bool * pbValue) const
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

UT_sint32 XAP_App::safefindFrame( XAP_Frame * f)
{
	long ff = reinterpret_cast<long>(f);
	UT_sint32 num_frames = m_vecFrames.getItemCount();
	UT_sint32 i;
	for( i = 0; i< num_frames; i++)
	{
		long lf = reinterpret_cast<long>( m_vecFrames.getNthItem(i));
		if( lf == ff) break;
	}
	if( i == num_frames ) i = -1;
	return i;
}

void XAP_App::clearLastFocussedFrame()
{
	m_lastFocussedFrame = static_cast<XAP_Frame *>(NULL);
}

XAP_Frame* XAP_App::getLastFocussedFrame() 
{
		if(m_lastFocussedFrame == static_cast<XAP_Frame *>(NULL))
			return static_cast<XAP_Frame *>(NULL);
	UT_sint32 i = safefindFrame(m_lastFocussedFrame);
	if( i>= 0)
		return m_lastFocussedFrame;
	return static_cast<XAP_Frame *>(NULL);
}

XAP_Frame * XAP_App::findValidFrame()
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
	for(i=0; (i<= NUM_MODELESSID) && (m_IdTable[i].id !=  -1); i++);
	UT_ASSERT( i <= NUM_MODELESSID );
	UT_ASSERT( m_IdTable[i].id == -1 );
	UT_ASSERT( pDialog);
	m_IdTable[i].id =  id;
	m_IdTable[i].pDialog =  pDialog;
}

void XAP_App::forgetModelessId( UT_sint32 id )
{
	// remove the id, pDialog pair from the m_IdTable

	UT_sint32 i;
	for(i=0; i <= NUM_MODELESSID && m_IdTable[i].id != id; i++) ;
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
	for(i=0; i <= NUM_MODELESSID && m_IdTable[i].id != id; i++) ;
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
            nh = strtoul(++next, &next, 10);
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

/*!
    Enumerates currently open document associated with the
    application, excluding document pointed to by pExclude

    \param v: UT_Vector into which to store the document pointers
    
    \para pExclude: pointer to a document to exclude from enumeration,
                    can be NULL (e.g., if this function is called from
                    inside a document, it might be desirable to
                    exclude that document)
*/
void XAP_App::enumerateDocuments(UT_Vector & v, const AD_Document * pExclude)
{
	UT_sint32 iIndx;

	for(UT_uint32 i = 0; i < getFrameCount(); ++i)
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
	UT_ASSERT(m_pInputModes);
	return m_pInputModes->getCurrentMap();
}

UT_sint32 XAP_App::setInputMode(const char * szName)
{
	UT_uint32 i;
	
	UT_ASSERT(m_pInputModes);
	const char * szCurrentName = m_pInputModes->getCurrentMapName();
	if (UT_stricmp(szName,szCurrentName) == 0)
		return -1;					// already set, no change required

	EV_EditEventMapper * p = m_pInputModes->getMapByName(szName);
	if (!p)
	{
		// map not previously loaded -- we need to install it first

		EV_EditBindingMap * pBindingMap = m_pApp->getBindingMap(szName);
		UT_ASSERT(pBindingMap);
		bool bResult;
		bResult = m_pInputModes->createInputMode(szName,pBindingMap);
		UT_ASSERT(bResult);
	}
	
	// note: derrived classes will need to update keyboard
	// note: and mouse after we return.

	UT_DEBUGMSG(("Setting InputMode to [%s] for the current window.\n",szName));
	
	bool bStatus = m_pInputModes->setCurrentMap(szName);
	
	// notify all the frames about the INPUTMODE change
	for (i = 0; i < getFrameCount(); i++) {
		getFrame(i)->getCurrentView()->notifyListeners(AV_CHG_INPUTMODE);
	}
	
	return (bStatus);
}

const char * XAP_App::getInputMode(void) const
{
	return m_pInputModes->getCurrentMapName();
}

GR_Graphics * XAP_App::newGraphics(UT_uint32 iClassId, GR_AllocInfo * param) const
{
	UT_return_val_if_fail(m_pGraphicsFactory, NULL);

	return m_pGraphicsFactory->newGraphics(iClassId, param);
}
void XAP_App::setDefaultGraphicsId(UT_uint32 i)
{
	if(i == GRID_UNKNOWN)
		return;
	
	m_iDefaultGraphicsId = i;

	if(i < GRID_LAST_BUILT_IN)
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

	// we frequently assume that XML_Char * is only one byte in size
	UT_ASSERT(sizeof(XML_Char) == sizeof(char));
}
#endif
