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

#include <windows.h>
#include <commctrl.h>   // includes the common control header
#include <crtdbg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>

#include "ut_debugmsg.h"
#include "ut_bytebuf.h"
#include "ut_string.h"
#include "xap_Args.h"
#include "ap_Convert.h"
#include "ap_Win32Frame.h"
#include "ap_Win32App.h"
#include "sp_spell.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"

#include "ap_Win32Resources.rc2"
#include "ap_Clipboard.h"
#include "ie_types.h"
#include "ie_exp_Text.h"
#include "ie_exp_RTF.h"
#include "ie_exp_AbiWord_1.h"
#include "ie_exp_HTML.h"
#include "ie_imp_Text.h"
#include "ie_imp_RTF.h"
#include "ap_EditMethods.h"

#include "fp_Run.h"

/*****************************************************************/

AP_Win32App::AP_Win32App(HINSTANCE hInstance, XAP_Args * pArgs, const char * szAppName)
	: XAP_Win32App(hInstance, pArgs,szAppName)
{
	m_pStringSet = NULL;
	m_pClipboard = NULL;
}

AP_Win32App::~AP_Win32App(void)
{
	SpellCheckCleanup();

	DELETEP(m_pStringSet);
	DELETEP(m_pClipboard);
}

static bool s_createDirectoryIfNecessary(const char * szDir)
{
	struct _stat statbuf;
	
	if (_stat(szDir,&statbuf) == 0)								// if it exists
	{
		if ( (statbuf.st_mode & _S_IFDIR) == _S_IFDIR )			// and is a directory
			return true;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return false;
	}

	if (CreateDirectory(szDir,NULL))
		return true;

	UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
	return false;
}	

bool AP_Win32App::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
	UT_ASSERT(bVerified);

	// load the preferences.
	
	m_prefs = new AP_Win32Prefs(this);
	m_prefs->fullInit();
		   
	// now that preferences are established, let the xap init

	m_pClipboard = new AP_Win32Clipboard();
	UT_ASSERT(m_pClipboard);
	   
	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_ASSERT(m_pBindingSet);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);

	if (! XAP_Win32App::initialize())
		return false;

	// let various window types register themselves

	if (!AP_Win32Frame::RegisterClass(this))
	{
		UT_DEBUGMSG(("couldn't register class\n"));
		return false;
	}

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
				((szISpellDirectory[strlen(szISpellDirectory)-1]=='\\') ? "" : "\\"),
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
			getPrefsValueDirectory(true,AP_PREF_KEY_StringSetDirectory,&szDirectory);
			UT_ASSERT((szDirectory) && (*szDirectory));

			char * szPathname = (char *)calloc(sizeof(char),strlen(szDirectory)+strlen(szStringSet)+100);
			UT_ASSERT(szPathname);

			sprintf(szPathname,"%s%s%s.strings",
					szDirectory,
					((szDirectory[strlen(szDirectory)-1]=='\\') ? "" : "\\"),
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
	// on Win32, or even compile, so someone with a Windows box needs to check it
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

XAP_Frame * AP_Win32App::newFrame(void)
{
	AP_Win32Frame * pWin32Frame = new AP_Win32Frame(this);

	if (pWin32Frame)
		pWin32Frame->initialize();

	return pWin32Frame;
}

bool AP_Win32App::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

	return true;
}

bool AP_Win32App::getPrefsValueDirectory(bool bAppSpecific,
											const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return false;

	const XML_Char * psz = NULL;
	if (!m_prefs->getPrefsValue(szKey,&psz))
		return false;

	if ((*psz == '/') || (*psz == '\\'))
	{
		*pszValue = psz;
		return true;
	}

	const XML_Char * dir = ((bAppSpecific) ? getAbiSuiteAppDir() : getAbiSuiteLibDir());

	static XML_Char buf[1024];
	UT_ASSERT((strlen(dir) + strlen(psz) + 2) < sizeof(buf));
	
	sprintf(buf,"%s\\%s",dir,psz);
	*pszValue = buf;
	return true;
}

