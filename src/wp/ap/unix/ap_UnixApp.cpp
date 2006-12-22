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
#include "ut_locale.h"
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
#include "gr_UnixGraphics.h"
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

#ifndef WITHOUT_PRINTING
#include "xap_UnixGnomePrintGraphics.h"
#include "gr_UnixPangoGraphics.h"
#endif

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

#ifndef WITHOUT_PRINTING
#include <libart_lgpl/art_affine.h>
#include "xap_UnixGnomePrintGraphics.h"
#include <libgnomeprint/gnome-print.h>
#endif

#ifdef HAVE_GNOMEUI
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#endif

#ifdef HAVE_BONOBO
#include <libbonoboui.h>
#include <bonobo/bonobo-macros.h>
#include <bonobo/bonobo-object.h>
#include "ap_EditMethods.h"
#include "abiwidget.h"
static int mainBonobo(int argc, const char ** argv);
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
#ifndef HAVE_BONOBO
    // hack to link abi_widget - thanks fjf
	if(this == 0)
		/*GtkWidget * pUn =*/ abi_widget_new_with_file("fred.abw");
#endif
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
* \param pFallbackStringSet String set to be used for untranslated strings.
* \return AP_DiskStringSet * on success, NULL if not found
*/
AP_DiskStringSet * 
AP_UnixApp::loadStringsFromDisk(const char 			* szStringSet, 
								AP_BuiltinStringSet * pFallbackStringSet)
{
	UT_ASSERT(pFallbackStringSet);

	const char * szDirectory = NULL;
	getPrefsValueDirectory(true,
			       static_cast<const XML_Char*>(AP_PREF_KEY_StringSetDirectory),
			       static_cast<const XML_Char**>(&szDirectory));
	UT_ASSERT((szDirectory) && (*szDirectory));

	UT_String szPathname = szDirectory;
	if (szDirectory[szPathname.size()-1]!='/')
		szPathname += "/";
	szPathname += szStringSet;
	szPathname += ".strings";

	AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);
	if (pDiskStringSet->loadStringsFromDisk(szPathname.c_str()))
	{
		pDiskStringSet->setFallbackStringSet(pFallbackStringSet);
		UT_DEBUGMSG(("Using StringSet [%s]\n",szPathname.c_str()));
		return pDiskStringSet;
	}
	else
	{
		DELETEP(pDiskStringSet);
		UT_DEBUGMSG(("Unable to load StringSet [%s] -- using builtin strings instead.\n",szPathname.c_str()));
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
													static_cast<const XML_Char*>(AP_PREF_DEFAULT_StringSet));
		UT_ASSERT(pBuiltinStringSet);

		// try loading strings by preference
		const char * szStringSet = NULL;
		if (   (getPrefsValue(AP_PREF_KEY_StringSet,
							  static_cast<const XML_Char**>(&szStringSet)))
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
	if (getPrefsValue( AP_PREF_KEY_StringSet, static_cast<const XML_Char**>(&szMenuLabelSetName))
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
//
// Save our toolbars if we're customizable
//
	if(areToolbarsCustomizable())
	{
		m_pToolbarFactory->saveToolbarsInCurrentScheme();
	}
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
				// don't own, don't free
				const UT_ByteBuf * png = 0;
	  
				pView->saveSelectedImage (&png);
				if (png && png->getLength() > 0)
					{
						m_pClipboard->addPNGData(target, static_cast<const UT_Byte*>(png->getPointer(0)), png->getLength());
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
		iLen = UT_strnlen(reinterpret_cast<const char *>(pData),iLen);

		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		bSuccess = pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);
    }
	else if (AP_UnixClipboard::isHTMLTag (szFormatFound))
	{
		IE_Imp_Text_Sniffer SniffBuf;
		const char * szRes = SniffBuf.recognizeContentsType(reinterpret_cast<const char *>(pData),iLen);
		if(UT_strcmp(szRes,"none") != 0)
		{
			UT_uint32 iread,iwritten = 0;
			const char * szutf8= static_cast<const char *>(UT_convert(reinterpret_cast<const char *>(pData),iLen,szRes,"UTF-8",&iread,&iwritten));
			IE_Imp_XHTML * pImpHTML = new IE_Imp_XHTML(pDocRange->m_pDoc);
			bSuccess = pImpHTML->pasteFromBuffer(pDocRange,reinterpret_cast<const unsigned char *>(szutf8),iwritten,"UTF-8");
			free(const_cast<char *>(szutf8));
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
		iLen = UT_strnlen(reinterpret_cast<const char *>(pData),iLen);
		
		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc,"UTF-8");
		bSuccess = pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);
    }

 retry_text:

	// we failed to paste *anything.* try plaintext as a last-ditch effort
	if(!bSuccess && m_pClipboard->getTextData(tFrom,reinterpret_cast<const void **>(&pData),&iLen, &szFormatFound)) {
		UT_DEBUGMSG(("DOM: pasting text as an absolute fallback (bug 7666)\n"));
		
		iLen = UT_strnlen(reinterpret_cast<const char *>(pData),iLen);

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
				  free(namelist[n]);
				  continue;
			  }
			  if(strcmp (namelist[n]->d_name+(len-3), "."G_MODULE_SUFFIX) != 0)
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
    *ppData = const_cast<void *>(static_cast<const void *>(m_selectionByteBuf.getPointer(0)));
    *pLen = m_selectionByteBuf.getLength();
    *pszFormatFound = formatList[j];
    return true;
}

