/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * BIDI Copyright (c) 2001-2004 Tomas Frydrych
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

#define WIN32_LEAN_AND_MEAN

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <windows.h>
#include <direct.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_path.h"
#include "ut_Win32Uuid.h"
#include "ut_Win32LocaleString.h"
#include "xap_Win32App.h"
#include "xap_Win32Clipboard.h"
#include "xap_Frame.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32Toolbar_Icons.h"
#include "xap_Win32_TB_CFactory.h"
#include "xap_Win32Slurp.h"
#include "xap_Win32EncodingManager.h"
#include "xap_Prefs.h"
#include "gr_Win32Graphics.h"
#include "gr_Win32USPGraphics.h"

#include <locale.h>

#ifdef _MSC_VER
#pragma warning(disable:4355)	// 'this' used in base member initializer list
#endif

#ifdef __MINGW32__
#include <w32api.h>
#endif

char XAP_Win32App::m_buffer[MAX_CONVBUFFER*6] = "";
WCHAR XAP_Win32App::m_wbuffer[MAX_CONVBUFFER] = L"";

// do not want to include wv.h here, since it defines some types that
// overlap with stuff in windov.h
extern "C" {const char * wvLIDToLangConverter(UT_uint16);}

/*****************************************************************/

XAP_Win32App::XAP_Win32App(HINSTANCE hInstance, const char * szAppName)
:	XAP_App(szAppName),
	m_hInstance(hInstance),
	m_dialogFactory(this)
{
	UT_return_if_fail(hInstance);

	_setAbiSuiteLibDir();
	_setBidiOS();

	// create an instance of UT_UUIDGenerator or appropriate derrived class
	_setUUIDGenerator(new UT_Win32UUIDGenerator());

	// for some reason on win32 the locale used by things like
	// strftime is not set correctly; what's worse, MS setlocale does
	// not recognise standard ISO language and country codes. So, here
	// we retrieve the long language and country name for the users
	// default locale and pass these to setlocale. As long as folk use
	// UT_LocaleTransactor to temporarily change to some other locale,
	// this will ensure the C lib uses correct locale. Tomas Feb 5, 2004
	UT_String s;
	char buf[100];

	GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SENGLANGUAGE, &buf[0], 100);
	s += buf;
	s += "_";
	
	GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SENGCOUNTRY, &buf[0], 100);
	s += buf;
	setlocale(LC_ALL, s.c_str());

	// register graphics allocator
	GR_GraphicsFactory * pGF = getGraphicsFactory();
	UT_ASSERT_HARMLESS( pGF );

	if(pGF)
	{
		bool bSuccess = pGF->registerClass(GR_Win32Graphics::graphicsAllocator,
										   GR_Win32Graphics::graphicsDescriptor,
										   GR_Win32Graphics::s_getClassId());

		// we are in deep trouble if this did not succeed
		UT_return_if_fail( bSuccess );

		// this is our fall back ...
		pGF->registerAsDefault(GR_Win32Graphics::s_getClassId(), true);
		pGF->registerAsDefault(GR_Win32Graphics::s_getClassId(), false);
		
		// try to load Uniscribe; if we succeed we will make USP
		// graphics the default

#if ABI_OPT_DISABLE_USP
		HINSTANCE hUniscribe = NULL;
#else
		HINSTANCE hUniscribe = LoadLibraryW(L"usp10.dll");
#endif

		if(hUniscribe && (NULL == g_getenv("ABIWORD_DISABLE_UNISCRIBE")))
		{
			// register Uniscribe graphics and make it the default
			bSuccess = pGF->registerClass(GR_Win32USPGraphics::graphicsAllocator,
										  GR_Win32USPGraphics::graphicsDescriptor,
										  GR_Win32USPGraphics::s_getClassId());

			UT_ASSERT_HARMLESS( bSuccess );
			if(bSuccess)
			{
				pGF->registerAsDefault(GR_Win32USPGraphics::s_getClassId(), true);
				pGF->registerAsDefault(GR_Win32USPGraphics::s_getClassId(), false);
			
				// now g_free the library (GR_Win32USPGraphics will load it
				// on its own behalf
			
				FreeLibrary(hUniscribe);
			}
		}
		else
		{
			UT_DEBUGMSG(("XAP_Win32App: could not load Uniscribe library"));
		}
	}
}