const char * AP_Win32App::getAbiSuiteAppDir(void) const
{
	// we return a static string, use it quickly.
	
	static XML_Char buf[1024];
	UT_ASSERT((strlen(getAbiSuiteLibDir()) + strlen(ABIWORD_APP_LIBDIR) + 2) < sizeof(buf));

	sprintf(buf,"%s\\%s",getAbiSuiteLibDir(),ABIWORD_APP_LIBDIR);
	return buf;
}

HICON AP_Win32App::getIcon(void)
{

	int sy = GetSystemMetrics(SM_CYICON);
	int sx = GetSystemMetrics(SM_CXICON);
	UT_DEBUGMSG(("GetIcon(): system metrics [%d %d]\n",sx,sy));
	
	if ((sx==32) && (sy==32))
		return LoadIcon(getInstance(), MAKEINTRESOURCE(AP_RID_ICON_APPLICATION_32));
	else
		return (HICON) LoadImage(getInstance(), MAKEINTRESOURCE(AP_RID_ICON_APPLICATION_32), IMAGE_ICON, 0,0,0);
}

HICON AP_Win32App::getSmallIcon(void)
{

	int sy = GetSystemMetrics(SM_CYICON);
	int sx = GetSystemMetrics(SM_CXICON);
	UT_DEBUGMSG(("GetIcon(): system metrics [%d %d]\n",sx,sy));

	if ((sx==16) && (sy==16))
		return LoadIcon(getInstance(), MAKEINTRESOURCE(AP_RID_ICON_APPLICATION_16));
	else
		return (HICON) LoadImage(getInstance(), MAKEINTRESOURCE(AP_RID_ICON_APPLICATION_16), IMAGE_ICON, 0,0,0);
}

const XAP_StringSet * AP_Win32App::getStringSet(void) const
{
	return m_pStringSet;
}

void AP_Win32App::copyToClipboard(PD_DocumentRange * pDocRange)
{
	// copy the given subset of the given document to the
	// system clipboard in a variety of formats.
	// MSFT requests that we post them in the order of
	// importance to us (most preserving to most lossy).
	//
	// TODO on NT, do we need to put unicode text on the clipboard ??
	// TODO do we need to put something in .ABW format on the clipboard ??

	if (!m_pClipboard->openClipboard())			// try to lock the clipboard
		return;
	{
		m_pClipboard->clearClipboard();

		// put RTF on the clipboard
		
		IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
		if (pExpRtf)
		{
			UT_ByteBuf buf;
			UT_Error status = pExpRtf->copyToBuffer(pDocRange,&buf);
			UT_Byte b = 0;
			buf.append(&b,1);			// NULL terminate the string
			m_pClipboard->addData(AP_CLIPBOARD_RTF,(UT_Byte *)buf.getPointer(0),buf.getLength());
			DELETEP(pExpRtf);
			UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in RTF format.\n",buf.getLength()));
			//UT_DEBUGMSG(("CopyToClipboard: [%s]\n",buf.getPointer(0)));
		}

		// put raw 8bit text on the clipboard
		
		IE_Exp_Text * pExpText = new IE_Exp_Text(pDocRange->m_pDoc);
		if (pExpText)
		{
			UT_ByteBuf buf;
			UT_Error status = pExpText->copyToBuffer(pDocRange,&buf);
			UT_Byte b = 0;
			buf.append(&b,1);			// NULL terminate the string
			m_pClipboard->addData(AP_CLIPBOARD_TEXTPLAIN_8BIT,(UT_Byte *)buf.getPointer(0),buf.getLength());
			DELETEP(pExpText);
			UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in TEXTPLAIN format.\n",buf.getLength()));
			//UT_DEBUGMSG(("CopyToClipboard: [%s]\n",buf.getPointer(0)));
		}
	}

	m_pClipboard->closeClipboard();				// release clipboard lock
}

