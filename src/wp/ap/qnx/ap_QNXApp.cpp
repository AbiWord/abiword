/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <Pt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ut_debugmsg.h"
#include "ut_misc.h"

#include "xap_Args.h"
#include "ap_QNXFrame.h"
#include "ap_QNXApp.h"
#include "sp_spell.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
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
#include "gr_QNXGraphics.h"
#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_debugmsg.h"

#include "fv_View.h"

/*****************************************************************/

AP_QNXApp::AP_QNXApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_QNXApp(pArgs,szAppName)
{
	m_prefs = NULL;
	m_pStringSet = NULL;
	m_pClipboard = NULL;

	m_bHasSelection = UT_FALSE;
	m_bSelectionInFlux = UT_FALSE;
	m_cacheDeferClear = UT_FALSE;
	m_pViewSelection = NULL;
	m_pFrameSelection = NULL;
	m_cacheSelectionView = NULL;
}

AP_QNXApp::~AP_QNXApp(void)
{
	SpellCheckCleanup();

	DELETEP(m_prefs);
	DELETEP(m_pStringSet);
	DELETEP(m_pClipboard);
}

static UT_Bool s_createDirectoryIfNecessary(const char * szDir)
{
	struct stat statbuf;
	
	if (stat(szDir,&statbuf) == 0)								// if it exists
	{
		if (S_ISDIR(statbuf.st_mode))							// and is a directory
			return UT_TRUE;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return UT_FALSE;
	}
	
	if (mkdir(szDir,0700) == 0)
		return UT_TRUE;
	

	UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
	return UT_FALSE;
}	

UT_Bool AP_QNXApp::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	UT_Bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
	UT_ASSERT(bVerified);
	
	// load the preferences.

	m_prefs = new AP_QNXPrefs(this);
	m_prefs->fullInit();
	
	// now that preferences are established, let the xap init
		   
	m_pClipboard = new AP_QNXClipboard(this);
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

	if (! XAP_QNXApp::initialize())
		return UT_FALSE;
	
	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
	{
		const char * szISpellDirectory = NULL;
		getPrefsValueDirectory(UT_FALSE,AP_PREF_KEY_SpellDirectory,&szISpellDirectory);
		UT_ASSERT((szISpellDirectory) && (*szISpellDirectory));

		const char * szSpellCheckWordList = NULL;
		getPrefsValue(AP_PREF_KEY_SpellCheckWordList,&szSpellCheckWordList);
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
		
		// we silently go on if we cannot load it....
	}
	
	//////////////////////////////////////////////////////////////////
	// load the dialog and message box strings
	//////////////////////////////////////////////////////////////////
	
	{
		// assume we will be using the builtin set (either as the main
		// set or as the fallback set).
		
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,AP_PREF_DEFAULT_StringSet);
		UT_ASSERT(pBuiltinStringSet);
		m_pStringSet = pBuiltinStringSet;

		// see if we should load an alternate set from the disk
		
		const char * szDirectory = NULL;
		const char * szStringSet = NULL;

		if (   (getPrefsValue(AP_PREF_KEY_StringSet,&szStringSet))
			&& (szStringSet)
			&& (*szStringSet)
			&& (UT_stricmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
			getPrefsValueDirectory(UT_TRUE,AP_PREF_KEY_StringSetDirectory,&szDirectory);
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

	//////////////////////////////////////////////////////////////////

	return UT_TRUE;
}

XAP_Frame * AP_QNXApp::newFrame(void)
{
	AP_QNXFrame * pQNXFrame = new AP_QNXFrame(this);

	if (pQNXFrame)
		pQNXFrame->initialize();

	return pQNXFrame;
}

UT_Bool AP_QNXApp::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

	return UT_TRUE;
}

UT_Bool AP_QNXApp::getPrefsValueDirectory(UT_Bool bAppSpecific,
										   const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return UT_FALSE;

	const XML_Char * psz = NULL;
	if (!m_prefs->getPrefsValue(szKey,&psz))
		return UT_FALSE;

	if (*psz == '/')
	{
		*pszValue = psz;
		return UT_TRUE;
	}

	const XML_Char * dir = ((bAppSpecific) ? getAbiSuiteAppDir() : getAbiSuiteLibDir());

	static XML_Char buf[1024];
	UT_ASSERT((strlen(dir) + strlen(psz) + 2) < sizeof(buf));
	
	sprintf(buf,"%s/%s",dir,psz);
	*pszValue = buf;
	return UT_TRUE;
}

