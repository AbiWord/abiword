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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"

#include "xap_Args.h"
#include "ap_Convert.h"
#include "ap_UnixFrame.h"
#include "ap_UnixApp.h"
#include "sp_spell.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Dialog_Id.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"
#include "xav_View.h"

#include "ie_imp.h"
#include "ie_types.h"
#include "ie_exp_Text.h"
#include "ie_exp_RTF.h"
#include "ie_exp_AbiWord_1.h"
#include "ie_exp_HTML.h"
#include "ie_imp_Text.h"
#include "ie_imp_RTF.h"

#include "gr_Graphics.h"
#include "gr_UnixGraphics.h"
#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_dialogHelper.h"
#include "ut_debugmsg.h"

#include "fv_View.h"
#include "fp_Run.h"

#include "xap_EncodingManager.h"

#ifdef GTK_WIN_POS_CENTER_ALWAYS
#define WIN_POS GTK_WIN_POS_CENTER_ALWAYS
#else
#define WIN_POS GTK_WIN_POS_CENTER
#endif

/*****************************************************************/

/*!
  Construct an AP_UnixApp.
  /param pArgs Arguments from command line
  /param szAppName A string representing the name of the app.
	Currently always AbiWord (I think).
*/
AP_UnixApp::AP_UnixApp(XAP_Args * pArgs, const char * szAppName)
    : XAP_UNIXBASEAPP(pArgs,szAppName)
{
    m_pStringSet = NULL;
    m_pClipboard = NULL;
    
    m_bHasSelection = false;
    m_bSelectionInFlux = false;
    m_cacheDeferClear = false;
    m_pViewSelection = NULL;
    m_pFrameSelection = NULL;
    m_cacheSelectionView = NULL;
}

/*!
  Destructor for AP_UnixApp's.  Cleans up spellcheck, clipboard, and
	StringSet.  
*/
AP_UnixApp::~AP_UnixApp(void)
{
    SpellCheckCleanup();
    
    DELETEP(m_pStringSet);
    DELETEP(m_pClipboard);
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
  Initailize the application.  This involves preferences, keybindings,
  toolbars, graphics, spelling and everything else.  
  /return True if successfully initalized, False otherwise. if false
  the app is unusable, and loading should not continue.   
  /bug This function is 136 lines - way too long.  Needs to be
  refactored, to use a buzzword.  
*/
bool AP_UnixApp::initialize(void)
{
    const char * szUserPrivateDirectory = getUserPrivateDirectory();
    bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
    UT_ASSERT(bVerified);
	
    // load the preferences.
    
    m_prefs = new AP_UnixPrefs(this);
    m_prefs->fullInit();
    
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
    // initializes the spell checker.
    //////////////////////////////////////////////////////////////////
    
    {
#ifndef HAVE_PSPELL
	// we're using ispell
	const char * szISpellDirectory = NULL;
	getPrefsValueDirectory(false,
			       (const XML_Char*)AP_PREF_KEY_SpellDirectory,
			       (const XML_Char**)&szISpellDirectory);
	UT_ASSERT((szISpellDirectory) && (*szISpellDirectory));
	    
	const char * szSpellCheckWordList = NULL;
	getPrefsValue(AP_PREF_KEY_SpellCheckWordList,
		      (const XML_Char**)&szSpellCheckWordList);
	UT_ASSERT((szSpellCheckWordList) && (*szSpellCheckWordList));
	    
	char * szPathname = (char *)calloc(sizeof(char),strlen(szISpellDirectory)+strlen(szSpellCheckWordList)+2);
	UT_ASSERT(szPathname);
	    
	sprintf(szPathname,"%s%s%s",
		szISpellDirectory,
		((szISpellDirectory[strlen(szISpellDirectory)-1]=='/') ? "" : "/"),
		szSpellCheckWordList);
	    
	UT_DEBUGMSG(("Loading SpellCheckWordList [%s]\n",szPathname));
	SpellCheckInit(szPathname);
	free(szPathname);
#else 
	// we're using pspell, it's safe to cast to a char * here
	SpellCheckInit((char *)xap_encoding_manager_get_language_iso_name());
#endif
	    
	// we silently go on if we cannot load it....
    }
	
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
		
	    char * szPathname = (char *)calloc(sizeof(char),strlen(szDirectory)+strlen(szStringSet)+100);
	    UT_ASSERT(szPathname);
		
	    sprintf(szPathname,"%s%s%s.strings",
		    szDirectory,
		    ((szDirectory[strlen(szDirectory)-1]=='/') ? "" : "/"),
		    szStringSet);
		
	    AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);
	    UT_ASSERT(pDiskStringSet);
		
	    if (pDiskStringSet->loadStringsFromDisk(szPathname))
	    {
		pDiskStringSet->setFallbackStringSet(m_pStringSet);
		m_pStringSet = pDiskStringSet;
		UT_DEBUGMSG(("Using StringSet [%s]\n",szPathname));
	    }
	    else
	    {
		DELETEP(pDiskStringSet);
		UT_DEBUGMSG(("Unable to load StringSet [%s] -- using builtin strings instead.\n",szPathname));
	    }
		
	    free(szPathname);
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

    //////////////////////////////////////////////////////////////////

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
void AP_UnixApp::copyToClipboard(PD_DocumentRange * pDocRange)
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
	m_pClipboard->addData(AP_CLIPBOARD_RTF,(UT_Byte *)bufRTF.getPointer(0),bufRTF.getLength());
    if (bufTEXT.getLength() > 0)
	m_pClipboard->addData(AP_CLIPBOARD_TEXTPLAIN_8BIT,(UT_Byte *)bufTEXT.getPointer(0),bufTEXT.getLength());

    return;
}

