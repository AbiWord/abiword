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

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <Pt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"

#include "xap_Args.h"
#include "ap_Convert.h"
#include "ap_QNXFrame.h"
#include "ap_QNXApp.h"
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

#include "gr_Graphics.h"
#include "gr_QNXGraphics.h"
#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_debugmsg.h"
#include "ut_qnxHelper.h"

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

void signalWrapper(int sig_num);

/*****************************************************************/

AP_QNXApp::AP_QNXApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_QNXApp(pArgs,szAppName)
{
	m_prefs = NULL;
	m_pStringSet = NULL;
	m_pClipboard = NULL;

	m_bHasSelection = false;
	m_bSelectionInFlux = false;
	m_cacheDeferClear = false;
	m_pViewSelection = NULL;
	m_pFrameSelection = NULL;
	m_cacheSelectionView = NULL;
}

AP_QNXApp::~AP_QNXApp(void)
{
	UT_DEBUGMSG(("TODO: Clean up the spell check stuff \n"));
//	SpellCheckCleanup();

	DELETEP(m_prefs);
	DELETEP(m_pStringSet);
	DELETEP(m_pClipboard);

	IE_Exp::unregisterAllExporters ();
	IE_Imp::unregisterAllImporters ();
}

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

bool AP_QNXApp::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	bool bVerified;
	bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
	
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
		return false;
	
	//////////////////////////////////////////////////////////////////
	// Initialize the importers/exporters
	//////////////////////////////////////////////////////////////////
	IE_ImpExp_RegisterXP();

#if 0
	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
	{
		const char * szISpellDirectory = NULL;
		getPrefsValueDirectory(false,AP_PREF_KEY_SpellDirectory,&szISpellDirectory);
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

		UT_DEBUGMSG(("TODO: Clean up the spell check stuff \n"));
//		UT_DEBUGMSG(("Loading SpellCheckWordList [%s]\n",szPathname));
//		SpellCheckInit(szPathname);
		free(szPathname);
		
		// we silently go on if we cannot load it....
	}
#endif
	
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
			getPrefsValueDirectory(true,AP_PREF_KEY_StringSetDirectory,&szDirectory);
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
	// CHECK THIS: the following was added by a Linux developer who can't test
	// on QNX, or even compile, so someone with a QNX box (or fridge? ;-)) needs to check it
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

XAP_Frame * AP_QNXApp::newFrame(void)
{
	AP_QNXFrame * pQNXFrame = new AP_QNXFrame(this);

	if (pQNXFrame)
		pQNXFrame->initialize();

	return pQNXFrame;
}

bool AP_QNXApp::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs()) {
		m_prefs->savePrefsFile();
	}

	return true;
}

void AP_QNXApp::reallyExit(void)
{
	shutdown();
	XAP_QNXApp::reallyExit();
}