XAP_Win32App::~XAP_Win32App(void)
{
	m_pSlurp->disconnectSlurper();
	DELETEP(m_pSlurp);
}

HINSTANCE XAP_Win32App::getInstance() const
{
	return m_hInstance;
}

bool XAP_Win32App::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue)
{
	// let our base class do it's thing.

	XAP_App::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue);

	// do anything else we need here...

	m_pSlurp = new XAP_Win32Slurp(this);
	m_pSlurp->connectSlurper();
	WCHAR bufExePathname[2048];
	GetModuleFileNameW(NULL,bufExePathname,G_N_ELEMENTS(bufExePathname));

	// TODO these are Application-Specific values.  Move them out of here.
	m_pSlurp->stuffRegistry(".abw",getApplicationName(),bufExePathname,"application/abiword");
	m_pSlurp->stuffRegistry(".zabw",getApplicationName(),bufExePathname,"application/abiword-compressed");

	return true;
}

void XAP_Win32App::reallyExit(void)
{
	PostQuitMessage (0);
}

XAP_DialogFactory * XAP_Win32App::getDialogFactory(void)
{
	return &m_dialogFactory;
}

XAP_Toolbar_ControlFactory * XAP_Win32App::getControlFactory(void)
{
	return &m_controlFactory;
}

UT_uint32 XAP_Win32App::_getExeDir(LPWSTR pDirBuf, UT_uint32 iBufLen)
{
	UT_uint32 iResult = GetModuleFileNameW(NULL, pDirBuf, iBufLen);

	if (iResult > 0)
	{
		LPWSTR p = pDirBuf + wcslen(pDirBuf);
		while (*p != '\\')
		{
			p--;
		}
		UT_ASSERT(p > pDirBuf);
		p++;
		*p = 0;
		iResult = (p - pDirBuf);
	}

	return iResult;
}

static bool isWriteable(LPWSTR lpPath)
{
	bool  result = false;
	DWORD len = lstrlenW(lpPath);
	DWORD rem = MAX_PATH - len - 1;
	DWORD err;
	if (rem < 13) return false;

	lstrcpyW(&lpPath[len],L"/abiword.flg");
	if (CreateDirectoryW(lpPath,NULL)) {
		RemoveDirectoryW(lpPath);
		err=GetLastError();
		if (err != ERROR_ACCESS_DENIED) result=true;
	} else {
		err=GetLastError();
	}
	lpPath[len]=0;
	return result;
}

