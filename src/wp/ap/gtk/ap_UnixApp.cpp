/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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

#define ABIWORD_INTERNAL

//#define DUMP_CLIPBOARD_PASTE 1
//#define DUMP_CLIPBOARD_COPY  1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef DUMP_CLIPBOARD_PASTE
#include <fstream>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <glib.h>
#include <gsf/gsf-output-memory.h>

#include "ut_debugmsg.h"
#include "ut_path.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_locale.h"
#include "ut_sleep.h"
#include "ut_files.h"

#include "xap_Args.h"
#include "ap_Args.h"
#include "ap_Convert.h"
#include "ap_UnixFrame.h"
#include "ap_UnixFrameImpl.h"
#include "ap_UnixApp.h"
#include "xap_UnixApp.h"

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
#include "gr_UnixCairoGraphics.h"
#include "ie_exp_DocRangeListener.h"

#ifdef GTK_WIN_POS_CENTER_ALWAYS
#define WIN_POS GTK_WIN_POS_CENTER_ALWAYS
#else
#define WIN_POS GTK_WIN_POS_CENTER
#endif

#include "ie_impGraphic.h"
#include "ut_math.h"
#include "abi-builtin-plugins.h"

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
  /param szAppName A string representing the name of the app.
	Currently always AbiWord (I think).
