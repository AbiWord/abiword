/* AbiWord
 * Copyright (C) 1998-2002 AbiSource, Inc.
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

#define ABIWORD_INTERNAL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#ifdef ABI_OPT_PERL
#include "ut_PerlBindings.h"
#endif
#include "ut_Script.h"
#include "ut_unixDirent.h"
#include "ut_sleep.h"

#include "xap_Args.h"
#include "ap_Args.h"
#include "ap_Convert.h"
#include "ap_UnixFrame.h"
#include "ap_UnixApp.h"
#include "spell_manager.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Dialog_Id.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Menu_Layouts.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_Toolbar_Layouts.h"
#include "xav_View.h"

#include "gr_Graphics.h"
#include "gr_UnixGraphics.h"
#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "xap_UnixDialogHelper.h"

#include "fv_View.h"
#include "fp_Run.h"

#include "ut_string_class.h"
#include "xap_EncodingManager.h"

#include "ie_impexp_Register.h"
#include "xap_EditMethods.h"
#include "ev_EditMethod.h"
#include "xap_ModuleManager.h"
#include "xap_Module.h"

#include "ie_exp.h"
#include "ie_exp_RTF.h"
#include "ie_exp_Text.h"
#include "ie_exp_HTML.h"

#include "ie_imp.h"
#include "ie_imp_RTF.h"
#include "ie_imp_Text.h"

#include "xap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"
#include "gr_Image.h"

#include "xap_ModuleManager.h"
#include "xap_UnixPSGraphics.h"
#include "abiwidget.h"

#ifdef HAVE_CURL
#include "ap_UnixHashDownloader.h"
#endif

#ifdef GTK_WIN_POS_CENTER_ALWAYS
#define WIN_POS GTK_WIN_POS_CENTER_ALWAYS
#else
#define WIN_POS GTK_WIN_POS_CENTER
#endif

#include <popt.h>

#include "ie_impGraphic.h"
#include "ut_math.h"

#ifdef HAVE_GNOME
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#endif

// quick hack - this is defined in ap_EditMethods.cpp
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);

/*****************************************************************/

/*!
  Construct an AP_UnixApp.
  /param pArgs Arguments from command line
  /param szAppName A string representing the name of the app.
	Currently always AbiWord (I think).
*/
AP_UnixApp::AP_UnixApp(XAP_Args * pArgs, const char * szAppName)
    : AP_App(pArgs,szAppName),
	  m_pStringSet(0),
	  m_pClipboard(0),
	  m_bHasSelection(false),
	  m_bSelectionInFlux(false),
	  m_pViewSelection(0),
	  m_cacheSelectionView(0),
	  m_pFrameSelection(0)
{
#ifdef HAVE_CURL
	m_pHashDownloader = (XAP_HashDownloader *)(new AP_UnixHashDownloader());
#endif

//
// hack to link abi_widget - thanks fjf
//
	if(this == 0)
	{
		GtkWidget * pUn = abi_widget_new_with_file("fred.abw");
	}
}