const char * XAP_Win32App::getUserPrivateDirectory(void) const
{
	/* return a pointer to a static buffer */

	LPCWSTR szAbiDir = L"AbiSuite";

	bool path_ok = false;

	static WCHAR wbuf[PATH_MAX];
	static char buf[PATH_MAX*6];
	memset(buf,0,sizeof(buf));

	DWORD len, len1, len2;

	UT_Win32LocaleString str;
	UT_UTF8String utf8;

	// On W2K and later, APPDATA seems to be set to the directory containing per-user
	// information.

	len = GetEnvironmentVariableW(L"APPDATA",wbuf,PATH_MAX);
	if (len)
	{
		if (isWriteable(wbuf)) path_ok=true;
#ifdef DEBUG
		str.fromLocale(wbuf);
		utf8=str.utf8_str();
		UT_DEBUGMSG(("Getting preferences directory from USERPROFILE [%s].\n",utf8.utf8_str()));
#endif
	}

	if (!path_ok)
	{
		// On NT, USERPROFILE seems to be modern equivalent of $HOME

		len = GetEnvironmentVariableW(L"USERPROFILE",wbuf,PATH_MAX);
		if (len)
		{
			if (isWriteable(wbuf)) path_ok=true;
#ifdef DEBUG
			str.fromLocale(wbuf);
			utf8=str.utf8_str();
			UT_DEBUGMSG(("Getting preferences directory from USERPROFILE [%s].\n",utf8.utf8_str()));
#endif
		}
	}

	if (!path_ok)
	{
		// If that doesn't work, look for HOMEDRIVE and HOMEPATH.  HOMEPATH
		// is mentioned in the GetWindowsDirectory() documentation at least.
		// These may be set if the SysAdmin did so in the Admin tool....

		len1 = GetEnvironmentVariableW(L"HOMEDRIVE",wbuf,PATH_MAX);
		len2 = GetEnvironmentVariableW(L"HOMEPATH",&wbuf[len1],PATH_MAX-len1);
		if (len1 && len2)
		{
			if (isWriteable(wbuf)) path_ok=true;
#ifdef DEBUG
			str.fromLocale(wbuf);
			utf8=str.utf8_str();
			UT_DEBUGMSG(("Getting preferences directory from HOMEDRIVE and HOMEPATH [%s].\n",utf8.utf8_str()));
#endif
		}
	}

	if (!path_ok)
	{
		// If that doesn't work, let's just stick it in the WINDOWS directory.

		len = GetWindowsDirectoryW(wbuf,PATH_MAX);
		if (len)
		{
			if (isWriteable(wbuf)) path_ok=true;
#ifdef DEBUG
			str.fromLocale(wbuf);
			utf8=str.utf8_str();
			UT_DEBUGMSG(("Getting preferences directory from GetWindowsDirectory() [%s].\n",utf8.utf8_str()));
#endif
		}
	}

	if (!path_ok)
	{
		// If that doesn't work, stick it in "C:\"...

		lstrcpyW(wbuf,L"C:\\");
		if (isWriteable(wbuf)) path_ok=true;
		UT_DEBUGMSG(("Getting preferences directory defaulted to C:\\.\n"));
	}

	if (!path_ok)
	{
		// As penultimate resort try to use temporary dir

		len = GetTempPathW(MAX_PATH,wbuf);
		if (len)
		{
			if (isWriteable(wbuf)) path_ok=true;
#ifdef DEBUG
			str.fromLocale(wbuf);
			utf8=str.utf8_str();
			UT_DEBUGMSG(("Getting preferences directory from GetTempPath() [%s].\n",utf8.utf8_str()));
#endif
		}
	}

	if (!path_ok)
	{
		// As last resort trying to use TMP dir

		len = GetEnvironmentVariableW(L"TMP",wbuf,PATH_MAX);
		if (len)
		{
			if (isWriteable(wbuf)) path_ok=true;
#ifdef DEBUG
			str.fromLocale(wbuf);
			utf8=str.utf8_str();
			UT_DEBUGMSG(("Getting preferences directory from TMP [%s].\n",utf8.utf8_str()));
#endif
		}
	}

	UT_ASSERT(path_ok=true)

	if (lstrlenW(wbuf)+lstrlenW(szAbiDir)+2 >= PATH_MAX)
		return NULL;

	if (wbuf[lstrlenW(wbuf)-1] != L'\\')
		lstrcatW(wbuf,L"\\");
	lstrcatW(wbuf,szAbiDir);

#ifdef NO_WIN32_UNICODE_SUPPORT_YET
	WideCharToMultiByte(CP_ACP,0,wbuf,-1,buf,MAX_PATH,"_",NULL);
#else
	str.fromLocale(wbuf);
	utf8=str.utf8_str();
	strcpy(buf,utf8.utf8_str());
#endif

	return buf;
}

void XAP_Win32App::_setAbiSuiteLibDir(void)
{
	char *utf8;
	WCHAR buf[PATH_MAX];

	// see if ABIWORD_DATADIR was set in the environment
	if (GetEnvironmentVariableW(L"ABIWORD_DATADIR",buf,sizeof(buf)) > 0)
	{
		utf8 = (char*) getUTF8String(buf);
		char * p = utf8;
		int len = strlen(p);
		if ( (p[0]=='"') && (p[len-1]=='"') )
		{
			// trim leading and trailing DQUOTES
			p[len-1]=0;
			p++;
			len -= 2;
		}
		if (p[len-1]=='\\')				// trim trailing slash
			p[len-1] = 0;
		XAP_App::_setAbiSuiteLibDir(p);
		return;
	}

	if (_getExeDir(buf,sizeof(buf)) > 0)
	{
		char *base;
		size_t len, baselen;

		utf8 = (char*) getUTF8String(buf);

		len = strlen(utf8);
		base = g_path_get_basename(utf8);
		baselen = strlen(base);
		g_free (base), base = NULL;
		if (len+1 > baselen && utf8[len - baselen - 2] == '\\')
			utf8[len - baselen - 2] = 0;
		else utf8[len - baselen - 1] = '\0';
#ifdef _MSC_VER
		XAP_App::_setAbiSuiteLibDir(utf8);
#else
		gchar * dir = g_build_filename(utf8, "share", PACKAGE "-" ABIWORD_SERIES, NULL);
		XAP_App::_setAbiSuiteLibDir(dir);
		g_free (dir), dir = NULL;
#endif
		return;
	}

	// otherwise, use the hard-coded value
	XAP_App::_setAbiSuiteLibDir(getAbiSuiteHome());
	return;
}

