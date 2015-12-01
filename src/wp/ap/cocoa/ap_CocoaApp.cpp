/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002-2005 Francis James Franklin
 * Copyright (C) 2001-2005, 2009 Hubert Figuiere
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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include <string>

#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_math.h"
#include "ut_misc.h"
#include "ut_png.h"
#include "ut_files.h"

#include "ut_Script.h"

#include "ev_CocoaMenuBar.h"

#include "xap_Args.h"
#include "xap_CocoaAppController.h"
#include "xap_CocoaFrame.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_EditMethods.h"
#include "xap_EncodingManager.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Menu_Layouts.h"
#include "xap_Module.h"
#include "xap_ModuleManager.h"
#include "xap_Prefs.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_Toolbar_Layouts.h"
#include "abi-builtin-plugins.h"

#include "xav_View.h"

#include "gr_CocoaImage.h"
#include "gr_Graphics.h"
#include "gr_Image.h"

#include "fp_Run.h"

#include "fv_View.h"

#include "ap_Args.h"
#include "ap_CocoaApp.h"
#include "ap_CocoaClipboard.h"
#include "ap_CocoaFrame.h"
#include "ap_CocoaFrameImpl.h"
#include "ap_CocoaPrefs.h"
#include "ap_Convert.h"
#include "ap_LoadBindings.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_Strings.h"

#include "ie_impexp_Register.h"

#include "ie_exp.h"
#include "ie_exp_RTF.h"
#include "ie_exp_Text.h"

#include "ie_imp.h"
#include "ie_imp_RTF.h"
#include "ie_imp_Text.h"
#include "ie_impGraphic.h"

#include "spell_manager.h"



// quick hack - this is defined in ap_EditMethods.cpp
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);

/*****************************************************************/

/*!
  Construct an AP_CocoaApp.
  /param pArgs Arguments from command line
  /param szAppName A string representing the name of the app.
	Currently always AbiWord (I think).
*/
AP_CocoaApp::AP_CocoaApp(const char * szAppName)
    : AP_App(szAppName),
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
  Initialize the application.  This involves preferences, keybindings,
  toolbars, graphics, spelling and everything else.  
  \return True if successfully initalized, False otherwise. if false
  the app is unusable, and loading should not continue.   
  \bug This function is 136 lines - way too long.  Needs to be
  refactored, to use a buzzword.  
*/
bool AP_CocoaApp::initialize(void)
{
	static const char * suffix = "/Library/Application Support/AbiSuite";

	if (UT_createDirectoryIfNecessary(suffix, true)) // let's create some system-level directories also, if we can
		{
			UT_UTF8String path(suffix);

			path += "/dictionary";
			UT_createDirectoryIfNecessary(path.utf8_str(), true);

			path  = suffix;
			path += "/templates";
			UT_createDirectoryIfNecessary(path.utf8_str(), true);

			path  = suffix;
			path += "/Plug-ins";
			UT_createDirectoryIfNecessary(path.utf8_str(), true);

			path  = suffix;
			path += "/math";
			UT_createDirectoryIfNecessary(path.utf8_str(), true);
		}

    const char * szUserPrivateDirectory = getUserPrivateDirectory();

    bool bVerified = false;

	if (szUserPrivateDirectory)
		{
			int suffix_length = strlen(suffix);
			int usrprv_length = strlen(szUserPrivateDirectory);

			if (usrprv_length > suffix_length)
				if (strcmp(szUserPrivateDirectory + (usrprv_length - suffix_length), suffix) == 0)
					{
						UT_UTF8String path(szUserPrivateDirectory, usrprv_length - suffix_length);

						if (UT_createDirectoryIfNecessary(path.utf8_str()))
							{
								path += "/Library";
								if (UT_createDirectoryIfNecessary(path.utf8_str()))
									{
										path += "/Application Support";
										if (UT_createDirectoryIfNecessary(path.utf8_str()))
											{
												UT_UTF8String path2(path);

												path2 += "/Enchant";
												UT_createDirectoryIfNecessary(path2.utf8_str());

												path += "/AbiSuite";
												if (UT_createDirectoryIfNecessary(path.utf8_str()))
													{
														bVerified = true;

														path2  = path;
														path2 += "/dictionary";
														UT_createDirectoryIfNecessary(path2.utf8_str());

														path2  = path;
														path2 += "/templates";
														UT_createDirectoryIfNecessary(path2.utf8_str());

														path2  = path;
														path2 += "/Plug-ins";
														UT_createDirectoryIfNecessary(path2.utf8_str());

														path2  = path;
														path2 += "/math";
														UT_createDirectoryIfNecessary(path2.utf8_str());
													}
											}
									}
							}
					}
		}
    UT_ASSERT(bVerified);
	
    // load the preferences.
    
    m_prefs = new AP_CocoaPrefs();
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
    
    if (! AP_App::initialize())
		return false;

	NSString * resources = [[NSBundle mainBundle] resourcePath];

	setenv ("ABIWORD_COCOA_BUNDLED_RESOURCES", [resources UTF8String], 1);

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
	    
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,(gchar*)AP_PREF_DEFAULT_StringSet);
		UT_ASSERT(pBuiltinStringSet);
		m_pStringSet = pBuiltinStringSet;
	    
		// see if we should load an alternative set from the disk
	    