/*!
  Destructor for AP_UnixApp's.  Cleans up spellcheck, clipboard, and
	StringSet.  
*/
AP_UnixApp::~AP_UnixApp(void)
{
#ifdef HAVE_CURL
    DELETEP(m_pHashDownloader);
#endif

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
bool AP_UnixApp::initialize(void)
{
    const char * szUserPrivateDirectory = getUserPrivateDirectory();
    bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);

    if (!bVerified)
      {
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      }

    // load the preferences.
    
    m_prefs = new AP_UnixPrefs(this);
    m_prefs->fullInit();

    //////////////////////////////////////////////////////////////////
    // load the dialog and message box strings
    //
    // (we want to do this as soon as possible so that any errors in
    // the initialization could be properly localized before being
    // reported to the user)
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

    // now that preferences are established, let the xap init
		   
    m_pClipboard = new AP_UnixClipboard(this);
    UT_ASSERT(m_pClipboard);
    m_pClipboard->initialize();
    
    m_pEMC = AP_GetEditMethods();
    UT_ASSERT(m_pEMC);
    
    m_pBindingSet = new AP_BindingSet(m_pEMC);
    UT_ASSERT(m_pBindingSet);
	
    m_pMenuActionSet = AP_CreateMenuActionSet();
    UT_ASSERT(m_pMenuActionSet);
    
    m_pToolbarActionSet = AP_CreateToolbarActionSet();
    UT_ASSERT(m_pToolbarActionSet);
    
    if (! XAP_UNIXBASEAPP::initialize())
		return false;

	//////////////////////////////////////////////////////////////////
	// Initialize the importers/exporters
	//////////////////////////////////////////////////////////////////
	IE_ImpExp_RegisterXP ();
	
    // Now we have the strings loaded we can populate the field names correctly
    int i;
	
    for (i = 0; fp_FieldTypes[i].m_Type != FPFIELDTYPE_END; i++)
      (&fp_FieldTypes[i])->m_Desc = m_pStringSet->getValue(fp_FieldTypes[i].m_DescId);

    for (i = 0; fp_FieldFmts[i].m_Tag != NULL; i++)
      (&fp_FieldFmts[i])->m_Desc = m_pStringSet->getValue(fp_FieldFmts[i].m_DescId);

    ///////////////////////////////////////////////////////////////////////
    /// Build a labelset so the plugins can add themselves to something ///
    ///////////////////////////////////////////////////////////////////////

	const char * szMenuLabelSetName = NULL;
	if (getPrefsValue( AP_PREF_KEY_StringSet, (const XML_Char**)&szMenuLabelSetName)
		&& (szMenuLabelSetName) && (*szMenuLabelSetName))
	{
		;
	}
	else
		szMenuLabelSetName = AP_PREF_DEFAULT_StringSet;

	getMenuFactory()->buildMenuLabelSet(szMenuLabelSetName);
	bool bLoadPlugins = true;
	bool bFound = getPrefsValueBool(XAP_PREF_KEY_AutoLoadPlugins,&bLoadPlugins);
	if(bLoadPlugins || !bFound)
		loadAllPlugins();

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
XAP_Frame * AP_UnixApp::newFrame(void)
{
    AP_UnixFrame * pUnixFrame = new AP_UnixFrame(this);

    if (pUnixFrame)
		pUnixFrame->initialize();

    return pUnixFrame;
}

/*!
  If the user has set the preferences to save them selves on shutdown,
	save them.  
  \return This function always returns true.  
  \todo The return value should be fixed to check the return values of
	the functions it calls, and potentially handle errors.  At a
	minimum, it should return false on errors.  
*/
bool AP_UnixApp::shutdown(void)
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
bool AP_UnixApp::getPrefsValueDirectory(bool bAppSpecific,
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
const char * AP_UnixApp::getAbiSuiteAppDir(void) const
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
const XAP_StringSet * AP_UnixApp::getStringSet(void) const
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
void AP_UnixApp::copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard)
{

    UT_ByteBuf bufRTF;
    UT_ByteBuf bufHTML;
    UT_ByteBuf bufTEXT;

    // create RTF buffer to put on the clipboard
		
    IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
    if (pExpRtf)
    {
		pExpRtf->copyToBuffer(pDocRange,&bufRTF);
		DELETEP(pExpRtf);
    }

    // create XHTML buffer to put on the clipboard

    IE_Exp_HTML * pExpHtml = new IE_Exp_HTML(pDocRange->m_pDoc);
    if ( pExpHtml )
    {
	pExpHtml->set_HTML4(true);
        pExpHtml->copyToBuffer(pDocRange, &bufHTML);
        DELETEP(pExpHtml);
    }

    // create UTF-8 text buffer to put on the clipboard
		
    IE_Exp_Text * pExpText = new IE_Exp_Text(pDocRange->m_pDoc, "UTF-8");
    if (pExpText)
    {
		pExpText->copyToBuffer(pDocRange,&bufTEXT);
		DELETEP(pExpText);
    }

    // NOTE: this clearData() will actually release our ownership of
    // NOTE: the CLIPBOARD property in addition to clearing any
    // NOTE: stored buffers.  I'm omitting it since we seem to get
    // NOTE: clr callback after we have done some other processing
    // NOTE: (like adding the new stuff).
    // m_pClipboard->clearData(true,false);
	
    // TODO: handle CLIPBOARD vs PRIMARY
    XAP_UnixClipboard::T_AllowGet target = ((bUseClipboard)
					    ? XAP_UnixClipboard::TAG_ClipboardOnly
					    : XAP_UnixClipboard::TAG_PrimaryOnly);

    if (bufRTF.getLength() > 0)
		m_pClipboard->addRichTextData(target, (UT_Byte *)bufRTF.getPointer(0),bufRTF.getLength());
    if (bufHTML.getLength() > 0)
                m_pClipboard->addHtmlData(target, (UT_Byte *)bufHTML.getPointer(0), bufHTML.getLength());
    if (bufTEXT.getLength() > 0)
		m_pClipboard->addTextData(target, (UT_Byte *)bufTEXT.getPointer(0),bufTEXT.getLength());

    {
      // TODO: we have to make a good way to tell if the current selection is just an image
      FV_View * pView = static_cast<FV_View*>(getLastFocussedFrame()->getCurrentView());
      if (pView && !pView->isSelectionEmpty())
	{
	  // don't own, don't free
	  const UT_ByteBuf * png = 0;
	  
	  pView->saveSelectedImage (&png);
	  if (png && png->getLength() > 0)
	    {
	      m_pClipboard->addPNGData(target, (UT_Byte*)png->getPointer(0), png->getLength());
	    }
	}
    }

    return;
}

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
void AP_UnixApp::pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard,
				    bool bHonorFormatting)
{
    XAP_UnixClipboard::T_AllowGet tFrom = ((bUseClipboard)
					   ? XAP_UnixClipboard::TAG_ClipboardOnly
					   : XAP_UnixClipboard::TAG_PrimaryOnly);

    const char * szFormatFound = NULL;
    const unsigned char * pData = NULL;
    UT_uint32 iLen = 0;

    bool bFoundOne = false;
    
    if ( bHonorFormatting )
      bFoundOne = m_pClipboard->getSupportedData(tFrom,reinterpret_cast<const void **>(&pData),&iLen,&szFormatFound);
    else
      bFoundOne = m_pClipboard->getTextData(tFrom,reinterpret_cast<const void **>(&pData),&iLen,&szFormatFound);

    if (!bFoundOne)
    {
		UT_DEBUGMSG(("PasteFromClipboard: did not find anything to paste.\n"));
		return;
    }

    if (AP_UnixClipboard::isRichTextTag(szFormatFound))
    {
		iLen = UT_MIN(iLen,strlen((const char *)pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in format [%s].\n",iLen,szFormatFound));

		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);

		return;
    }
    else if (AP_UnixClipboard::isImageTag(szFormatFound))
      {
	IE_ImpGraphic * pIEG = NULL;
	FG_Graphic * pFG = NULL;
	IEGraphicFileType iegft = IEGFT_Unknown;
	UT_Error error = UT_OK;

	XAP_Frame * pFrame = getLastFocussedFrame ();

	UT_ByteBuf * bytes = new UT_ByteBuf( iLen );

	bytes->append (pData, iLen);

	error = IE_ImpGraphic::constructImporter(bytes, iegft, &pIEG);
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
	FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
	
	UT_String newName = UT_String_sprintf ( "paste_image_%d", UT_newNumber() ) ;

	DELETEP(pIEG);

	error = pView->cmdInsertGraphic(pFG, newName.c_str());
	if (error)
	  {
	    UT_DEBUGMSG(("DOM: could not insert graphic (%d)\n", error));
	    DELETEP(pFG);
	    return;
	  }
	
	DELETEP(pFG);
      }
    else // ( AP_UnixClipboard::isTextTag(szFormatFound) )
    {
		iLen = UT_MIN(iLen,strlen((const char *)pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in format [%s].\n",iLen,szFormatFound));

		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc,"UTF-8");
		pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);

		return;
    }

    return;
}

bool AP_UnixApp::canPasteFromClipboard(void)
{
    return m_pClipboard->canPaste(XAP_UnixClipboard::TAG_ClipboardOnly);
}

extern "C" {

	// return > 0 for directory entries ending in ".so"
#if defined (__APPLE__) || defined (__FreeBSD__) || defined (__OpenBSD__) \
	|| defined(_AIX) || defined(__sgi)
	static int so_only (struct dirent *d)
#else
	static int so_only (const struct dirent *d)
#endif
	{
		const char * name = d->d_name;
		
		if ( name )
		{
			int len = strlen (name);
			
			if (len >= 3)
			{
				if(!strcmp(name+(len-3), ".so"))
	    return 1;
			}
		}
		return 0;
	}
} // extern "C" block

void AP_UnixApp::loadAllPlugins ()
{
  struct dirent **namelist;
  int n = 0;

  UT_String pluginList[2];
  UT_String pluginDir;

  // the global plugin directory
  pluginDir = ABIWORD_PLUGINDIR;
  pluginList[0] = pluginDir;

  // the user-local plugin directory
  pluginDir = getUserPrivateDirectory ();
  pluginDir += "/AbiWord-2.0/plugins/";
  pluginList[1] = pluginDir;

  for(UT_uint32 i = 0; i < (sizeof(pluginList)/sizeof(pluginList[0])); i++)
  {
      pluginDir = pluginList[i];

      n = scandir(pluginDir.c_str(), &namelist, so_only, alphasort);
      UT_DEBUGMSG(("DOM: found %d plugins in %s\n", n, pluginDir.c_str()));

      if (n > 0)
	  {
		  while(n--) 
		  {
			  UT_String plugin (pluginDir + namelist[n]->d_name);

			  UT_DEBUGMSG(("DOM: loading plugin %s\n", plugin.c_str()));

			  int len = strlen (namelist[n]->d_name);
			  if (len < 4)
			  {
				  UT_DEBUGMSG(("FJF: bad name for a plugin\n"));
				  free(namelist[n]);
				  continue;
			  }
			  if(strcmp (namelist[n]->d_name+(len-3), ".so") != 0)
			  {
				  UT_DEBUGMSG(("FJF: not really a plugin?\n"));
				  free(namelist[n]);
				  continue;
			  }

			  if (XAP_ModuleManager::instance().loadModule (plugin.c_str()))
			  {
				  UT_DEBUGMSG(("DOM: loaded plugin: %s\n", namelist[n]->d_name));
			  }
			  else
			  {
				  UT_DEBUGMSG(("DOM: didn't load plugin: %s\n", namelist[n]->d_name));
			  }
			  free(namelist[n]);
		  }
		  free(namelist);
      }
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
void AP_UnixApp::setSelectionStatus(AV_View * pView)
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
      m_bHasSelection = true;
      m_pClipboard->assertSelection();
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
	m_bHasSelection = false;
	//m_pClipboard->clearData(false,true);
    }
	
    setViewSelection(pView);
    m_pFrameSelection = (XAP_Frame *)pView->getParentData();

    m_bSelectionInFlux = false;
    return;
}

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

bool AP_UnixApp::forgetFrame(XAP_Frame * pFrame)
{
    if (m_pFrameSelection && (pFrame==m_pFrameSelection))
    {
		m_pClipboard->clearData(false,true);
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
void AP_UnixApp::clearSelection(void)
{
    if (m_bSelectionInFlux)
		return;
    m_bSelectionInFlux = true;
	
    if (m_pViewSelection && m_pFrameSelection && m_bHasSelection)
    {
		FV_View *pView = static_cast<FV_View *>(m_pViewSelection);
		pView->cmdUnselectSelection();
		m_bHasSelection = false;
    }
	
    m_bSelectionInFlux = false;
}

void AP_UnixApp::cacheCurrentSelection(AV_View * pView)
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
		m_cacheDeferClear = false;
    }
    else
    {
		if (m_cacheDeferClear)
		{
			m_cacheDeferClear = false;
			m_bHasSelection = false;
		}
		m_cacheSelectionView = NULL;
    }
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
bool AP_UnixApp::getCurrentSelection(const char** formatList,
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
    }
    else
    {
		// TODO if we ever support multiple view types, we'll have to
		// TODO change this.
		FV_View * pFVView = static_cast<FV_View *>(m_pViewSelection);
	
		pFVView->getDocumentRangeOfCurrentSelection(&dr);
    }
	
    m_selectionByteBuf.truncate(0);

    for (j=0; (formatList[j]); j++)
    {
		if ( AP_UnixClipboard::isRichTextTag(formatList[j]) )
		{
			IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(dr.m_pDoc);
			if (!pExpRtf)
				return false;		// give up on memory errors

			pExpRtf->copyToBuffer(&dr,&m_selectionByteBuf);
			DELETEP(pExpRtf);
			goto ReturnThisBuffer;
		}
			
		if ( AP_UnixClipboard::isTextTag(formatList[j]) )
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

static GtkWidget * wSplash = NULL;
static GR_Image * pSplashImage = NULL;
static GR_UnixGraphics * pUnixGraphics = NULL;
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
		DELETEP(pUnixGraphics);
		DELETEP(pSplashImage);
    }
    return TRUE;
}

/*!
  Calls s_hideSplash().  Used for button clicks before the regular app
  starts.  This only gets called if the button click is on the splash
  screen.  
*/
static void s_button_event(GtkWidget * /*window*/)
{
    s_hideSplash(NULL);
}

/*!
  Actually draws the splash screen.  
  \return Always returns FALSE.
  \bug When boolean functions return meaningless values, they should
	return TRUE, not FALSE.  
*/
static gint s_drawingarea_expose(GtkWidget * /* widget */,
								 GdkEventExpose * /* pExposeEvent */)
{
    if (pUnixGraphics && pSplashImage)
    {
		pUnixGraphics->drawImage(pSplashImage, 0, 0);

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
GR_Image * AP_UnixApp::_showSplash(UT_uint32 delay)
{
    wSplash = NULL;
    pSplashImage = NULL;

    UT_ByteBuf* pBB = NULL;

    // store value for use by the expose event, which attaches the timer
    splashTimeoutValue = delay;
	
    extern unsigned char g_pngSplash[];		// see ap_wp_Splash.cpp
    extern unsigned long g_pngSplash_sizeof;	// see ap_wp_Splash.cpp
	
    pBB = new UT_ByteBuf();
    if (
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
		// create a centered window the size of our image
		wSplash = gtk_window_new(GTK_WINDOW_POPUP); //GTK_WINDOW_DIALOG
		gtk_window_set_default_size (GTK_WINDOW (wSplash),
									 iSplashWidth, iSplashHeight);
		gtk_window_set_policy(GTK_WINDOW(wSplash), FALSE, FALSE, FALSE);

		// create a frame to add depth
		GtkWidget * frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(wSplash), frame);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
		gtk_widget_show(frame);

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
#ifndef WITH_PANGO
		pUnixGraphics = new GR_UnixGraphics(da->window, NULL, m_pApp);
#else
		pUnixGraphics = new GR_UnixGraphics(da->window, m_pApp);
#endif
		pSplashImage = pUnixGraphics->createNewImage("splash", pBB, pUnixGraphics->tlu(iSplashWidth), 
													 pUnixGraphics->tlu(iSplashHeight));

		// another for luck (to bring it up forward and paint)
		gtk_widget_show(wSplash);

		// trigger an expose event to get us started
		s_drawingarea_expose(da, NULL);
    }

    DELETEP(pBB);

    return pSplashImage;
}

/*****************************************************************/

int AP_UnixApp::main(const char * szAppName, int argc, const char ** argv)
{
    // This is a static function.
    
    UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
    UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
    UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
    UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
    UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
    UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));
    
    // initialize our application.
    
	XAP_Args XArgs = XAP_Args(argc,argv);
	AP_UnixApp * pMyUnixApp = new AP_UnixApp(&XArgs, szAppName);
	AP_Args Args = AP_Args(&XArgs, szAppName, pMyUnixApp);
    
	// Step 1: Initialize GTK and create the APP.
    // HACK: these calls to gtk reside properly in 
    // HACK: XAP_UNIXBASEAPP::initialize(), but need to be here 
    // HACK: to throw the splash screen as soon as possible.
	// hack needed to intialize gtk before ::initialize
    gtk_set_locale();
    gboolean have_display = gtk_init_check(&XArgs.m_argc,(char ***)&XArgs.m_argv);

    if (have_display || Args.getShowApp()) {
      // this is just like an abort() but with a useful error messsage
#ifndef HAVE_GNOME
      gtk_init (&XArgs.m_argc,(char ***)&XArgs.m_argv);
#else
	  //gnome_program_init ("AbiWord", ABI_BUILD_VERSION, NULL, XArgs.m_argc, const_cast<char **>(XArgs.m_argv), GNOME_PARAM_APP_DATADIR, "/usr/share", GNOME_PARAM_NONE);
	  gnome_init ("AbiWord", ABI_BUILD_VERSION, XArgs.m_argc, const_cast<char **>(XArgs.m_argv));
      gnome_vfs_init ();
#endif
    }

    UT_DEBUGMSG(("UnixApp: about to initialize \n"));
    // if the initialize fails, we don't have icons, fonts, etc.
    if (!pMyUnixApp->initialize())
	{
		delete pMyUnixApp;
		return -1;	// make this something standard?
	}

    // do we show the app&splash?
    bool bShowSplash = Args.getShowSplash();
    bool bShowApp = Args.getShowApp();
    pMyUnixApp->setDisplayStatus(bShowApp);

    const XAP_Prefs * pPrefs = pMyUnixApp->getPrefs();
    UT_ASSERT(pPrefs);
    bool bSplashPref = true;
    if (pPrefs && 
	pPrefs->getPrefsValueBool (AP_PREF_KEY_ShowSplash, &bSplashPref))
      {
	bShowSplash = bShowSplash && bSplashPref;
      }
    
    if (bShowSplash)
      _showSplash(1500);
    
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
    
    // Step 3: Create windows as appropriate.
    // if some args are botched, it returns false and we should
    // continue out the door.
    if (pMyUnixApp->parseCommandLine(Args.poptcon) && bShowApp)
      {
	// turn over control to gtk
	gtk_main();
      }
    else
      {
	UT_DEBUGMSG(("DOM: not parsing command line or showing app\n"));
      }
    
    // Step 4: Destroy the App.  It should take care of deleting all frames.
    pMyUnixApp->shutdown();
    delete pMyUnixApp;
    
    return 0;
}