void XAP_Win32App::enableAllTopLevelWindows(bool b)
{
	UT_uint32 iCount = m_vecFrames.getItemCount();

	for (UT_uint32 ndx=0; ndx<iCount; ndx++)
	{
		XAP_Frame * pFrame = (XAP_Frame *) m_vecFrames.getNthItem(ndx);

		EnableWindow(static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(), b);
	}
}

// This function gets saved geometry from preference file, attempts to validate
// and sets window's placement.  It returns the flag that should be called to
// ShowWindow on the first call.
UT_sint32 XAP_Win32App::setupWindowFromPrefs(UT_sint32 iCmdShow, HWND hwndFrame)
{
	UT_uint32 iHeight, iWidth;
	UT_sint32 x,y;
	UT_uint32 flag;

	// if width & height are <= 0 then assume invalid values so just use system defaults
	if (getGeometry(&x,&y,&iWidth,&iHeight,&flag) && (iWidth > 0) && (iHeight > 0))
	{
		WINDOWPLACEMENT wndPlacement;
		wndPlacement.length = sizeof(WINDOWPLACEMENT); // must do
		// get current windowplacement info, so anything we don't set is correct
		if (!GetWindowPlacement(hwndFrame, &wndPlacement))
		{
			memset(&wndPlacement, 0, sizeof(WINDOWPLACEMENT));
			wndPlacement.length = sizeof(WINDOWPLACEMENT);
		}
		if (flag && (flag <= SW_MAX)) // validate flag
		{
			wndPlacement.showCmd = flag;
			iCmdShow = flag; /* SW_SHOW; */
		}
		else
		{
			wndPlacement.showCmd = iCmdShow; /* SW_SHOWNORMAL; */
		}
		wndPlacement.rcNormalPosition.left = x;
		wndPlacement.rcNormalPosition.top = y;
		wndPlacement.rcNormalPosition.right = x + iWidth;
		wndPlacement.rcNormalPosition.bottom = y + iHeight;
		SetWindowPlacement(hwndFrame, &wndPlacement);
	}

	return iCmdShow;
}

/*
	most of the code in the following function comes from Mozilla
*/
void XAP_Win32App::_setBidiOS(void)
{

	m_eBidiOS = XAP_App::BIDI_SUPPORT_NONE;
/*
	I have run into problems with the built-in win32 bidi support -- it is inconsistent
	It treats some fonts (i.e., the MS fonts) correctly, but some common Hebrew fonts
	it will not reorder. Consequently, I felt that the best solution would be to disable
	the win32 bidi altogether and treating it as a non-bidi system, but there does
	not seem to be a simple way of doing this

	I am going to have another shot at disabling the built-in support
	for the main window, and leaving it for the GUI only. The main
	changes are in gr_Win32Graphics class. Tomas, Jan 19th, 2003
*/
	const UT_UCS2Char araAin  = 0x0639;
	const UT_UCS2Char one     = 0x0031;

	int distanceArray[2];
	UT_UCS2Char glyphArray[2];
	UT_UCS2Char outStr[] = {0, 0};

	GCP_RESULTSW gcpResult;
	gcpResult.lStructSize = sizeof(GCP_RESULTS);
	gcpResult.lpOutString = (LPWSTR) outStr;     // Output string
	gcpResult.lpOrder = NULL;			// Ordering indices
	gcpResult.lpDx = distanceArray;     // Distances between character cells
	gcpResult.lpCaretPos = NULL;		// Caret positions
	gcpResult.lpClass = NULL;         // Character classifications
// w32api changed lpGlyphs from UINT * to LPWSTR to match MS PSDK in w32api v2.4
#ifdef __MINGW32__
#if (__W32API_MAJOR_VERSION == 2 && __W32API_MINOR_VERSION < 4)
	gcpResult.lpGlyphs = (UINT *) glyphArray;    // Character glyphs
#else
	gcpResult.lpGlyphs = (LPWSTR) glyphArray;    // Character glyphs
#endif
#else	
	gcpResult.lpGlyphs = (LPWSTR) glyphArray;    // Character glyphs
#endif
	gcpResult.nGlyphs = 2;              // Array size

	UT_UCS2Char inStr[] = {araAin, one};

	HDC displayDC = GetDC(NULL);

	if(!displayDC)
	{
		return;
	}

	if (GetCharacterPlacementW(displayDC, (LPCWSTR)inStr, 2, 0, &gcpResult, GCP_REORDER)
		&& (inStr[0] == outStr[1]) )
	{
		m_eBidiOS = XAP_App::BIDI_SUPPORT_GUI;
		UT_DEBUGMSG(("System has bidi and glyph shaping\n"));
	}
	else
	{
		const UT_UCSChar hebAlef = 0x05D0;
		inStr[0] = hebAlef;
		inStr[1] = one;
		if (GetCharacterPlacementW(displayDC, (LPCWSTR)inStr, 2, 0, &gcpResult, GCP_REORDER)
			&& (inStr[0] == outStr[1]) )
		{
			m_eBidiOS = XAP_App::BIDI_SUPPORT_GUI;
			UT_DEBUGMSG(("System has bidi\n"));
		}
	}

	ReleaseDC(NULL,displayDC);
}

