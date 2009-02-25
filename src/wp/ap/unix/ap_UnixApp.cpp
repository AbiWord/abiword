/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "ut_debugmsg.h"
#include "ut_path.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_locale.h"
#ifdef ABI_OPT_PERL
  #include "ut_PerlBindings.h"
  #include "ut_Script.h"
#endif
#include "ut_unixDirent.h"
#include "ut_sleep.h"

#include "xap_Args.h"
#include "ap_Args.h"
#include "ap_Convert.h"
#include "ap_UnixFrame.h"
#include "ap_UnixApp.h"

#ifdef ENABLE_SPELL
  #include "spell_manager.h"
#endif

#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "ap_UnixStockIcons.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Dialog_Id.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Menu_Layouts.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_Toolbar_Layouts.h"
#include "xav_View.h"
#include <gdk/gdk.h>
#include "gr_Graphics.h"
#include "gr_Image.h"
#include "gr_UnixImage.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "xap_UnixDialogHelper.h"
#include "ut_iconv.h"
#include "fv_View.h"
#include "fp_Run.h"

#include "ut_string_class.h"
#include "xap_EncodingManager.h"

#include "ie_types.h"

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
#include "ie_imp_XHTML.h"
#include "gr_DrawArgs.h"

#include "xap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"
#include "gr_Image.h"

#include "abiwidget.h"
#include "ut_sleep.h"
#include "gr_Painter.h"
#include "ap_Preview_Abi.h"
#include "xap_UnixDialogHelper.h"
#include "gr_UnixPangoGraphics.h"
#include "gr_UnixPangoPixmapGraphics.h"

#ifdef ENABLE_BINRELOC
  #include "prefix.h"
#endif // ENABLE_BINRELOC


#ifdef GTK_WIN_POS_CENTER_ALWAYS
#define WIN_POS GTK_WIN_POS_CENTER_ALWAYS
#else
#define WIN_POS GTK_WIN_POS_CENTER
#endif

#include <popt.h>
#include "ie_impGraphic.h"
#include "ut_math.h"

#ifdef ENABLE_PRINT
  #include <libart_lgpl/art_affine.h>
  #include <libgnomeprint/gnome-print.h>
#endif

#ifdef WITH_GNOMEUI
  #include <libgnome/libgnome.h>
  #include <libgnomeui/libgnomeui.h>
#endif

#ifdef LOGFILE
static FILE * logfile;
extern FILE * getlogfile(void)
{
	return logfile;
}
#endif
// quick hack - this is defined in ap_EditMethods.cpp
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);

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
	  m_cacheDeferClear(0),
	  m_pViewSelection(0),
	  m_cacheSelectionView(0),
	  m_pFrameSelection(0)
{
    // hack to link abi_widget - thanks fjf
	if(this == 0)
		/*GtkWidget * pUn =*/ abi_widget_new_with_file("fred.abw");
}

/*!
  Destructor for AP_UnixApp's.  Cleans up spellcheck, clipboard, and
	StringSet.  
*/
AP_UnixApp::~AP_UnixApp(void)
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
#ifdef LOGFILE
	fprintf(getlogfile(),"New Directory created \n");
#endif
   
    if (mkdir(szDir,0700) == 0)
		return true;
    
    
    UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
    return false;
}	

/*!
* Try loading a string-set.
* \param szStringSet Language id, e.g. de_AT
* \param pDefaultStringSet String set to be used for untranslated strings.
* \return AP_DiskStringSet * on success, NULL if not found
*/
AP_DiskStringSet * 
AP_UnixApp::loadStringsFromDisk(const char 			* szStringSet, 
								AP_BuiltinStringSet * pDefaultStringSet)
{
	UT_ASSERT(pDefaultStringSet);

	const char * szDirectory = NULL;
	getPrefsValueDirectory(true,
			       static_cast<const gchar*>(AP_PREF_KEY_StringSetDirectory),
			       static_cast<const gchar**>(&szDirectory));
	UT_return_val_if_fail((szDirectory) && (*szDirectory), NULL);

	// fo_BA.strings
	UT_String szPath = szDirectory;
	if (szDirectory[szPath.size()-1]!='/')
		szPath += "/";
	szPath += szStringSet;
	szPath += ".strings";

	// fo.strings
	UT_String szFallbackPath;
	if (szStringSet && strlen(szStringSet) > 2) {
		szFallbackPath = szDirectory;
		if (szDirectory[szFallbackPath.size()-1]!='/')
			szFallbackPath += "/";
		szFallbackPath += szStringSet[0];
		szFallbackPath += szStringSet[1];
		szFallbackPath += ".strings";
	}

	AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);
	if (pDiskStringSet->loadStringsFromDisk(szPath.c_str()))
	{
		pDiskStringSet->setFallbackStringSet(pDefaultStringSet);
		UT_DEBUGMSG(("Using StringSet [%s]\n",szPath.c_str()));
		return pDiskStringSet;
	}
	else if (szFallbackPath.size() && pDiskStringSet->loadStringsFromDisk(szFallbackPath.c_str())) 
	{
		pDiskStringSet->setFallbackStringSet(pDefaultStringSet);
		UT_DEBUGMSG(("Using StringSet [%s]\n",szFallbackPath.c_str()));
		return pDiskStringSet;
	}
	else
	{
		DELETEP(pDiskStringSet);
		UT_DEBUGMSG(("Unable to load StringSet [%s] -- using builtin strings instead.\n",szPath.c_str()));
		return NULL;
	}
}

