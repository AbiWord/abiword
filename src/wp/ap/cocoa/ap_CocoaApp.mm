/* -*-c++-*-
 * AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <popt.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_PerlBindings.h"
#include "ut_Script.h"

#include "xap_Args.h"
#include "ap_Args.h"
#include "ap_Convert.h"
#include "ap_CocoaFrame.h"
#include "ap_CocoaApp.h"
#include "spell_manager.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "ap_CocoaClipboard.h"
#include "ap_CocoaPrefs.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Dialog_Id.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Menu_Layouts.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_Toolbar_Layouts.h"
#include "xav_View.h"

#include "ev_EditMethod.h"
#include "gr_Graphics.h"
#include "gr_CocoaGraphics.h"
#include "gr_CocoaImage.h"
#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_debugmsg.h"

#include "fv_View.h"
#include "fp_Run.h"

#include "ut_string_class.h"
#include "xap_EncodingManager.h"

#include "ie_impexp_Register.h"

#include "ie_exp.h"
#include "ie_exp_RTF.h"
#include "ie_exp_Text.h"

#include "ie_imp.h"
#include "ie_imp_RTF.h"
#include "ie_imp_Text.h"

#include "xap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"
#include "gr_Image.h"

#include "xap_Module.h"
#include "xap_ModuleManager.h"
//#include "xap_CocoaPSGraphics.h"


// quick hack - this is defined in ap_EditMethods.cpp
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);

/*****************************************************************/

/*!
  Construct an AP_CocoaApp.
  /param pArgs Arguments from command line
  /param szAppName A string representing the name of the app.
	Currently always AbiWord (I think).
*/
AP_CocoaApp::AP_CocoaApp(XAP_Args * pArgs, const char * szAppName)
    : AP_App(pArgs,szAppName),
	  m_pStringSet(0),
	  m_pClipboard(0),
	  m_bHasSelection(false),
	  m_bSelectionInFlux(false),
	  m_pViewSelection(0),
	  m_cacheSelectionView(0),
	  m_pFrameSelection(0)
{
}

/*!
  Destructor for AP_CocoaApp's.  Cleans up spellcheck, clipboard, and
	StringSet.  
*/
AP_CocoaApp::~AP_CocoaApp(void)
{
    DELETEP(m_pStringSet);
    DELETEP(m_pClipboard);

    IE_ImpExp_UnRegisterXP ();
}

/*!
  Creates a directory if the specified one does not yet exist.
  /param A character string representing the to-be-created directory. 
  /return True, if the directory already existed, or was successfully
	created.  False, if the input path was already a file, not a
	directory, or if the directory was unable to be created.
  /todo Do domething with error status if the directory couldn't be
	created? 
*/
static bool s_createDirectoryIfNecessary(const char * szDir)
{
    struct stat statbuf;
    
    if (stat(szDir,&statbuf) == 0)								// if it exists
    {
		if (S_ISDIR(statbuf.st_mode))							// and is a directory
			return true;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return false;
    }
    
    if (mkdir(szDir,0700) == 0)
		return true;
    
    
    UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
    return false;
}	

/*!
  Initialize the application.  This involves preferences, keybindings,
  toolbars, graphics, spelling and everything else.  
  \return True if successfully initalized, False otherwise. if false
  the app is unusable, and loading should not continue.   
  \bug This function is 136 lines - way too long.  Needs to be
  refactored, to use a buzzword.  
*/
bool AP_CocoaApp::initialize(void)
{
    const char * szUserPrivateDirectory = getUserPrivateDirectory();
    bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
    UT_ASSERT(bVerified);
	
    // load the preferences.
    
    m_prefs = new AP_CocoaPrefs(this);
    m_prefs->fullInit();

    // now that preferences are established, let the xap init
		   
    m_pClipboard = new AP_CocoaClipboard();
    UT_ASSERT(m_pClipboard);
 //   m_pClipboard->initialize();
    
    m_pEMC = AP_GetEditMethods();
    UT_ASSERT(m_pEMC);
    
    m_pBindingSet = new AP_BindingSet(m_pEMC);
    UT_ASSERT(m_pBindingSet);
	
    m_pMenuActionSet = AP_CreateMenuActionSet();
    UT_ASSERT(m_pMenuActionSet);
    
    m_pToolbarActionSet = AP_CreateToolbarActionSet();
    UT_ASSERT(m_pToolbarActionSet);
    
    if (! XAP_CocoaApp::initialize())
		return false;

	//////////////////////////////////////////////////////////////////
	// Initialize the importers/exporters
	//////////////////////////////////////////////////////////////////
	IE_ImpExp_RegisterXP ();
    
    //////////////////////////////////////////////////////////////////
    // load the dialog and message box strings
    //////////////////////////////////////////////////////////////////
	
    {
		// assume we will be using the builtin set (either as the main
		// set or as the fallback set).
	    
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,(XML_Char*)AP_PREF_DEFAULT_StringSet);
		UT_ASSERT(pBuiltinStringSet);
		m_pStringSet = pBuiltinStringSet;
	    
		// see if we should load an alternative set from the disk
	    
		const char * szDirectory = NULL;
		const char * szStringSet = NULL;
	    
		if (   (getPrefsValue(AP_PREF_KEY_StringSet,
							  (const XML_Char**)&szStringSet))
			   && (szStringSet)
			   && (*szStringSet)
			   && (strcmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
			getPrefsValueDirectory(true,
								   (const XML_Char*)AP_PREF_KEY_StringSetDirectory,
								   (const XML_Char**)&szDirectory);
			UT_ASSERT((szDirectory) && (*szDirectory));

			UT_String szPathname = szDirectory;
			if (szDirectory[szPathname.size()-1]!='/')
				szPathname += "/";
			szPathname += szStringSet;
			szPathname += ".strings";
		
			AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);
			UT_ASSERT(pDiskStringSet);
		
			if (pDiskStringSet->loadStringsFromDisk(szPathname.c_str()))
			{
				pDiskStringSet->setFallbackStringSet(m_pStringSet);
				m_pStringSet = pDiskStringSet;
				UT_DEBUGMSG(("Using StringSet [%s]\n",szPathname.c_str()));
			}
			else
			{
				DELETEP(pDiskStringSet);
				UT_DEBUGMSG(("Unable to load StringSet [%s] -- using builtin strings instead.\n",szPathname.c_str()));
			}
		}
    }
	
    // Now we have the strings loaded we can populate the field names correctly
    int i;
	
    for (i = 0; fp_FieldTypes[i].m_Type != FPFIELDTYPE_END; i++)
    {
		(&fp_FieldTypes[i])->m_Desc = m_pStringSet->getValue(fp_FieldTypes[i].m_DescId);
		UT_DEBUGMSG(("Setting field type desc for type %d, desc=%s\n", fp_FieldTypes[i].m_Type, fp_FieldTypes[i].m_Desc));
    }

    for (i = 0; fp_FieldFmts[i].m_Tag != NULL; i++)
    {
		(&fp_FieldFmts[i])->m_Desc = m_pStringSet->getValue(fp_FieldFmts[i].m_DescId);
		UT_DEBUGMSG(("Setting field desc for field %s, desc=%s\n", fp_FieldFmts[i].m_Tag, fp_FieldFmts[i].m_Desc));
    }

    ///////////////////////////////////////////////////////////////////////
    /// Build a labelset so the plugins can add themselves to something ///
    ///////////////////////////////////////////////////////////////////////

	const char * szMenuLabelSetName = NULL;
	if (getPrefsValue( AP_PREF_KEY_MenuLabelSet, (const XML_Char**)&szMenuLabelSetName)
		&& (szMenuLabelSetName) && (*szMenuLabelSetName))
	{
		;
	}
	else
		szMenuLabelSetName = AP_PREF_DEFAULT_MenuLabelSet ;

	getMenuFactory()->buildMenuLabelSet(szMenuLabelSetName);
	bool bLoadPlugins = true;
	bool bFound = getPrefsValueBool(XAP_PREF_KEY_AutoLoadPlugins,&bLoadPlugins);
	if(bLoadPlugins || !bFound)
	{
		loadAllPlugins();
	}
    //////////////////////////////////////////////////////////////////