bool AP_QNXApp::getPrefsValueDirectory(bool bAppSpecific,
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
#define CLIP_FORMAT_COUNT 2
#define Ph_CLIPBOARD_TYPE_RTF "RTF"

void AP_QNXApp::copyToClipboard(PD_DocumentRange * pDocRange)
{
	PhClipHeader clip[CLIP_FORMAT_COUNT];		//RTF and 8 bit text
	UT_ByteBuf rtfbuf, txtbuf;
	int			format_count;

	// copy the given subset of the given document to the
	// system clipboard in a variety of formats.
	//
	m_pClipboard->clearClipboard();
	
	// also put RTF on the clipboard
	format_count = 0;
	
	IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
	if (pExpRtf)
	{
		UT_Error status;
		status = pExpRtf->copyToBuffer(pDocRange,&rtfbuf);
		UT_Byte b = 0;
		rtfbuf.append(&b,1);			// null terminate string
		UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in RTF format.",rtfbuf.getLength()));
#if defined(USE_ABI_CLIPBOARD)
		m_pClipboard->addData(AP_CLIPBOARD_RTF,(UT_Byte *)rtfbuf.getPointer(0),rtfbuf.getLength());
#else
		strcpy(clip[format_count].type, Ph_CLIPBOARD_TYPE_RTF);
		clip[format_count].length = rtfbuf.getLength();
		clip[format_count].data = (void *)rtfbuf.getPointer(0);
		format_count++;
#endif
		DELETEP(pExpRtf);
	}

	// put raw 8bit text on the clipboard
		
	IE_Exp_Text * pExpText = new IE_Exp_Text(pDocRange->m_pDoc);
	if (pExpText)
	{
		UT_Error status;
		status = pExpText->copyToBuffer(pDocRange,&txtbuf);
		UT_Byte b = 0;
		txtbuf.append(&b,1);			// null terminate string
		UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in TEXTPLAIN format.\n",txtbuf.getLength()));
#if defined(USE_ABI_CLIPBOARD)
		m_pClipboard->addData(AP_CLIPBOARD_TEXTPLAIN_8BIT,(UT_Byte *)txtbuf.getPointer(0),txtbuf.getLength());
#else
		strcpy(clip[format_count].type, Ph_CLIPBOARD_TYPE_TEXT);
		clip[format_count].length = txtbuf.getLength();
		clip[format_count].data = (void *)txtbuf.getPointer(0);
		format_count++;
#endif
		DELETEP(pExpText);
	}

#if !defined(USE_ABI_CLIPBOARD)
	PhClipboardCopy(PhInputGroup(NULL), format_count, clip);
#endif
}

void AP_QNXApp::pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, 
									bool bHonorFormatting)
{
	PhClipHeader *clip;
	void *handle;

	// paste from the system clipboard using the best-for-us format
	// that is present.


	handle = PhClipboardPasteStart(PhInputGroup(NULL));

	if (handle && (clip = PhClipboardPasteType(handle, Ph_CLIPBOARD_TYPE_RTF)) && bHonorFormatting) {
		unsigned char * pData = (unsigned char *)clip->data;
		UT_uint32 iLen = clip->length;
		iLen = MyMin(iLen,strlen((const char *) pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in RTF format.\n",iLen));
		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);
	}
	else if (handle && (clip = PhClipboardPasteType(handle, Ph_CLIPBOARD_TYPE_TEXT))) {
		unsigned char * pData = (unsigned char *)clip->data;
		UT_uint32 iLen = clip->length;
		iLen = MyMin(iLen,strlen((const char *) pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in TEXTPLAIN format.\n",iLen));
		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc);
		pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);
	}
	else if (m_pClipboard->hasFormat(AP_CLIPBOARD_RTF))
	{
		unsigned char * pData = NULL;
		UT_uint32 iLen = 0;
		bool bResult;
		bResult = m_pClipboard->getClipboardData(AP_CLIPBOARD_RTF,(void**)&pData,&iLen);
		UT_ASSERT(bResult);
		iLen = MyMin(iLen,strlen((const char *) pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in RTF format.\n",iLen));
		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);
	}
	else if (m_pClipboard->hasFormat(AP_CLIPBOARD_TEXTPLAIN_8BIT))
	{
		unsigned char * pData = NULL;
		UT_uint32 iLen = 0;
		bool bResult;
		bResult = m_pClipboard->getClipboardData(AP_CLIPBOARD_TEXTPLAIN_8BIT,(void**)&pData,&iLen);
		UT_ASSERT(bResult);
		iLen = MyMin(iLen,strlen((const char *) pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in TEXTPLAIN format.\n",iLen));
		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc);
		pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);
	}
	else {
		// TODO figure out what to do with an image....
		UT_DEBUGMSG(("PasteFromClipboard: TODO support this format..."));
	}
	
	if (handle) {
		PhClipboardPasteFinish(handle);
	}

	return;
}

bool AP_QNXApp::canPasteFromClipboard(void)
{
	void *handle;

	handle = PhClipboardPasteStart(PhInputGroup(NULL));
	if (handle) {
		if (PhClipboardPasteType(handle, Ph_CLIPBOARD_TYPE_RTF) ||
		    PhClipboardPasteType(handle, Ph_CLIPBOARD_TYPE_TEXT)) {
			PhClipboardPasteFinish(handle);
			return true;
		}
		PhClipboardPasteFinish(handle);
	}

	if (m_pClipboard->hasFormat(AP_CLIPBOARD_RTF))
		return true;
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_TEXTPLAIN_8BIT))
		return true;

	return false;
}