bool AP_UnixApp:: makePngPreview(const char * pszInFile, const char * pszPNGFile, UT_sint32 iWidth, UT_sint32 iHeight)
{
	//
	// Create a private visual to draw with
	//
	GdkVisual * vis = gdk_visual_get_system (); // don't delete this!
	//
	// Create a private color map to draw with.
	//
	GdkColormap*  visColorMap = gdk_colormap_new(vis,true);
	//
	// Create a private set of attributes for our GdkWindow
	// 
	GdkWindowAttr attributes;
	attributes.title = NULL;
	attributes.event_mask = 0;
	attributes.x = 0;
	attributes.y = 0;
	attributes.width = iWidth;
	attributes.height = iHeight;
	attributes.wclass = 	GDK_INPUT_OUTPUT ;
	attributes.visual = vis;
	attributes.colormap = visColorMap;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.cursor = NULL;
	attributes.wmclass_name = NULL;
	attributes.wmclass_class = NULL;
	attributes.override_redirect = true;

	gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_COLORMAP | GDK_WA_VISUAL;
	GdkWindow*  pWindow = gdk_window_new (NULL, &attributes,
                                             attributes_mask);
	GdkPixmap * pMap =  gdk_pixmap_new(pWindow,iWidth,iHeight,-1); 	

	//GR_UnixGraphics * pG =  new GR_UnixGraphics(pMap, getFontManager(), this,true);
	GR_UnixAllocInfo ai(pMap, getFontManager(), true);
	GR_UnixGraphics * pG = (GR_UnixGraphics*) XAP_App::getApp()->newGraphics(ai);

	pG->createCaret();
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
	pPrevAbi->getView()->draw(0, &da);
	GR_Painter * pPaint = new GR_Painter(pG);
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
	delete pWindow;
	delete pMap;
	delete visColorMap;
	DELETEP(pPrevAbi); // This deletes pNewDoc
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
		g_source_remove(death_timeout_handler);
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
		GR_Painter painter (pUnixGraphics);
		painter.drawImage(pSplashImage, 0, 0);

		// on the first full paint of the image, start a 2 second timer
		if (!firstExpose)
		{
			firstExpose = true;
			// kill the window after splashTimeoutValue ms
			death_timeout_handler = g_timeout_add_full(0,splashTimeoutValue, s_hideSplash, NULL,NULL);
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
		wSplash = gtk_window_new(GTK_WINDOW_TOPLEVEL); //GTK_WINDOW_DIALOG
		gtk_window_set_decorated (GTK_WINDOW(wSplash), FALSE);
		gtk_window_set_default_size (GTK_WINDOW (wSplash),
									 iSplashWidth, iSplashHeight);
		gtk_window_set_resizable(GTK_WINDOW(wSplash), FALSE);

		// create a frame to add depth
		GtkWidget * frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(wSplash), frame);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
		gtk_widget_show(frame);

		// create a drawing area
		GtkWidget * da = createDrawingArea ();
		gtk_widget_set_events(da, GDK_ALL_EVENTS_MASK);
		gtk_widget_set_size_request(da, iSplashWidth, iSplashHeight);
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
		//pUnixGraphics = new GR_UnixGraphics(da->window, NULL, m_pApp);
		GR_UnixAllocInfo ai(da->window, NULL);
		pUnixGraphics = (GR_UnixGraphics*) XAP_App::getApp()->newGraphics(ai);
		
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

GR_Graphics * AP_UnixApp::newDefaultScreenGraphics() const
{
	XAP_Frame * pFrame = findValidFrame();
	UT_return_val_if_fail( pFrame, NULL );
	
	AP_UnixFrameImpl * pFI = static_cast<AP_UnixFrameImpl *>(pFrame->getFrameImpl());
	UT_return_val_if_fail( pFI, NULL );

	GtkWidget * da = pFI->getDrawingArea();
	UT_return_val_if_fail( da, NULL );
	
	GR_UnixAllocInfo ai(da->window, getFontManager());
	return XAP_App::getApp()->newGraphics(ai);
}


/*****************************************************************/

int AP_UnixApp::main(const char * szAppName, int argc, const char ** argv)
{
    // This is a static function.
    
    // initialize our application.
	XAP_Args XArgs = XAP_Args(argc,argv);
	AP_UnixApp * pMyUnixApp = new AP_UnixApp(&XArgs, szAppName);
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
#ifndef HAVE_GNOMEUI
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

#ifdef HAVE_BONOBO
#ifdef LOGFILE
	fprintf(logfile,"About to init bonobo \n");
#endif	  
	  bonobo_init (&XArgs.m_argc, const_cast<char **>(XArgs.m_argv));
#ifdef LOGFILE
	fprintf(logfile,"bonobo initialized \n");
#endif
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

#ifdef HAVE_BONOBO
		//
		// Check to see if we've been activated as a control by OAF
		//
		bool bControlFactory = false;
		for (UT_sint32 k = 1; k < XArgs.m_argc; k++)
		{
			if (*XArgs.m_argv[k] == '-')
				if (strstr(XArgs.m_argv[k],"GNOME_AbiWord_ControlFactory") != 0)
				{
					bControlFactory = true;
					break;
				}
		}
		if(bControlFactory)
		{
			int rtn = mainBonobo(XArgs.m_argc, XArgs.m_argv);
#ifdef LOGFILE
			fprintf(logfile,"mainBonobo Finished \n");
			fclose(logfile);
#endif
			pMyUnixApp->shutdown();
			delete pMyUnixApp;
			return rtn;
		}
#endif
		
		// do we show the splash?
		bool bShowSplash = Args.getShowSplash();
		
		const XAP_Prefs * pPrefs = pMyUnixApp->getPrefs();
		UT_ASSERT(pPrefs);
		bool bSplashPref = true;
		if (pPrefs && 
			pPrefs->getPrefsValueBool (AP_PREF_KEY_ShowSplash, &bSplashPref))
			bShowSplash = bShowSplash && bSplashPref;
		
		if (bShowSplash)
			_showSplash(1500);
    
		// Step 3: Create windows as appropriate.
		// if some args are botched, it returns false and we should
		// continue out the door.
		// We used to check for bShowApp here.  It shouldn't be needed
		// anymore, because doWindowlessArgs was supposed to bail already. -PL

		if (pMyUnixApp->openCmdLineFiles(&Args))
		{
#ifdef HAVE_HILDON
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
		UT_DEBUGMSG(("No DISPLAY: this may not be what you want.\n"));
	}
	// unload all loaded plugins (remove some of the memory leaks shown at shutdown :-)
	XAP_ModuleManager::instance().unloadAllPlugins();

	// Step 4: Destroy the App.  It should take care of deleting all frames.
	pMyUnixApp->shutdown();
	delete pMyUnixApp;

	
	return 0;
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
#ifdef HAVE_GNOMEUI
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
#ifndef WITHOUT_PRINTING
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

			UT_uint32 iDefaultPrintClass = pGF->getDefaultClass(false);		   
			
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
			GR_Graphics *print_graphics;
			XAP_UnixGnomePrintGraphics * gnome_print_graphics;

			gnome_print_graphics = new XAP_UnixGnomePrintGraphics(job);
#if defined(USE_PANGO)
			if(iDefaultPrintClass == GRID_UNIX_PANGO_PRINT || iDefaultPrintClass == GRID_UNIX_PANGO)
				print_graphics = new GR_UnixPangoPrintGraphics(gnome_print_graphics);
			else
#endif
				print_graphics = gnome_print_graphics;

			bSuccess = conv.print (Args->m_sFile, print_graphics, Args->m_sFileExtension);

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

#ifndef WITHOUT_PRINTING

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
				if(UT_strcmp(szName,szRequest) == 0)
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
		if(!getFontManager())
			{
				/*
				   need to temporarily set the Unix graphics as default to
				   force the font loading
				*/
				GR_GraphicsFactory * pGF = getGraphicsFactory();
				UT_return_val_if_fail( pGF, false );

				UT_uint32 iGrId = pGF->getDefaultClass(true /*screen*/);
				pGF->registerAsDefault(GRID_UNIX_NULL, true);
				_loadFonts();
				pGF->registerAsDefault(iGrId, true);
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
		UT_ASSERT(curFrame);
		if (NULL == curFrame->getFilename())
		  curFrame->backup(".abw.SAVED",abiType);
		else
		  curFrame->backup(".SAVED",abiType);
    }
    
    fflush(stdout);
    
    // Abort and dump core
    abort();
}

#ifdef HAVE_BONOBO

//-------------------------------------------------------------------
// Bonobo Control factory stuff
//-------------------------------------------------------------------

static BonoboControl * AbiWidget_control_new (AbiWidget * abi);


/* 
 * get a value from abiwidget
 */ 
static void get_prop (BonoboPropertyBag 	*bag,
	  BonoboArg 		*arg,
	  guint 		 arg_id,
	  CORBA_Environment 	*ev,
	  gpointer 		 user_data)
{
	GObject 	*abi;
	
	g_return_if_fail (IS_ABI_WIDGET(user_data));
		
	/*
	 * get data from our AbiWidget
	 */
//
// first create fresh GValue
//
	GValue gVal = {0, };
	GParamSpec ParamSpec;
//
// Now extract the requested GValue from abiwidget using arg_id
//
	abi = G_OBJECT(user_data); 
	abi_widget_get_property(abi,arg_id,&gVal,&ParamSpec);
//
// Now copy it back to the bonobo argument.
//
	bonobo_arg_from_gvalue (arg, &gVal);
//
// Free up allocated memory
//
	if (G_VALUE_TYPE(&gVal) == G_TYPE_STRING && g_value_get_string(&gVal))
	{
		g_free (const_cast<void*>(static_cast<const void*>(g_value_get_string(&gVal))));
	}
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
	GObject 	*abi;
	
	g_return_if_fail (IS_ABI_WIDGET(user_data));
#ifdef LOGFILE
	fprintf(logfile,"UnixApp::set_prop id %d \n",arg_id);
#endif
	abi = G_OBJECT(user_data); 
//
// define a fresh GValue and fill it from bonobo_arg
//
	GValue gVal = {0, };
	GParamSpec ParamSpec;
	bonobo_arg_to_gvalue(&gVal,arg);
//
// Now send it to abiwidget via send prop
//
	abi_widget_set_property(abi,arg_id,&gVal,&ParamSpec);
//
// Free up allocated memory
//
//
// Free up allocated memory
//
	if (G_VALUE_TYPE(&gVal) == G_TYPE_STRING && g_value_get_string(&gVal))
	{
		g_free (const_cast<void*>(static_cast<const void*>(g_value_get_string(&gVal))));
	}
}


#if 0
/****************************************************************
 Whole bunch of bonobo UI verb implementations to enable useful things
 to be done from a merged menuBar.
**********************************************************************/
static void
verb_Undo_cb (BonoboUIComponent *uic, gpointer data, const char *name)
{

	g_return_if_fail (IS_ABI_WIDGET(data));

	AbiWidget * abi = ABI_WIDGET (G_OBJECT(data));
	GObject * object = G_OBJECT(abi);
	AbiWidgetClass * abi_klazz = ABI_WIDGET_CLASS (G_OBJECT_GET_CLASS(object));
	abi_klazz->undo(abi);
}

static void
verb_generic_cb (BonoboUIComponent *uic, gpointer data, const char *name)
{

	g_return_if_fail (IS_ABI_WIDGET(data));

	AbiWidget * abi = ABI_WIDGET (G_OBJECT(data));
	GObject * object = G_OBJECT(abi);
	AbiWidgetClass * abi_klazz = ABI_WIDGET_CLASS (G_OBJECT_GET_CLASS(object));
	if(UT_strcmp(name,"redo") == 0)
	{
		abi_klazz->redo(abi);
	}
	else if(UT_strcmp(name,"copy") == 0)
	{
		abi_klazz->copy(abi);
	}
	else if(UT_strcmp(name,"cut") == 0)
	{
		abi_klazz->cut(abi);
	}
	else if(UT_strcmp(name,"paste") == 0)
	{
		abi_klazz->paste(abi);
	}
	else if(UT_strcmp(name,"print") == 0)
	{
		abi_widget_invoke(abi,"print");
	}
	else if(UT_strcmp(name,"printPreview") == 0)
	{
		abi_widget_invoke(abi,"printPreview");
	}
	else if(UT_strcmp(name,"fileSave") == 0)
	{
		abi_widget_invoke(abi,"fileSave");
	}
	else if(UT_strcmp(name,"fileSaveAs") == 0)
	{
		abi_widget_invoke(abi,"fileSaveAs");
	}
}

static BonoboUIVerb abi_nautilus_verbs[] = {
	BONOBO_UI_VERB ("fileSave",            verb_generic_cb),
	BONOBO_UI_VERB ("fileSaveAs",          verb_generic_cb),
	BONOBO_UI_VERB ("print",               verb_generic_cb),
	BONOBO_UI_VERB ("printPreview",        verb_generic_cb),
	BONOBO_UI_VERB ("undo",                verb_Undo_cb),
	BONOBO_UI_VERB ("redo",                verb_generic_cb),
	BONOBO_UI_VERB ("copy",                verb_generic_cb),
	BONOBO_UI_VERB ("cut",                 verb_generic_cb),
	BONOBO_UI_VERB ("paste",                 verb_generic_cb),
	BONOBO_UI_VERB_END
};
#endif

/*!
 * Do this after
 */
static void
abi_nautilus_view_create_ui (AbiWidget *abi)
{
#if 0
	g_return_if_fail (IS_ABI_WIDGET(abi));
	BonoboUIComponent * uic = abi_widget_get_Bonobo_uic(abi);

	/* Connect the UI component to the control frame's UI container. */

// see nautilus/libnautilus/nautilus-view.c:994
//  
//	ui_container = bonobo_control_get_remote_ui_container (view->details->control, NULL);
//	bonobo_ui_component_set_container (ui_component, ui_container, NULL);
//	bonobo_object_release_unref (ui_container, NULL);

	/* Set up the UI from XML file. 
	   Have to find a way to get this from a global
	installation
	*/
	AP_UnixApp * pApp = static_cast<AP_UnixApp *>(XAP_App::getApp());
	const char * szDir =   pApp->getUserPrivateDirectory();
	bonobo_ui_util_set_ui (uic, "/home/msevior",
			       "abi-nautilus-view-file.xml", "AbiWordNautilusView", NULL);
	bonobo_ui_component_add_verb_list_with_data (uic, abi_nautilus_verbs,
						     abi);
#else
	UT_DEBUGMSG(("DOM: abi_nautilus_view_create_ui not implemented\n"));
#endif
}

static void
control_set_ui_container (AbiWidget *abi,
			  Bonobo_UIContainer ui_container)
{
	g_return_if_fail (IS_ABI_WIDGET(abi));
	g_return_if_fail (ui_container != CORBA_OBJECT_NIL);

	bonobo_ui_component_set_container (abi_widget_get_Bonobo_uic(abi), ui_container, NULL);

	abi_nautilus_view_create_ui (abi);
}

static void
control_unset_ui_container (AbiWidget * abi)
{
	g_return_if_fail (IS_ABI_WIDGET(abi));

	bonobo_ui_component_unset_container (abi_widget_get_Bonobo_uic(abi), NULL);
}

static void abi_control_activate_cb(BonoboControl *object, gboolean state, gpointer data)
{
	AbiWidget * abi;
	g_return_if_fail (IS_ABI_WIDGET(G_OBJECT(data)));
	abi = ABI_WIDGET(G_OBJECT(data));
	if(state)
	{
		Bonobo_UIContainer ui_container;
		ui_container = bonobo_control_get_remote_ui_container (object, NULL);
		if (ui_container != CORBA_OBJECT_NIL) 
		{
			control_set_ui_container (abi, ui_container);
			bonobo_object_release_unref (ui_container, NULL);
		}

	} 
	else
	{
		control_unset_ui_container (abi);
	}
}



/*****************************************************************/
/* Implements the Bonobo/Persist:1.0, Bonobo/PersistStream:1.0,
   Bonobo/PersistFile:1.0 Interfaces */
/*****************************************************************/

#if 0

// (working) code patiently waiting for Bonobo printing to rise from the grave

static void
print_document (GnomePrintContext *ctx,
				double                     inWidth,
				double                     inHeight,
				const Bonobo_PrintScissor *opt_scissor,
				gpointer                   user_data)
{
	// assert pre-conditions
	g_return_if_fail (user_data != NULL);
	g_return_if_fail (IS_ABI_WIDGET (user_data));
	
	// get me!
	AbiWidget * abi = ABI_WIDGET(user_data);
	
	// get our frame
	XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
	UT_return_if_fail(pFrame != NULL);
	
	// get our current view so we can get the document being worked on
	FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
	UT_return_if_fail(pView!=NULL);
	
	// get the current document
	PD_Document * pDoc = pView->getDocument () ;
	UT_return_if_fail(pDoc!=NULL);
	
	// get the current app
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(XAP_App::getApp () );
	
	// create a graphics drawing class
	GR_Graphics *pGraphics = new XAP_UnixGnomePrintGraphics (ctx, inWidth, inHeight) ;
	UT_return_if_fail(pGraphics!=NULL);
	
	// layout the document
	FL_DocLayout * pDocLayout = new FL_DocLayout(pDoc,pGraphics);
	UT_ASSERT(pDocLayout);
	
	// create a new printing view of the document
	FV_View printView (pFrame->getApp(),pFrame,pDocLayout);
	pDocLayout->setView (&printView);
	pDocLayout->fillLayouts();
	pDocLayout->formatAll();
	
	// get the best fit width & height of the printed pages
	UT_sint32 iWidth  =  pDocLayout->getWidth();
	UT_sint32 iHeight =  pDocLayout->getHeight();
	//UT_sint32 iPages  = pDocLayout->countPages();
	UT_uint32 width   = MIN(iWidth, pGraphics->tluD(inWidth));
	UT_uint32 height  = MIN(iHeight, pGraphics->tluD(inHeight));
	
	// figure out roughly how many pages to print
	UT_sint32 iPagesToPrint = static_cast<UT_sint32>(height/pGraphics->tluD(pDoc->m_docPageSize.Height(DIM_PT)));
	if (iPagesToPrint < 1)
		iPagesToPrint = 1;
	
	// actually print - TODO: can make this better if we figure out what page is focussed
	s_actuallyPrint (pDoc, pGraphics,
					 &printView, "bonobo_printed_document",
					 1, false,
					 width, height,
					 1, iPagesToPrint ) ;
	
	// clean up
	DELETEP(pGraphics);
	DELETEP(pDocLayout);
}

#endif

#define ABI_BUFFER_SIZE 32768

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
	size_t len_read;
	FILE * tmpfile;
#ifdef LOGFILE
	fprintf(logfile,"Load file from stream \n");
#endif

	g_return_if_fail (data != NULL);
	g_return_if_fail (IS_ABI_WIDGET (data));
	
	abiwidget = static_cast<AbiWidget *>(data);
#ifdef LOGFILE
	fprintf(logfile,"At entry Load file from stream refcount %d \n",G_OBJECT(abiwidget)->ref_count);
#endif
	
	//
	// Create a temp file name.
	//
	char szTempfile[ 2048 ];
	UT_tmpnam(szTempfile);
	
	tmpfile = fopen(szTempfile, "wb");
#ifdef LOGFILE
	fprintf(logfile,"Create Temp filename %s \n",szTempfile);
	fprintf(logfile," After Create Temp ref count %d \n",G_OBJECT(abiwidget)->ref_count);
#endif
	
	do 
	{
		Bonobo_Stream_read (stream, ABI_BUFFER_SIZE, &buffer, ev);
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
#ifdef LOGFILE
	fprintf(logfile," After load_document_from_stream ref count %d \n",G_OBJECT(abiwidget)->ref_count);
#endif

	fclose(tmpfile);

	//
	// Load the file.
	//
	//
	g_object_set(G_OBJECT(abiwidget),"AbiWidget--unlink-after-load",static_cast<gboolean>(TRUE),NULL);
	g_object_set(G_OBJECT(abiwidget),"AbiWidget--load-file",static_cast<gchar *>(szTempfile),NULL);
	abi_widget_map_to_screen(abiwidget);

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
	CORBA_octet buffer [ ABI_BUFFER_SIZE ] = "" ;
	CORBA_long len_read = 0;
	FILE * tmpfile = NULL;

	g_return_if_fail (data != NULL);
	g_return_if_fail (IS_ABI_WIDGET (data));

	abiwidget = static_cast<AbiWidget *>(data);

	//
	// Create a temp file name.
	//
	char szTempfile[ 2048 ];
	UT_tmpnam(szTempfile);

	char * ext = ".abw" ;

	if ( !strcmp ( "application/msword", type ) )
	  ext = ".doc" ; 
	else if ( !strcmp ( "application/rtf", type ) || !strcmp ("text/rtf", type) || !strcmp ("text/richtext", type) )
	  ext = ".rtf" ;
	else if ( !strcmp ( "text/plain", type ) )
	  ext = ".txt" ;
	else if ( !strcmp ( "text/html", type ) )
	  ext = ".html" ;
	else if ( !strcmp ( "application/xhtml+xml", type ) )
	  ext = ".xhtml" ;
	else if ( !strcmp ( "application/x-applix-word", type ) )
	  ext = ".aw";
	else if ( !strcmp ( "appplication/vnd.palm", type ) )
	  ext = ".pdb" ;
	else if ( !strcmp ( "text/vnd.wap.wml", type ) )
	  ext = ".wml" ;

	// todo: vary this based on the ContentType
	if ( !abi_widget_save_ext ( abiwidget, szTempfile, ext ) )
	  return ;

	tmpfile = fopen(szTempfile, "wb");
	if (!tmpfile)
		return; // should never happen, but who knows...

	do 
	{
		len_read = fread ( buffer, sizeof(CORBA_octet), ABI_BUFFER_SIZE, tmpfile ) ;

		stream_buffer = Bonobo_Stream_iobuf__alloc ();
		stream_buffer->_buffer = static_cast<CORBA_octet*>(buffer);
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

	object = BONOBO_OBJECT (AbiWidget_control_new(abi));

	if (object == NULL)
		return NULL;

	corba_object = bonobo_object_corba_objref (object);
#ifdef LOGFILE
	fprintf(logfile," After get_object ref count %d \n",G_OBJECT(object)->ref_count);
#endif

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
	g_object_set(G_OBJECT(abiwidget),"AbiWidget--load-file",reinterpret_cast<const gchar *>(filename),NULL);
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
	return bonobo_persist_generate_content_types (12, "application/x-abiword", "text/abiword", "application/msword", 
												  "application/rtf", "text/rtf", "text/richtext", "text/plain", "text/html", "application/xhtml+xml",
												  "application/x-applix-word", "appplication/vnd.palm", "text/vnd.wap.wml");
}

/*****************************************************************/
/* Implements the Bonobo/Zoomable:1.0 Interface */
/*****************************************************************/

// increment/decrement zoom percentages by this amount
#define ZOOM_PCTG 10

static float preferred_zoom_levels[] = {
	1.0 / 4.0, 1.0 / 2.0, 3.0 / 4.0, 1.0, 1.5, 2.0,
};

static const gint n_zoom_levels = (sizeof (preferred_zoom_levels) / sizeof (float));

static void zoom_level_func(GObject * z, float lvl, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  if ( lvl <= 0.0 )
    return ;

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  pFrame->setZoomType (XAP_Frame::z_PERCENT);
  pFrame->quickZoom (static_cast<UT_uint32>(lvl));
}

static void zoom_in_func(GObject * z, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  UT_sint32 zoom_lvl = pFrame->getZoomPercentage();
  zoom_lvl += ZOOM_PCTG ;

  pFrame->setZoomType (XAP_Frame::z_PERCENT);
  pFrame->quickZoom (zoom_lvl);  
}

static void zoom_out_func(GObject * z, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  UT_sint32 zoom_lvl = pFrame->getZoomPercentage();
  zoom_lvl -= ZOOM_PCTG ;

  if ( zoom_lvl <= 0 )
    return ;

  pFrame->setZoomType (XAP_Frame::z_PERCENT);
  pFrame->quickZoom (zoom_lvl);  
}

static void zoom_to_fit_func(GObject * z, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
  UT_return_if_fail(pView!=NULL);

  UT_uint32 newZoom = pView->calculateZoomPercentForWholePage();
  pFrame->setZoomType( XAP_Frame::z_WHOLEPAGE );
  pFrame->quickZoom(newZoom);
}

static void zoom_to_default_func(GObject * z, gpointer data)
{
  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_ABI_WIDGET(data));

  AbiWidget * abi = ABI_WIDGET(data);

  XAP_Frame * pFrame = abi_widget_get_frame ( abi ) ;
  UT_return_if_fail ( pFrame != NULL ) ;

  pFrame->setZoomType (XAP_Frame::z_100);
  pFrame->quickZoom (100);  
}

/*****************************************************************/
/* Bonobo Inteface-Adding Code */
/*****************************************************************/

#define ABIWORD_OAF_IID "OAFIID:GNOME_AbiWord_Control"

//
// Add extra interfaces to load data into the control
//
BonoboObject *
AbiControl_add_interfaces (AbiWidget *abiwidget,
						   BonoboObject *to_aggregate)
{
	BonoboPersistFile   *file;
	BonoboPersistStream *stream;
	BonoboItemContainer *item_container;
	BonoboZoomable      *zoomable;

	g_return_val_if_fail (IS_ABI_WIDGET(abiwidget), NULL);
	g_return_val_if_fail (BONOBO_IS_OBJECT (to_aggregate), NULL);

	/* Inteface Bonobo::PropertyBag */

	guint n_pspecs = 0;
	BonoboPropertyBag * pb = bonobo_property_bag_new (get_prop, set_prop, abiwidget);
	const GParamSpec ** pspecs = const_cast<const GParamSpec **>(g_object_class_list_properties (G_OBJECT_GET_CLASS (G_OBJECT (abiwidget)), &n_pspecs));
	bonobo_property_bag_map_params (pb, G_OBJECT (abiwidget), pspecs, n_pspecs);
	bonobo_control_set_properties (BONOBO_CONTROL(to_aggregate),BONOBO_OBJREF (pb), NULL);

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate), BONOBO_OBJECT (pb));
#ifdef LOGFILE
	fprintf(logfile,"AbiControl_add_interfaces Property Bag with %d parameters added \n",n_pspecs);