void AP_Win32App::pasteFromClipboard(PD_DocumentRange * pDocRange, bool)
{
	// paste from the system clipboard using the best-for-us format
	// that is present.

	// We get a handle to the object in the requested format
	// and then lock it an use the system buffer -- rather
	// then copying it into our own.
	//
	// we jump thru a few bogus steps w/r/t the length of the
	// object because MSFT docs state that the length of the
	// object may be less than the length actually returned by
	// GlobalSize().
	//
	// therefore, we do a strlen() and **hope** that this is
	// right.  Oh, and the value returned by GlobalSize() varies
	// from call-to-call on the same object.... sigh.

#define TRY_TO_PASTE_IN_FORMAT(fmt,type)							\
	do {															\
		HANDLE hData;												\
		hData = m_pClipboard->getHandleInFormat( fmt );				\
		if (hData)													\
		{															\
			unsigned char * pData = (unsigned char *)GlobalLock(hData);				\
			UT_DEBUGMSG(("Paste: [fmt %s][hdata 0x%08lx][pData 0x%08lx]\n",			\
						 ##fmt, hData, pData));						\
			UT_uint32 iSize = GlobalSize(hData);					\
			UT_uint32 iStrLen = strlen((const char *)pData);		\
			UT_uint32 iLen = MyMin(iSize,iStrLen);					\
																	\
			type * pImp = new type (pDocRange->m_pDoc);				\
			if (pImp)												\
			{														\
				pImp->pasteFromBuffer(pDocRange,pData,iLen);		\
				delete pImp;										\
			}														\
																	\
			GlobalUnlock(hData);									\
			goto MyEnd;												\
		}															\
	} while (0)


	if (!m_pClipboard->openClipboard())			// try to lock the clipboard
		return;
	
	{
		// TODO decide what the proper order is for these.
		// TODO decide if we need to support .ABW on the clipboard.

		TRY_TO_PASTE_IN_FORMAT(AP_CLIPBOARD_RTF, IE_Imp_RTF);
		TRY_TO_PASTE_IN_FORMAT(AP_CLIPBOARD_TEXTPLAIN_8BIT, IE_Imp_Text);
		
		// TODO figure out what to do with an image and other formats....
		UT_DEBUGMSG(("PasteFromClipboard: TODO support this format..."));
	}

MyEnd:
	m_pClipboard->closeClipboard();				// release clipboard lock
	return;
}

bool AP_Win32App::canPasteFromClipboard(void)
{
	if (!m_pClipboard->openClipboard())
		return false;

	// TODO decide if we need to support .ABW format on the clipboard.
	
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_RTF))
		goto ReturnTrue;
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_TEXTPLAIN_8BIT))
		goto ReturnTrue;

	m_pClipboard->closeClipboard();
	return false;

ReturnTrue:
	m_pClipboard->closeClipboard();
	return true;
}

/*****************************************************************/

#ifdef   _DEBUG
#define  SET_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
#endif

/*****************************************************************/

#define SPLASH 1

#if SPLASH
#include "gr_Graphics.h"
#include "gr_Win32Graphics.h"
#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_png.h"

static HWND hwndSplash = NULL;
static GR_Image * pSplash = NULL;
static char s_SplashWndClassName[256];

static void _hideSplash(void)
{
	if (hwndSplash)
	{
		DestroyWindow(hwndSplash);
		hwndSplash = NULL;
	}
	
	DELETEP(pSplash);
}

static LRESULT CALLBACK _SplashWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    
    switch (message) 
	{
    case WM_CREATE:
        // Set the timer for the specified number of ms
        SetTimer(hWnd, 0, 2000, NULL);  
        break;

#if 0		
	// Handle the palette messages in case 
	// another app takes over the palette
    case WM_PALETTECHANGED:
        if ((HWND) wParam == hWnd)
            return 0;
    case WM_QUERYNEWPALETTE:
        InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		return TRUE;
#endif     

    // Destroy the window if... 
    case WM_LBUTTONDOWN:      // ...the user pressed the left mouse button
    case WM_RBUTTONDOWN:      // ...the user pressed the right mouse button
    case WM_TIMER:            // ...the timer timed out
        _hideSplash();		  // Close the window
        break;
        
        // Draw the window
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
		{
			// TODO: find XAP_App pointer for this
			GR_Graphics * pG = new GR_Win32Graphics(hdc, hwndSplash, 0);
			pG->drawImage(pSplash, 0, 0);
			DELETEP(pG);
		}
        EndPaint(hWnd, &ps);
        break;
        
    default:
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}