#ifdef ABI_OPT_PERL
    // hack to keep the perl bindings working on unix
    UT_ScriptLibrary& instance = UT_ScriptLibrary::instance(); 
    instance.registerScript ( new UT_PerlScriptSniffer () );
#endif

    return true;
}

/*!
  Create a new frame based on the current one.  
  \return A pointer to the new frame.  
*/
XAP_Frame * AP_CocoaApp::newFrame(void)
{
    AP_CocoaFrame * pCocoaFrame = new AP_CocoaFrame(this);

    if (pCocoaFrame)
		pCocoaFrame->initialize();

    return pCocoaFrame;
}

/*!
  If the user has set the preferences to save them selves on shutdown,
	save them.  
  \return This function always returns true.  
  \todo The return value should be fixed to check the return values of
	the functions it calls, and potentially handle errors.  At a
	minimum, it should return false on errors.  
*/
bool AP_CocoaApp::shutdown(void)
{
//
// Save our toolbars if we're customizable
//
	if(areToolbarsCustomizable())
	{
		m_pToolbarFactory->saveToolbarsInCurrentScheme();
	}
    if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

    return true;
}

/*!
  This function returns the absolute path to the directory specified
  in the desired key.  In other words, if you want the path to the
  spelling files, you can't just get the prefs value, since that path
  isn't always absolute.  This function gives it to you in absolute
  terms.  
  \param bAppSpecific Is this key specific to the app, or is it
  general to AbiSuite?
  \param szKey A string of XML_Chars representing the desired key
  \param pszValue pointer for the value to be returned in. 
  \return True if successful, false otherwise.  
  \todo support meaningful return values.
*/
bool AP_CocoaApp::getPrefsValueDirectory(bool bAppSpecific,
										const XML_Char * szKey, const XML_Char ** pszValue) const
{
    if (!m_prefs)
		return false;

    const XML_Char * psz = NULL;
    if (!m_prefs->getPrefsValue(szKey,&psz))
		return false;

    if (*psz == '/')
    {
		*pszValue = psz;
		return true;
    }

    const XML_Char * dir = ((bAppSpecific) ? getAbiSuiteAppDir() : getAbiSuiteLibDir());

    static XML_Char buf[1024];
    UT_ASSERT((strlen(dir) + strlen(psz) + 2) < sizeof(buf));
	
    sprintf(buf,"%s/%s",dir,psz);
    *pszValue = buf;
    return true;
}

/*!
  This returns the AbiSuite application directory.
  \return A const string containting the directory path
*/
const char * AP_CocoaApp::getAbiSuiteAppDir(void) const
{
    // we return a static string, use it quickly.
	
    static XML_Char buf[1024];
    UT_ASSERT((strlen(getAbiSuiteLibDir()) + strlen(ABIWORD_APP_LIBDIR) + 2) < sizeof(buf));

    sprintf(buf,"%s/%s",getAbiSuiteLibDir(),ABIWORD_APP_LIBDIR);
    return buf;
}