//	fprintf(logfile," After Property Bag interface ref count %d \n",  bonobo_object_get_ao_ref_count(BONOBO_OBJECT(to_aggregate)));
#endif

	/* Interface Bonobo::Persist */

	/* Interface Bonobo::PersistStream */

	stream = bonobo_persist_stream_new (load_document_from_stream, 
										save_document_to_stream, 
										pstream_get_content_types, 
										ABIWORD_OAF_IID,
										abiwidget);
	if (!stream) {
		bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
				     BONOBO_OBJECT (stream));
#ifdef LOGFILE
	fprintf(logfile,"AbiControl_add_interfaces stream created \n");
//	fprintf(logfile," After Stream interface ref count %d \n",  bonobo_object_get_ao_ref_count(BONOBO_OBJECT(to_aggregate)));
#endif


	/* Interface Bonobo::PersistFile */

	file = bonobo_persist_file_new (load_document_from_file,
									save_document_to_file, 
									ABIWORD_OAF_IID,
									abiwidget);
	if (!file) {
		bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
				     BONOBO_OBJECT (file));
	
	/* Interface Bonobo/ItemContainer */

	item_container = bonobo_item_container_new ();

	g_signal_connect (G_OBJECT (item_container),
					  "get_object",
					  G_CALLBACK (abiwidget_get_object),
					  abiwidget);