XAP_Frame * AP_UnixApp::newFrame(AP_App * app)
{
  AP_UnixFrame * pFrame = new AP_UnixFrame(app);
  if (pFrame)
    pFrame->initialize();
  
  return pFrame;
}

void AP_UnixApp::errorMsgBadArg(AP_Args * Args, int nextopt)
{
  printf ("Error on option %s: %s.\nRun '%s --help' to see a full list of available command line options.\n",
	  poptBadOption (Args->poptcon, 0),
	  poptStrerror (nextopt),
	  Args->XArgs->m_argv[0]);
}

void AP_UnixApp::errorMsgBadFile(XAP_Frame * pFrame, const char * file, 
				 UT_Error error)
{
  s_CouldNotLoadFileMessage (pFrame, file, error);
}

/*!
 * A callback for AP_Args's doWindowlessArgs call which handles
 * platform-specific windowless args.
 */
bool AP_UnixApp::doWindowlessArgs(const AP_Args *Args)
{
  if (Args->m_sGeometry)
    {
      // [--geometry <X geometry string>]
      
      // TODO : does X have a dummy geometry value reserved for this?
      gint dummy = 1 << ((sizeof(gint) * 8) - 1);
      gint x = dummy;
      gint y = dummy;
		guint width = 0;
		guint height = 0;
		
		XParseGeometry(Args->m_sGeometry, &x, &y, &width, &height);
		
		// use both by default
		UT_uint32 f = (XAP_UNIXBASEAPP::GEOMETRY_FLAG_SIZE
					   | XAP_UNIXBASEAPP::GEOMETRY_FLAG_POS);
		
		// if pos (x and y) weren't provided just use size
		if (x == dummy || y == dummy)
			f = XAP_UNIXBASEAPP::GEOMETRY_FLAG_SIZE;
		
		// if size (width and height) weren't provided just use pos
		if (width == 0 || height == 0)
			f = XAP_UNIXBASEAPP::GEOMETRY_FLAG_POS;
		
		// set the xap-level geometry for future frame use
		Args->getApp()->setGeometry(x, y, width, height, f);
	}

 	AP_UnixApp * pMyUnixApp = static_cast<AP_UnixApp*>(Args->getApp());
	if (Args->m_sPrintTo) 
	{
		if ((Args->m_sFile = poptGetArg (Args->poptcon)) != NULL)
	    {
			UT_DEBUGMSG(("DOM: Printing file %s\n", Args->m_sFile));
			
			AP_Convert conv ;
			conv.setVerbose(Args->m_iVerbose);
			
			PS_Graphics * pG = new PS_Graphics ((Args->m_sPrintTo[0] == '|' ? Args->m_sPrintTo+1 : Args->m_sPrintTo), Args->m_sFile, 
												pMyUnixApp->getApplicationName(), pMyUnixApp->getFontManager(),
												(Args->m_sPrintTo[0] != '|'), pMyUnixApp);
			
			conv.print (Args->m_sFile, pG);
	      
			delete pG;
	    }
		else
	    {
			// couldn't load document
			printf("Error: no file to print!\n");
	    }

		if (!Args->m_iShow)
			return false;
	}

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
		EV_EditMethodContainer* pEMC = pMyUnixApp->getEditMethodContainer();
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
		ev_EditMethod_invoke(pInvoke, "Called From Unix[Gnome]App");
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
    AP_UnixApp *pApp = (AP_UnixApp *) XAP_App::getApp();
    pApp->catchSignals(sig_num);
}