/*
  I've reordered AP_CLIPBOARD_STRING and AP_CLIPBOARD_TEXTPLAIN_8BIT
  since for non-Latin1 text the data in AP_CLIPBOARD_TEXTPLAIN_8BIT
  format has name of encoding as prefix, and AP_CLIPBOARD_STRING
  doesn't - hvv.
*/
static const char * aszFormatsAccepted[] = { AP_CLIPBOARD_RTF,
					     AP_CLIPBOARD_STRING,
					     AP_CLIPBOARD_TEXTPLAIN_8BIT,
					     0 /* must be last */ };

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
void AP_UnixApp::pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard)
{
    XAP_UnixClipboard::T_AllowGet tFrom = ((bUseClipboard)
					   ? XAP_UnixClipboard::TAG_ClipboardOnly
					   : XAP_UnixClipboard::TAG_PrimaryOnly);

    const char * szFormatFound = NULL;
    unsigned char * pData = NULL;
    UT_uint32 iLen = 0;

    bool bFoundOne = m_pClipboard->getData(tFrom,aszFormatsAccepted,(void**)&pData,&iLen,&szFormatFound);
    if (!bFoundOne)
    {
	UT_DEBUGMSG(("PasteFromClipboard: did not find anything to paste.\n"));
	return;
    }
	
    if (strcmp(szFormatFound,AP_CLIPBOARD_RTF) == 0)
    {
	iLen = MyMin(iLen,strlen((const char *)pData));
	UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in format [%s].\n",iLen,szFormatFound));

	IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
	pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
	DELETEP(pImpRTF);

	return;
    }

    if (   (strcmp(szFormatFound,AP_CLIPBOARD_TEXTPLAIN_8BIT) == 0)
	   || (strcmp(szFormatFound,AP_CLIPBOARD_STRING) == 0))
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
bool AP_UnixApp::canPasteFromClipboard(void)
{
#if 0
    const char * szFormatFound = NULL;
    unsigned char * pData = NULL;
    UT_uint32 iLen = 0;

    XAP_UnixClipboard::T_AllowGet tFrom = XAP_UnixClipboard::TAG_ClipboardOnly;

    // first, try to see if we can paste from the clipboard
    bool bFoundOne = m_pClipboard->getData(tFrom,aszFormatsAccepted,(void**)&pData,&iLen,&szFormatFound);
    if (bFoundOne)
	return true;

    // didn't work, try out the primary selection
    tFrom = XAP_UnixClipboard::TAG_PrimaryOnly;
    bFoundOne = m_pClipboard->getData(tFrom,aszFormatsAccepted,(void**)&pData,&iLen,&szFormatFound);
    return bFoundOne;
#else
    return true;
#endif
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
	m_bHasSelection = bSelectionStateInThisView;
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
	m_bHasSelection = bSelectionStateInThisView;
	m_pClipboard->clearData(false,true);
    }
	
    UT_DEBUGMSG(("here we go whooooo\n"));
    setViewSelection(pView);
    m_pFrameSelection = (XAP_Frame *)pView->getParentData();

    m_bSelectionInFlux = false;
    return;
}
#if 0
void    AP_UnixApp::setViewSelection( AV_View * pView)
{
    m_pViewSelection = pView;
}