#ifdef LOGFILE
//	fprintf(logfile," After get_object signal connect ref count %d \n",  bonobo_object_get_ao_ref_count(BONOBO_OBJECT(to_aggregate)));
#endif
	
	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
								 BONOBO_OBJECT (item_container));

	/* Interface Bonobo::Zoomable */

	zoomable = bonobo_zoomable_new () ;
	if ( !zoomable ) {
	  bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
	  return NULL;
	}

	bonobo_zoomable_set_parameters_full
		(zoomable,
		 1.0,
		 0.1,
		 5.0,
		 TRUE, TRUE, TRUE,
		 preferred_zoom_levels,
		 NULL,
		 n_zoom_levels);

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

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
								 BONOBO_OBJECT (zoomable));
#ifdef LOGFILE
//	fprintf(logfile," After zoomable connects ref count %d \n",  bonobo_object_get_ao_ref_count(BONOBO_OBJECT(to_aggregate)));
	fprintf(logfile," After zoomable connects gobject ref count %d \n",G_OBJECT(to_aggregate)->ref_count);
#endif

/* UI Component */
	BonoboUIComponent * uic = bonobo_ui_component_new ("AbiWordNautilusView");
	abi_widget_set_Bonobo_uic(abiwidget,uic);

	g_signal_connect (BONOBO_CONTROL(to_aggregate), "activate", G_CALLBACK (abi_control_activate_cb), abiwidget);

	return to_aggregate;
}