const char * AP_QNXApp::getAbiSuiteAppDir(void) const
{
	// we return a static string, use it quickly.
	
	static XML_Char buf[1024];
	UT_ASSERT((strlen(getAbiSuiteLibDir()) + strlen(ABIWORD_APP_LIBDIR) + 2) < sizeof(buf));

	sprintf(buf,"%s/%s",getAbiSuiteLibDir(),ABIWORD_APP_LIBDIR);
	return buf;
}

const XAP_StringSet * AP_QNXApp::getStringSet(void) const
{
	return m_pStringSet;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_QNXApp::copyToClipboard(PD_DocumentRange * pDocRange)
{
	// copy the given subset of the given document to the
	// system clipboard in a variety of formats.
	//
	// to minimize the effects of race-conditions, we create
	// all of the buffers we need and then post them to the
	// server (well sorta) all at one time.

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
	// m_pClipboard->clearData(UT_TRUE,UT_FALSE);
	
	if (bufRTF.getLength() > 0)
		m_pClipboard->addData(AP_CLIPBOARD_RTF,(UT_Byte *)bufRTF.getPointer(0),bufRTF.getLength());
	if (bufTEXT.getLength() > 0)
		m_pClipboard->addData(AP_CLIPBOARD_TEXTPLAIN_8BIT,(UT_Byte *)bufTEXT.getPointer(0),bufTEXT.getLength());

	return;
}

void AP_QNXApp::pasteFromClipboard(PD_DocumentRange * pDocRange, UT_Bool bUseClipboard)
{
	// paste from the system clipboard using the best-for-us format
	// that is present.  try to get the content in the order listed.

	static const char * aszFormatsAccepted[] = { AP_CLIPBOARD_RTF,
												 AP_CLIPBOARD_TEXTPLAIN_8BIT,
												 0 /* must be last */ };

	// TODO currently i have this set so that a ^v or Menu[Edit/Paste] will
	// TODO use the CLIPBOARD property and a MiddleMouseClick will use the
	// TODO PRIMARY property -- this seems to be the "X11 way" (sigh).
	// TODO consider having a preferences switch to allow ^v and Menu[Edit/Paste]
	// TODO to use the most recent property... this might be a nice way of
	// TODO unifying things -- or it might not -- this is probably an area
	// TODO for investigation or some usability testing.
	
	XAP_QNXClipboard::T_AllowGet tFrom = ((bUseClipboard)
										   ? XAP_QNXClipboard::TAG_ClipboardOnly
										   : XAP_QNXClipboard::TAG_PrimaryOnly);

	const char * szFormatFound = NULL;
	unsigned char * pData = NULL;
	UT_uint32 iLen = 0;

	UT_Bool bFoundOne = m_pClipboard->getData(tFrom,aszFormatsAccepted,(void**)&pData,&iLen,&szFormatFound);
	if (!bFoundOne)
	{
		UT_DEBUGMSG(("PasteFromClipboard: did not find anything to paste.\n"));
		return;
	}
	
	if (UT_stricmp(szFormatFound,AP_CLIPBOARD_RTF) == 0)
	{
		iLen = MyMin(iLen,strlen((const char *)pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in format [%s].\n",iLen,szFormatFound));

		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);

		return;
	}

	if (UT_stricmp(szFormatFound,AP_CLIPBOARD_TEXTPLAIN_8BIT) == 0)
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

UT_Bool AP_QNXApp::canPasteFromClipboard(void)
{
	// TODO fix this...
	return UT_TRUE;
}

/*****************************************************************/
/*****************************************************************/

void AP_QNXApp::setSelectionStatus(AV_View * pView)
{
	// this is called by the view-listeners when the state
	// of the X Selection is changed by the user on one of
	// our windows.
	//
	// we need to notify the clipboard so that it can assert
	// or release the X Selection.
	//
	// we remember the last view that called us so that
	// clearSelection() can do it's job when another application
	// asserts the X Selection.

	if (m_bSelectionInFlux)
		return;
	m_bSelectionInFlux = UT_TRUE;

	UT_Bool bSelectionStateInThisView = ( ! pView->isSelectionEmpty() );
	
	if (m_pViewSelection && m_pFrameSelection && m_bHasSelection && (pView != m_pViewSelection))
	{
		// one window has a selection currently and another window just
		// asserted one.  we force clear the old one to enforce the X11
		// style.

		m_pViewSelection->cmdUnselectSelection();
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
		m_cacheDeferClear = UT_TRUE;
	}
	else
	{
		m_bHasSelection = bSelectionStateInThisView;
		m_pClipboard->clearData(UT_FALSE,UT_TRUE);
	}
	
	m_pViewSelection = pView;
	m_pFrameSelection = (XAP_Frame *)pView->getParentData();

	m_bSelectionInFlux = UT_FALSE;
	return;
}

UT_Bool AP_QNXApp::forgetFrame(XAP_Frame * pFrame)
{
	// we intercept this so that we can erase our
	// selection-related variables if necessary.
	// wouldn't want to hold onto a stale frame or
	// view pointer of a closed window when the
	// selection is changed....

	if (m_pFrameSelection && (pFrame==m_pFrameSelection))
	{
		m_pClipboard->clearData(UT_FALSE,UT_TRUE);
		m_pFrameSelection = NULL;
		m_pViewSelection = NULL;
	}
	
	return XAP_App::forgetFrame(pFrame);
}

void AP_QNXApp::clearSelection(void)
{
	// this method goes with setSelectionStatus().
	//
	// we are called by the clipboard (thru the callback chain)
	// in response to another application stealing the X Selection.
	//
	// we need to notify the view so that it can clear the screen
	// as is the custom on X -- only one selection at any time.
	//
	// we have to watch out here because when we call up to clear
	// the selection, the view will notify the view-listeners of
	// the change, which may cause setSelectionStatus() to get
	// called and thus update the clipboard -- this could recurse
	// a while....

	if (m_bSelectionInFlux)
		return;
	m_bSelectionInFlux = UT_TRUE;
	
	if (m_pViewSelection && m_pFrameSelection && m_bHasSelection)
	{
		m_pViewSelection->cmdUnselectSelection();
		m_bHasSelection = UT_FALSE;
	}
	
	m_bSelectionInFlux = UT_FALSE;
	return;
}

void AP_QNXApp::cacheCurrentSelection(AV_View * pView)
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
#if defined(__WATCOMC__)
		FV_View * pFVView = (FV_View *)(pView);
#else
		FV_View * pFVView = static_cast<FV_View *>(pView);
#endif
		pFVView->getDocumentRangeOfCurrentSelection(&m_cacheDocumentRangeOfSelection);

		m_cacheSelectionView = pView;
		UT_DEBUGMSG(("Clipboard::cacheCurrentSelection: [view %p][range %d %d]\n",
					 pFVView,
					 m_cacheDocumentRangeOfSelection.m_pos1,
					 m_cacheDocumentRangeOfSelection.m_pos2));
		m_cacheDeferClear = UT_FALSE;
	}
	else
	{
		if (m_cacheDeferClear)
		{
			m_cacheDeferClear = UT_FALSE;
			m_bHasSelection = UT_FALSE;
			m_pClipboard->clearData(UT_FALSE,UT_TRUE);
		}
		m_cacheSelectionView = NULL;
	}

	return;
}

UT_Bool AP_QNXApp::getCurrentSelection(const char** formatList,
										void ** ppData, UT_uint32 * pLen,
										const char **pszFormatFound)
{
	// get the current contents of the selection in the
	// window last known to have a selection using one
	// of the formats in the given list.

	int j;
	
	*ppData = NULL;				// assume failure
	*pLen = 0;
	*pszFormatFound = NULL;
	
	if (!m_pViewSelection || !m_pFrameSelection || !m_bHasSelection)
		return UT_FALSE;		// can't do it, give up.

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
#if defined(__WATCOMC__)
		FV_View * pFVView = (FV_View *)(m_pViewSelection);
#else
		FV_View * pFVView = static_cast<FV_View *>(m_pViewSelection);
#endif
	
		pFVView->getDocumentRangeOfCurrentSelection(&dr);
		UT_DEBUGMSG(("Clipboard::getCurrentSelection: [view %p][range %d %d]\n",pFVView,dr.m_pos1,dr.m_pos2));
	}
	
	m_selectionByteBuf.truncate(0);

	for (j=0; (formatList[j]); j++)
	{
		UT_DEBUGMSG(("Clipboard::getCurrentSelection: considering format [%s]\n",formatList[j]));

		if (UT_stricmp(formatList[j],AP_CLIPBOARD_RTF) == 0)
		{
			IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(dr.m_pDoc);
			if (!pExpRtf)
				return UT_FALSE;		// give up on memory errors

			pExpRtf->copyToBuffer(&dr,&m_selectionByteBuf);
			DELETEP(pExpRtf);
			goto ReturnThisBuffer;
		}
			
		if (   (UT_stricmp(formatList[j],AP_CLIPBOARD_TEXTPLAIN_8BIT) == 0)
			|| (UT_stricmp(formatList[j],AP_CLIPBOARD_STRING) == 0))
		{
			IE_Exp_Text * pExpText = new IE_Exp_Text(dr.m_pDoc);
			if (!pExpText)
				return UT_FALSE;

			pExpText->copyToBuffer(&dr,&m_selectionByteBuf);
			DELETEP(pExpText);
			goto ReturnThisBuffer;
		}

		// TODO add other formats as necessary
	}

	UT_DEBUGMSG(("Clipboard::getCurrentSelection: cannot create anything in one of requested formats.\n"));
	return UT_FALSE;

ReturnThisBuffer:
	UT_DEBUGMSG(("Clipboard::getCurrentSelection: copying %d bytes in format [%s].\n",
				 m_selectionByteBuf.getLength(),formatList[j]));
	*ppData = (void *)m_selectionByteBuf.getPointer(0);
	*pLen = m_selectionByteBuf.getLength();
	*pszFormatFound = formatList[j];
	return UT_TRUE;
}