AV_View* AP_UnixApp::getViewSelection(void)
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
	UT_DEBUGMSG(("crash2\n"));
	FV_View *pView = static_cast<FV_View *>(m_pViewSelection);
	pView->cmdUnselectSelection();
	m_bHasSelection = false;
    }
	
    m_bSelectionInFlux = false;
    return;
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
	    m_pClipboard->clearData(false,true);
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

	if (strcmp(formatList[j],AP_CLIPBOARD_RTF) == 0)
	{
	    IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(dr.m_pDoc);
	    if (!pExpRtf)
		return false;		// give up on memory errors

	    pExpRtf->copyToBuffer(&dr,&m_selectionByteBuf);
	    DELETEP(pExpRtf);
	    goto ReturnThisBuffer;
	}
			
	if (   (strcmp(formatList[j],AP_CLIPBOARD_TEXTPLAIN_8BIT) == 0)
	       || (strcmp(formatList[j],AP_CLIPBOARD_STRING) == 0))
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

    /*
      What this does is look in the the user's private AbiSuite
      directory (usually ~/.AbiSuite) for the file splash.png and
      use that instead of the standard splash image on startup.
      The only use for this that I can see would be testing new
      images.  
    */

#ifdef DEBUG
    const char * szDirectory = _getUserPrivateDirectory();

    const char * szFile = "splash.png";

    char * buf;

    if (strlen(szDirectory) + strlen(szFile) + 2 >= PATH_MAX)
	buf = NULL;
    else
    {
	buf = (char *)malloc(strlen(szDirectory) + strlen(szFile) + 2);
	memset(buf,0,sizeof(buf));
	strcpy(buf,szDirectory);
	int len = strlen(buf);
	if ( (len == 0) || (buf[len-1] != '/') )
	    strcat(buf,"/");
	strcat(buf,szFile);
    }
#endif
		
    // store value for use by the expose event, which attaches the timer
    splashTimeoutValue = delay;
	
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
	// create a centered window the size of our image
	wSplash = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_default_size (GTK_WINDOW (wSplash),
				     iSplashWidth, iSplashHeight);
	gtk_window_set_policy(GTK_WINDOW(wSplash), FALSE, FALSE, FALSE);

	// create a frame to add depth
	GtkWidget * frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(wSplash), frame);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_widget_show(frame);

	// create a drawing area
	GtkWidget * da = gtk_drawing_area_new ();
	gtk_widget_set_events(da, GDK_ALL_EVENTS_MASK);
	gtk_drawing_area_size(GTK_DRAWING_AREA (da), iSplashWidth, iSplashHeight);
	gtk_signal_connect(GTK_OBJECT(da), "expose_event",
			   GTK_SIGNAL_FUNC(s_drawingarea_expose), NULL);
	gtk_signal_connect(GTK_OBJECT(da), "button_press_event",
			   GTK_SIGNAL_FUNC(s_button_event), NULL);
	gtk_container_add(GTK_CONTAINER(frame), da);
	gtk_widget_show(da);

	// now bring the window up front & center
	gtk_window_set_position(GTK_WINDOW(wSplash), WIN_POS);

	// create the window so we can attach a GC to it
	gtk_widget_show(wSplash);
		
	// create image context
	pUnixGraphics = new GR_UnixGraphics(da->window, NULL, m_pApp);
	pSplashImage = pUnixGraphics->createNewImage("splash", pBB, iSplashWidth, iSplashHeight);

	// another for luck (to bring it up forward and paint)
	gtk_widget_show(wSplash);

	// trigger an expose event to get us started
	s_drawingarea_expose(da, NULL);
    }

    DELETEP(pBB);
#ifdef DEBUG
    DELETEP(buf);
#endif

    return pSplashImage;
}

/*****************************************************************/

