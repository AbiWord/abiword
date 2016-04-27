/*
 * Generic Plugin
 * This is intended only as a base class for other plugins.
 *
 * It is derived from AbiPaint, which is in turn derived
 *   from AbiGimp copyright 2002 Martin Sevior which in turn
 *   is based on AiksaurusABI - Abiword plugin for Aiksaurus
 *   Copyright (C) 2001 by Jared Davis
 * Also tidbits taken from ImageMagick plugin & ScriptHappy
 *   plugin, copyright 2002 by Dom Lachowicz [and others]
 * And chunks taken from AbiCommand plugin, copyright 2002
 *   by Martin Sevior
 * This generic plugin initially assembled from above pieces
 *   by Kenneth J. Davis, though I [KJD] claim no copyright
 *   on any portion of AbiWord (TM) nor related plugins.
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


#include "AbiGeneric.h"
#include "ap_Menu_Id.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_paint_register
#define abi_plugin_unregister abipgn_paint_unregister
#define abi_plugin_supports_version abipgn_paint_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("Paint")
#endif

/*
 * Preference / User modifiable settings
 * All settings are stored in a '_plugin_' scheme with corresponding plugin name
 */
XAP_Prefs * prefs = NULL;

/*
 * Abiword Plugin Interface 
 * These are implemented by this generic class,
 * any work should be done by extending appropriate methods, not implementing these.
 */
    
ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	prefs = XAP_App::getApp()->getPrefs();
	UT_ASSERT(prefs != NULL);	// This is only fatal if the plugin uses preferences

	// get info from actual plugin
	XAP_ModuleInfo * pluginInfo = getModuleInfo();	
	UT_return_val_if_fail(pluginInfo != NULL, 0);

	// copy values over to AbiWord provided structure
	UT_return_val_if_fail(mi != NULL, 0);	// somethings messed up
	mi->name = pluginInfo->name;
	mi->desc = pluginInfo->desc;
	mi->version = pluginInfo->version;
	mi->author = pluginInfo->author;
	mi->usage = pluginInfo->usage;

	// give the plugin a chance to do any other initialization
	// such as add any menu entries
	if (!doRegistration()) return 0;

	// assume we are good to go
	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	// clear module info
	UT_ASSERT(mi != NULL);	// mi should never be NULL here
	if (mi != NULL)
	{
		mi->name = NULL;
		mi->desc = NULL;
		mi->version = NULL;
		mi->author = NULL;
		mi->usage = NULL;
	}

	// let the plugin do any uninitialization
	// such as remove any menu entries added
	doUnregistration();

	// for now just return alls ok
	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, UT_uint32 /*release*/)
{
	// doesn't really matter as it will fail to load if the imports don't match.
	return 1; 
}


/*
 *   Adds [or removes] all of this plugins menu items to AbiWord
 *   amo is an array of the above structure, each element of the array defines an
 *     entry to add to the Main and/or Context menu.
 *   num_menuitems is how many entries are in the array (usually sizeof(amo)/sizeof(amo[0]))
 *   prevMM is the [English] Main menu item we place our 1st menu item after
 *   prevCM is the [English] Context menu item we place our 1st context menu item after
 *     prevMM and prevCM should not be NULL unless there is no entry added to the respective menu
 */
UT_Error addToMenus(AbiMenuOptions amo[], UT_uint32 num_menuitems, XAP_Menu_Id prevMM, XAP_Menu_Id prevCM)
{
  UT_uint32 i;

  // First we need to get a pointer to the application itself.
  XAP_App *pApp = XAP_App::getApp();
  
  // Now we need to get the EditMethod container for the application.
  // This holds a series of Edit Methods and links names to callbacks.
  EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer();

  // We need to go through and add the menu element to each "frame" 
  // of the application.  We can iterate through the frames by doing
  // XAP_App::getFrameCount() to tell us how many frames there are,
  // then calling XAP_App::getFrame(i) to get the i-th frame.
  UT_uint32 frameCount = pApp->getFrameCount();
  XAP_Menu_Factory * pFact = pApp->getMenuFactory();

  // Now we need to grab an ActionSet.  This is going to be used later
  // on in our for loop.  Take a look near the bottom.
  EV_Menu_ActionSet* pActionSet = pApp->getMenuActionSet();
  for (i = 0; i < num_menuitems; i++)
  {
      // Create an EditMethod that will link our method's name with
      // it's callback function.  This is used to link the name to 
      // the callback.
      EV_EditMethod *myEditMethod = new EV_EditMethod(
						      amo[i].methodName,  // name of callback function
						      amo[i].method,      // callback function itself.
						      0,                  // no additional data required.
						      ""                  // description -- allegedly never used for anything
						      );

      // We have to add our EditMethod to the application's EditMethodList
      // so that the application will know what callback to call when a call
      // to amo[i].methodName is received.
      pEMC->addEditMethod(myEditMethod);

	// 
	// Generate an unique id for this menu item
	// (We could also pass 0 or leave off newID and have addNewMenuAfter return one)
	//
	amo[i].id = pFact->getNewID();

	// TODO does it matter if this is before an addNewMenuAfter() call or not?
     	pFact->addNewLabel(NULL, amo[i].id, amo[i].label, amo[i].description);

      //
      // Put it in the main menu
      //
	if (amo[i].inMainMenu)
	{
		pFact->addNewMenuAfter("Main", NULL, prevMM, amo[i].flags, amo[i].id);
		prevMM = amo[i].id;
	}

      //
      // Put it in the context menu.
      //
	if (amo[i].inContextMenu)
	{
		pFact->addNewMenuAfter("ContextImageT", NULL, prevCM, amo[i].flags, amo[i].id);
		prevCM = amo[i].id;
	}

      // Create the Action that will be called.
      EV_Menu_Action* myAction = new EV_Menu_Action(
						    amo[i].id,                // id that the layout said we could use
						    amo[i].hasSubMenu,    // do we have a sub menu.
						    amo[i].hasDialog,     // do we raise a dialog (or in case a whole new program).
						    amo[i].checkBox,      // do we have a checkbox.
						    0,                    // no radio buttons for me, thank you
						    amo[i].methodName,    // name of callback function to call.
						    amo[i].pfnGetState,   // something about menu state, usually NULL
						    amo[i].pfnGetDynLabel // dynamic menu label
						    );

      // Now what we need to do is add this particular action to the ActionSet
      // of the application.  This forms the link between our new ID that we 
      // got for this particular frame with the EditMethod that knows how to 
      // call our callback function.  
      pActionSet->addAction(myAction);
  }

  // rebuild the menus
  for(i = 0; i < frameCount; i++)
  {
      // Get the current frame that we're iterating through.
      XAP_Frame* pFrame = pApp->getFrame(i);
      pFrame->rebuildMenus();
  }

  return UT_OK;
}