/*****************************************************************/
/*****************************************************************/

static void * wSplash = NULL;
static GR_Image * pSplashImage = NULL;
static GR_QNXGraphics * pQNXGraphics = NULL;
static UT_Bool firstExpose = UT_FALSE;
static UT_uint32 splashTimeoutValue = 0;

#if 0
static int s_hideSplash(void * /*data*/)
{
	if (wSplash)
	{
		gtk_timeout_remove(timeout_handler);
		gtk_widget_destroy(wSplash);
		wSplash = NULL;
		DELETEP(pQNXGraphics);
		DELETEP(pSplashImage);
	}
	return UT_TRUE;
}

// GTK never seems to let me have this event on a pop-up style window
//static void s_key_event(GtkWidget * /*window*/, GdkEventKey * /*key*/)
//{
//	s_hideSplash(NULL);
//}

static void s_button_event(void * /*window*/)
{
	s_hideSplash(NULL);
}

static int s_drawingarea_expose(GtkWidget * /* widget */,
								 GdkEventExpose * /* pExposeEvent */)
{
	if (pQNXGraphics && pSplashImage)
	{
		pQNXGraphics->drawImage(pSplashImage, 0, 0);

		// on the first full paint of the image, start a 2 second timer
		if (!firstExpose)
		{
			firstExpose = UT_TRUE;
			timeout_handler = gtk_timeout_add(splashTimeoutValue, s_hideSplash, NULL);
		}
	}

	return FALSE;
}
#endif