/*!
  This returns the current StringSet
  \return A const pointer to the StringSet
  \todo This function should be inilined.  
*/
const XAP_StringSet * AP_CocoaApp::getStringSet(void) const
{
    return m_pStringSet;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

/*!
  copy the given subset of the given document to the
  system clipboard in a variety of formats.

  to minimize the effects of race-conditions, we create
  all of the buffers we need and then post them to the
  server (well sorta) all at one time.
  \param pDocRange a range of the document to be copied
*/
void AP_CocoaApp::copyToClipboard(PD_DocumentRange * pDocRange)
{

    UT_ByteBuf bufRTF;
    UT_ByteBuf bufTEXT;

	// create RTF buffer to put on the clipboard
		
    IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
    if (pExpRtf)
    {
		pExpRtf->copyToBuffer(pDocRange,&bufRTF);
		DELETEP(pExpRtf);
		UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in RTF format.\n",bufRTF.getLength()));
    }

    // create raw 8bit text buffer to put on the clipboard
		
    IE_Exp_Text * pExpText = new IE_Exp_Text(pDocRange->m_pDoc);
    if (pExpText)
    {
		pExpText->copyToBuffer(pDocRange,&bufTEXT);
		DELETEP(pExpText);
		UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in TEXTPLAIN format.\n",bufTEXT.getLength()));
    }

    // NOTE: this clearData() will actually release our ownership of
    // NOTE: the CLIPBOARD property in addition to clearing any
    // NOTE: stored buffers.  I'm omitting it since we seem to get
    // NOTE: clr callback after we have done some other processing
    // NOTE: (like adding the new stuff).
    // m_pClipboard->clearData(true,false);
	
    if (bufRTF.getLength() > 0)
		m_pClipboard->addData(AP_CocoaClipboard::AP_CLIPBOARD_RTF,(UT_Byte *)bufRTF.getPointer(0),bufRTF.getLength());
    if (bufTEXT.getLength() > 0)
		m_pClipboard->addData(AP_CocoaClipboard::AP_CLIPBOARD_TEXTPLAIN_8BIT,(UT_Byte *)bufTEXT.getPointer(0),bufTEXT.getLength());

    return;
}

/*
  I've reordered AP_CLIPBOARD_STRING and AP_CLIPBOARD_TEXTPLAIN_8BIT
  since for non-Latin1 text the data in AP_CLIPBOARD_TEXTPLAIN_8BIT
  format has name of encoding as prefix, and AP_CLIPBOARD_STRING
  doesn't - hvv.
*/
static const char * aszFormatsAccepted[] = { AP_CocoaClipboard::AP_CLIPBOARD_RTF,
											 AP_CocoaClipboard::AP_CLIPBOARD_STRING,
											 AP_CocoaClipboard::AP_CLIPBOARD_TEXTPLAIN_8BIT,
											 0 /* must be last */ };
static const char * txtszFormatsAccepted[] = { AP_CocoaClipboard::AP_CLIPBOARD_STRING,
											   AP_CocoaClipboard::AP_CLIPBOARD_TEXTPLAIN_8BIT,
											   0 };

/*!
  paste from the system clipboard using the best-for-us format
  that is present.  try to get the content in the order listed.
  
  \todo currently i have this set so that a ^v or Menu[Edit/Paste] will
  use the CLIPBOARD property and a MiddleMouseClick will use the
  PRIMARY property -- this seems to be the "X11 way" (sigh).
  consider having a preferences switch to allow ^v and Menu[Edit/Paste]
  to use the most recent property... this might be a nice way of
  unifying things -- or it might not -- this is probably an area
  for investigation or some usability testing.
*/
void AP_CocoaApp::pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard,
									bool bHonorFormatting)
{
//    XAP_CocoaClipboard::T_AllowGet tFrom = ((bUseClipboard)
//										   ? XAP_CocoaClipboard::TAG_ClipboardOnly
//										   : XAP_CocoaClipboard::TAG_PrimaryOnly);

    const char * szFormatFound = NULL;
    unsigned char * pData = NULL;
    UT_uint32 iLen = 0;

    bool bFoundOne = m_pClipboard->getClipboardData(aszFormatsAccepted,(void**)&pData,&iLen,&szFormatFound);
    if (!bFoundOne)
    {
		UT_DEBUGMSG(("PasteFromClipboard: did not find anything to paste.\n"));
		return;
    }
	
    if (strcmp(szFormatFound,AP_CocoaClipboard::AP_CLIPBOARD_RTF) == 0 && bHonorFormatting)
    {
		iLen = MyMin(iLen,strlen((const char *)pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in format [%s].\n",iLen,szFormatFound));

		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);

		return;
    }

	bFoundOne = m_pClipboard->getClipboardData(/*tFrom,*/txtszFormatsAccepted,(void**)&pData,&iLen,&szFormatFound);	

	if (!bFoundOne)
	{
		UT_DEBUGMSG(("PasteFromClipboard: did not find anything to paste.\n"));
		return;
	}

    if (   (strcmp(szFormatFound,AP_CocoaClipboard::AP_CLIPBOARD_TEXTPLAIN_8BIT) == 0)
		   || (strcmp(szFormatFound,AP_CocoaClipboard::AP_CLIPBOARD_STRING) == 0))
    {
		iLen = MyMin(iLen,strlen((const char *)pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in format [%s].\n",iLen,szFormatFound));

		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc);
		pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);

		return;
    }

    return;
}

/*!
  Theoretically, this should determine if we can paste from the
  cliboard.  However, it isn't really possible to get this info, since
  X has a weird clipboard model.  So we always return true.  If we
  return true with no data on the clipboard, then the paste just ends
  up being empty.  As far as we can tell, this has no bad effects.  
  \bug Is this really right?  It seems like a hack.  
  \return Always true. 
*/
bool AP_CocoaApp::canPasteFromClipboard(void)
{
#if 0
    const char * szFormatFound = NULL;
    unsigned char * pData = NULL;
    UT_uint32 iLen = 0;

    XAP_CocoaClipboard::T_AllowGet tFrom = XAP_CocoaClipboard::TAG_ClipboardOnly;

    // first, try to see if we can paste from the clipboard
    bool bFoundOne = m_pClipboard->getData(tFrom,aszFormatsAccepted,(void**)&pData,&iLen,&szFormatFound);
    if (bFoundOne)
		return true;

    // didn't work, try out the primary selection
    tFrom = XAP_CocoaClipboard::TAG_PrimaryOnly;
    bFoundOne = m_pClipboard->getData(tFrom,aszFormatsAccepted,(void**)&pData,&iLen,&szFormatFound);
    return bFoundOne;
#else
    return true;
#endif
}

/* return > 0 for directory entries ending in ".Abi"
 */
static int s_Abi_only (struct dirent * d)
{
  const char * name = d->d_name;

  if (name)
    {
      int length = strlen (name);

      if (length >= 4)
	if (strcmp (name + length - 4, ".Abi") == 0)
	  return 1;
    }
  return 0;
}

/* return true if dirname exists and is a directory; symlinks probably not counted
 */
static bool s_dir_exists (const char * dirname)
{
  struct stat sbuf;

  if (stat (dirname, &sbuf) == 0)
    if ((sbuf.st_mode & S_IFMT) == S_IFDIR)
      return true;

  return false;
}

/* MacOSX applications look for plugins in Contents/Plug-ins, and there's probably
 * no need to jump through scandir hoops identifying these. Third party plugins or
 * plugins not distributed with AbiWord.app can be found in the system directory
 * "/Library/Application Support" or in the user's home equivalent - I'm choosing
 * to make the plug-in directory "/Library/Application Support/AbiWord/Plug-ins".
 * 
 * The System Overview recommends that plugins be given a suffix that the
 * application claims (presumably as a document type) so I'm opting for the suffix
 * ".Abi".
 */
void AP_CocoaApp::loadAllPlugins ()
{
  /* 1. TODO: Load from AbiWord.app/Contents/Plug-ins
   */
  NSString * app_path = [[NSBundle mainBundle] bundlePath];
  if (app_path)
    {
      NSString * plugin_path = [app_path stringByAppendingString:@"/Contents/Plug-ins"];
      UT_DEBUGMSG(("FJF: path to bundle's plug-ins: %s\n",[plugin_path lossyCString]));
    }

  /* 2. Load from:
   *  a. "/Library/Application Support/AbiSuite/Plug-ins"
   *  b. "$HOME/Library/Application Support/AbiSuite/Plug-ins"
   */
  int support_dir_count = 0;

  UT_String support_dir[2];

  support_dir[0] = "/Library/Application Support/AbiSuite/Plug-ins";
  if (!s_dir_exists (support_dir[0].c_str()))
    {
      UT_DEBUGMSG(("FJF: %s: no such directory\n",support_dir[0].c_str()));
    }
  else
    {
      UT_DEBUGMSG(("FJF: adding to plug-in search path: %s\n",support_dir[0].c_str()));
      support_dir_count++;
    }

  const char * homedir = getUserPrivateDirectory ();
  if (homedir == 0)
    {
      UT_DEBUGMSG(("FJF: no home directory?\n"));
    }
  else if (!s_dir_exists (homedir))
    {
      UT_DEBUGMSG(("FJF: invalid home directory?\n"));
    }
  else
    {
      UT_String plugin_dir(homedir);
      plugin_dir += "/Plug-ins";
      if (!s_dir_exists (plugin_dir.c_str()))
	{
	  UT_DEBUGMSG(("FJF: %s: no such directory\n",plugin_dir.c_str()));
	}
      else
	{
	  UT_DEBUGMSG(("FJF: adding to plug-in search path: %s\n",plugin_dir.c_str()));
	  support_dir[support_dir_count++] = plugin_dir;
	}
    }

  for (int i = 0; i < support_dir_count; i++)
    {
      struct dirent ** namelist = 0;
      int n = scandir (support_dir[i].c_str(), &namelist, s_Abi_only, alphasort);
      UT_DEBUGMSG(("DOM: found %d plug-ins in %s\n", n, support_dir[i].c_str()));
      if (n == 0) continue;

      UT_String plugin_path;
      while (n--)
	{
	  plugin_path  = support_dir[i];
	  plugin_path += '/';
	  plugin_path += namelist[n]->d_name;

	  UT_DEBUGMSG(("DOM: loading plugin %s\n", plugin_path.c_str()));
	  if (XAP_ModuleManager::instance().loadModule (plugin_path.c_str()))
	    {
	      UT_DEBUGMSG(("DOM: loaded plug-in: %s\n", namelist[n]->d_name));
	    }
	  else
	    {
	      UT_DEBUGMSG(("DOM: didn't load plug-in: %s\n", namelist[n]->d_name));
	    }

	  free (namelist[n]);
	}
      free (namelist);
    }

  /* SPI modules don't register automatically on loading, so
   * now that we've loaded the modules we need to register them:
   */
  XAP_ModuleManager::instance().registerPending ();
}

/*****************************************************************/
/*****************************************************************/

/*!
 this is called by the view-listeners when the state
 of the X Selection is changed by the user on one of
 our windows.

 we need to notify the clipboard so that it can assert
 or release the X Selection.

 we remember the last view that called us so that
 clearSelection() can do it's job when another application
 asserts the X Selection.

 \param pView The view to be changed.
*/
void AP_CocoaApp::setSelectionStatus(AV_View * pView)
{

    if (m_bSelectionInFlux)
		return;
    m_bSelectionInFlux = true;

    bool bSelectionStateInThisView = ( ! pView->isSelectionEmpty() );
	
    if (m_pViewSelection && m_pFrameSelection && m_bHasSelection && (pView != m_pViewSelection))
    {
		// one window has a selection currently and another window just
		// asserted one.  we force clear the old one to enforce the X11
		// style.

		((FV_View *)m_pViewSelection)->cmdUnselectSelection();
    }

    // now fill in all of our variables for this window
    // and notify the XServer that we have/don't have a
    // selection.  if we were asked to clear the selection
    // and we match the cached value (because of the warp
    // effect on a X11 style middle mouse), we don't actually
    // tell the XServer of the release (so that the paste
    // that will immediately follow will short circut to us
    // rather than going to the server).


    if (bSelectionStateInThisView)
    {
		m_bHasSelection = bSelectionStateInThisView;
//		m_pClipboard->assertSelection();
    }
    else if (pView == m_cacheSelectionView)
    {
		// if we are going to use the cache, we do not
		// clear m_bHasSelection now.  rather, we defer
		// this and the server notification until afterwards.
		
		UT_ASSERT(m_bHasSelection);
		m_cacheDeferClear = true;
    }
    else
    {
		m_bHasSelection = bSelectionStateInThisView;
		m_pClipboard->clearClipboard();
    }
	
    xxx_UT_DEBUGMSG(("here we go whooooo\n"));
    setViewSelection(pView);
    m_pFrameSelection = (XAP_Frame *)pView->getParentData();

    m_bSelectionInFlux = false;
    return;
}
#if 0
void    AP_CocoaApp::setViewSelection( AV_View * pView)
{
    m_pViewSelection = pView;
}

AV_View* AP_CocoaApp::getViewSelection(void)
{
    return m_pViewSelection;
}
#endif
/*!
  we intercept this so that we can erase our
  selection-related variables if necessary.
  wouldn't want to hold onto a stale frame or
  view pointer of a closed window when the
  selection is changed....
  
  /param pFrame The frame to be forgotten
  /return The return value of the XAP_App::forgetFrame call
  /sa XAP_App::forgetFrame()
*/

bool AP_CocoaApp::forgetFrame(XAP_Frame * pFrame)
{


    if (m_pFrameSelection && (pFrame==m_pFrameSelection))
    {
		m_pClipboard->clearClipboard();
		m_pFrameSelection = NULL;
		m_pViewSelection = NULL;
    }
	
    return XAP_App::forgetFrame(pFrame);
}

/*!
  this method goes with setSelectionStatus().
  
  we are called by the clipboard (thru the callback chain)
  in response to another application stealing the X Selection.
  
  we need to notify the view so that it can clear the screen
  as is the custom on X -- only one selection at any time.
  
  we have to watch out here because when we call up to clear
  the selection, the view will notify the view-listeners of
  the change, which may cause setSelectionStatus() to get
  called and thus update the clipboard -- this could recurse
  a while....  
*/
void AP_CocoaApp::clearSelection(void)
{
    if (m_bSelectionInFlux)
		return;
    m_bSelectionInFlux = true;
	
    if (m_pViewSelection && m_pFrameSelection && m_bHasSelection)
    {
		UT_DEBUGMSG(("crash2\n"));
		FV_View *pView = static_cast<FV_View *>(m_pViewSelection);
		pView->cmdUnselectSelection();
		m_bHasSelection = false;
    }
	
    m_bSelectionInFlux = false;
    return;
}

void AP_CocoaApp::cacheCurrentSelection(AV_View * pView)
{
    if (pView)
    {
		// remember a temporary copy of the extent of the current
		// selection in the given view.  this is intended for the
		// X11 middle mouse trick -- where we need to warp to a
		// new location and paste the current selection (not the
		// clipboard) and the act of warping clears the selection.
		
		// TODO if we ever support multiple view types, we'll have to
		// TODO change this.
		FV_View * pFVView = static_cast<FV_View *>(pView);
		pFVView->getDocumentRangeOfCurrentSelection(&m_cacheDocumentRangeOfSelection);

		m_cacheSelectionView = pView;
		UT_DEBUGMSG(("Clipboard::cacheCurrentSelection: [view %p][range %d %d]\n",
					 pFVView,
					 m_cacheDocumentRangeOfSelection.m_pos1,
					 m_cacheDocumentRangeOfSelection.m_pos2));
		m_cacheDeferClear = false;
    }
    else
    {
		if (m_cacheDeferClear)
		{
			m_cacheDeferClear = false;
			m_bHasSelection = false;
			m_pClipboard->clearClipboard();
		}
		m_cacheSelectionView = NULL;
    }

    return;
}

/*! 
  get the current contents of the selection in the
  window last known to have a selection using one
  of the formats in the given list.

  \param formatList the list of acceptable formats
  \param ppData
  \param pLen a pointer to an integer representing the length
  \param pszFormatFound a pointer for the data to be returned in
  \return True if successful, false otherwise.
*/
bool AP_CocoaApp::getCurrentSelection(const char** formatList,
									 void ** ppData, UT_uint32 * pLen,
									 const char **pszFormatFound)
{
    int j;
	
    *ppData = NULL;				// assume failure
    *pLen = 0;
    *pszFormatFound = NULL;
	
    if (!m_pViewSelection || !m_pFrameSelection || !m_bHasSelection)
		return false;		// can't do it, give up.

    PD_DocumentRange dr;

    if (m_cacheSelectionView == m_pViewSelection)
    {
		dr = m_cacheDocumentRangeOfSelection;
		UT_DEBUGMSG(("Clipboard::getCurrentSelection: *using cached values* [range %d %d]\n",dr.m_pos1,dr.m_pos2));
    }
    else
    {
		// TODO if we ever support multiple view types, we'll have to
		// TODO change this.
		FV_View * pFVView = static_cast<FV_View *>(m_pViewSelection);
	
		pFVView->getDocumentRangeOfCurrentSelection(&dr);
		UT_DEBUGMSG(("Clipboard::getCurrentSelection: [view %p][range %d %d]\n",pFVView,dr.m_pos1,dr.m_pos2));
    }
	
    m_selectionByteBuf.truncate(0);

    for (j=0; (formatList[j]); j++)
    {
		UT_DEBUGMSG(("Clipboard::getCurrentSelection: considering format [%s]\n",formatList[j]));

		if (strcmp(formatList[j],AP_CocoaClipboard::AP_CLIPBOARD_RTF) == 0)
		{
			IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(dr.m_pDoc);
			if (!pExpRtf)
				return false;		// give up on memory errors

			pExpRtf->copyToBuffer(&dr,&m_selectionByteBuf);
			DELETEP(pExpRtf);
			goto ReturnThisBuffer;
		}
			
		if (   (strcmp(formatList[j],AP_CocoaClipboard::AP_CLIPBOARD_TEXTPLAIN_8BIT) == 0)
			   || (strcmp(formatList[j],AP_CocoaClipboard::AP_CLIPBOARD_STRING) == 0))
		{
			IE_Exp_Text * pExpText = new IE_Exp_Text(dr.m_pDoc);
			if (!pExpText)
				return false;

			pExpText->copyToBuffer(&dr,&m_selectionByteBuf);
			DELETEP(pExpText);
			goto ReturnThisBuffer;
		}

		// TODO add other formats as necessary
    }

    UT_DEBUGMSG(("Clipboard::getCurrentSelection: cannot create anything in one of requested formats.\n"));
    return false;

 ReturnThisBuffer:
    UT_DEBUGMSG(("Clipboard::getCurrentSelection: copying %d bytes in format [%s].\n",
				 m_selectionByteBuf.getLength(),formatList[j]));
    *ppData = (void *)m_selectionByteBuf.getPointer(0);
    *pLen = m_selectionByteBuf.getLength();
    *pszFormatFound = formatList[j];
    return true;
}

/*****************************************************************/
/*****************************************************************/

static AP_SplashController * splash = nil;

#if 0 // SPLASH
static GR_Image * pSplashImage = NULL;
static GR_CocoaGraphics * pCocoaGraphics = NULL;
static bool firstExpose = FALSE;
static UT_uint32 splashTimeoutValue = 0;

static guint death_timeout_handler = 0;

/*!
  Hide the splash image.
  \todo Shouldn't this return a bool?
  \todo Should we check the return values from the GTK calls?
  \return Always returns TRUE (1)
*/
static gint s_hideSplash(gpointer /*data*/)
{
    if (wSplash)
    {
		gtk_timeout_remove(death_timeout_handler);
		gtk_widget_destroy(wSplash);
		wSplash = NULL;
		DELETEP(pCocoaGraphics);
		DELETEP(pSplashImage);
    }
    return TRUE;
}

#if 0 //UNIX
/*!
  Calls s_hideSplash().  Used for button clicks before the regular app
  starts.  This only gets called if the button click is on the splash
  screen.  
*/
static void s_button_event(GtkWidget * /*window*/)
{
    s_hideSplash(NULL);
}
#endif

#if 0 //UNIX
/*!
  Actually draws the splash screen.  
  \return Always returns FALSE.
  \bug When boolean functions return meaningless values, they should
	return TRUE, not FALSE.  
*/
static gint s_drawingarea_expose(GtkWidget * /* widget */,
								 GdkEventExpose * /* pExposeEvent */)
{
    if (pCocoaGraphics && pSplashImage)
    {
		pCocoaGraphics->drawImage(pSplashImage, 0, 0);

		// on the first full paint of the image, start a 2 second timer
		if (!firstExpose)
		{
			firstExpose = true;
			// kill the window after splashTimeoutValue ms
			death_timeout_handler = gtk_timeout_add(splashTimeoutValue, s_hideSplash, NULL);
		}
    }

    return FALSE;
}
#endif

#endif // SPLASH

#ifdef DEBUG // This code is only used by ifdef'ed code in _showSplash
const char * _getUserPrivateDirectory(void)
{
	/* return a pointer to a static buffer */
	
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

	char * szAbiDir = ".AbiSuite";
	
	static char buf[PATH_MAX];
	memset(buf,0,sizeof(buf));
	
	char * szHome = getenv("HOME");
	if (!szHome || !*szHome)
		szHome = "./";
	
	if (strlen(szHome)+strlen(szAbiDir)+2 >= PATH_MAX)
		return NULL;
	
	strcpy(buf,szHome);
	if (buf[strlen(buf)-1] != '/')
		strcat(buf,"/");
	strcat(buf,szAbiDir);
	return buf;
}
#endif

/*! 
  szFile is optional; a NULL pointer will use the default splash screen.
  \param delay How long the splash should stay on screen in milliseconds.
  \return Pointer to the splash image.  
*/
GR_Image * AP_CocoaApp::_showSplash(UT_uint32 delay)
{
	UT_DEBUGMSG (("AP_CocoaApp::_showSplash\n"));

//    pSplashImage = NULL;

    UT_ByteBuf* pBB = NULL;

    /*
      What this does is look in the the user's private AbiSuite
      directory (usually ~/.AbiSuite) for the file splash.png and
      use that instead of the standard splash image on startup.
      The only use for this that I can see would be testing new
      images.  

rms:  I'm adding something here to get a localized splash screen
    */

#ifdef DEBUG
    const char * szDirectory = _getUserPrivateDirectory();

    const char * szFile = "splash.png";
    //const char * szFile = "splash"; // remove .png so I can add lang if exists

    //const char * sLANG = getenv("LANG");
    const char * sLANG = NULL;
    const short  iLANGsize = sLANG ? strlen(sLANG) : 0;

    // rms: I got tired of reading/writing this more than once
    const unsigned short iSplashPathSize = strlen(szDirectory) +
    					   strlen(szFile) + iLANGsize + 4 + 2;
    char * buf;

    if (iSplashPathSize >= PATH_MAX)
		buf = NULL;
    else
    {
		buf = (char *)malloc(iSplashPathSize);
		memset(buf,0,sizeof(buf));
		/* rms: I think the following LOC equals the next 5
		      remember that having multiple / in a path equals
		      having just one. So this is faster.
		*/
		snprintf(buf, iSplashPathSize, "%s/%s", szDirectory, szFile);
		/*
		strcpy(buf,szDirectory);
		int len = strlen(buf);
		if ( (len == 0) || (buf[len-1] != '/') )
			strcat(buf,"/");
		strcat(buf,szFile);
		*/
    }
#endif
		
    // store value for use by the expose event, which attaches the timer
//    splashTimeoutValue = delay;
	
    extern unsigned char g_pngSplash[];		// see ap_wp_Splash.cpp
    extern unsigned long g_pngSplash_sizeof;	// see ap_wp_Splash.cpp
	
    pBB = new UT_ByteBuf();
    if (
#ifdef DEBUG
		(pBB->insertFromFile(0, buf)) || 
#endif
		(pBB->ins(0, g_pngSplash, g_pngSplash_sizeof))
		)
    {
		// get splash size
		UT_sint32 iSplashWidth;
		UT_sint32 iSplashHeight;
		{
			bool pngReturnVal = UT_PNG_getDimensions(pBB, iSplashWidth, iSplashHeight);
			if(!pngReturnVal)
			{
				// uh oh
				UT_DEBUGMSG(("That image caused an error in the PNG library."));
				return NULL;
			}
		}
		splash = [[AP_SplashController alloc] initWithWindowNibName:@"ap_CocoaSplash"];
		NSWindow * theWindow = [splash window];
		UT_ASSERT (theWindow);
		NSSize size;
		size.width = iSplashWidth;
		size.height = iSplashHeight;
		[theWindow setContentSize:size];

		GR_CocoaImage *img = new GR_CocoaImage(NULL);
		img->convertFromBuffer(pBB, iSplashWidth, iSplashHeight);
		NSImage *theNSImage = img->getNSImage ();
		[[splash getImageView] setImage:theNSImage];
	

#if 0
		// create a drawing area
		GtkWidget * da = createDrawingArea ();
		gtk_widget_set_events(da, GDK_ALL_EVENTS_MASK);
		gtk_drawing_area_size(GTK_DRAWING_AREA (da), iSplashWidth, iSplashHeight);
		g_signal_connect(G_OBJECT(da), "expose_event",
						   G_CALLBACK(s_drawingarea_expose), NULL);
		g_signal_connect(G_OBJECT(da), "button_press_event",
						   G_CALLBACK(s_button_event), NULL);
		gtk_container_add(GTK_CONTAINER(frame), da);
		gtk_widget_show(da);

		// now bring the window up front & center
		gtk_window_set_position(GTK_WINDOW(wSplash), WIN_POS);

		// create the window so we can attach a GC to it
		gtk_widget_show(wSplash);
		
		// create image context
		pCocoaGraphics = new GR_CocoaGraphics(da->window, NULL, m_pApp);
		pSplashImage = pCocoaGraphics->createNewImage("splash", pBB, iSplashWidth, iSplashHeight);

		// another for luck (to bring it up forward and paint)
		gtk_widget_show(wSplash);

		// trigger an expose event to get us started
		s_drawingarea_expose(da, NULL);
#endif
    }

    DELETEP(pBB);
#ifdef DEBUG
    DELETEP(buf);
#endif
	return NULL; 	// FIXME
//    return pSplashImage;
}

/*****************************************************************/

int AP_CocoaApp::main(const char * szAppName, int argc, const char ** argv)
{
    // This is a static function.
    
    UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
    UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
    UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
    UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
    UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
    UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));
    
    // initialize our application.
	[NSApplication sharedApplication];
	
    XAP_Args XArgs = XAP_Args(argc,argv);
	AP_CocoaApp * pMyCocoaApp = new AP_CocoaApp(&XArgs, szAppName);
	AP_Args Args = AP_Args(&XArgs, szAppName, pMyCocoaApp);
    
	// Step 1: Initialize Cocoa and create the APP.
    // if the initialize fails, we don't have icons, fonts, etc.
    if (!pMyCocoaApp->initialize())
	{
		delete pMyCocoaApp;
		return -1;	// make this something standard?
	}

	// do we show the app&splash?
  	bool bShowSplash = Args.getShowSplash();
 	bool bShowApp = Args.getShowApp();
    pMyCocoaApp->setDisplayStatus(bShowApp);

    const XAP_Prefs * pPrefs = pMyCocoaApp->getPrefs();
	UT_ASSERT(pPrefs);
    bool bSplashPref = true;
    if (pPrefs && 
		pPrefs->getPrefsValueBool (AP_PREF_KEY_ShowSplash, &bSplashPref))
	{
		bShowSplash = bShowSplash && bSplashPref;
	}
    
    if (bShowSplash)
		_showSplash(2000);

	// Step 2: Handle all non-window args.

	if (!Args.doWindowlessArgs())
		return false;

    // Setup signal handlers, primarily for segfault
    // If we segfaulted before here, we *really* blew it
    
    struct sigaction sa;
    
    sa.sa_handler = signalWrapper;
    
    sigfillset(&sa.sa_mask);  // We don't want to hear about other signals
    sigdelset(&sa.sa_mask, SIGABRT); // But we will call abort(), so we can't ignore that