/*!
  Initialize the application.  This involves preferences, keybindings,
  toolbars, graphics, spelling and everything else.  
  \return True if successfully initalized, False otherwise. if false
  the app is unusable, and loading should not continue.   
  \bug This function is 136 lines - way too long.  Needs to be
  refactored, to use a buzzword.  
*/
bool AP_UnixApp::initialize(bool has_display)
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
		// Loading default string set for untranslated messages
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this, 
													static_cast<const gchar*>(AP_PREF_DEFAULT_StringSet));
		UT_ASSERT(pBuiltinStringSet);

		// try loading strings by preference
		const char * szStringSet = NULL;
		if (   (getPrefsValue(AP_PREF_KEY_StringSet,
							  static_cast<const gchar**>(&szStringSet)))
			   && (szStringSet)
			   && (*szStringSet)
			   && (strcmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
			m_pStringSet = loadStringsFromDisk(szStringSet, pBuiltinStringSet);
		}

		// try loading fallback strings for the language, e.g. es-ES for es-AR
		if (m_pStringSet == NULL) 
		{
			const char *szFallbackStringSet = UT_getFallBackStringSetLocale(szStringSet);
			m_pStringSet = loadStringsFromDisk(szFallbackStringSet, pBuiltinStringSet);
		}

		// load the builtin string set
		// this is the default
		if (m_pStringSet == NULL) 
		{
			m_pStringSet = pBuiltinStringSet;
		}
    }

    // now that preferences are established, let the xap init
	if (has_display) {	   
		m_pClipboard = new AP_UnixClipboard(this);
		UT_ASSERT(m_pClipboard);
		m_pClipboard->initialize();

		abi_stock_init ();
    }

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
	if (getPrefsValue( AP_PREF_KEY_StringSet, static_cast<const gchar**>(&szMenuLabelSetName))
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
    UT_ScriptLibrary * instance = UT_ScriptLibrary::instance(); 
    instance->registerScript ( new UT_PerlScriptSniffer () );
#endif

    return true;
}

