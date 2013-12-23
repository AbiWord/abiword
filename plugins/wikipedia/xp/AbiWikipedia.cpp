/*
 * 
 * Copyright (C) 2001 by Dom Lachowicz & Michael D. Pritchett
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

#include "xap_Module.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "ap_Menu_Id.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "ev_EditMethod.h"
#include "xap_Menu_Layouts.h"
#include "ut_string_class.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_wikipedia_register
#define abi_plugin_unregister abipgn_wikipedia_unregister
#define abi_plugin_supports_version abipgn_wikipedia_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE ("Wikipedia")
#endif

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

//
// _ucsToAscii
// -----------------------
//   Helper function to convert UCS-4 strings into Ascii.
//   NOTE: you must call delete[] on the returned text!!!
//
inline static char*
_ucsToAscii(const UT_UCSChar* text)
{
  UT_return_val_if_fail(text,0);

  // calculate length of text so that we can create a character
  // buffer of equal size.
  const unsigned int length = UT_UCS4_strlen(static_cast<const UT_UCS4Char *>(text));
  
  // allocate ascii characters plus room for a null terminator.    
  char* ret = new char[length+1];
  
  // do the string conversion.  this is simple we just cast to 
  // char since UCS-4 is the same as Ascii for english.    
  for(unsigned int i = 0;i < length;++i)
    {
      ret[i] = static_cast<char>(text[i]);
    }
  
  // finally null terminate the string.    
  ret[length] = '\0';
  
  // and now return it.
  return ret;    
}

//
// Wikipedia_invoke
// -------------------
//   This is the function that we actually call to invoke the on-line encyclopedia.
//   It should be called when the user selects from the context menu
//
static bool 
Wikipedia_invoke(AV_View* /*v*/, EV_EditMethodCallData * /*d*/)
{
  // Get the current view that the user is in.
  XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
  FV_View* pView = static_cast<FV_View*>(pFrame->getCurrentView());
  
  // If the user is on a word, but does not have it selected, we need
  // to go ahead and select that word so that the search/replace goes
  // correctly.
  pView->moveInsPtTo(FV_DOCPOS_EOW_MOVE);
  pView->moveInsPtTo(FV_DOCPOS_BOW);
  pView->extSelTo(FV_DOCPOS_EOW_SELECT);   
  
  // Now we will figure out what word to look up when we open our dialog.
  // http://en.wikipedia.org/w/wiki.phtml?search=war&go=Go
  UT_String url ("http://www.wikipedia.com/");
  if (!pView->isSelectionEmpty())
    {
	url += "w/wiki.phtml?search=";
      // We need to get the Ascii version of the current word.
      UT_UCS4Char *ucs4ST;
      pView->getSelectionText(*&ucs4ST);
      char* search = _ucsToAscii(
				 ucs4ST
				 );
      
      url += search;
      DELETEPV(search);
      FREEP(ucs4ST);

	url += "&go=Go";
    }

  XAP_App::getApp()->openURL( url.c_str() ); 

  return true;
}

static const char* Wikipedia_MenuLabel = "Wi&ki Encyclopedia";
static const char* Wikipedia_MenuTooltip = "Opens the libre Wiki on-line encyclopedia";

static void
Wikipedia_removeFromMenus()
{
  // First we need to get a pointer to the application itself.
  XAP_App *pApp = XAP_App::getApp();

  // remove the edit method
  EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer() ;
  EV_EditMethod * pEM = ev_EditMethod_lookup ( "Wikipedia_invoke" ) ;
  pEMC->removeEditMethod ( pEM ) ;
  DELETEP( pEM ) ;

  // now remove crap from the menus
  int frameCount = pApp->getFrameCount();
  XAP_Menu_Factory * pFact = pApp->getMenuFactory();

  pFact->removeMenuItem("Main",NULL,Wikipedia_MenuLabel);
  pFact->removeMenuItem("contextText",NULL,Wikipedia_MenuLabel);
  for(int i = 0;i < frameCount;++i)
    {
      // Get the current frame that we're iterating through.
      XAP_Frame* pFrame = pApp->getFrame(i);
      pFrame->rebuildMenus();
    }
}