*/
AP_UnixApp::AP_UnixApp(const char * szAppName)
    : AP_App(szAppName),
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

	UT_String szPathVariant[4];
	char * p_strbuf = strdup("");
	char * p_modifier = NULL;
	int  cur_id = 0;
	bool three_letters = false; // some have 3!

	if (szStringSet) {
		FREEP(p_strbuf);
		p_strbuf = strdup(szStringSet);
		p_modifier = strrchr(p_strbuf,'@');
		
		char t = szStringSet[2];
		if (t && t!='-' && t!='@' && t!='_') three_letters = true;
	}

	if (p_modifier) {
		// fo_BA@xxx.strings
		szPathVariant[cur_id] = szDirectory;
		if (szDirectory[strlen(szDirectory)-1]!='/')
			szPathVariant[cur_id] += "/";
		szPathVariant[cur_id] += p_strbuf;
		szPathVariant[cur_id] += ".strings";

		cur_id++;

		// fo@xxx.strings
		if (szStringSet && strlen(szStringSet) > 2) {
			szPathVariant[cur_id] = szDirectory;
			if (szDirectory[strlen(szDirectory)-1]!='/')
				szPathVariant[cur_id] += "/";
			szPathVariant[cur_id] += p_strbuf[0];
			szPathVariant[cur_id] += p_strbuf[1];
			if (three_letters)
				szPathVariant[cur_id] += p_strbuf[2];
			szPathVariant[cur_id] += p_modifier;
			szPathVariant[cur_id] += ".strings";
		}

		cur_id++;

		// trim modifier part
		*p_modifier = 0;
	}
	
	// fo_BA.strings
	UT_String szPath = szDirectory;
	if (szDirectory[szPath.size()-1]!='/')
		szPath += "/";
	szPath += p_strbuf;
	szPath += ".strings";

	// fo.strings
	UT_String szFallbackPath;
	if (szStringSet && strlen(szStringSet) > 2) {
		szFallbackPath = szDirectory;
		if (szDirectory[szFallbackPath.size()-1]!='/')
			szFallbackPath += "/";
		szFallbackPath += p_strbuf[0];
		szFallbackPath += p_strbuf[1];
		if (three_letters)
			szFallbackPath += p_strbuf[2];
		szFallbackPath += ".strings";
	}

	AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);

	FREEP(p_strbuf);

	// trying to load specific strings first
	for (int i=0; i<cur_id; i++) {
		if (pDiskStringSet->loadStringsFromDisk(szPathVariant[i].c_str())) {
			pDiskStringSet->setFallbackStringSet(pDefaultStringSet);
			UT_DEBUGMSG(("Using [v] StringSet [%s]\n",szPathVariant[i].c_str()));
			return pDiskStringSet;
		}
	}

	// then generic ones
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
    bool bVerified = UT_createDirectoryIfNecessary(szUserPrivateDirectory);
    if (!bVerified)
      {
		  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      }

    // COPYPASTA WARNING (ap_Win32App.cpp)
    // create templates directory
    UT_String sTemplates = szUserPrivateDirectory;
    sTemplates += "/templates";
    UT_createDirectoryIfNecessary(sTemplates.c_str());

    // load the preferences.
    
    m_prefs = new AP_UnixPrefs();
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
			if (szFallbackStringSet)
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

	abi_register_builtin_plugins();

	bool bLoadPlugins = true;
	bool bFound = getPrefsValueBool(XAP_PREF_KEY_AutoLoadPlugins,&bLoadPlugins);
	if(bLoadPlugins || !bFound)
		loadAllPlugins();
	//
	// Now all the plugins are loaded we can initialize the clipboard
	//
	if(m_pClipboard)
		m_pClipboard->initialize();

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
	if(!isBonoboRunning())
	{
		if (m_prefs->getAutoSavePrefs())
			m_prefs->savePrefsFile();
		XAP_UnixApp::shutdown();
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
  This returns the directory that holds the application UI files.
  \return A const string containting the directory path
*/
const std::string& AP_UnixApp::getAbiSuiteAppUIDir(void) const
{
	static const std::string dir = std::string(getAbiSuiteLibDir()) + "/ui";
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
    UT_ByteBuf bufODT;

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

	// Look to see if the ODT plugin is loaded

	IEFileType ftODT = IE_Exp::fileTypeForMimetype("application/vnd.oasis.opendocument.text");
	bool bExpODT = false;
	if(ftODT != IEFT_Unknown)
	{
		// ODT plugin is present construct an exporter
		//
		IE_Exp * pODT = NULL;
		IEFileType genIEFT = IEFT_Unknown;
		GsfOutput * outBuf =  gsf_output_memory_new();
		UT_Error err = IE_Exp::constructExporter(pDocRange->m_pDoc,outBuf,
												 ftODT,&pODT,& genIEFT);
		if(pODT && (genIEFT == ftODT))
		{
			//												 
			// Copy to the buffer
			//
			err = pODT->copyToBuffer(pDocRange, &bufODT);
			bExpODT = (err == UT_OK);
            UT_DEBUGMSG(("Putting ODF on the clipboard...e:%d bExpODT:%d\n", err, bExpODT ));

#ifdef DUMP_CLIPBOARD_COPY
            std::ofstream oss("/tmp/abiword-clipboard-copy.odt");
            oss.write( (const char*)bufODT.getPointer (0), bufODT.getLength () );
            oss.close();
#endif
		}
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
	if (bExpODT && bufODT.getLength () > 0)
		m_pClipboard->addODTData (target, bufODT.getPointer (0), bufODT.getLength ());
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

#ifdef DUMP_CLIPBOARD_PASTE
    if (bFoundOne)
    {
        std::ofstream oss("/tmp/clips");
        oss.write( (const char*)pData, iLen );
        oss.close();
    }
#endif

    if (!bFoundOne)
    {
		UT_DEBUGMSG(("PasteFromClipboard: did not find anything to paste.\n"));
		return;
    }
    if (AP_UnixClipboard::isDynamicTag (szFormatFound))
	{
		UT_DEBUGMSG(("Dynamic Format Found = %s \n",szFormatFound));
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
		if(szRes && strcmp(szRes,"none") != 0)
		{
			UT_uint32 iread,iwritten = 0;
			const char * szutf8= static_cast<const char *>(UT_convert(reinterpret_cast<const char *>(pData),iLen,szRes,"UTF-8",&iread,&iwritten));
			if (szutf8) { 
				IE_Imp_XHTML * pImpHTML = new IE_Imp_XHTML(pDocRange->m_pDoc);
				bSuccess = pImpHTML->pasteFromBuffer(pDocRange,reinterpret_cast<const unsigned char *>(szutf8),iwritten,"UTF-8");
				g_free(const_cast<char *>(szutf8));
				DELETEP(pImpHTML);
			}
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
		  
		  UT_ByteBuf bytes( iLen );
		  
		  bytes.append (pData, iLen);
		  
		  error = IE_ImpGraphic::loadGraphic(bytes, iegft, &pFG);
		  if(!pFG || error)
		  {
			  UT_DEBUGMSG(("DOM: could not import graphic (%d)\n", error));
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

static bool is_so (const char *file) {

	size_t len = strlen (file);
	if (len < (strlen(G_MODULE_SUFFIX) + 2)) // this is ".so" and at least one char for the filename
		return false;
	const char *suffix = file+(len-3);
	if(0 == strcmp (suffix, "." G_MODULE_SUFFIX))
		return true;
	return false;
}

void AP_UnixApp::loadAllPlugins ()
{
  UT_String pluginList[2];
  UT_String pluginDir;

  // the global plugin directory
  pluginDir += ABIWORD_PLUGINSDIR "/";

  UT_DEBUGMSG(("pluginDir: '%s'\n", pluginDir.c_str ()));

  pluginList[0] = pluginDir;

  // the user-local plugin directory
  pluginDir = getUserPrivateDirectory ();
  pluginDir += "/" PACKAGE_NAME "/plugins/";
  UT_DEBUGMSG (("ROB: private plugins in '%s'\n", pluginDir.c_str ()));
  pluginList[1] = pluginDir;

  for(UT_uint32 i = 0; i < G_N_ELEMENTS(pluginList); i++)
  {
	const UT_String &path = pluginList[i];

	if (!g_file_test (path.c_str(), G_FILE_TEST_IS_DIR))
		continue;

	GError *err = NULL;
	GDir *dir = g_dir_open (path.c_str(), 0, &err);
	if (err) {
		g_warning ("%s", err->message);
		g_error_free (err), err = NULL;
		continue;
	}

	const char *name;
	while (NULL != (name = g_dir_read_name (dir))) {
		if (is_so (name)) {
			UT_String plugin (path + name);
			UT_DEBUGMSG(("DOM: loading plugin %s\n", plugin.c_str()));

			if (XAP_ModuleManager::instance().loadModule (plugin.c_str())) {
			  UT_DEBUGMSG(("DOM: loaded plugin: %s\n", name));
			} else {
			  UT_DEBUGMSG(("DOM: didn't load plugin: %s\n", name));
			}
		}
	}
	g_dir_close (dir), dir = NULL;
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

bool AP_UnixApp::makePngPreview(const char * pszInFile, const char * pszPNGFile, UT_sint32 iWidth, UT_sint32 iHeight)
{
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, iWidth, iHeight);
	cairo_t *cr = cairo_create (surface);

	GR_UnixCairoAllocInfo ai(NULL, false);

	GR_CairoGraphics * pG = static_cast<GR_CairoGraphics*>(GR_UnixCairoGraphics::graphicsAllocator(ai));
	pG->setCairo(cr);
	pG->beginPaint(); // needed to avoid cairo reference loss

	UT_Error error = UT_OK;
	PD_Document * pNewDoc = new PD_Document();
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
	pG->endPaint();
	cairo_destroy(cr);
	DELETEP(pPaint);
	cairo_surface_write_to_png(surface, pszPNGFile);
	cairo_surface_destroy(surface);
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
	
	GR_UnixCairoAllocInfo ai(da);
	return XAP_App::getApp()->newGraphics(ai);
}

#ifdef WITH_CHAMPLAIN
#include <champlain-gtk/champlain-gtk.h>
#include <clutter-gtk/clutter-gtk.h>
#endif

/*****************************************************************/

int AP_UnixApp::main(const char * szAppName, int argc, char ** argv)
{
    // This is a static function.

#if !GLIB_CHECK_VERSION(2,32,0)
	if (!g_thread_supported ())
		g_thread_init (NULL);
#endif

    // initialize our application.
	int exit_status = 0;
	AP_UnixApp * pMyUnixApp = new AP_UnixApp(szAppName);

#ifdef WITH_CHAMPLAIN
	ClutterInitError err = gtk_clutter_init (&argc, &argv);
	if (err != CLUTTER_INIT_SUCCESS) {
		g_warning("clutter failed %d, get a life.", err);
	}
#endif

	/* this brace is here to ensure that our local variables on the stack
	 * do not outlive the application object by giving them a lower scope
	 */
	{
#ifdef LOGFILE
		UT_String sLogFile = pMyUnixApp->getUserPrivateDirectory();
		sLogFile += "abiLogFile";
		logfile = fopen(sLogFile.c_str(),"a+");
		fprintf(logfile,"About to do gtk_set_locale \n");
		fprintf(logfile,"New logfile \n");
#endif
		// Step 1: Initialize GTK and create the APP.
		// hack needed to intialize gtk before ::initialize
		setlocale(LC_ALL, "");
		gboolean have_display = gtk_init_check(&argc, &argv);
#ifdef LOGFILE
		fprintf(logfile,"Got display %d \n",have_display);
		fprintf(logfile,"Really display %d \n",have_display);
#endif
		// gtk_init_check() modifies argv/argc if --display is specified
		XAP_Args XArgs = XAP_Args(argc, argv);
		AP_Args Args = AP_Args(&XArgs, szAppName, pMyUnixApp);
		if (have_display > 0) {
			Args.addOptions(gtk_get_option_group(TRUE));
			Args.parseOptions();
		}
		else {
			// no display, but we still need to at least parse our own arguments, damnit, for --to, --to-png, and --print
			Args.addOptions(gtk_get_option_group(FALSE));
			Args.parseOptions();
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
		sa.sa_handler = &XAP_App::signalWrapper;
    
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
	
void AP_UnixApp::errorMsgBadArg(const char *msg)
{
  fprintf (stderr, "%s.\nRun '%s --help' to see a full list of available command line options.\n",
	  msg, g_get_prgname());
}

void AP_UnixApp::errorMsgBadFile(XAP_Frame * pFrame, const char * file, 
				 UT_Error error)
{
  s_CouldNotLoadFileMessage (pFrame, file, error);
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

	if (Args->m_sPrintTo) 
	{
		fprintf(stderr, "%s\n", m_pStringSet->getValue(AP_STRING_ID_COMMAND_LINE_PRINTING_DEPRECATED));

		bSuccess = false;
		return false;
	}
	if (Args->m_iToThumb > 0) 
	{

		if (Args->m_sFiles[0])
	    {
#if 0 // work out how to do this witg pixbuf graphics class later

#endif
			return true;
	    }
		else
	    {
			// couldn't load document
			fprintf(stderr, "Error: no file to convert!\n");
			bSuccess = false;
	    }
		
		return false;
	}

	return openCmdLinePlugins(Args, bSuccess);
}

static gint s_signal_count = 0;

/*!
  This function actually handles signals.  The most commonly recieved
  one is SIGSEGV, the segfault signal.  We want to clean up, save the
  user's files to backup locations (currently <filename>.saved) and then
  call abort, so we still get a core dump that we can debug.
  \param sig_num the integer representing which signal we recieved
*/
void AP_UnixApp::catchSignals(int /*sig_num*/)
{
    // Reset the signal handler
    // (not that it matters - this is mostly for race conditions)
    signal(SIGSEGV, &XAP_App::signalWrapper);

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

    saveRecoveryFiles();

    fflush(stdout);

    // Abort and dump core
    abort();
}