const char * XAP_Win32App::getDefaultEncoding () const
{
	return "UTF-8";	
}

const WCHAR * XAP_Win32App::getWideString (const char * utf8input)
{
	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8input, -1, NULL, 0);
	UT_ASSERT(wlen);
	if (wlen && (wlen < MAX_CONVBUFFER)) {
		wlen = MultiByteToWideChar(CP_UTF8, 0, utf8input, -1, m_wbuffer, MAX_CONVBUFFER);
		UT_ASSERT(wlen);
		return m_wbuffer;
	}
	else 
	{
		UT_ASSERT(wlen < MAX_CONVBUFFER);
		UT_DEBUGMSG(("getWideString:converted string too long %d", wlen));
		return NULL;
	}
}

const char * XAP_Win32App::getUTF8String (const WCHAR * p_str)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, p_str, -1, NULL, 0, NULL, NULL);
	UT_ASSERT(len);
	if (len && (len < MAX_CONVBUFFER*6)) {
		len = WideCharToMultiByte(CP_UTF8, 0, p_str, -1, m_buffer, MAX_CONVBUFFER*6, NULL, NULL);
		UT_ASSERT(len);
		return m_buffer;
	}
	else 
	{
		UT_ASSERT(len < MAX_CONVBUFFER*6);
		UT_DEBUGMSG(("getUTF8String:converted string too long %d", len));
		return NULL;
	}
}


void XAP_Win32App::getDefaultGeometry(UT_uint32& width, UT_uint32& height, UT_uint32& flags)
{
	flags |= PREF_FLAG_GEOMETRY_MAXIMIZED;
	
	width = GetSystemMetrics(SM_CXFULLSCREEN);
	height = GetSystemMetrics(SM_CYFULLSCREEN);	
}

const char * XAP_Win32App::_getKbdLanguage()
{
	// make sure that m_hkl is in sync with the current value
	m_hkl = GetKeyboardLayout(0);
	
	WORD langID = LANGIDFROMLCID(LOWORD(m_hkl));
	const char * pszLang = wvLIDToLangConverter((unsigned short)langID);
	UT_DEBUGMSG(("XAP_Win32App::_getKbdLanguage: %s\n",pszLang));
	return pszLang;
}

// wrapper around XAP_App::setKbdLanguage(const char *)
void XAP_Win32App::setKbdLanguage(HKL hkl)
{
	m_hkl = hkl;

	WORD langID = LANGIDFROMLCID(LOWORD(hkl));
	const char * pszLang = wvLIDToLangConverter((unsigned short)langID);
	UT_DEBUGMSG(("XAP_Win32App::setKbdLanguage: %s\n",pszLang));

	// set the language
	XAP_App::setKbdLanguage(pszLang);
}