static void
Wikipedia_addToMenus()
{
  // First we need to get a pointer to the application itself.
  XAP_App *pApp = XAP_App::getApp();
    
  // Create an EditMethod that will link our method's name with
  // it's callback function.  This is used to link the name to 
  // the callback.
  EV_EditMethod *myEditMethod = new EV_EditMethod(
						  "Wikipedia_invoke",  // name of callback function
						  Wikipedia_invoke,    // callback function itself.
						  0,                      // no additional data required.
						  ""                      // description -- allegedly never used for anything
						  );
  
  // Now we need to get the EditMethod container for the application.
  // This holds a series of Edit Methods and links names to callbacks.
  EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer();
  
  // We have to add our EditMethod to the application's EditMethodList
  // so that the application will know what callback to call when a call
  // to "Wikipedia_invoke" is received.
  pEMC->addEditMethod(myEditMethod);
  
  
  // Now we need to grab an ActionSet.  This is going to be used later
  // on in our for loop.  Take a look near the bottom.
  EV_Menu_ActionSet* pActionSet = pApp->getMenuActionSet();
  
  
  // We need to go through and add the menu element to each "frame" 
  // of the application.  We can iterate through the frames by doing
  // XAP_App::getFrameCount() to tell us how many frames there are,
  // then calling XAP_App::getFrame(i) to get the i-th frame.
  
  int frameCount = pApp->getFrameCount();
  XAP_Menu_Factory * pFact = pApp->getMenuFactory();

  //
  // Put it in the context menu.
  //
  XAP_Menu_Id newID = pFact->addNewMenuAfter("contextText",NULL,"Bullets and &Numbering",EV_MLF_Normal);
  pFact->addNewLabel(NULL,newID,Wikipedia_MenuLabel, Wikipedia_MenuTooltip);

  //
  // Also put it under word Wount in the main menu,
  //
  pFact->addNewMenuAfter("Main",NULL,"&Word Count",EV_MLF_Normal,newID);
  
  // Create the Action that will be called.
  EV_Menu_Action* myAction = new EV_Menu_Action(
						newID,                     // id that the layout said we could use
						0,                      // no, we don't have a sub menu.
						0,                      // no, we don't raise a dialog.
						0,                      // no, we don't have a checkbox.
						0,
						"Wikipedia_invoke",  // name of callback function to call.
						NULL,                   // don't know/care what this is for
						NULL                    // don't know/care what this is for
						);
  
  // Now what we need to do is add this particular action to the ActionSet
  // of the application.  This forms the link between our new ID that we 
  // got for this particular frame with the EditMethod that knows how to 
  // call our callback function.  
  
  pActionSet->addAction(myAction);
  
  for(int i = 0;i < frameCount;++i)
    {
      // Get the current frame that we're iterating through.
      XAP_Frame* pFrame = pApp->getFrame(i);
      pFrame->rebuildMenus();
    }
}

// -----------------------------------------------------------------------
//
//      Abiword Plugin Interface 
//
// -----------------------------------------------------------------------

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
    mi->name = "Wikipedia plugin";
    mi->desc = "On-line Encyclopedia support for AbiWord. Search site is http://www.wikipedia.com/";
    mi->version = ABI_VERSION_STRING;
    mi->author = "Francis James Franklin"; // and Michael D. Pritchett and Dom Lachowicz
    mi->usage = "No Usage";
    
    // Add the dictionary to AbiWord's menus.
    Wikipedia_addToMenus();
    
    return 1;
}


ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
    mi->name = 0;
    mi->desc = 0;
    mi->version = 0;
    mi->author = 0;
    mi->usage = 0;

    Wikipedia_removeFromMenus();

    return 1;
}


ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, UT_uint32 /*release*/)
{
    return 1; 
}