// szFile is optional; a NULL pointer will use the default splash screen.
// The delay is how long the splash should stay on screen in milliseconds.
static GR_Image * _showSplash(UT_uint32 delay)
{
	wSplash = NULL;
	pSplashImage = NULL;

	UT_ByteBuf* pBB = NULL;

	// use a default if they haven't specified anything
	const char * szFile = "splash.png";

	// store value for use by the expose event, which attaches the timer
	splashTimeoutValue = delay;
	
	extern unsigned char g_pngSplash[];		// see ap_wp_Splash.cpp
	extern unsigned long g_pngSplash_sizeof;	// see ap_wp_Splash.cpp

	pBB = new UT_ByteBuf();
	if (
		(pBB->insertFromFile(0, szFile))
		|| (pBB->ins(0, g_pngSplash, g_pngSplash_sizeof))
		)
	{
#if 0
		// get splash size
		UT_sint32 iSplashWidth;
		UT_sint32 iSplashHeight;
		UT_PNG_getDimensions(pBB, iSplashWidth, iSplashHeight);

		// create a centered window the size of our image
		wSplash = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_object_set_data(GTK_OBJECT(wSplash), "wSplash", wSplash);
		gtk_widget_set_usize(wSplash, iSplashWidth, iSplashHeight);
		gtk_window_set_policy(GTK_WINDOW(wSplash), FALSE, FALSE, FALSE);

		// create a frame to add depth
		GtkWidget * frame = gtk_frame_new(NULL);
		gtk_object_set_data(GTK_OBJECT(wSplash), "frame", frame);
		gtk_container_add(GTK_CONTAINER(wSplash), frame);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
		gtk_widget_show(frame);

		// create a drawing area
		GtkWidget * da = gtk_drawing_area_new ();
		gtk_object_set_data(GTK_OBJECT(wSplash), "da", da);
		gtk_widget_set_events(da, GDK_ALL_EVENTS_MASK);
		gtk_widget_set_usize(da, iSplashWidth, iSplashHeight);
		gtk_signal_connect(GTK_OBJECT(da), "expose_event",
						   GTK_SIGNAL_FUNC(s_drawingarea_expose), NULL);
//		gtk_signal_connect(GTK_OBJECT(da), "key_press_event",
//						   GTK_SIGNAL_FUNC(s_key_event), NULL);
		gtk_signal_connect(GTK_OBJECT(da), "button_press_event",
						   GTK_SIGNAL_FUNC(s_button_event), NULL);
		gtk_container_add(GTK_CONTAINER(frame), da);
		gtk_widget_show(da);

		// now bring the window up front & center
		gtk_window_set_position(GTK_WINDOW(wSplash), GTK_WIN_POS_CENTER);

		// create the window so we can attach a GC to it
		gtk_widget_show(wSplash);

		// create image context
		pQNXGraphics = new GR_QNXGraphics(da->window, NULL);
		pSplashImage = pQNXGraphics->createNewImage("splash", pBB, iSplashWidth, iSplashHeight);

		// another for luck (to bring it up forward and paint)
		gtk_widget_show(wSplash);
#endif
	}

	DELETEP(pBB);

	return pSplashImage;
}