/* #ifndef AIX - I presume these are always #define not extern... -fjf */
#if defined (SA_NODEFER) && defined (SA_RESETHAND)
    sa.sa_flags = SA_NODEFER | SA_RESETHAND; // Don't handle nested signals
#else
    sa.sa_flags = 0;
#endif
    
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    // TODO: handle SIGABRT
    
    // this function takes care of all the command line args.
    // if some args are botched, it returns false and we should
    // continue out the door.
    if (pMyCocoaApp->parseCommandLine(Args.poptcon) && bShowApp)
    {
		// turn over control to Cocoa
		[NSApp run];
	}
    else
	{
		UT_DEBUGMSG(("DOM: not parsing command line or showing app\n"));
	}
    
    // destroy the App.  It should take care of deleting all frames.
    pMyCocoaApp->shutdown();
    delete pMyCocoaApp;
    
	poptFreeContext (Args.poptcon);
    return 0;
}

XAP_Frame * AP_CocoaApp::newFrame(AP_App * app)
{
	AP_CocoaFrame * pFrame = new AP_CocoaFrame(app);
	if (pFrame)
		pFrame->initialize();

	return pFrame;
}

void AP_CocoaApp::errorMsgBadArg(AP_Args * Args, int nextopt)
{
	printf ("Error on option %s: %s.\nRun '%s --help' to see a full list of available command line options.\n",
			poptBadOption (Args->poptcon, 0),
			poptStrerror (nextopt),
			Args->XArgs->m_argv[0]);
}