//		const char * szDirectory = NULL;
		const char * szStringSet = NULL;
	    
		if (   (getPrefsValue(AP_PREF_KEY_StringSet,
							  (const gchar**)&szStringSet))
			   && (szStringSet)
			   && (*szStringSet)
			   && (strcmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
#if 0			
			getPrefsValueDirectory(true,
								   (const gchar*)AP_PREF_KEY_StringSetDirectory,
								   (const gchar**)&szDirectory);
			UT_ASSERT((szDirectory) && (*szDirectory));

			std::string szPathname = szDirectory;
			if (szDirectory[szPathname.size()-1]!='/')
				szPathname += "/";
			szPathname += szStringSet;
			szPathname += ".strings";
#endif

			NSString* stringSet = [resources stringByAppendingPathComponent:[NSString stringWithFormat:@"AbiWord/strings/%s%@",szStringSet,@".strings"]];

			AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);
			UT_ASSERT(pDiskStringSet);
		
			if (pDiskStringSet->loadStringsFromDisk([stringSet UTF8String]))
			{
				pDiskStringSet->setFallbackStringSet(m_pStringSet);
				m_pStringSet = pDiskStringSet;
				UT_DEBUGMSG(("Using StringSet [%s]\n",[stringSet UTF8String]));
			}
			else
			{
				DELETEP(pDiskStringSet);
				UT_DEBUGMSG(("Unable to load StringSet [%s] -- using builtin strings instead.\n",[stringSet UTF8String]));
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
	if (getPrefsValue( AP_PREF_KEY_StringSet, (const gchar**)&szMenuLabelSetName)
		&& (szMenuLabelSetName) && (*szMenuLabelSetName))
	{
		;
	}
	else {
		szMenuLabelSetName = AP_PREF_DEFAULT_StringSet;
	}
	FREEP(m_szMenuLabelSetName);
	m_szMenuLabelSetName = g_strdup(szMenuLabelSetName);
	UT_ASSERT(m_szMenuLabelSetName);
	if (!m_szMenuLabelSetName)
		return false;
	
	getMenuFactory()->buildMenuLabelSet(m_szMenuLabelSetName);
	const char * szMenuLayoutName = NULL;
	if ((getPrefsValue(AP_PREF_KEY_MenuLayout, static_cast<const gchar**>(&szMenuLayoutName))) &&
	    (szMenuLayoutName) && (*szMenuLayoutName)) {
		;
	}
	else {
		szMenuLayoutName = AP_PREF_DEFAULT_MenuLayout;
	}
	FREEP(m_szMenuLayoutName);
	m_szMenuLayoutName = g_strdup(szMenuLayoutName);
	UT_ASSERT(m_szMenuLayoutName);
	if (!m_szMenuLayoutName)
		return false;

	// synthesize a menu from the info in our base class.

	m_pCocoaMenu = new EV_CocoaMenuBar(m_szMenuLayoutName, m_szMenuLabelSetName);
	UT_ASSERT(m_pCocoaMenu);
	if (!m_pCocoaMenu)
		return false;

	abi_register_builtin_plugins();

	bool bLoadPlugins = true;
	bool bFound = getPrefsValueBool(XAP_PREF_KEY_AutoLoadPlugins,&bLoadPlugins);
	if(bLoadPlugins || !bFound)
	{
		XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
		[pController setAutoLoadPluginsAfterLaunch:YES];
	}

    return true;
}

void AP_CocoaApp::rebuildMenus(void)
{
	// getMenuFactory()->buildMenuLabelSet(m_szMenuLabelSetName);

	DELETEP(m_pCocoaMenu);
	m_pCocoaMenu = new EV_CocoaMenuBar(m_szMenuLayoutName, m_szMenuLabelSetName);
	UT_ASSERT(m_pCocoaMenu);

	m_pCocoaMenu->buildAppMenu();

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	[pController reappendPluginMenuItems];
}

/*!
  Create a new frame based on the current one.  
  \return A pointer to the new frame.  
*/
XAP_Frame * AP_CocoaApp::newFrame(void)
{
	UT_DEBUGMSG(("AP_CocoaApp::newFrame()\n"));
    AP_CocoaFrame * pCocoaFrame = new AP_CocoaFrame();

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
  \param szKey A string of gchars representing the desired key
  \param pszValue pointer for the value to be returned in. 
  \return True if successful, false otherwise.  
  \todo support meaningful return values.
*/
bool AP_CocoaApp::getPrefsValueDirectory(bool bAppSpecific,
										const gchar * szKey, const gchar ** pszValue) const
{
    if (!m_prefs)
		return false;

    const gchar * psz = NULL;
    if (!m_prefs->getPrefsValue(szKey,&psz))
		return false;

    if (*psz == '/')
    {
		*pszValue = psz;
		return true;
    }

    const gchar * dir = ((bAppSpecific) ? getAbiSuiteAppDir() : getAbiSuiteLibDir());

    static gchar buf[1024];
    UT_ASSERT((strlen(dir) + strlen(psz) + 2) < sizeof(buf));
	
    snprintf(buf, 1024, "%s/%s",dir,psz);
    *pszValue = buf;
    return true;
}

/*!
  This returns the AbiSuite application directory.
  \return A const string containting the directory path
*/
const char * AP_CocoaApp::getAbiSuiteAppDir(void) const
{
	static const char * SystemAppDir = "/Library/Application Support/AbiSuite/AbiWord"; // FIXME ??
	return SystemAppDir;
#if 0
    // we return a static string, use it quickly.
	
    static gchar buf[1024];
    UT_ASSERT((strlen(getAbiSuiteLibDir()) + strlen(PACKAGE_NAME) + 2) < sizeof(buf));

    snprintf(buf, 1024, "%s/%s",getAbiSuiteLibDir(),PACKAGE_NAME);
    return buf;
#endif
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
void AP_CocoaApp::copyToClipboard(PD_DocumentRange * pDocRange, bool /*bUseClipboard*/)
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
		
    IE_Exp_Text * pExpText = new IE_Exp_Text(pDocRange->m_pDoc, "UTF-8");
    if (pExpText)
    {
		pExpText->copyToBuffer(pDocRange,&bufTEXT);
		DELETEP(pExpText);
		UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in TEXTPLAIN (UTF-8) format.\n",bufTEXT.getLength()));
    }

    // NOTE: this clearData() will actually release our ownership of
    // NOTE: the CLIPBOARD property in addition to clearing any
    // NOTE: stored buffers.  I'm omitting it since we seem to get
    // NOTE: clr callback after we have done some other processing
    // NOTE: (like adding the new stuff).
    // m_pClipboard->clearData(true,false);
	
	
	m_pClipboard->prepareForText();
    if (bufRTF.getLength() > 0) {
		m_pClipboard->addData(XAP_CocoaClipboard::XAP_CLIPBOARD_RTF,(UT_Byte *)bufRTF.getPointer(0),bufRTF.getLength());
	}
    if (bufTEXT.getLength() > 0) {
		m_pClipboard->addData(XAP_CocoaClipboard::XAP_CLIPBOARD_TEXTPLAIN_8BIT,(UT_Byte *)bufTEXT.getPointer(0),bufTEXT.getLength());
	}

    return;
}

static const char * aszFormatsAccepted[] = { XAP_CocoaClipboard::XAP_CLIPBOARD_RTF,
											 XAP_CocoaClipboard::XAP_CLIPBOARD_STRING,
											 XAP_CocoaClipboard::XAP_CLIPBOARD_TEXTPLAIN_8BIT,
											 XAP_CocoaClipboard::XAP_CLIPBOARD_IMAGE,
											 0 /* must be last */ };

/*!
  paste from the system clipboard using the best-for-us format
  that is present.  try to get the content in the order listed.
*/
// FIXME: this code is butt ugly.
void AP_CocoaApp::pasteFromClipboard(PD_DocumentRange * pDocRange, bool /*bUseClipboard*/,
									bool bHonorFormatting)
{
    const char * szFormatFound = NULL;
    unsigned char * pData = NULL;
    UT_uint32 iLen = 0;

    bool bFoundOne = false;
	
	if (bHonorFormatting) {
		bFoundOne = m_pClipboard->getClipboardData(aszFormatsAccepted,(void**)&pData,&iLen,&szFormatFound);
	}
	else {
		const char * formats[] = {
							XAP_CocoaClipboard::XAP_CLIPBOARD_TEXTPLAIN_8BIT, 
							XAP_CocoaClipboard::XAP_CLIPBOARD_STRING,
							NULL 
							};
		bFoundOne = m_pClipboard->getClipboardData(formats,(void**)&pData,&iLen,&szFormatFound);
	}
    if (!bFoundOne)
    {
		UT_DEBUGMSG(("PasteFromClipboard: did not find anything to paste.\n"));
		return;
    }
	
    if (strcmp(szFormatFound, XAP_CocoaClipboard::XAP_CLIPBOARD_RTF) == 0)
    {
		iLen = UT_MIN(iLen,strlen((const char *)pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in format [%s].\n",iLen,szFormatFound));

		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);
    }
	else if (   (strcmp(szFormatFound, XAP_CocoaClipboard::XAP_CLIPBOARD_TEXTPLAIN_8BIT) == 0)
		   || (strcmp(szFormatFound, XAP_CocoaClipboard::XAP_CLIPBOARD_STRING) == 0))
    {
		iLen = UT_MIN(iLen,strlen((const char *)pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in format [%s].\n",iLen,szFormatFound));

		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc);
		pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);

    }
	else if (strcmp(szFormatFound, XAP_CocoaClipboard::XAP_CLIPBOARD_IMAGE) == 0) {
		  IE_ImpGraphic * pIEG = NULL;
		  FG_Graphic * pFG = NULL;
		  IEGraphicFileType iegft = IEGFT_Unknown;
		  UT_Error error = UT_OK;
		  
		  XAP_Frame * pFrame = getLastFocussedFrame ();
		  
		  UT_ByteBuf * bytes = new UT_ByteBuf( iLen );
		  
		  bytes->append (pData, iLen);
		  
		  error = IE_ImpGraphic::constructImporter(*bytes, iegft, &pIEG);
		  if(error)
		  {
			  UT_DEBUGMSG(("DOM: could not construct importer (%d)\n", 
						   error));
			  DELETEP(bytes);
			  return;
		  }
		  
		  error = pIEG->importGraphic(bytes, &pFG);
		  if(!pFG || error)
		  {
			  UT_DEBUGMSG(("DOM: could not import graphic (%d)\n", error));
			  DELETEP(bytes);
			  DELETEP(pIEG);
			  return;
		  }
		  
		  // at this point, 'bytes' is owned by pFG
		  bytes = NULL;
		  FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
		  
		  UT_UTF8String newName = UT_UTF8String_sprintf ( "paste_image_%d", UT_newNumber() ) ;
		  
		  DELETEP(pIEG);
		  
		  error = pView->cmdInsertGraphic(pFG);
		  if (error)
		  {
			  UT_DEBUGMSG(("DOM: could not insert graphic (%d)\n", error));
			  DELETEP(pFG);
			  return;
		  }
		  
		  DELETEP(pFG);	
	}

	FREEP(pData);
    return;
}

/*!
  This should determine if we can paste from the
  cliboard. 
*/
bool AP_CocoaApp::canPasteFromClipboard(void)
{
    // first, try to see if we can paste from the clipboard
    bool bFoundOne = m_pClipboard->hasFormats(aszFormatsAccepted);
	
	return bFoundOne;
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
//		m_pClipboard->clearClipboard();
    }
	
    UT_DEBUGMSG(("here we go whooooo\n"));
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

		if (strcmp(formatList[j], XAP_CocoaClipboard::XAP_CLIPBOARD_RTF) == 0)
		{
			IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(dr.m_pDoc);
			if (!pExpRtf)
				return false;		// give up on memory errors

			pExpRtf->copyToBuffer(&dr,&m_selectionByteBuf);
			DELETEP(pExpRtf);
			goto ReturnThisBuffer;
		}
			
		if (   (strcmp(formatList[j], XAP_CocoaClipboard::XAP_CLIPBOARD_TEXTPLAIN_8BIT) == 0)
			   || (strcmp(formatList[j], XAP_CocoaClipboard::XAP_CLIPBOARD_STRING) == 0))
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

int AP_CocoaApp::main(const char * szAppName, int argc, char ** argv)
{
    // This is a static function.	

#if !GLIB_CHECK_VERSION(2,32,0)
    if (!g_thread_supported ())
        g_thread_init (NULL);	
#endif
    
    UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
    UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
    UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
    UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
    UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
    UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));
    
	/* we create this autorelease pool because app startup will create a lot of things */
	/* we'll delete it before starting the main loop to cleanup memory */
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    // initialize our application.
	[XAP_CocoaApplication sharedApplication];
	
	AP_CocoaApp * pMyCocoaApp = new AP_CocoaApp(szAppName);
	
	{
		XAP_Args XArgs = XAP_Args(argc, argv);
		AP_Args Args = AP_Args(&XArgs, szAppName, pMyCocoaApp);
		
		Args.parseOptions();

		// Step 1: Initialize Cocoa and create the APP.
		// if the initialize fails, we don't have icons, fonts, etc.
		if (!pMyCocoaApp->initialize())
		{
			delete pMyCocoaApp;
			return -1;	// make this something standard?
		}

		// Step 2: Handle all non-window args.

		bool windowlessArgsWereSuccessful = true;
		if (!Args.doWindowlessArgs(windowlessArgsWereSuccessful))
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
		// We used to check for bShowApp here.  It shouldn't be needed
		// anymore, because doWindowlessArgs was supposed to bail already. -PL
		// if (pMyCocoaApp->openCmdLineFiles(&Args))

		if (true) // really don't want to be opening files atm anyway
		{
			[pool release];

			// turn over control to Cocoa
			[NSApp run];

			pool = [[NSAutoreleasePool alloc] init];
		}
		else
		{
			UT_DEBUGMSG(("DOM: not parsing command line or showing app\n"));
		}
		
		/* destroy the App.  It should take care of deleting all frames.
		 */
		pMyCocoaApp->shutdown();

	}
    delete pMyCocoaApp;
    
	if (pool)
	{
		[pool release];
	}
    return 0;
}

void AP_CocoaApp::errorMsgBadArg(const char *msg)
{
	printf ("%s.\nRun '%s --help' to see a full list of available command line options.\n",
			msg, g_get_prgname());
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
bool AP_CocoaApp::doWindowlessArgs(const AP_Args *Args, bool & bSuccess)
{
	bSuccess = true;

	return openCmdLinePlugins(Args, bSuccess);
}


static int s_signal_count = 0;

/*!
  This function actually handles signals.  The most commonly recieved
  one is SIGSEGV, the segfault signal.  We want to clean up, save the
  user's files to backup locations (currently <filename>.saved) and then
  call abort, so we still get a core dump that we can debug.
  \param sig_num the integer representing which signal we recieved
*/
void AP_CocoaApp::catchSignals(int /*sig_num*/)
{
    // Reset the signal handler 
    // (not that it matters - this is mostly for race conditions)
    signal(SIGSEGV, &XAP_App::signalWrapper);
    
    s_signal_count = s_signal_count + 1;
    if(s_signal_count > 1)
    {
		UT_DEBUGMSG(("Segfault during filesave - no file saved  \n"));
		fflush(stdout);
		abort();
    }
    
    UT_DEBUGMSG(("Oh no - we just segfaulted!\n"));

    saveRecoveryFiles();

    fflush(stdout);
    
    // Abort and dump core
    abort();
}