/*****************************************************************/

int AP_QNXApp::main(const char * szAppName, int argc, char ** argv)
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

	// Do a quick and dirty find for "-nosplash"
	UT_Bool bShowSplash = UT_TRUE;
	for (int k = 1; k < Args.m_argc; k++)
		if (*Args.m_argv[k] == '-')
			if (UT_stricmp(Args.m_argv[k],"-nosplash") == 0)
			{
				bShowSplash = UT_FALSE;
				break;
			}

	if (bShowSplash)
		_showSplash(2000);
			
	AP_QNXApp * pMyQNXApp = new AP_QNXApp(&Args, szAppName);

	// if the initialize fails, we don't have icons, fonts, etc.
	printf("App: Calling initialize \n");
	if (!pMyQNXApp->initialize())
	{
		delete pMyQNXApp;
		return -1;	// make this something standard?
	}

	// this function takes care of all the command line args.
	// if some args are botched, it returns false and we should
	// continue out the door.
	printf("App: Calling parseCommandLine \n");
	if (pMyQNXApp->parseCommandLine())
	{
		printf("App: Turning over to photon \n");
		PtMainLoop();
	}
	printf("App: Exiting \n");
	
	// destroy the App.  It should take care of deleting all frames.
	pMyQNXApp->shutdown();
	delete pMyQNXApp;
	
	return 0;
}