void AP_CocoaApp::errorMsgBadFile(XAP_Frame * pFrame, const char * file, 
							 UT_Error error)
{
	s_CouldNotLoadFileMessage (pFrame, file, error);
}

/*!
 * A callback for AP_Args's doWindowlessArgs call which handles
 * platform-specific windowless args.
 */
bool AP_CocoaApp::doWindowlessArgs(const AP_Args *Args)
{
 	AP_CocoaApp * pMyCocoaApp = static_cast<AP_CocoaApp*>(Args->getApp());

	if(Args->m_sPlugin)
	{
//
// Start a plugin rather than the main abiword application.
//
	    const char * szName = NULL;
		XAP_Module * pModule = NULL;
		Args->m_sPlugin = poptGetArg(Args->poptcon);
		bool bFound = false;
		if(Args->m_sPlugin != NULL)
		{
			const UT_Vector * pVec = XAP_ModuleManager::instance().enumModules ();
			for (UT_uint32 i = 0; (i < pVec->size()) && !bFound; i++)
			{
				pModule = (XAP_Module *)pVec->getNthItem (i);
				szName = pModule->getModuleInfo()->name;
				if(UT_strcmp(szName,Args->m_sPlugin) == 0)
					bFound = true;
			}
		}
		if(!bFound)
		{
			printf("Plugin %s not found or loaded \n",Args->m_sPlugin);
			return false;
		}
//
// You must put the name of the ev_EditMethod in the usage field
// of the plugin registered information.
//
		const char * evExecute = pModule->getModuleInfo()->usage;
		EV_EditMethodContainer* pEMC = pMyCocoaApp->getEditMethodContainer();
		const EV_EditMethod * pInvoke = pEMC->findEditMethodByName(evExecute);
		if(!pInvoke)
		{
			printf("Plugin %s invoke method %s not found \n",
				   Args->m_sPlugin,evExecute);
			return false;
		}
//
// Execute the plugin, then quit
//
		ev_EditMethod_invoke(pInvoke, "Called From CocoaApp");
		return false;
	}

	return true;
}