/*****************************************************************/
/*****************************************************************/

void AP_QNXApp::setSelectionStatus(AV_View * pView)
{
#if 0
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
	m_bSelectionInFlux = true;

	bool bSelectionStateInThisView = ( ! pView->isSelectionEmpty() );
	
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
		m_cacheDeferClear = true;
	}
	else
	{
		m_bHasSelection = bSelectionStateInThisView;
		m_pClipboard->clearData(false,true);
	}
	
	m_pViewSelection = pView;
	m_pFrameSelection = (XAP_Frame *)pView->getParentData();

	m_bSelectionInFlux = false;
#else
	UT_DEBUGMSG(("Hunt me down and kill me!"));
#endif
	return;
}

bool AP_QNXApp::forgetFrame(XAP_Frame * pFrame)
{
	// we intercept this so that we can erase our
	// selection-related variables if necessary.
	// wouldn't want to hold onto a stale frame or
	// view pointer of a closed window when the
	// selection is changed....

	if (m_pFrameSelection && (pFrame==m_pFrameSelection))
	{
		m_pClipboard->clearClipboard();
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
	m_bSelectionInFlux = true;
	
	if (m_pViewSelection && m_pFrameSelection && m_bHasSelection)
	{
		m_pViewSelection->cmdUnselectSelection();
		m_bHasSelection = false;
	}
	
	m_bSelectionInFlux = false;
	return;
}

void AP_QNXApp::cacheCurrentSelection(AV_View * pView)
{
#if 0
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
#else
	UT_DEBUGMSG(("Hunt me down and kill me!"));
#endif
}

bool AP_QNXApp::getCurrentSelection(const char** formatList,
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
				return false;		// give up on memory errors

			pExpRtf->copyToBuffer(&dr,&m_selectionByteBuf);
			DELETEP(pExpRtf);
			goto ReturnThisBuffer;
		}
			
		if (   (UT_stricmp(formatList[j],AP_CLIPBOARD_TEXTPLAIN_8BIT) == 0)
			|| (UT_stricmp(formatList[j],AP_CLIPBOARD_STRING) == 0))
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

static void * wSplash = NULL;
static GR_Image * pSplashImage = NULL;
static GR_QNXGraphics * pQNXGraphics = NULL;
static bool firstExpose = false;
static UT_uint32 splashTimeoutValue = 0;

static int s_hideSplash(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	if (wSplash) {
		PtDestroyWidget((PtWidget_t *)wSplash);
		wSplash = NULL;
		DELETEP(pQNXGraphics);
		DELETEP(pSplashImage);
	}
	return Pt_CONTINUE;
}

static int s_drawingarea_expose(PtWidget_t *widget, PhTile_t *damage) {
	if (pQNXGraphics && pSplashImage) {
		pQNXGraphics->drawImage(pSplashImage, 0, 0);

		// on the first full paint of the image, start a 2 second timer
		if (!firstExpose) {
			PtArg_t args[1];
			PtSetArg(&args[0], Pt_ARG_TIMER_INITIAL, splashTimeoutValue, 0);
			PtWidget_t *timer = PtCreateWidget(PtTimer, widget, 1, args);
			PtAddCallback(timer, Pt_CB_TIMER_ACTIVATE, s_hideSplash, NULL);
			PtRealizeWidget(timer);
			firstExpose = true;
		}
	}

	return Pt_CONTINUE;
}

// szFile is optional; a NULL pointer will use the default splash screen.
// The delay is how long the splash should stay on screen in milliseconds.
static GR_Image * _showSplash(PtWidget_t *spwin, UT_uint32 delay)
{
	wSplash = spwin;
	pSplashImage = NULL;

	UT_ByteBuf* pBB = NULL;

	// use a default if they haven't specified anything
	const char * szFile = "splash.png";

	// store value for use by the expose event, which attaches the timer
	splashTimeoutValue = delay;
	
	extern unsigned char g_pngSplash[];		// see ap_wp_Splash.cpp
	extern unsigned long g_pngSplash_sizeof;	// see ap_wp_Splash.cpp

	pBB = new UT_ByteBuf();
	if ((pBB->insertFromFile(0, szFile)) || 
        (pBB->ins(0, g_pngSplash, g_pngSplash_sizeof)))
	{
		PtArg_t	args[10];
		int     n = 0;

		// get splash size
		UT_sint32 iSplashWidth;
		UT_sint32 iSplashHeight;
		UT_PNG_getDimensions(pBB, iSplashWidth, iSplashHeight);

		// create a centered window the size of our image
		PtSetArg(&args[n++], Pt_ARG_WIDTH, iSplashWidth, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, iSplashHeight, 0);
		PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 
				 0, 
				Ph_WM_RENDER_RESIZE | Ph_WM_RENDER_TITLE | Ph_WM_RENDER_MENU);
		PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 
				0,
				Ph_WM_CLOSE | Ph_WM_RESIZE | Ph_WM_HIDE | Ph_WM_MAX);
		PtSetResources(spwin, n, args);
		UT_QNXCenterWindow(NULL, spwin);

		// create a frame to add depth

		// create a drawing area
		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, iSplashWidth, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, iSplashHeight, 0);
		//PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
		PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_drawingarea_expose, 1);
		PtWidget_t *da = PtCreateWidget(PtRaw, spwin, n, args);
		PtAddEventHandler(da, Ph_EV_BUT_RELEASE, s_hideSplash, NULL);

		// create image context
		// TODO: find an XAP_App pointer for the following call:
		pQNXGraphics = new GR_QNXGraphics(spwin, da, 0);
		pSplashImage = pQNXGraphics->createNewImage("splash", pBB, iSplashWidth, iSplashHeight);

		PtRealizeWidget(spwin);
	}

	DELETEP(pBB);

	return pSplashImage;
}