static gint s_signal_count = 0;

/*!
  This function actually handles signals.  The most commonly recieved
  one is SIGSEGV, the segfault signal.  We want to clean up, save the
  user's files to backup locations (currently <filename>.SEGFAULTED) and then
  call abort, so we still get a core dump that we can debug.
  \param sig_num the integer representing which signal we recieved
*/
void AP_UnixApp::catchSignals(int sig_num)
{
    // Reset the signal handler 
    // (not that it matters - this is mostly for race conditions)
    signal(SIGSEGV, signalWrapper);

    s_signal_count = s_signal_count + 1;
    if(s_signal_count > 1)
    {
		UT_DEBUGMSG(("Crash during filesave - no file saved\n"));
		fflush(stdout);
		abort();
    }
    
    UT_DEBUGMSG(("Oh no - we just crashed!\n"));
	
    UT_uint32 i = 0;
    for(;i<m_vecFrames.getItemCount();i++)
    {
		AP_UnixFrame * curFrame = (AP_UnixFrame*) m_vecFrames[i];
		UT_ASSERT(curFrame);
		if (NULL == curFrame->getFilename())
		  curFrame->backup(".abw.CRASHED");
		else
		  curFrame->backup(".CRASHED");
    }
    
    fflush(stdout);
    
    // Abort and dump core
    abort();
}