int AP_UnixApp::main(const char * szAppName, int argc, char ** argv)
{
    // This is a static function.
    
    UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
    UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
    UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
    UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
    UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
    UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));
    
    // initialize our application.
    
    XAP_Args Args = XAP_Args(argc,argv);
    
    bool bShowSplash = true;
    bool bShowApp = true;
    bool bTo = false;
    bool bShow = false;
    bool bNoSplash = false;
    bool bHelp = false;
    
    for (int k = 1; k < Args.m_argc; k++)
	if (*Args.m_argv[k] == '-')
	{
	    // Do a quick and dirty find for "-to"
	    if ((strcmp(Args.m_argv[k],"-to") == 0)
		|| (strcmp(Args.m_argv[k],"--to") == 0))
		bTo = true;
		
		// Do a quick and dirty find for "-show"
	    else if ((strcmp(Args.m_argv[k],"-show") == 0)
		     || (strcmp(Args.m_argv[k],"--show") == 0))
		bShow = true;
		
		// Do a quick and dirty find for "-nosplash"
	    else if ((strcmp(Args.m_argv[k],"-nosplash") == 0)
		     || (strcmp(Args.m_argv[k],"--nosplash") == 0))
		bNoSplash = true;
		
		// Do a quick and dirty find for "-help",
		// "--help" or "-h"
	    else if (strncmp(Args.m_argv[k],"-h",2) == 0 ||
		     strncmp(Args.m_argv[k],"--h",3) == 0)
		bHelp = true;
	}
    
    if((bTo && !bShow) || bHelp)
    {
	bShowSplash = false;
	bShowApp = false;
    }
    else if (bNoSplash)
	bShowSplash = false;
    
    // HACK : these calls to gtk reside properly in XAP_UNIXBASEAPP::initialize(),
    // HACK : but need to be here to throw the splash screen as
    // HACK : soon as possible.
	
    if (bShowSplash || bShowApp)
    {
	gtk_set_locale();
	gtk_init(&Args.m_argc,&Args.m_argv);
    }
    
    if (bShowSplash)
	_showSplash(2000);
    
    AP_UnixApp * pMyUnixApp = new AP_UnixApp(&Args, szAppName);
    
    if(bHelp)
    {
	/* there's no need to stay around any longer than
	   neccessary.  */
	pMyUnixApp->_printUsage();
	delete pMyUnixApp;
	return 0;
    }
    
    // Setup signal handlers, primarily for segfault
    // If we segfaulted before here, we *really* blew it
    
    struct sigaction sa;
    
    sa.sa_handler = signalWrapper;
    
    sigfillset(&sa.sa_mask);  // We don't want to hear about other signals
    sigdelset(&sa.sa_mask, SIGABRT); // But we will call abort(), so we can't ignore that

#ifndef AIX
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
    
    if (bShowApp)
    {
	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyUnixApp->initialize())
	{
	    delete pMyUnixApp;
	    return -1;	// make this something standard?
	}
    }
    
    // this function takes care of all the command line args.
    // if some args are botched, it returns false and we should
    // continue out the door.
    if (pMyUnixApp->parseCommandLine() && bShowApp)
    {
	// turn over control to gtk
	gtk_main();
	pMyUnixApp->shutdown();
    }
    
    // destroy the App.  It should take care of deleting all frames.
    delete pMyUnixApp;
    
    return 0;
}