static GR_Image * _showSplash(HINSTANCE hInstance, XAP_Args * pArgs, const char * szAppName)
{
	hwndSplash = NULL;
	pSplash = NULL;

	UT_ByteBuf* pBB = NULL;
	bool bShowSplash = true;
	const char * szFile = NULL;

	// Win32 does not put the program name in argv[0], so [0] is the first argument
	int nFirstArg = 0;
	int k;
	
	// scan args for splash-related stuff
    for (k=nFirstArg; (k<pArgs->m_argc); k++)
	{
		if (*pArgs->m_argv[k] == '-')
		{
			if (UT_stricmp(pArgs->m_argv[k],"-to") == 0 || UT_stricmp(pArgs->m_argv[k],"-help") == 0)
			{
				bShowSplash = false;
				break;
			}
		}
	}	
	
    for (k=nFirstArg; (k<pArgs->m_argc); k++)
	{
		if (*pArgs->m_argv[k] == '-')
		{
			if (UT_stricmp(pArgs->m_argv[k],"-show") == 0)
			{
				bShowSplash = true;
				break;
			}
		}
	}	

	for (k=nFirstArg; (k<pArgs->m_argc); k++)
	{
		if (*pArgs->m_argv[k] == '-')
		{
			if (UT_stricmp(pArgs->m_argv[k],"-nosplash") == 0)
			{
				bShowSplash = false;
				break;
			}
#if DEBUG
			else if (UT_stricmp(pArgs->m_argv[k],"-splash") == 0)
			{
				// [-splash filename]
				szFile = pArgs->m_argv[k+1];
				break;

				// NOTE: this switch is just for debugging artwork, so 
				// it's OK that the filename also gets opened as a document
			}
#endif
		}

		// TODO: platform-specific reasons to not show splash?
		// TODO: for example, if being launched via DDE or OLE??
	}

	if (!bShowSplash)
		goto Done;

	extern unsigned char g_pngSplash[];		// see ap_wp_Splash.cpp
	extern unsigned long g_pngSplash_sizeof;	// see ap_wp_Splash.cpp

	pBB = new UT_ByteBuf();
	if (
		(szFile && szFile[0] && (pBB->insertFromFile(0, szFile)))
		|| (pBB->ins(0, g_pngSplash, g_pngSplash_sizeof))
		)
	{
		// NB: can't access 'this' members from a static member function
		WNDCLASSEX  wndclass;
		ATOM a;
	
		sprintf(s_SplashWndClassName, "%sSplash", szAppName /* app->getApplicationName() */);

		// register class for the splash window
		wndclass.cbSize        = sizeof(wndclass);
		wndclass.style         = 0;
		wndclass.lpfnWndProc   = _SplashWndProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = hInstance /* app->getInstance() */;
		wndclass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(AP_RID_ICON_APPLICATION_32)) /* app->getIcon() */;
		wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH) GetStockObject(NULL_BRUSH);
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = s_SplashWndClassName;
		wndclass.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(AP_RID_ICON_APPLICATION_16)) /* app->getSmallIcon() */;

		a = RegisterClassEx(&wndclass);
		UT_ASSERT(a);

		// get the extents of the desktop window
		RECT rect;
		GetWindowRect(GetDesktopWindow(), &rect);

		// get splash size
		UT_sint32 iSplashWidth;
		UT_sint32 iSplashHeight;
		UT_PNG_getDimensions(pBB, iSplashWidth, iSplashHeight);

		// create a centered window the size of our bitmap
		hwndSplash = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,s_SplashWndClassName, 
								  NULL, WS_POPUP | WS_BORDER,
								  (rect.right  / 2) - (iSplashWidth  / 2),
								  (rect.bottom / 2) - (iSplashHeight / 2),
								  iSplashWidth,
								  iSplashHeight,
								  NULL, NULL, hInstance, NULL);
		UT_ASSERT(hwndSplash);
    
		if (hwndSplash) 
		{
			// create image first
			// TODO: find XAP_App pointer for this
			GR_Graphics * pG = new GR_Win32Graphics(GetDC(hwndSplash), hwndSplash, 0);
			pSplash = pG->createNewImage("splash", pBB, iSplashWidth, iSplashHeight);
			DELETEP(pG);

			// now bring the window up front & center
			ShowWindow(hwndSplash, SW_SHOWNORMAL);
			UpdateWindow(hwndSplash);
		}
	}

	DELETEP(pBB);