#if ABI_OPT_BONOBO
//-------------------------------------------------------------------
// Bonobo Control factory stuff
//-------------------------------------------------------------------

/*****************************************************************/
/* Implements the Bonobo/PropertyBag:1.0 interface               */
/*****************************************************************/

/* 
 * get a value from abiwidget
 */ 
static void get_prop (BonoboPropertyBag 	*bag,
	  BonoboArg 		*arg,
	  guint 		 arg_id,
	  CORBA_Environment 	*ev,
	  gpointer 		 user_data)
{
	g_return_if_fail (user_data != NULL);
	g_return_if_fail (IS_ABI_WIDGET(user_data));

	AbiWidget * abi = ABI_WIDGET(user_data); 

	//
	// first create a fresh gtkargument.
	//
	GtkArg * gtk_arg = (GtkArg *) g_new0 (GtkArg,1);

	//
	// Now copy the bonobo argument to this so we know what to extract from
	// AbiWidget.
	//

	bonobo_arg_to_gtk(gtk_arg,arg);

	//
	// OK get the data from the widget. Only one argument at a time.
	//
	gtk_object_getv(GTK_OBJECT(abi),1,gtk_arg);

	//
	// Now copy it back to the bonobo argument.
	//
	bonobo_arg_from_gtk (arg, gtk_arg);

	//
	// Free up allocated memory
	//
	if (gtk_arg->type == GTK_TYPE_STRING && GTK_VALUE_STRING (*gtk_arg))
	{
		g_free (GTK_VALUE_STRING (*gtk_arg));
	}
	g_free(gtk_arg);
}

/*
 * Tell abiwidget to do something.
 */
static void set_prop (BonoboPropertyBag 	*bag,
	  const BonoboArg 	*arg,
	  guint 		 arg_id,
	  CORBA_Environment 	*ev,
	  gpointer 		 user_data)
{
	AbiWidget 	*abi;
	
	g_return_if_fail (user_data != NULL);
	g_return_if_fail (IS_ABI_WIDGET(user_data));		

	abi = ABI_WIDGET (user_data); 

	//
	// Have to translate BonoboArg to GtkArg now. This is really easy.
	//
	GtkArg * gtk_arg = (GtkArg *) g_new0 (GtkArg,1);
	bonobo_arg_to_gtk(gtk_arg,arg);

	//
	// Can only pass one argument at a time.
	//
	gtk_object_setv(GTK_OBJECT(abi),1,gtk_arg);

	//
	// Free up allocated memory
	//
	if (gtk_arg->type == GTK_TYPE_STRING && GTK_VALUE_STRING (*gtk_arg))
	{
		g_free (GTK_VALUE_STRING (*gtk_arg));
	}
	g_free(gtk_arg);
}

/*****************************************************************/
/* Implements the Bonobo/Persist:1.0, Bonobo/PersistStream:1.0,
   Bonobo/PersistFile:1.0 Interfaces */
/*****************************************************************/

/*
 * Loads a document from a Bonobo_Stream. Code gratitutously stolen 
 * from ggv
 */