/*!
  parse the command line
  <app> -[option]* [<documentname>]*
  
  \todo when we refactor the App classes, consider moving
	this to app-specific, cross-platform.

  \todo replace this with getopt or something similar.
  
  Unix puts the program name in argv[0], so [1] is the first argument.

  \return False if an unknow command line option was used, true
  otherwise.  
*/
bool AP_UnixApp::parseCommandLine(void)
{
    
    int nFirstArg = 1;
    int k;
    int kWindowsOpened = 0;
    char *to = NULL;
    int verbose = 1;
    bool show = false;
    
    for (k=nFirstArg; (k<m_pArgs->m_argc); k++)
    {
	if (*m_pArgs->m_argv[k] == '-')
	{
	    
#if 0
	    // This code is currently unused.
	    // We don't load libraries, and we don't
	    // have scripts
	    
	    if ((strcmp(m_pArgs->m_argv[k],"-script") == 0)
		|| (strcmp(m_pArgs->m_argv[k],"--script") == 0))
	    {
		// [-script scriptname]
		k++;
	    }
	    else if ((strcmp(m_pArgs->m_argv[k],"-lib") == 0)
		     || (strcmp(m_pArgs->m_argv[k],"--lib") == 0))
	    {
		// [-lib <AbiSuiteLibDirectory>]
		// we've already processed this when we initialized the App class
		k++;
	    }
#endif
	    
	    if ((strcmp(m_pArgs->m_argv[k],"-dumpstrings") == 0)
		|| (strcmp(m_pArgs->m_argv[k],"--dumpstrings") == 0))
	    {
		// [-dumpstrings]
#ifdef DEBUG
       		// dump the string table in english as a template for translators.
		// see abi/docs/AbiSource_Localization.abw for details.
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,(XML_Char*)AP_PREF_DEFAULT_StringSet);
		pBuiltinStringSet->dumpBuiltinSet("en-US.strings");
		delete pBuiltinStringSet;
#endif
	    }
	    else if ((strcmp(m_pArgs->m_argv[k],"-nosplash") == 0)
		     || (strcmp(m_pArgs->m_argv[k],"--nosplash") == 0))
	    {
		// we've alrady processed this before we initialized the App class
	    }
	    else if ((strcmp(m_pArgs->m_argv[k],"-geometry") == 0)
		     || (strcmp(m_pArgs->m_argv[k],"--geometry") == 0))
	    {
		// [-geometry <X geometry string>]
		// let us at the next argument
		k++;
		
		// TODO : does X have a dummy geometry value reserved for this?
		gint dummy = 1 << ((sizeof(gint) * 8) - 1);
		gint x = dummy;
		gint y = dummy;
		guint width = 0;
		guint height = 0;
		    
		XParseGeometry(m_pArgs->m_argv[k], &x, &y, &width, &height);
		
		// use both by default
		XAP_UNIXBASEAPP::windowGeometryFlags f = (XAP_UNIXBASEAPP::windowGeometryFlags)
		    (XAP_UNIXBASEAPP::GEOMETRY_FLAG_SIZE
		     | XAP_UNIXBASEAPP::GEOMETRY_FLAG_POS);
		
		// if pos (x and y) weren't provided just use size
		if (x == dummy || y == dummy)
		    f = XAP_UNIXBASEAPP::GEOMETRY_FLAG_SIZE;
		    
      		// if size (width and height) weren't provided just use pos
		if (width == 0 || height == 0)
		    f = XAP_UNIXBASEAPP::GEOMETRY_FLAG_POS;
		
		// set the xap-level geometry for future frame use
		setGeometry(x, y, width, height, f);
	    }
	    else if ((strcmp (m_pArgs->m_argv[k],"-to") == 0)
		     || (strcmp (m_pArgs->m_argv[k],"--to") == 0))
	    {
		k++;
		to = m_pArgs->m_argv[k];
	    }
	    else if ((strcmp (m_pArgs->m_argv[k], "-show") == 0)
		     || (strcmp (m_pArgs->m_argv[k], "--show") == 0))
	    {
		show = true;
	    }
	    else if ((strcmp (m_pArgs->m_argv[k], "-verbose") == 0)
		     || (strcmp (m_pArgs->m_argv[k], "--verbose") == 0))
	    {
		k++;
		if(k<m_pArgs->m_argc)
		{
		    /* if we don't check we segfault
		       when there aren't any numbers
		       after --verbose
		    */
		    verbose = atoi (m_pArgs->m_argv[k]);
			
		}
	    }
	    else  
	    {
		UT_DEBUGMSG(("Unknown command line option [%s]\n",m_pArgs->m_argv[k]));
				// TODO don't know if it has a following argument or not -- assume not
		_printUsage();
		return false;
	    }
	}
	else
	{
	    // [filename]
	    if (to) 
	    {
		AP_Convert * conv = new AP_Convert();
		conv->setVerbose(verbose);
		conv->convertTo(m_pArgs->m_argv[k], to);
		delete conv;
	    }
	    else
	    {
		AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(this);
		pFirstUnixFrame->initialize();
		UT_Error error = pFirstUnixFrame->loadDocument(m_pArgs->m_argv[k], IEFT_Unknown);
		if (!error)
		{
		    kWindowsOpened++;
		}
		else
		{
		    // TODO: warn user that we couldn't open that file
		    
#if 1
		    // TODO we crash if we just delete this without putting something
		    // TODO in it, so let's go ahead and open an untitled document
		    // TODO for now.  this would cause us to get 2 untitled documents
		    // TODO if the user gave us 2 bogus pathnames....
		    kWindowsOpened++;
		    pFirstUnixFrame->loadDocument(NULL, IEFT_Unknown);
		    
		    pFirstUnixFrame->raise();
		    
		    XAP_DialogFactory * pDialogFactory
			= (XAP_DialogFactory *)(pFirstUnixFrame->getDialogFactory());
		    
		    XAP_Dialog_MessageBox * pDialog
			= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
		    UT_ASSERT(pDialog);
		    
		    const XAP_StringSet * pSS = pFirstUnixFrame->getApp()->getStringSet();
		    
		    switch (error)
		    {
		    case -301:
			pDialog->setMessage((char*)pSS->getValue(AP_STRING_ID_MSG_IE_FileNotFound),m_pArgs->m_argv[k]);
			break;
			
		    case -302:
			pDialog->setMessage((char*)pSS->getValue(AP_STRING_ID_MSG_IE_NoMemory),m_pArgs->m_argv[k]);
			break;
			
		    case -303:
			pDialog->setMessage((char*)pSS->getValue(AP_STRING_ID_MSG_IE_UnknownType),m_pArgs->m_argv[k]);
			break;
			
		    case -304:
			pDialog->setMessage((char*)pSS->getValue(AP_STRING_ID_MSG_IE_BogusDocument),m_pArgs->m_argv[k]);
			break;
			
		    case -305:
			pDialog->setMessage((char*)pSS->getValue(AP_STRING_ID_MSG_IE_CouldNotOpen),m_pArgs->m_argv[k]);
			break;
			
		    case -306:
			pDialog->setMessage((char*)pSS->getValue(AP_STRING_ID_MSG_IE_CouldNotWrite),m_pArgs->m_argv[k]);
			break;
			
		    case -307:
			pDialog->setMessage((char*)pSS->getValue(AP_STRING_ID_MSG_IE_FakeType),m_pArgs->m_argv[k]);
			break;
			
		    case -311:
			pDialog->setMessage((char*)pSS->getValue(AP_STRING_ID_MSG_IE_UnsupportedType),m_pArgs->m_argv[k]);
			break;
			
		    default:
			pDialog->setMessage((char*)pSS->getValue(AP_STRING_ID_MSG_ImportError),m_pArgs->m_argv[k]);
		    }
		    pDialog->setButtons(XAP_Dialog_MessageBox::b_O);
		    pDialog->setDefaultAnswer(XAP_Dialog_MessageBox::a_OK);
		    
		    pDialog->runModal(pFirstUnixFrame);
		    
		    //XAP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();
		    
		    pDialogFactory->releaseDialog(pDialog);
		    
#else
		    delete pFirstUnixFrame;
#endif
		}
	    }
	}
    }
	
    // command-line conversion may not open any windows at all
    if (to && !show)
	return true;
    
    if (kWindowsOpened == 0)
    {
	// no documents specified or were able to be opened, open an untitled one
	
	AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(this);
	pFirstUnixFrame->initialize();
	pFirstUnixFrame->loadDocument(NULL, IEFT_Unknown);
    }
    
    return true;
}

/*!
  This is a global function to call our signal handler.  It needs to
  be global so that we can pass a function pointer to it to C code
  that handles signals.  
  \todo Could this be a static member function?
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
  user's files to backup locations (currently <filename>.bak) and then
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
	UT_DEBUGMSG(("Segfault during filesave - no file saved  \n"));
	fflush(stdout);
	abort();
    }
    
    UT_DEBUGMSG(("Oh no - we just segfaulted!\n"));
	
    UT_uint32 i = 0;
    for(;i<m_vecFrames.getItemCount();i++)
    {
	AP_UnixFrame * curFrame = (AP_UnixFrame*) m_vecFrames[i];
	UT_ASSERT(curFrame);
	curFrame->backup();
    }
    
    fflush(stdout);
    
    // Abort and dump core
    abort();
}
	