/*****************************************************************/
AP_QNXApp * gQNXApp = NULL; 
PtWidget_t	*gTimerWidget = NULL;

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

	//Initial state
	bool bShowSplash = true;
	bool bShowApp = true;

	// Do a quick and dirty find for "-to"
	for (int k = 1; k < Args.m_argc; k++)
 		if (*Args.m_argv[k] == '-')
 			if (UT_stricmp(Args.m_argv[k],"-to") == 0)
 			{
				bShowApp = false;
 				bShowSplash = false;
 				break;
 			}

	// Do a quick and dirty find for "-show"
 	for (int k = 1; k < Args.m_argc; k++)
 		if (*Args.m_argv[k] == '-')
 			if (UT_stricmp(Args.m_argv[k],"-show") == 0)
 			{
				bShowApp = true;
 				bShowSplash = true;
 				break;
 			}

	// Do a quick and dirty find for "-nosplash"
	for (int k = 1; k < Args.m_argc; k++)
		if (*Args.m_argv[k] == '-')
			if (UT_stricmp(Args.m_argv[k],"-nosplash") == 0)
			{
				bShowSplash = false;
				break;
			}


	//TODO: Do a PtAppInit() here with the main window being the splash screen
	PtWidget_t *spwin;
	spwin = PtAppInit(NULL, NULL /* Args.m_argc */, NULL /* Args.m_argv */, 0, NULL);
	if (bShowSplash) {
		_showSplash(spwin, 2000);
	}
	else {
		PtDestroyWidget(spwin);
	}
	
	AP_QNXApp * pMyQNXApp;
	gQNXApp = pMyQNXApp = new AP_QNXApp(&Args, szAppName);

    // Setup signal handlers, primarily for segfault
    // If we segfaulted before here, we *really* blew it

    struct sigaction sa;

    sa.sa_handler = signalWrapper;

    sigfillset(&sa.sa_mask);  // We don't want to hear about other signals
    sigdelset(&sa.sa_mask, SIGABRT); // But we will call abort(), so we can't ignore that

    sa.sa_flags = SA_NODEFER | SA_RESETHAND; // Don't handle nested signals

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    // TODO: handle SIGABRT

	//This is used by all the timer classes, and should probably be in the XAP contructor
	PtArg_t args[2];
	PtSetArg(&args[0], Pt_ARG_REGION_FIELDS, Ph_REGION_EV_SENSE, Ph_REGION_EV_SENSE);
	PtSetArg(&args[1], Pt_ARG_REGION_SENSE, Ph_EV_TIMER, Ph_EV_TIMER);
	PtSetParentWidget(NULL);
	gTimerWidget = PtCreateWidget(PtRegion, NULL, 2, args);
	PtRealizeWidget(gTimerWidget);

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyQNXApp->initialize())
	{
		delete pMyQNXApp;
		return -1;	// make this something standard?
	}

	// this function takes care of all the command line args.
	// if some args are botched, it returns false and we should
	// continue out the door.
	if (pMyQNXApp->parseCommandLine() && bShowApp)
	{
		PtMainLoop();
	}
	
	// destroy the App.  It should take care of deleting all frames.
	pMyQNXApp->shutdown();
	delete pMyQNXApp;
	
	return 0;
}