/*!
  This is a global function to call our signal handler.  It needs to
  be global so that we can pass a function pointer to it to C code
  that handles signals.  
  \todo Could this be a static member function?
  JCA: No, but it can be extern "C" { static void signalWrapper(int) }
  JCA: (well, there is a way to use a static member function and to remain
  JCA: correct, but it's a bit cumbersome.)
*/
void signalWrapper(int sig_num)
{
    AP_CocoaApp *pApp = (AP_CocoaApp *) XAP_App::getApp();
    pApp->catchSignals(sig_num);
}

static int s_signal_count = 0;

/*!
  This function actually handles signals.  The most commonly recieved
  one is SIGSEGV, the segfault signal.  We want to clean up, save the
  user's files to backup locations (currently <filename>.SEGFAULTED) and then
  call abort, so we still get a core dump that we can debug.
  \param sig_num the integer representing which signal we recieved
*/
void AP_CocoaApp::catchSignals(int sig_num)
{
    // Reset the signal handler 
    // (not that it matters - this is mostly for race conditions)
    signal(SIGSEGV, signalWrapper);
    
    s_signal_count = s_signal_count + 1;
    if(s_signal_count > 1)
    {
		UT_DEBUGMSG(("Segfault during filesave - no file saved  \n"));
		fflush(stdout);
		abort();
    }
    
    UT_DEBUGMSG(("Oh no - we just segfaulted!\n"));
	
    UT_uint32 i = 0;
    for(;i<m_vecFrames.getItemCount();i++)
    {
		AP_CocoaFrame * curFrame = (AP_CocoaFrame*) m_vecFrames[i];
		UT_ASSERT(curFrame);
		curFrame->backup(".CRASHED");
    }
    
    fflush(stdout);
    
    // Abort and dump core
    abort();
}
	

@implementation AP_SplashController
-(NSImageView*) getImageView
{
	return imageView;
}

@end