/*!
  Create a new frame based on the current one.  
  \return A pointer to the new frame.  
*/
XAP_Frame * AP_UnixApp::newFrame(void)
{
    AP_UnixFrame * pUnixFrame = new AP_UnixFrame();

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
// Save our toolbars if we're customizable
// feature is off, the GtkToolItem based toolbars don't support that
#if 0
	if(areToolbarsCustomizable())
	{
		m_pToolbarFactory->saveToolbarsInCurrentScheme();
	}
#endif
	if(!isBonoboRunning())
	{
		if (m_prefs->getAutoSavePrefs())
			m_prefs->savePrefsFile();
	}
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
bool AP_UnixApp::getPrefsValueDirectory(bool bAppSpecific,
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
    return getAbiSuiteLibDir();
}

/*!
  This returns the directory that holds the application .glade files.
  \return A const string containting the directory path
*/
const char * AP_UnixApp::getAbiSuiteAppGladeDir(void) const
{
	static const gchar *dir = NULL;

	if (!dir) {
		UT_UTF8String s ("");
		s += getAbiSuiteLibDir();
		s += "/glade";
		dir = g_strdup (s.utf8_str ());
	}
	
    return dir;
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

#define CLIPBOARD_IS_HTML4 true

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
    UT_ByteBuf bufHTML4;
    UT_ByteBuf bufXHTML;
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
	if (pExpHtml)
		{
			pExpHtml->set_HTML4 (false);
			pExpHtml->copyToBuffer (pDocRange, &bufXHTML);
			DELETEP(pExpHtml);
		}

	// create HTML4 buffer to put on the clipboard
	
	pExpHtml = new IE_Exp_HTML(pDocRange->m_pDoc);
	if (pExpHtml)
		{
			pExpHtml->set_HTML4 (true);
			pExpHtml->copyToBuffer(pDocRange, &bufHTML4);
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

	if (bufRTF.getLength () > 0)
		m_pClipboard->addRichTextData (target, bufRTF.getPointer (0), bufRTF.getLength ());
	if (bufXHTML.getLength () > 0)
		m_pClipboard->addHtmlData (target, bufXHTML.getPointer (0), bufXHTML.getLength (), true);
	if (bufHTML4.getLength () > 0)
		m_pClipboard->addHtmlData (target, bufHTML4.getPointer (0), bufHTML4.getLength (), false);
	if (bufTEXT.getLength () > 0)
		m_pClipboard->addTextData (target, bufTEXT.getPointer (0), bufTEXT.getLength ());

	{
		// TODO: we have to make a good way to tell if the current selection is just an image
		FV_View * pView = NULL;
		if(getLastFocussedFrame())
			pView = static_cast<FV_View*>(getLastFocussedFrame()->getCurrentView());

		if (pView && !pView->isSelectionEmpty())
			{
				// don't own, don't g_free
				const UT_ByteBuf * png = 0;
	  
				pView->saveSelectedImage (&png);
				if (png && png->getLength() > 0)
					{
						m_pClipboard->addPNGData(target, static_cast<const UT_Byte*>(png->getPointer(0)), png->getLength());
					}
			}
    }

	m_pClipboard->finishedAddingData();

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
	bool bSuccess = false;
    
    if ( bHonorFormatting )
      bFoundOne = m_pClipboard->getSupportedData(tFrom,reinterpret_cast<const void **>(&pData),&iLen,&szFormatFound);
    else
      bFoundOne = m_pClipboard->getTextData(tFrom,reinterpret_cast<const void **>(&pData),&iLen, &szFormatFound);

    if (!bFoundOne)
    {
		UT_DEBUGMSG(("PasteFromClipboard: did not find anything to paste.\n"));
		return;
    }

    if (AP_UnixClipboard::isRichTextTag(szFormatFound))
    {
		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		bSuccess = pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);
    }
	else if (AP_UnixClipboard::isHTMLTag (szFormatFound))
	{
		IE_Imp_Text_Sniffer SniffBuf;
		const char * szRes = SniffBuf.recognizeContentsType(reinterpret_cast<const char *>(pData),iLen);
		if(strcmp(szRes,"none") != 0)
		{
			UT_uint32 iread,iwritten = 0;
			const char * szutf8= static_cast<const char *>(UT_convert(reinterpret_cast<const char *>(pData),iLen,szRes,"UTF-8",&iread,&iwritten));
			IE_Imp_XHTML * pImpHTML = new IE_Imp_XHTML(pDocRange->m_pDoc);
			bSuccess = pImpHTML->pasteFromBuffer(pDocRange,reinterpret_cast<const unsigned char *>(szutf8),iwritten,"UTF-8");
			g_free(const_cast<char *>(szutf8));
			DELETEP(pImpHTML);
		}
		else
		{
			IE_Imp_XHTML * pImpHTML = new IE_Imp_XHTML(pDocRange->m_pDoc);
			bSuccess = pImpHTML->pasteFromBuffer(pDocRange,reinterpret_cast<const unsigned char *>(pData),iLen);
			DELETEP(pImpHTML);
		}
	}
	else if (AP_UnixClipboard::isDynamicTag (szFormatFound))
	{
		UT_DEBUGMSG(("Format Found = %s \n",szFormatFound));
		IE_Imp * pImp = NULL;
		IEFileType ieft = IE_Imp::fileTypeForMimetype(szFormatFound);
		UT_DEBUGMSG(("found file type %d\n",ieft));
		IE_Imp::constructImporter(pDocRange->m_pDoc,ieft,&pImp);
		if(pImp == NULL)
			 goto retry_text;
		bSuccess = pImp->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImp);
	}
    else if (AP_UnixClipboard::isImageTag(szFormatFound))
      {
		  UT_DEBUGMSG(("Format Found = %s \n",szFormatFound));
		  if(strncmp(szFormatFound,"application",11) == 0) // embedded object
		  {
			  IE_Imp * pImp = NULL;
			  IEGraphicFileType iegft = IE_Imp::fileTypeForMimetype(szFormatFound);
			  IE_Imp::constructImporter(pDocRange->m_pDoc,iegft,&pImp);
			  if(pImp == NULL)
			  {
					  goto retry_text;
			  }
			  /*bool b = */ pImp->pasteFromBuffer(pDocRange,pData,iLen);
			  DELETEP(pImp);
			  return;
		  }

		  FG_Graphic * pFG = NULL;
		  IEGraphicFileType iegft = IEGFT_Unknown;
		  UT_Error error = UT_OK;
		  
		  UT_ByteBuf * bytes = new UT_ByteBuf( iLen );
		  
		  bytes->append (pData, iLen);
		  
		  error = IE_ImpGraphic::loadGraphic(bytes, iegft, &pFG);
		  if(!pFG || error)
		  {
			  UT_DEBUGMSG(("DOM: could not import graphic (%d)\n", error));
			  DELETEP(bytes);
			  goto retry_text;
		  }
		  
		  // at this point, 'bytes' is owned by pFG
		  FV_View * pView = static_cast<FV_View*>(getLastFocussedFrame ()->getCurrentView());
		  
		  error = pView->cmdInsertGraphic(pFG);
		  DELETEP(pFG);
		  if (!error)
			  bSuccess = true;
      }
    else // ( AP_UnixClipboard::isTextTag(szFormatFound) )
    {
		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc,"UTF-8");
		bSuccess = pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);
    }

 retry_text:

	// we failed to paste *anything.* try plaintext as a last-ditch effort
	if(!bSuccess && m_pClipboard->getTextData(tFrom,reinterpret_cast<const void **>(&pData),&iLen, &szFormatFound)) {
		UT_DEBUGMSG(("DOM: pasting text as an absolute fallback (bug 7666)\n"));		

		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc,"UTF-8");
		bSuccess = pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);
	}
}