Done:
	return pSplash;
}
#endif

/*****************************************************************/

typedef BOOL __declspec(dllimport) (CALLBACK *InitCommonControlsEx_fn)(LPINITCOMMONCONTROLSEX lpInitCtrls);

int AP_Win32App::WinMain(const char * szAppName, HINSTANCE hInstance, 
						 HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	bool bShowApp = true;

	// this is a static function and doesn't have a 'this' pointer.
	MSG msg;

	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);

	// Ensure that common control DLL is loaded
	HINSTANCE hinstCC = LoadLibrary("comctl32.dll");
	UT_ASSERT(hinstCC);
	InitCommonControlsEx_fn  pInitCommonControlsEx = NULL;
	if( hinstCC != NULL )
		pInitCommonControlsEx = (InitCommonControlsEx_fn)GetProcAddress( hinstCC, "InitCommonControlsEx" );
	if( pInitCommonControlsEx != NULL )
	{
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES 	// load the rebar and toolbar
					| ICC_TAB_CLASSES | ICC_UPDOWN_CLASS	// and tab and spin controls
					;
		pInitCommonControlsEx(&icex);
	}
	else
	{
		InitCommonControls();
		MessageBox(NULL,
			"AbiWord is designed for a newer version of the system file COMCTL32.DLL\n"
			"then the one currently on your system.\n"
			"A solution to this problem is explained in the FAQ on AbiSource web site\n"
			"\n\thttp://www.abisource.com\n\n"
			"We hope this problem can be solved, until then you can use the program,\n"
			"but the toolbar will be missing.", NULL, MB_OK);
	}

	// HACK: load least-common-denominator Rich Edit control
	// TODO: fix Spell dlg so we don't rely on this
	// ALT:  make it a Preview widget instead

	HINSTANCE hinstRich = LoadLibrary("riched32.dll");
	if (!hinstRich)
		hinstRich = LoadLibrary("riched20.dll");
	UT_ASSERT(hinstRich);

	// initialize our application.

	XAP_Args Args = XAP_Args(szCmdLine);

#if SPLASH
	_showSplash(hInstance, &Args, szAppName);
#endif
	
	AP_Win32App * pMyWin32App = new AP_Win32App(hInstance, &Args, szAppName);
	pMyWin32App->initialize();

	pMyWin32App->ParseCommandLine(iCmdShow);

#if 0 /* SPLASH */
	_hideSplash();
#endif
	
	// Win32 does not put the program name in argv[0], so [0] is the first argument
	int nFirstArg = 0;
	int k;
	
	// quick & dirty check to make sure we didn't do a command-line conversion
   for (k=nFirstArg; (k<Args.m_argc); k++) {
		if (*Args.m_argv[k] == '-') {
			if (UT_stricmp(Args.m_argv[k],"-to") == 0 || UT_stricmp(Args.m_argv[k],"-help") == 0) {
 				bShowApp = false;
			}
		}
	}	

    for (k=nFirstArg; (k<Args.m_argc); k++) {
		if (*Args.m_argv[k] == '-') {
			if (UT_stricmp(Args.m_argv[k],"-show") == 0) {
 				bShowApp = true;
			}
		}
	}	

	if (bShowApp)
	{
		while (GetMessage(&msg, NULL, 0, 0))
		{
			// Note: we do not call TranslateMessage() because
			// Note: the keybinding mechanism is responsible
			// Note: for deciding if/when to do this.

			if( pMyWin32App->handleModelessDialogMessage( &msg ) )
				continue;

			DispatchMessage(&msg);
		}
	}

	// destroy the App.  It should take care of deleting all frames.

	pMyWin32App->shutdown();
	delete pMyWin32App;

	SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

	return msg.wParam;
}