bool AP_QNXApp::parseCommandLine(void)
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
	char *to = NULL;
	int verbose = 1;
	bool show = false;
        
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
				pBuiltinStringSet->dumpBuiltinSet("en-US.strings");
				delete pBuiltinStringSet;
#endif
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-nosplash") == 0)
			{
				// we've alrady processed this before we initialized the App class
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-geometry") == 0)
			{
				// [-geometry [WIDTHxHEIGHT][+-xoffset+-yoffset]
				k++;
				parseAndSetGeometry(m_pArgs->m_argv[k]);	
			}
			else if (UT_stricmp (m_pArgs->m_argv[k],"-to") == 0)
			{
				k++;
				to = m_pArgs->m_argv[k];
			}
			else if (UT_stricmp (m_pArgs->m_argv[k], "-show") == 0)
			{
				show = true;
			}
			else if (UT_stricmp (m_pArgs->m_argv[k], "-verbose") == 0)
			{
				k++;
				verbose = atoi (m_pArgs->m_argv[k]);
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
			if (to) {
 				AP_Convert * conv = new AP_Convert(getApp());
 				conv->setVerbose(verbose);
 				conv->convertTo(m_pArgs->m_argv[k], to);
 				delete conv;
  			} 
			else {
                        
				AP_QNXFrame * pFirstQNXFrame = new AP_QNXFrame(this);
				pFirstQNXFrame->initialize();
				UT_Error error = pFirstQNXFrame->loadDocument(m_pArgs->m_argv[k], IEFT_Unknown, true);
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
					pFirstQNXFrame->loadDocument(NULL, IEFT_Unknown);

					pFirstQNXFrame->raise();
					
					XAP_DialogFactory * pDialogFactory
						= (XAP_DialogFactory *)(pFirstQNXFrame->getDialogFactory());
					
					XAP_Dialog_MessageBox * pDialog
						= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
					UT_ASSERT(pDialog);
					
					const XAP_StringSet * pSS = pFirstQNXFrame->getApp()->getStringSet();
					
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
					
					pDialog->runModal(pFirstQNXFrame);
					
					//XAP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();
					pDialog->getAnswer();
					
					pDialogFactory->releaseDialog(pDialog);
#else
					delete pFirstQNXFrame;
#endif
				}
			}
		}
	}

	// command-line conversion may not open any windows at all
	if (to && !show) 
	{
		return true;
	}

	if (kWindowsOpened == 0)
	{
		// no documents specified or were able to be opened, open an untitled one

		AP_QNXFrame * pFirstQNXFrame = new AP_QNXFrame(this);
		pFirstQNXFrame->initialize();
		pFirstQNXFrame->loadDocument(NULL, IEFT_Unknown);
	}
	return true;
}

/*** Signal handling functionality ***/
void signalWrapper(int sig_num)
{
	AP_QNXApp *pApp = (AP_QNXApp *) XAP_App::getApp();
	pApp->catchSignals(sig_num);
}

static int s_signal_count = 0;

void AP_QNXApp::catchSignals(int sig_num)
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
		AP_QNXFrame * curFrame = (AP_QNXFrame*) m_vecFrames[i];
		UT_ASSERT(curFrame);
		curFrame->backup();
	}

	fflush(stdout);

	// Abort and dump core
	abort();
}