bool AP_UnixApp::canPasteFromClipboard(void)
{
    return m_pClipboard->canPaste(XAP_UnixClipboard::TAG_ClipboardOnly);
}

extern "C" {

	// return > 0 for directory entries ending in ".so" ".sl" and the like
	static int so_only (ABI_SCANDIR_SELECT_QUALIFIER struct dirent *d)
	{
		const char * name = d->d_name;
		
		if ( name )
		{
			int len = strlen (name);
			
			if (len >= 3)
			{
				if(!strcmp(name+(len-3), "."G_MODULE_SUFFIX))
					return 1;
			}
		}
		return 0;
	}
} // extern "C" block

#ifdef ABI_PLUGIN_BUILTIN
extern void abipgn_builtin_register ();
#endif

void AP_UnixApp::loadAllPlugins ()
{
#ifdef ABI_PLUGIN_BUILTIN
  UT_DEBUGMSG(("load preloaded plugins:\n"));
  abipgn_builtin_register ();
  UT_DEBUGMSG(("finished loading preloaded plugins.\n"));
#endif

  struct dirent **namelist;
  int n = 0;

  UT_String pluginList[2];
  UT_String pluginDir;

  // the global plugin directory
  // TODO Rob: re-enable binreloc
  pluginDir += ABIWORD_PLUGINSDIR "/";

  UT_DEBUGMSG(("pluginDir: '%s'\n", pluginDir.c_str ()));

  pluginList[0] = pluginDir;

  // the user-local plugin directory
  pluginDir = getUserPrivateDirectory ();
  pluginDir += "/" PACKAGE_NAME "/plugins/";
  UT_DEBUGMSG (("ROB: private plugins in '%s'\n", pluginDir.c_str ()));
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
				  g_free(namelist[n]);
				  continue;
			  }
			  if(strcmp (namelist[n]->d_name+(len-3), "."G_MODULE_SUFFIX) != 0)
			  {
				  UT_DEBUGMSG(("FJF: not really a plugin?\n"));
				  g_free(namelist[n]);
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
			  g_free(namelist[n]);
		  }
		  g_free(namelist);
      }
  }
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
		static_cast<FV_View *>(m_pViewSelection)->cmdUnselectSelection();
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
    m_pFrameSelection = static_cast<XAP_Frame *>(pView->getParentData());

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

		if ( AP_UnixClipboard::isHTMLTag(formatList[j]) )
		{
			IE_Exp_HTML * pExpHTML = new IE_Exp_HTML(dr.m_pDoc);
			if (!pExpHTML)
				return false;

			pExpHTML->set_HTML4 (!strcmp (formatList[j], "text/html"));
			pExpHTML->copyToBuffer(&dr,&m_selectionByteBuf);
			DELETEP(pExpHTML);
			goto ReturnThisBuffer;
		}

		if ( AP_UnixClipboard::isImageTag(formatList[j]) )
		{
			// TODO: we have to make a good way to tell if the current selection is just an image
			FV_View * pView = NULL;
			if(getLastFocussedFrame())
				pView = static_cast<FV_View*>(getLastFocussedFrame()->getCurrentView());

			if (pView && !pView->isSelectionEmpty())
				{
					// don't own, don't g_free
					const UT_ByteBuf * png = 0;
	  
					pView->saveSelectedImage (&png);
					if (png && png->getLength() > 0)
						{
							m_selectionByteBuf.ins (0, png->getPointer (0), png->getLength ());
							goto ReturnThisBuffer;
						}
				}
		}
			
		if ( AP_UnixClipboard::isTextTag(formatList[j]) )
		{
			IE_Exp_Text * pExpText = new IE_Exp_Text(dr.m_pDoc, "UTF-8");
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
    *ppData = const_cast<void *>(static_cast<const void *>(m_selectionByteBuf.getPointer(0)));
    *pLen = m_selectionByteBuf.getLength();
    *pszFormatFound = formatList[j];
    return true;
}