void AP_Win32App::ParseCommandLine(int iCmdShow)
{
	// parse the command line
	// <app> [-script <scriptname>]* [-dumpstrings] [-lib <AbiSuiteLibDirectory>] [<documentname>]*
	
	// TODO when we refactor the App classes, consider moving
	// TODO this to app-specific, cross-platform.

	// TODO replace this with getopt or something similar.
	
	// Win32 does not put the program name in argv[0], so [0] is the first argument.

	int nFirstArg = 0;
	int k;
	int kWindowsOpened = 0;
	char *to = NULL;
	int verbose = 1;
	bool show = false, bHelp = false;
	
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
				if(k<m_pArgs->m_argc)
				{
					/* if we don't check we segfault when there aren't any numbers
						after --verbose */
					verbose = atoi (m_pArgs->m_argv[k]);
				}
			}
			else if ( (UT_stricmp (m_pArgs->m_argv[k], "-help") == 0) && (!bHelp) )
			{
				char *pszMessage = (char*)malloc( 500 );

				strcpy( pszMessage, "Usage: AbiWord.exe [option]... [file]...\n\n" );
				strcat( pszMessage, "-to\nThe target format of the file\n\n" );
				strcat( pszMessage, "-verbose\nThe verbosity level (0, 1, 2)\n\n" );
				strcat( pszMessage, "-show\nIf you really want to start the GUI (even if you use the -to or -help options)\n\n" );
#ifdef DEBUG
				strcat( pszMessage, "-dumpstrings\nDump strings strings to file\n\n" );
#endif
				strcat( pszMessage, "-geometry <geom>\nSet initial frame geometry [UNIMPLEMENTED]\n\n" );
				strcat( pszMessage, "-lib <dir>\nUse dir for application components\n\n" );
				strcat( pszMessage, "-nosplash\nDo not show splash screen\n\n" );
				
				MessageBox(NULL,
					pszMessage, "Command Line Options", MB_OK);

				free( pszMessage );

				bHelp = true;
			}
			else
			{
				UT_DEBUGMSG(("Unknown command line option [%s]\n",m_pArgs->m_argv[k]));
				// TODO don't know if it has a following argument or not -- assume not
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
				AP_Win32Frame * pFirstWin32Frame = new AP_Win32Frame(this);
				pFirstWin32Frame->initialize();

				UT_Error error = pFirstWin32Frame->loadDocument(m_pArgs->m_argv[k], IEFT_Unknown);
				if (!error)
				{
					kWindowsOpened++;

					HWND hwnd = pFirstWin32Frame->getTopLevelWindow();
					ShowWindow(hwnd, iCmdShow);
					UpdateWindow(hwnd);
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
					pFirstWin32Frame->loadDocument(NULL, IEFT_Unknown);
					HWND hwnd = pFirstWin32Frame->getTopLevelWindow();
					ShowWindow(hwnd, iCmdShow);
					UpdateWindow(hwnd);
#else
					delete pFirstWin32Frame;
#endif
				}
			}
		}
	}
					
	// command-line conversion may not open any windows at all
	if ((bHelp || to) && !show)
		return;

	if (kWindowsOpened == 0)
	{
		// no documents specified or were able to be opened, open an untitled one

		AP_Win32Frame * pFirstWin32Frame = new AP_Win32Frame(this);
		pFirstWin32Frame->initialize();

		pFirstWin32Frame->loadDocument(NULL, IEFT_Unknown);

		HWND hwnd = pFirstWin32Frame->getTopLevelWindow();
		ShowWindow(hwnd, iCmdShow);
		UpdateWindow(hwnd);
	}

	return;
}

UT_Error AP_Win32App::fileOpen(XAP_Frame * pFrame, const char * pNewFile)
{
	return ::fileOpen(pFrame, pNewFile, IEFT_Unknown);
}

bool AP_Win32App::handleModelessDialogMessage( MSG * msg )
{
	int iCounter;
	HWND hWnd = NULL;

	// Try to knock off the easy case quickly
	if( m_IdTable[ 0 ].id == -1 )
		return false;

    for( iCounter = 0; iCounter <= NUM_MODELESSID; iCounter++ )
	{
		if( m_IdTable[ iCounter ].id != -1 )
		{
			hWnd = (HWND)m_IdTable[ iCounter ].pDialog->pGetWindowHandle();

			if( hWnd && IsDialogMessage( hWnd, msg ) )
				return true;
		}
		else
			break;
	}

	return false;
}