static void
load_document_from_stream (BonoboPersistStream *ps,
					 Bonobo_Stream stream,
					 Bonobo_Persist_ContentType type,
					 void *data,
					 CORBA_Environment *ev)
{
	AbiWidget *abiwidget;
	Bonobo_Stream_iobuf *buffer;
	CORBA_long len_read;
	FILE * tmpfile;

	g_return_if_fail (data != NULL);
	g_return_if_fail (IS_ABI_WIDGET (data));

	abiwidget = (AbiWidget *) data;

	//
	// Create a temp file name.
	//
	char szTempfile[ 2048 ];
	UT_tmpnam(szTempfile);

	tmpfile = fopen(szTempfile, "wb");

	do 
	{
		Bonobo_Stream_read (stream, 32768, &buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			goto exit_clean;

		len_read = buffer->_length;

		if (buffer->_buffer && len_read)
			if(fwrite(buffer->_buffer, 1, len_read, tmpfile) != len_read) 
			{
				CORBA_free (buffer);
				goto exit_clean;
			}

		CORBA_free (buffer);
	} 
	while (len_read > 0);

	fclose(tmpfile);

	//
	// Load the file.
	//
	//
	g_object_set(G_OBJECT(abiwidget),"AbiWidget::unlink_after_load",(gboolean) TRUE,NULL);
	g_object_set(G_OBJECT(abiwidget),"AbiWidget::load_file",(gchar *) szTempfile,NULL);
	return;

 exit_clean:
	fclose (tmpfile);
	unlink(szTempfile);
	return;
}

/*
 * Loads a document from a Bonobo_Stream. Code gratitutously stolen 
 * from ggv
 */
static void
save_document_to_stream (BonoboPersistStream *ps,
			 Bonobo_Stream stream,
			 Bonobo_Persist_ContentType type,
			 void *data,
			 CORBA_Environment *ev)
{
	AbiWidget *abiwidget;
	Bonobo_Stream_iobuf *stream_buffer;
	CORBA_octet buffer [ 32768 ] = "" ;
	CORBA_long len_read = 0;
	FILE * tmpfile = NULL;

	g_return_if_fail (data != NULL);
	g_return_if_fail (IS_ABI_WIDGET (data));

	abiwidget = (AbiWidget *) data;

	//
	// Create a temp file name.
	//
	char szTempfile[ 2048 ];
	UT_tmpnam(szTempfile);

	char * ext = ".abw" ;

	if ( !strcmp ( "application/msword", type ) )
	  ext = ".rtf" ; // should this be .rtf or .doc??
	else if ( !strcmp ( "application/rtf", type ) )
	  ext = ".rtf" ;
	else if ( !strcmp ( "application/x-applix-word", type ) )
	  ext = ".aw";
	else if ( !strcmp ( "appplication/vnd.palm", type ) )
	  ext = ".pdb" ;
	else if ( !strcmp ( "text/plain", type ) )
	  ext = ".txt" ;
	else if ( !strcmp ( "text/html", type ) )
	  ext = ".html" ;
	else if ( !strcmp ( "text/vnd.wap.wml", type ) )
	  ext = ".wml" ;

	// todo: vary this based on the ContentType
	if ( ! abi_widget_save_ext ( abiwidget, szTempfile, ext ) )
	  return ;

	tmpfile = fopen(szTempfile, "wb");

	do 
	{
	  len_read = fread ( buffer, sizeof(CORBA_octet), 32768, tmpfile ) ;

	  stream_buffer = Bonobo_Stream_iobuf__alloc ();
	  stream_buffer->_buffer = (CORBA_octet*)buffer;
	  stream_buffer->_length = len_read;

	  Bonobo_Stream_write (stream, stream_buffer, ev);

	  if (ev->_major != CORBA_NO_EXCEPTION)
	    goto exit_clean;
	  
	  CORBA_free (buffer);
	} 
	while (len_read > 0);

 exit_clean:
	fclose (tmpfile);
	unlink(szTempfile);
	return;
}

//
// Implements the get_object interface.
//
static Bonobo_Unknown
abiwidget_get_object(BonoboItemContainer *item_container,
							   CORBA_char          *item_name,
							   CORBA_boolean       only_if_exists,
							   CORBA_Environment   *ev,
							   AbiWidget *  abi)
{
	Bonobo_Unknown corba_object;
	BonoboObject *object = NULL;

	g_return_val_if_fail(abi != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail(IS_ABI_WIDGET(abi), CORBA_OBJECT_NIL);

	g_message ("abiwiget_get_object: %d - %s",
			   only_if_exists, item_name);

	object = (BonoboObject *) AbiWidget_control_new(abi);


	if (object == NULL)
		return NULL;

	corba_object = bonobo_object_corba_objref (object);

	return bonobo_object_dup_ref (corba_object, ev);
}

/*
 * Loads a document from a Bonobo_File. Code gratitutously stolen 
 * from ggv
 */
static int
load_document_from_file(BonoboPersistFile *pf, const CORBA_char *filename,
				   CORBA_Environment *ev, void *data)
{
	AbiWidget *abiwidget;

	g_return_val_if_fail (data != NULL,-1);
	g_return_val_if_fail (IS_ABI_WIDGET (data),-1);

	abiwidget = ABI_WIDGET (data);

	//
	// Load the file.
	//
	g_object_set(G_OBJECT(abiwidget),"AbiWidget::load_file",(gchar *) filename,NULL);
	return 0;
}

static int
save_document_to_file(BonoboPersistFile *pf, const CORBA_char *filename,
		      CORBA_Environment *ev, void *data)
{
  AbiWidget *abiwidget;
  
  g_return_val_if_fail (data != NULL,-1);
  g_return_val_if_fail (IS_ABI_WIDGET (data),-1);
  
  abiwidget = ABI_WIDGET (data);

  abi_widget_save ( abiwidget, filename ) ;

  return 0 ;
}

//
// Data content for persist stream
//
static Bonobo_Persist_ContentTypeList *
pstream_get_content_types (BonoboPersistStream *ps, void *closure,
			   CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (9, "application/msword", "application/rtf", "application/x-abiword", "application/x-applix-word", "application/wordperfect5.1", "appplication/vnd.palm", "text/abiword", "text/plain", "text/vnd.wap.wml");
}

/*****************************************************************/
/* Implements the Bonobo/Print:1.0 Interface */
/*****************************************************************/

static void
print_document (GnomePrintContext         *ctx,
		double                     inWidth,
		double                     inHeight,
		const Bonobo_PrintScissor *opt_scissor,
		gpointer                   user_data)
{
  //fprintf ( bonobo_logfile, "DOM: printing!!\n" ) ;
  //fflush ( bonobo_logfile ) ;

  // assert pre-conditions
  g_return_if_fail (user_data != NULL);
  g_return_if_fail (IS_ABI_WIDGET (user_data));

  // get me!
  AbiWidget * abi = ABI_WIDGET(user_data);

  // get our frame
  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail(pFrame != NULL);

  // get our current view so we can get the document being worked on
  FV_View * pView = (FV_View*) pFrame->getCurrentView();
  UT_return_if_fail(pView!=NULL);

  // get the current document
  PD_Document * pDoc = pView->getDocument () ;
  UT_return_if_fail(pDoc!=NULL);

  //fprintf ( bonobo_logfile, "DOM: past round #1\n" ) ;
  //fflush ( bonobo_logfile ) ;

  // get the current app
  XAP_UnixGnomeApp * pApp = (XAP_UnixGnomeApp*) XAP_App::getApp () ;

  // create a graphics drawing class
  GR_Graphics *pGraphics = new XAP_UnixGnomePrintGraphics ( ctx, pApp->getFontManager(), pApp ) ;
  UT_return_if_fail(pGraphics!=NULL);

  //fprintf ( bonobo_logfile, "DOM: created gfx\n" ) ;
  //fflush ( bonobo_logfile ) ;

  // layout the document
  FL_DocLayout * pDocLayout = new FL_DocLayout(pDoc,pGraphics);
  UT_ASSERT(pDocLayout);
  
  // create a new printing view of the document
  FV_View * pPrintView = new FV_View(pFrame->getApp(),pFrame,pDocLayout);
  UT_ASSERT(pPrintView);

  // fill the layouts
  pDocLayout->fillLayouts();

  //fprintf ( bonobo_logfile, "DOM: got view, filled layouts\n" ) ;
  //fflush ( bonobo_logfile ) ;

  // get the best fit width & height of the printed pages
  UT_sint32 iWidth  =  pDocLayout->getWidth();
  UT_sint32 iHeight =  pDocLayout->getHeight();
  UT_sint32 iPages  = pDocLayout->countPages();
  UT_uint32 width =  MIN(iWidth, inWidth);
  UT_uint32 height = MIN(iHeight, inHeight);

  // figure out roughly how many pages to print
  UT_sint32 iPagesToPrint = (UT_sint32) (height/pDoc->m_docPageSize.Height(DIM_PT));
  if (iPagesToPrint < 1)
    iPagesToPrint = 1;

  //fprintf ( bonobo_logfile, "DOM: %g\n", pDoc->m_docPageSize.Height(DIM_PT) ) ;
  //fprintf ( bonobo_logfile, "DOM: printing 0x%X 0x%X 0x%X bonobo_printed_document 1 false %d %d 1 %d\n", pDoc, pGraphics, pPrintView, width, height, iPagesToPrint ) ;
  //fflush ( bonobo_logfile ) ;

  // actually print
  s_actuallyPrint ( pDoc, pGraphics,
		    pPrintView, "bonobo_printed_document",
		    1, false,
		    width, height,
		    1, iPagesToPrint ) ;
  
  // clean up
  DELETEP(pGraphics);
  DELETEP(pPrintView);
  DELETEP(pDocLayout);

  return;
}

/*****************************************************************/
/* Implements the Bonobo/Zoomable:1.0 Interface */
/*****************************************************************/

// increment/decrement zoom percentages by this amount
#define ZOOM_PCTG 10

static void zoom_level_func(GObject * z, float lvl, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  //fprintf ( bonobo_logfile, "DOM: zooming to %g!!\n", lvl ) ;
  //fflush ( bonobo_logfile ) ;

  if ( lvl <= 0.0 )
    return ;

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  pFrame->setZoomType (XAP_Frame::z_PERCENT);
  pFrame->setZoomPercentage ((UT_uint32)lvl);
}

static void zoom_in_func(GObject * z, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  //fprintf ( bonobo_logfile, "DOM: zooming in!!\n" ) ;
  //fflush ( bonobo_logfile ) ;

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  UT_sint32 zoom_lvl = pFrame->getZoomPercentage();
  zoom_lvl += ZOOM_PCTG ;

  pFrame->setZoomType (XAP_Frame::z_PERCENT);
  pFrame->setZoomPercentage (zoom_lvl);  
}

static void zoom_out_func(GObject * z, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  //fprintf ( bonobo_logfile, "DOM: zooming out!!\n" ) ;
  //fflush ( bonobo_logfile ) ;

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  UT_sint32 zoom_lvl = pFrame->getZoomPercentage();
  zoom_lvl -= ZOOM_PCTG ;

  if ( zoom_lvl <= 0 )
    return ;

  pFrame->setZoomType (XAP_Frame::z_PERCENT);
  pFrame->setZoomPercentage (zoom_lvl);  
}

static void zoom_to_fit_func(GObject * z, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  //fprintf ( bonobo_logfile, "DOM: zooming to fit!!\n" ) ;
  //fflush ( bonobo_logfile ) ;

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  FV_View * pView = (FV_View*) pFrame->getCurrentView();
  UT_return_if_fail(pView!=NULL);

  UT_uint32 newZoom = pView->calculateZoomPercentForWholePage();
  pFrame->setZoomType( XAP_Frame::z_WHOLEPAGE );
  pFrame->setZoomPercentage(newZoom);
}

static void zoom_to_default_func(GObject * z, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  //fprintf ( bonobo_logfile, "DOM: zooming default!!\n" ) ;
  //fflush ( bonobo_logfile ) ;

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  pFrame->setZoomType (XAP_Frame::z_100);
  pFrame->setZoomPercentage (100);  
}

/*****************************************************************/
/* Bonobo Inteface-Adding Code */
/*****************************************************************/

static BonoboView *
bonobo_AbiWidget_view_factory (BonoboEmbeddable      *embeddable,
			       const Bonobo_ViewFrame view_frame,
			       void                  *closure)
{
	BonoboView *view = bonobo_view_new (GTK_WIDGET(closure));

	bonobo_view_set_view_frame (view, view_frame);

	return view;
}

//
// Add extra interfaces to load data into the control
//
BonoboObject *
AbiControl_add_interfaces (AbiWidget *abiwidget,
			   BonoboObject *to_aggregate)
{
	BonoboPersistFile   *file;
	BonoboPersistStream *stream;
	BonoboPrint         *printer;
	BonoboItemContainer *item_container;
	BonoboZoomable      *zoomable;
	BonoboEmbeddable    *embeddable;

	g_return_val_if_fail (IS_ABI_WIDGET(abiwidget), NULL);
	g_return_val_if_fail (BONOBO_IS_OBJECT (to_aggregate), NULL);

	/* Interface Bonobo::PersistStream */

	stream = bonobo_persist_stream_new (load_document_from_stream, 
					    save_document_to_stream, NULL, pstream_get_content_types, abiwidget);
	if (!stream) {
		bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
				     BONOBO_OBJECT (stream));

	/* Interface Bonobo::PersistFile */

	file = bonobo_persist_file_new (load_document_from_file,
					save_document_to_file, 
					abiwidget);
	if (!file) 
	{
		bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
				     BONOBO_OBJECT (file));
	
	/* Interface Bonobo::Print */

	printer = bonobo_print_new (print_document, abiwidget);
	if (!printer) {
	  bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
	  return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
				     BONOBO_OBJECT (printer));

	/* Interface Bonobo/ItemContainer */

	item_container = bonobo_item_container_new ();

	g_signal_connect (G_OBJECT (item_container),
			  "get_object",
			  G_CALLBACK (abiwidget_get_object),
			  abiwidget);
	
	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
				     BONOBO_OBJECT (item_container));

	/* Interface Bonobo::Embeddable */

	embeddable = bonobo_embeddable_new (bonobo_AbiWidget_view_factory, 
					    abiwidget);
	
	// now advertise that we implement the embeddable interface
	
	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
				     BONOBO_OBJECT (embeddable));

	/* Interface Bonobo::Zoomable */

	zoomable = bonobo_zoomable_new () ;
	if ( !zoomable ) {
	  bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
	  return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
				     BONOBO_OBJECT (zoomable));

	g_signal_connect(G_OBJECT(zoomable), "zoom_in",
			 G_CALLBACK(zoom_in_func), abiwidget);
	g_signal_connect(G_OBJECT(zoomable), "zoom_out",
			 G_CALLBACK(zoom_out_func), abiwidget);
	g_signal_connect(G_OBJECT(zoomable), "zoom_to_fit",
			 G_CALLBACK(zoom_to_fit_func), abiwidget);
	g_signal_connect(G_OBJECT(zoomable), "zoom_to_default",
			 G_CALLBACK(zoom_to_default_func), abiwidget);
	g_signal_connect(G_OBJECT(zoomable), "set_zoom_level",
			 G_CALLBACK(zoom_level_func), abiwidget);

	return to_aggregate;
}