bool AP_UnixApp:: makePngPreview(const char * pszInFile, const char * pszPNGFile, UT_sint32 iWidth, UT_sint32 iHeight)
{
	GdkPixmap*  pPixmap = gdk_pixmap_new(NULL,iWidth,iHeight,24);

	GR_UnixPixmapAllocInfo ai(pPixmap);

	GR_UnixPangoPixmapGraphics * pG = (GR_UnixPangoPixmapGraphics*) GR_UnixPangoPixmapGraphics::graphicsAllocator(ai);

	UT_Error error = UT_OK;
	PD_Document * pNewDoc = new PD_Document(this);
	error = pNewDoc->readFromFile(pszInFile,IEFT_Unknown, NULL);

	if (error != UT_OK) 
	{
		return false;
	}
	AP_Preview_Abi * pPrevAbi = new AP_Preview_Abi(pG,iWidth,iHeight,NULL, PREVIEW_ZOOMED,pNewDoc);
	dg_DrawArgs da;
	memset(&da, 0, sizeof(da));
	da.pG = pG;
	GR_Painter * pPaint = new GR_Painter(pG);
	pPaint->clearArea(0,0,pG->tlu(iWidth),pG->tlu(iHeight));
	pPrevAbi->getView()->draw(0, &da);
	UT_Rect r;
	r.left = 0;
	r.top = 0;
	r.width = pG->tlu(iWidth);
	r.height = pG->tlu(iHeight);
	GR_Image * pImage = pPaint->genImageFromRectangle(r);
	DELETEP(pPaint);
	static_cast<GR_UnixImage *>(pImage)->saveToPNG( pszPNGFile);
	DELETEP(pImage);
	DELETEP(pG);
	DELETEP(pPrevAbi); // This deletes pNewDoc
	return true;
}

/*****************************************************************/
/*****************************************************************/
GR_Graphics * AP_UnixApp::newDefaultScreenGraphics() const
{
	XAP_Frame * pFrame = findValidFrame();
	UT_return_val_if_fail( pFrame, NULL );
	UT_DEBUGMSG(("AP_UnixApp::newDefaultScreenGraphics() \n"));
	AP_UnixFrameImpl * pFI = static_cast<AP_UnixFrameImpl *>(pFrame->getFrameImpl());
	UT_return_val_if_fail( pFI, NULL );

	GtkWidget * da = pFI->getDrawingArea();
	UT_return_val_if_fail( da, NULL );
	
	GR_UnixAllocInfo ai(da->window);
	return XAP_App::getApp()->newGraphics(ai);
}


/*****************************************************************/