static BonoboControl * AbiWidget_control_new (AbiWidget * abi)
{
  // create a BonoboControl from a widget
#ifdef LOGFILE
	fprintf(logfile," Just before bonobo_control_new ref count %d \n",G_OBJECT(abi)->ref_count);
#endif
  BonoboControl * control = bonobo_control_new (GTK_WIDGET(abi));
#ifdef LOGFILE
	fprintf(logfile," Just after bonobo_control_new GOBJECT ref count %d \n",G_OBJECT(control)->ref_count);
//	fprintf(logfile," Just after bonobo_control_new BOnobo ref count %d \n",  bonobo_object_get_ao_ref_count(BONOBO_OBJECT(control)));
#endif
//
// This fixes the double reference from the bonobo_control_new
//
	g_object_unref(G_OBJECT(abi));
#ifdef LOGFILE
//	fprintf(logfile," Just after the un_ref count control %d \n", bonobo_object_get_ao_ref_count(BONOBO_OBJECT(control)));
	fprintf(logfile," Just after the un_ref abi count %d \n",G_OBJECT(abi)->ref_count);
#endif
#if 0
  AbiWidgetClass * abi_klazz = ABI_WIDGET_CLASS (G_OBJECT_GET_CLASS(G_OBJECT(abi)));
  BonoboObjectClass *bonobo_object_class = (BonoboObjectClass *)abi_klazz;
  GObjectClass *gobject_class = G_OBJECT_CLASS(abi_klazz);
#endif
  AbiControl_add_interfaces (ABI_WIDGET(abi),
							 BONOBO_OBJECT(control));
#ifdef LOGFILE
	fprintf(logfile," After AbiControl_add_interfaces ref count %d \n",G_OBJECT(control)->ref_count);
#endif

  return control;
}