static BonoboControl* 	AbiControl_construct(BonoboControl * control, AbiWidget * abi)
{
	BonoboPropertyBag 	*prop_bag;
	g_return_val_if_fail(abi != NULL, NULL);
	g_return_val_if_fail(control != NULL, NULL);
	g_return_val_if_fail(IS_ABI_WIDGET(abi), NULL);

	/* 
	 * create a property bag:
	 * we provide our accessor functions for properties, and 
	 * the gtk widget
	 */
	prop_bag = bonobo_property_bag_new (get_prop, set_prop, abi);
	bonobo_control_set_properties (control, prop_bag);

	// put all AbiWidget's arguments in the property bag - way cool!!
  
	bonobo_property_bag_add_gtk_args (prop_bag,G_OBJECT(abi));

	// now advertise that we implement the property-bag interface
	bonobo_object_add_interface (BONOBO_OBJECT (control),
				     BONOBO_OBJECT (prop_bag));

	//
	// persist_stream , persist_file interfaces/methods, item container
	// 
	
	AbiControl_add_interfaces (ABI_WIDGET(abi),
				   BONOBO_OBJECT(control));
	/*
	 *  we don't need the property bag anymore here, so unref it
	 */
	
	bonobo_object_unref (BONOBO_OBJECT(prop_bag));
	return control;
}