int AP_UnixApp::main(const char * szAppName, int argc, const char ** argv)
{
    // This is a static function.
    if (!g_thread_supported ())
        g_thread_init (NULL);

    // initialize our application.
	XAP_Args XArgs = XAP_Args(argc,argv);
	AP_UnixApp * pMyUnixApp = new AP_UnixApp(&XArgs, szAppName);

	int exit_status = 0;

	/* this brace is here to ensure that our local variables on the stack
	 * do not outlive the application object by giving them a lower scope
	 */
	{
		AP_Args Args = AP_Args(&XArgs, szAppName, pMyUnixApp);

#ifdef LOGFILE
		UT_String sLogFile = pMyUnixApp->getUserPrivateDirectory();
//	UT_String sLogFile = "/home/msevior/.AbiSuite";
		sLogFile += "abiLogFile";
		logfile = fopen(sLogFile.c_str(),"a+");
		fprintf(logfile,"About to do gtk_set_locale \n");
		fprintf(logfile,"New logfile \n");
#endif
    
		// Step 1: Initialize GTK and create the APP.
		// hack needed to intialize gtk before ::initialize
		gtk_set_locale();

		gboolean have_display = gtk_init_check(&XArgs.m_argc,const_cast<char ***>(&XArgs.m_argv));

#ifdef LOGFILE
		fprintf(logfile,"Got display %d \n",have_display);
		fprintf(logfile,"Really display %d \n",have_display);
#endif

		if (have_display > 0) {
#ifndef WITH_GNOMEUI
			gtk_init (&XArgs.m_argc,const_cast<char ***>(&XArgs.m_argv));
			Args.parsePoptOpts();
#else
#ifdef LOGFILE
			fprintf(logfile,"About to start gnome_program_init \n");
#endif
			GnomeProgram * program = gnome_program_init (PACKAGE, VERSION, 
														 LIBGNOMEUI_MODULE, XArgs.m_argc, const_cast<char **>(XArgs.m_argv),
														 GNOME_PARAM_APP_PREFIX, PREFIX,
														 GNOME_PARAM_APP_SYSCONFDIR, SYSCONFDIR,
														 GNOME_PARAM_APP_DATADIR,	PREFIX "/" PACKAGE "-" ABIWORD_SERIES,
														 GNOME_PARAM_APP_LIBDIR, PREFIX "/" PACKAGE "-" ABIWORD_SERIES,
														 GNOME_PARAM_POPT_TABLE, AP_Args::options, 
														 GNOME_PARAM_NONE);
#ifdef LOGFILE
			fprintf(logfile,"gnome_program_init completed \n");
#endif

			g_object_get (G_OBJECT (program),
						  GNOME_PARAM_POPT_CONTEXT, &Args.poptcon,
						  NULL);
#ifdef LOGFILE
			fprintf(logfile,"g_object_get completed \n");
#endif

			// GNOME handles 'parsePoptOpts'.  Isn't it grand?
#endif
		}
		else {
			// no display, but we still need to at least parse our own arguments, damnit, for --to, --to-png, and --print
			Args.parsePoptOpts();
		}

		// if the initialize fails, we don't have icons, fonts, etc.
		if (!pMyUnixApp->initialize(have_display))
		{
			delete pMyUnixApp;
			return -1;	// make this something standard?
		}
	
		// Setup signal handlers, primarily for segfault
		// If we segfaulted before here, we *really* blew it
	
		struct sigaction sa;
	
		sa.sa_handler = signalWrapper;
    
		sigfillset(&sa.sa_mask);  // We don't want to hear about other signals
		sigdelset(&sa.sa_mask, SIGABRT); // But we will call abort(), so we can't ignore that
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
	
		// Step 2: Handle all non-window args.
    
		bool windowlessArgsWereSuccessful = true;
		if (!Args.doWindowlessArgs(windowlessArgsWereSuccessful )) {
			delete pMyUnixApp;
			return (windowlessArgsWereSuccessful ? 0 : -1);
		}

		if (have_display) {

			// Step 3: Create windows as appropriate.
			// if some args are botched, it returns false and we should
			// continue out the door.
			// We used to check for bShowApp here.  It shouldn't be needed
			// anymore, because doWindowlessArgs was supposed to bail already. -PL

			if (pMyUnixApp->openCmdLineFiles(&Args))
			{
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
				s_bInitDone = true;
				pMyUnixApp->processStartupQueue();
#endif
				// turn over control to gtk
				gtk_main();
			}
			else
			{
				UT_DEBUGMSG(("DOM: not parsing command line or showing app\n"));
			}
		}
		else {
			fprintf(stderr, "No DISPLAY: this may not be what you want.\n");
			exit_status = 1;
		}
		// unload all loaded plugins (remove some of the memory leaks shown at shutdown :-)
		XAP_ModuleManager::instance().unloadAllPlugins();

		// Step 4: Destroy the App.  It should take care of deleting all frames.
		pMyUnixApp->shutdown();
	}
	
	delete pMyUnixApp;
	
	return exit_status;
}
	
void AP_UnixApp::errorMsgBadArg(AP_Args * Args, int nextopt)
{
  fprintf (stderr, "Error on option %s: %s.\nRun '%s --help' to see a full list of available command line options.\n",
	  poptBadOption (Args->poptcon, 0),
	  poptStrerror (nextopt),
	  Args->XArgs->m_argv[0]);
}

void AP_UnixApp::errorMsgBadFile(XAP_Frame * pFrame, const char * file, 
				 UT_Error error)
{
  s_CouldNotLoadFileMessage (pFrame, file, error);
}

/*! Prepares for popt to be callable by setting up Args->options.
 * For GNOME, only copies args up to 'version'.
 */
void AP_UnixApp::initPopt (AP_Args * Args)
{
#ifdef WITH_GNOMEUI
	UT_sint32 v = -1, i;

	// stop at --version.
	for (i = 0; Args->const_opts[i].longName != NULL; i++)
		if (!strcmp(Args->const_opts[i].longName, "version"))
		{ 
			v = i; break; 
		}

	if (v == -1)
		v = i;

	struct poptOption * opts = (struct poptOption *)
		UT_calloc(v+1, sizeof(struct poptOption));
	for (UT_sint32 j = 0; j < v; j++)
		opts[j] = Args->const_opts[j];

	Args->options = opts;
#else
	AP_App::initPopt(Args);
#endif
}