UT_Error removeFromMenus(const AbiMenuOptions amo[], UT_uint32 num_menuitems)
{
  UT_uint32 i;  // MSVC is going to treat it this way regardless...

  // First we need to get a pointer to the application itself.
  XAP_App *pApp = XAP_App::getApp();

  EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer() ;

  // now remove entries from the menus
  UT_uint32 frameCount = pApp->getFrameCount();
  XAP_Menu_Factory * pFact = pApp->getMenuFactory();

  for (i = 0; i < num_menuitems; i++)
  {
    // remove the edit method
    EV_EditMethod * pEM = ev_EditMethod_lookup ( amo[i].methodName ) ;
    pEMC->removeEditMethod ( pEM ) ;
    DELETEP( pEM ) ;

    // remove the contextual & main menu items
    if (amo[i].inMainMenu) pFact->removeMenuItem("Main",NULL, amo[i].id);
    if (amo[i].inContextMenu) pFact->removeMenuItem("ContextImageT", NULL, amo[i].id);
  }

  // rebuild the menus
  for(i = 0; i < frameCount; i++)
  {
      // Get the current frame that we're iterating through.
      XAP_Frame* pFrame = pApp->getFrame(i);
      pFrame->rebuildMenus();
  }

  return UT_OK;
}

/*
 * Helper functions
 *
 */

/* returns true only if user requested to cancel save, pass suggested path in */
bool getFileName(std::string &szFile, XAP_Frame * pFrame, XAP_Dialog_Id id,
                 const char **szDescList, const char **szSuffixList, int *ft)

{
	UT_ASSERT(pFrame);

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname(szFile);
	pDialog->setSuggestFilename(false);

	pDialog->setFileTypeList(szDescList, szSuffixList, ft);

	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
		szFile = pDialog->getPathname();
	else
		szFile.clear();

	pDialogFactory->releaseDialog(pDialog);

	return !bOK;
}

/* returns true only if currently an image is selected */
bool isImageSelected (void)
{
	// Get the current view that the user is in.
	XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
	FV_View* pView = static_cast<FV_View*>(pFrame->getCurrentView());

    return (pView->getSelectedImage(NULL) != 0);
}


/* locks or unlocks GUI, automatically aquires/releases lock/unlock EV_EditMethods */

// used so we don't have to query for lock/unlock calls each call nor require user init/uninit function
UT_uint32 _lockGUI_counter = 0;	// NOTE: for multithreaded support this may need to be protected

// actual methods to perform the required functionality
const EV_EditMethod * lockGUI = NULL;
const EV_EditMethod * unlockGUI = NULL;


void plugin_imp_lockGUI(EV_EditMethodCallData *d)
{
    if (!_lockGUI_counter)	// 1st call [after all unlock calls]
    {
      // Get some pointers so we can call the editMethod to lock out GUI operations
      XAP_App *pApp = XAP_App::getApp();

      // Now we need to get the EditMethod container for the application.
      // This holds a series of Edit Methods and links names to callbacks.
      EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer();

      // OK now get the methods to lock and unlock GUI operations
      lockGUI = pEMC->findEditMethodByName("lockGUI");
      unlockGUI = pEMC->findEditMethodByName("unlockGUI");
    }

    UT_ASSERT(lockGUI != NULL);
    UT_ASSERT(unlockGUI != NULL);

    // actually lock the GUI
    ev_EditMethod_invoke(lockGUI,d);

    // increment our counter
    _lockGUI_counter++;
}

void plugin_imp_unlockGUI(EV_EditMethodCallData *d)
{
    // ignore calls without a prior lock call
    if (!_lockGUI_counter) return;

    UT_ASSERT(lockGUI != NULL);
    UT_ASSERT(unlockGUI != NULL);

    // actually unlock the GUI
    ev_EditMethod_invoke(unlockGUI,d);

    // decrement out counter
    _lockGUI_counter--;

    // if we reach zero, then cleanup
    if (!_lockGUI_counter)
    {
      lockGUI = NULL;
      unlockGUI = NULL;
    }
}