UT_Bool AP_QNXApp::parseCommandLine(void)
{
	// parse the command line
	// <app> [-script <scriptname>]* [-dumpstrings] [<documentname>]*
        
	// TODO when we refactor the App classes, consider moving
	// TODO this to app-specific, cross-platform.

	// TODO replace this with getopt or something similar.
        
	// QNX puts the program name in argv[0], so [1] is the first argument.

	int nFirstArg = 1;
	int k;
	int kWindowsOpened = 0;
        
	for (k=nFirstArg; (k<m_pArgs->m_argc); k++)
	{
		if (*m_pArgs->m_argv[k] == '-')
		{
			if (UT_stricmp(m_pArgs->m_argv[k],"-script") == 0)
			{
				// [-script scriptname]
				k++;
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-lib") == 0)
			{
				// [-lib <AbiSuiteLibDirectory>]
				// we've already processed this when we initialized the App class
				k++;
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-dumpstrings") == 0)
			{
				// [-dumpstrings]
#ifdef DEBUG
				// dump the string table in english as a template for translators.
				// see abi/docs/AbiSource_Localization.abw for details.
				AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,AP_PREF_DEFAULT_StringSet);
				pBuiltinStringSet->dumpBuiltinSet("EnUS.strings");
				delete pBuiltinStringSet;
#endif
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-nosplash") == 0)
			{
				// we've alrady processed this before we initialized the App class
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-geometry") == 0)
			{
				// [-geometry <X geometry string>]

				// let us at the next argument
				k++;
				
				// TODO : does X have a dummy geometry value reserved for this?
				int dummy = 1 << ((sizeof(int) * 8) - 1);
				int x = dummy;
				int y = dummy;
				int width = 0;
				int height = 0;
			
				//XParseGeometry(m_pArgs->m_argv[k], &x, &y, &width, &height);

				// use both by default
				XAP_QNXApp::windowGeometryFlags f = (XAP_QNXApp::windowGeometryFlags)
					(XAP_QNXApp::GEOMETRY_FLAG_SIZE
					 | XAP_QNXApp::GEOMETRY_FLAG_POS);

				// if pos (x and y) weren't provided just use size
				if (x == dummy || y == dummy)
					f = XAP_QNXApp::GEOMETRY_FLAG_SIZE;

				// if size (width and height) weren't provided just use pos
				if (width == 0 || height == 0)
					f = XAP_QNXApp::GEOMETRY_FLAG_POS;
			
				// set the xap-level geometry for future frame use
				setGeometry(x, y, width, height, f);
			}
			else
			{
				UT_DEBUGMSG(("Unknown command line option [%s]\n",m_pArgs->m_argv[k]));
				// TODO don't know if it has a following argument or not -- assume not

				_printUsage();
				return UT_FALSE;
			}
		}
		else
		{
			// [filename]
                        
			AP_QNXFrame * pFirstQNXFrame = new AP_QNXFrame(this);
			pFirstQNXFrame->initialize();
			if (pFirstQNXFrame->loadDocument(m_pArgs->m_argv[k], IEFT_Unknown))
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
				pFirstQNXFrame->loadDocument(NULL, IEFT_Unknown);
#else
				delete pFirstQNXFrame;
#endif
			}
		}
	}

	if (kWindowsOpened == 0)
	{
		// no documents specified or were able to be opened, open an untitled one

		AP_QNXFrame * pFirstQNXFrame = new AP_QNXFrame(this);
		printf("APP: ParseCommandLine: Calling frame initialize \n");
		pFirstQNXFrame->initialize();
		printf("APP: ParseCommandLine: Calling load document \n");
		pFirstQNXFrame->loadDocument(NULL, IEFT_Unknown);
	}

	printf("APP: ParseCommandLine: Returning true \n");
	return UT_TRUE;
}

// TODO : MOVE THIS TO XP CODE!  This is a cut & paste job since each
// TODO : platform _can_ have different options, and we didn't sort
// TODO : out how to honor them correclty yet.  There is a copy of
// TODO : this function in other platforms.

void AP_QNXApp::_printUsage(void)
{
	// just print to stdout, not stderr
	printf("\nUsage: %s [option]... [file]...\n\n", m_pArgs->m_argv[0]);

#ifdef DEBUG
	printf("  -dumpstrings      dump strings strings to file\n");
#endif
	printf("  -geometry geom    set initial frame geometry\n");
	printf("  -lib dir          use dir for application components\n");
	printf("  -nosplash         do not show splash screen\n");
#ifdef ABI_OPT_JS
	printf("  -script file      execute file as script\n");
#endif

	printf("\n");
}