#define PMSCALE 0.5
#define PMW (PMSCALE * P_WIDTH)
#define PMH (PMSCALE * P_HEIGHT)

/*!
 * A callback for AP_Args's doWindowlessArgs call which handles
 * platform-specific windowless args.
 */
bool AP_UnixApp::doWindowlessArgs(const AP_Args *Args, bool & bSuccess)
{
	bSuccess = true;

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
		UT_uint32 f = (XAP_UnixApp::GEOMETRY_FLAG_SIZE
					   | XAP_UnixApp::GEOMETRY_FLAG_POS);
		
		// if pos (x and y) weren't provided just use size
		if (x == dummy || y == dummy)
			f = XAP_UnixApp::GEOMETRY_FLAG_SIZE;
		
		// if size (width and height) weren't provided just use pos
		if (width == 0 || height == 0)
			f = XAP_UnixApp::GEOMETRY_FLAG_POS;
		
		// set the xap-level geometry for future frame use
		Args->getApp()->setGeometry(x, y, width, height, f);
	}

 	AP_UnixApp * pMyUnixApp = static_cast<AP_UnixApp*>(Args->getApp());
	if (Args->m_sPrintTo) 
	{
#ifdef ENABLE_PRINT
		if ((Args->m_sFile = poptGetArg (Args->poptcon)) != NULL)
	    {
			AP_Convert conv ;

			if (Args->m_sMerge)
				conv.setMergeSource (Args->m_sMerge);

			if (Args->m_impProps)
				conv.setImpProps (Args->m_impProps);
			if (Args->m_expProps)
				conv.setExpProps (Args->m_expProps);

			conv.setVerbose(Args->m_iVerbose);

			GR_GraphicsFactory * pGF;

			pGF = XAP_App::getApp()->getGraphicsFactory();
			UT_return_val_if_fail(pGF, false);

			//UT_uint32 iDefaultPrintClass = pGF->getDefaultClass(false);		   
			
			GnomePrintJob *job = gnome_print_job_new (NULL);
			UT_return_val_if_fail(job, false);

			GnomePrintConfig *config = gnome_print_job_get_config (job);
			UT_return_val_if_fail(config, false);

			// Args->m_sPrintTo is a printer name, and "-" is our special name for the default printer.
			if(strcmp(Args->m_sPrintTo, "-") != 0) {
				// should we set 'Settings.Transport.Backend.Printer''? It looks deprecated, but maybe GnomePrint's lpr backend uses it...
				gnome_print_config_set(config, reinterpret_cast<const guchar*>("Settings.Transport.Backend.Printer"), 
									   reinterpret_cast<const guchar*>(Args->m_sPrintTo));
				gnome_print_config_set(config, reinterpret_cast<const guchar*>("Printer"), reinterpret_cast<const guchar*>(Args->m_sPrintTo));
			}
			GR_UnixPangoPrintGraphics * print_graphics;

			print_graphics = new GR_UnixPangoPrintGraphics(job);
			bSuccess = conv.print (Args->m_sFile, print_graphics,
								   Args->m_sFileExtension);

			delete print_graphics;
	    }
		else
	    {
			// couldn't load document
			fprintf(stderr, "Error: no file to print!\n");
			bSuccess = false;
	    }
#else
		fprintf(stderr,"Only works in GNOME build \n");
		bSuccess = false;
#endif

		return false;
	}
	if (Args->m_iToThumb > 0) 
	{

#ifdef ENABLE_PRINT

		if ((Args->m_sFile = poptGetArg (Args->poptcon)) != NULL)
	    {
#if 0 // work out how to do this later
			AP_Convert conv ;
			if (Args->m_impProps)
				conv.setImpProps (Args->m_impProps);
			if (Args->m_expProps)
				conv.setExpProps (Args->m_expProps);
			UT_String sdimXY = Args->m_sThumbXY;
			UT_uint32 loc = UT_String_findCh(sdimXY,'x');
			if(loc>(size_t)-10)
			{
				return false;
			}
			UT_String sX = sdimXY.substr(0,loc);
			UT_String sY = sdimXY.substr(loc+1,sdimXY.size());
			UT_sint32 iX = atoi(sX.c_str());
			UT_sint32 iY = atoi(sY.c_str());
			guchar * buf;
			gdouble p2b[6];
			gint bpp = 3;
			art_affine_scale (p2b, 1.0, -1.0);
			p2b[5] = iY; // number of pixels in height
			buf = g_new (guchar, iX * iY * bpp); 

			GnomePrintContext * pc = gnome_print_rbuf_new (buf, iX, iY, bpp * iX, p2b, FALSE);

			PD_Document *pDoc = new PD_Document(this);
			pDoc->readFromFile(Args->m_sFile, IEFT_Unknown, Args->m_impProps);
			double inWidth = pDoc->m_docPageSize.Width(DIM_IN);
			double inHeight = pDoc->m_docPageSize.Height(DIM_IN);
			XAP_UnixGnomePrintGraphics * pGraphics = new XAP_UnixGnomePrintGraphics(pc, inWidth,inHeight);
			conv.setVerbose(Args->m_iVerbose);
			conv.printFirstPage (pGraphics,pDoc);
			UNREFP(pDoc);
			gnome_print_context_close (pc);
			GdkPixbuf* pb = gdk_pixbuf_new_from_data (buf, GDK_COLORSPACE_RGB, 
													  false,
													  8, iX, iY, bpp * iX, 
													  NULL, NULL);
			GError * err;
			gdk_pixbuf_save(pb,Args->m_sName,"png",&err);
#endif
			return true;
	    }
		else
	    {
			// couldn't load document
			fprintf(stderr, "Error: no file to print!\n");
			bSuccess = false;
	    }
		
		return false;
#else
		fprintf(stderr,"Only works in GNOME build \n");
		bSuccess = false;
#endif
	}

	if(Args->m_sPlugin)
	{
//
// Start a plugin rather than the main abiword application.
//
	    const char * szName = NULL;
		XAP_Module * pModule = NULL;
		const char * szRequest = NULL;
		//		szRequest = poptGetArg(Args->poptcon);
		szRequest = Args->m_sPlugin;
		bool bFound = false;	
		if(Args->m_sPlugin != NULL)
		{
			const UT_GenericVector<XAP_Module*> * pVec = XAP_ModuleManager::instance().enumModules ();
			printf(" %d plugins loaded \n",pVec->getItemCount());
			for (UT_uint32 i = 0; (i < pVec->size()) && !bFound; i++)
			{
				pModule = pVec->getNthItem (i);
				szName = pModule->getModuleInfo()->name;
				if(strcmp(szName,szRequest) == 0)
				{
					bFound = true;
				}
			}
		}
		if(!bFound)
		{
			fprintf(stderr, "Plugin %s not found or loaded \n",szRequest);
			bSuccess = false;
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
			fprintf(stderr, "Plugin %s invoke method %s not found \n",
					Args->m_sPlugin,evExecute);
			bSuccess = false;
			return false;
		}
//
// Execute the plugin, then quit
//
		static UT_String sCommandLine;
		sCommandLine.clear();
		//
		// invoked with
		// AbiWord-2.6 --plugin AbiCollab .....
		UT_sint32 iCount =3;
		while(iCount < Args->XArgs->m_argc)
		{
				sCommandLine += Args->XArgs->m_argv[iCount];
				sCommandLine += " ";
				iCount++;
		}
	
		ev_EditMethod_invoke(pInvoke, sCommandLine);
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
    AP_UnixApp *pApp = static_cast<AP_UnixApp *>(XAP_App::getApp());

	/* make sure we have application, in case we have been called after
	 * the application object is gone
	 */
	if (pApp)
		pApp->catchSignals(sig_num);
}

static gint s_signal_count = 0;

/*!
  This function actually handles signals.  The most commonly recieved
  one is SIGSEGV, the segfault signal.  We want to clean up, save the
  user's files to backup locations (currently <filename>.SAVED) and then
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
//
// fixme: Enable this to help debug the bonobo component. After a crash the
// the program hangs here and you can connect to it from gdb and backtrace to
// the crash point
#ifdef LOGFILE
	fprintf(logfile,"abicrashed \n");
	fclose(logfile);
#endif

#if 0
	while(1)
	{
		UT_usleep(10000);
	}
#endif
    UT_uint32 i = 0;
	IEFileType abiType = IE_Imp::fileTypeForSuffix(".abw");
    for(;i<m_vecFrames.getItemCount();i++)
    {
		AP_UnixFrame * curFrame = const_cast<AP_UnixFrame*>(static_cast<const AP_UnixFrame*>(m_vecFrames[i]));
		UT_continue_if_fail(curFrame);
		if (NULL == curFrame->getFilename())
		  curFrame->backup(".abw.SAVED",abiType);
		else
		  curFrame->backup(".SAVED",abiType);
    }
    
    fflush(stdout);
    
    // Abort and dump core
    abort();
}