static BonoboControl * AbiWidget_control_new(AbiWidget * abi)
{
    g_return_val_if_fail(abi != NULL, NULL);
    g_return_val_if_fail(IS_ABI_WIDGET(abi), NULL);

    // create a BonoboControl from a widget
    BonoboControl * control = bonobo_control_new (GTK_WIDGET(abi));
    control = AbiControl_construct(control, abi);
    
    return control;
}

/*
 *  produce a brand new bonobo_AbiWord_control
 *  (this is a callback function, registered in 
 *  	'bonobo_generic_factory_new')
 */
static BonoboObject*
bonobo_AbiWidget_factory  (BonoboGenericFactory *factory, void *closure)
{
  BonoboControl    * control;
  GtkWidget        * abi;
  
  /*
   * create a new AbiWidget instance
   */
  
  AP_UnixApp * pApp = (AP_UnixApp *) XAP_App::getApp();
  abi = abi_widget_new_with_app (pApp);
  gtk_widget_show (abi);
  
  // create a BonoboControl from a widget
  
  control = AbiWidget_control_new (ABI_WIDGET(abi));

  return BONOBO_OBJECT (control);
}

static int mainBonobo(int argc, char * argv[])
{
	BonoboGenericFactory 	*factory;
	CORBA_ORB 		 orb;

	bonobo_logfile = stdout ; // fopen ( "/home/dom/abictl.log", "w" ) ;
	//fprintf ( bonobo_logfile, "Opened log file\n" ) ;
	//fflush ( bonobo_logfile ) ;	

	/*
	 * initialize oaf and bonobo
	 */
	orb = oaf_init (argc, argv);
	if (!orb)
	  {
	    printf ("initializing orb failed \n");
	    return -1 ;
	  }

	if (!bonobo_init (orb, NULL, NULL))
	  {
	    printf("initializing Bonobo failed \n");
	    return -1;
	  }

	/* register the factory (using OAF) */
	factory = bonobo_generic_factory_new
		("OAFIID:GNOME_AbiWord_ControlFactory",
		 bonobo_AbiWidget_factory, NULL);

	if (!factory)
	  {
	    printf("Registration of Bonobo generic factory failed");
	    return -1;
	  }
	
	/*
	 *  make sure we're unreffing upon exit;
	 *  enter the bonobo main loop
	 */
	bonobo_running_context_auto_exit_unref (BONOBO_OBJECT(factory));
	bonobo_main ();

	fclose ( bonobo_logfile ) ;

	return 0;
}


static int NautilusMain(int argc, char *argv[])
{
	int ires = 0;

#if 0
	ires = nautilus_view_standard_main ("abiword",
					    "1.0.6",
					    NULL,	/* Could be PACKAGE */
					    NULL,	/* Could be GNOMELOCALEDIR */
					    argc,
					    argv,
					    "OAFIID:nautilus_abiword_view_factory",
					    "OAFIID:nautilus_abiword_view",
					    nautilus_view_create_from_get_type_function,
					    NULL,
					    (void *)nautilus_abiword_content_view_get_type);
#endif
	return ires;
}
#endif /* OPT_BONOBO */