/*
 *  produce a brand new bonobo_AbiWord_control
 *  (this is a callback function, registered in 
 *  	'bonobo_generic_factory_new')
 */
static BonoboObject*
bonobo_AbiWidget_factory  (BonoboGenericFactory *factory, 
						   const char           *oaf_iid,
						   void *closure)
{
  /*
   * create a new AbiWidget instance
   */  
  AP_UnixApp * pApp = static_cast<AP_UnixApp *>(XAP_App::getApp());
  GtkWidget  * abi  = abi_widget_new_with_app (pApp);
#ifdef LOGFILE
	fprintf(logfile," After new_with_app ref count %d \n",G_OBJECT(abi)->ref_count);
#endif

	gtk_widget_show (abi);
#ifdef LOGFILE
	fprintf(logfile," After gtk_widget_show ref count %d \n",G_OBJECT(abi)->ref_count);
#endif

  return BONOBO_OBJECT (AbiWidget_control_new (ABI_WIDGET (abi)));
}



static int mainBonobo(int argc, const char ** argv)
{
	BONOBO_FACTORY_INIT ("abiword-component", "0.1", &argc, const_cast<char **>(argv));
	XAP_App * pApp = XAP_App::getApp();
	pApp->setBonoboRunning();
	return bonobo_generic_factory_main ("OAFIID:GNOME_AbiWord_ControlFactory",
										bonobo_AbiWidget_factory, NULL);
}

#endif /* HAVE_BONOBO */
