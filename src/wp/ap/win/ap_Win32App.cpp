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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#define WIN32_LEAN_AND_MEAN

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>   // includes the common control header
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

#if !defined(__WINE__) && (!defined(_MSC_VER) || _MSC_VER < 1310) && !defined(__MINGW32__)
#include <iostream.h>
#elif _MSC_VER >= 1310
#include <iostream>
#endif

#include <ole2.h>

#include "ut_debugmsg.h"
#include "ut_bytebuf.h"
#include "ut_string.h"
#include "xap_Args.h"
#include "ap_Args.h"
#include "ap_Convert.h"
#include "ap_Win32Frame.h"
#include "ap_Win32App.h"
#include "spell_manager.h"

#include "ap_Strings.h"
#include "ap_LoadBindings.h"
#include "xap_EditMethods.h"
#include "xap_Menu_Layouts.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_EncodingManager.h"
#include "xap_ModuleManager.h"
#include "ev_EditMethod.h"
#include "xap_Module.h"
#include "abi-builtin-plugins.h"

#include "ap_Win32Resources.rc2"
#include "ap_Clipboard.h"
#include "ap_EditMethods.h"

#include "fp_Run.h"
#include "ut_path.h"
#include "ut_Win32OS.h"
#include "ut_Win32Idle.h"
#include "ut_Language.h"
#include "ut_Win32LocaleString.h"

#include "ie_impexp_Register.h"

#include "ie_exp.h"
#include "ie_exp_RTF.h"
#include "ie_exp_Text.h"

#include "ie_imp.h"
#include "ie_imp_RTF.h"
#include "ie_imp_Text.h"
#include "ie_impGraphic.h"
#include "fg_Graphic.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "ap_FrameData.h"
#include "ut_Win32Locale.h"

#include "ap_Strings.h"

#include "pt_PieceTable.h"

#include "gr_Painter.h"
// extern prototype - this is defined in ap_EditMethods.cpp
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);
/*****************************************************************/

AP_Win32App::AP_Win32App(HINSTANCE hInstance, const char * szAppName)
	: AP_App(hInstance, szAppName)
{
	m_pStringSet = NULL;
	m_pClipboard = NULL;
}

AP_Win32App::~AP_Win32App(void)
{
	DELETEP(m_pStringSet);
	DELETEP(m_pClipboard);

	IE_ImpExp_UnRegisterXP ();
}

static bool s_createDirectoryIfNecessary(const char * szDir)
{
	struct _stat statbuf;
	UT_Win32LocaleString str;
	
	str.fromUTF8(szDir);

	if (_wstat(str.c_str(),&statbuf) == 0)								// if it exists
	{
		if ( (statbuf.st_mode & _S_IFDIR) == _S_IFDIR )			// and is a directory
			return true;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return false;
	}

	if (CreateDirectoryW(str.c_str(),NULL))
		return true;

	UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
	return false;
}

typedef BOOL __declspec(dllimport) (CALLBACK *InitCommonControlsEx_fn)(LPINITCOMMONCONTROLSEX lpInitCtrls);

bool AP_Win32App::initialize(void)
{
	bool bSuccess = true;
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);

	UT_return_val_if_fail (bVerified, false);

	// create templates directory
	UT_String sTemplates = szUserPrivateDirectory;
	sTemplates += "/templates";
	s_createDirectoryIfNecessary(sTemplates.c_str());

	// load the preferences.
	
	m_prefs = new AP_Win32Prefs();
	UT_return_val_if_fail (m_prefs, false);
	
	m_prefs->fullInit();
		   
	// now that preferences are established, let the xap init

	m_pClipboard = new AP_Win32Clipboard();
	UT_return_val_if_fail (m_pClipboard, false);
	   
	m_pEMC = AP_GetEditMethods();
	UT_return_val_if_fail (m_pEMC, false);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_return_val_if_fail (m_pBindingSet, false);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_return_val_if_fail (m_pMenuActionSet,false);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_return_val_if_fail (m_pToolbarActionSet,false);

	//////////////////////////////////////////////////////////////////
	// load the dialog and message box strings
	//////////////////////////////////////////////////////////////////
	
	{
		// assume we will be using the builtin set (either as the main
		// set or as the fallback set).
		
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,AP_PREF_DEFAULT_StringSet);
		UT_return_val_if_fail (pBuiltinStringSet, false);
		m_pStringSet = pBuiltinStringSet;

		// see if we should load an alternate set from the disk
		
		const char * szDirectory = NULL;
		const char * szStringSet = NULL;

		if (   (getPrefsValue(AP_PREF_KEY_StringSet,&szStringSet))
			&& (szStringSet)
			&& (*szStringSet)
			&& (g_ascii_strcasecmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
			getPrefsValueDirectory(true,AP_PREF_KEY_StringSetDirectory,&szDirectory);
			UT_return_val_if_fail ((szDirectory) && (*szDirectory), false);

			char * szPathname = (char *)UT_calloc(sizeof(char),strlen(szDirectory)+strlen(szStringSet)+100);
			UT_return_val_if_fail (szPathname, false);

			sprintf(szPathname,"%s%s%s.strings",
					szDirectory,
					((szDirectory[strlen(szDirectory)-1]=='\\') ? "" : "\\"),
					szStringSet);

			AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);
			UT_return_val_if_fail (pDiskStringSet, false);

			if (pDiskStringSet->loadStringsFromDisk(szPathname))
			{
				pDiskStringSet->setFallbackStringSet(m_pStringSet);
				m_pStringSet = pDiskStringSet;
				UT_Language_updateLanguageNames();
				UT_DEBUGMSG(("Using StringSet [%s]\n",szPathname));
			}
			else
			{
				UT_DEBUGMSG(("Unable to load StringSet [%s] -- using builtin strings instead.\n",szPathname));				
				DELETEP(pDiskStringSet);
			}
				
			g_free(szPathname);
		}
	}

	// AP_App::initilize() calls for us XAP_Win32App::initialize()
	if (! AP_App::initialize())
		return false;

	
	// let various window types register themselves

	if (!AP_Win32Frame::RegisterClass(this))
	{
		UT_DEBUGMSG(("couldn't register class\n"));
		return false;
	}

	//////////////////////////////////////////////////////////////////
	// Initialize the importers/exporters
	//////////////////////////////////////////////////////////////////
	IE_ImpExp_RegisterXP ();

	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
	{
#if ENABLE_SPELL
		SpellManager::instance();
#endif
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
	else
		szMenuLabelSetName = AP_PREF_DEFAULT_StringSet;

	getMenuFactory()->buildMenuLabelSet(szMenuLabelSetName);	
	
	//////////////////////////////////////////////////////////////////
	// Check for necessary DLLs now that we can do localized error messages
	//////////////////////////////////////////////////////////////////

	// Ensure that common control DLL is loaded
	HINSTANCE hinstCC = LoadLibraryW(L"comctl32.dll");
	UT_return_val_if_fail (hinstCC, false);
	InitCommonControlsEx_fn  pInitCommonControlsEx = NULL;
	if( hinstCC != NULL )
		pInitCommonControlsEx = (InitCommonControlsEx_fn)GetProcAddress( hinstCC, "InitCommonControlsEx");
	if( pInitCommonControlsEx != NULL )
	{
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES 	// load the rebar and toolbar
					| ICC_TAB_CLASSES | ICC_UPDOWN_CLASS	// and tab and spin controls
					| ICC_STANDARD_CLASSES;
		pInitCommonControlsEx(&icex);
	}
	else
	{
		InitCommonControls();

		UT_Win32LocaleString err;
		err.fromUTF8 (m_pStringSet->getValue(AP_STRING_ID_WINDOWS_COMCTL_WARNING));		
		MessageBoxW(NULL, err.c_str(), NULL, MB_OK);
	}

	//////////////////////////////////////////////////////////////////
	// load the all Plugins from the correct directory
	//////////////////////////////////////////////////////////////////

#ifndef DISABLE_BUILTIN_PLUGINS
	abi_register_builtin_plugins();
#endif

	bool bLoadPlugins = true;
	bool bFound = getPrefsValueBool(XAP_PREF_KEY_AutoLoadPlugins,&bLoadPlugins);

	if(bLoadPlugins || !bFound)
	{
		WCHAR szPath[PATH_MAX];
		WCHAR szPlugin[PATH_MAX];
		_getExeDir( szPath, PATH_MAX);
#ifdef _MSC_VER
		lstrcatW(szPath, L"..\\plugins\\*.dll");
#else
#define ABI_WIDE_STRING(t) L ## t
		lstrcatW(szPath, ABI_WIDE_STRING("..\\lib\\" PACKAGE L"-" ABIWORD_SERIES L"\\plugins\\*.dll"));
#endif

		WIN32_FIND_DATAW cfile;
		HANDLE findtag = FindFirstFileW( szPath, &cfile );
		if( findtag != INVALID_HANDLE_VALUE )
		{
			do
			{	
				_getExeDir( szPlugin, PATH_MAX );
#ifdef _MSC_VER
				lstrcatW( szPlugin, L"..\\plugins\\" );
#else
				lstrcatW( szPlugin, ABI_WIDE_STRING("..\\lib\\" PACKAGE L"-" ABIWORD_SERIES L"\\plugins\\" ));
#endif
				lstrcatW( szPlugin, cfile.cFileName );
				XAP_ModuleManager::instance().loadModule( getUTF8String(szPlugin) );
			} while( FindNextFileW ( findtag, &cfile ) );
			FindClose( findtag );
		}

		UT_String pluginName( getUserPrivateDirectory() ); 
		UT_String pluginDir( getUserPrivateDirectory() );
		pluginDir += "\\AbiWord\\plugins\\*.dll";
		UT_Win32LocaleString str;
		str.fromUTF8(pluginDir.c_str());
		findtag = FindFirstFileW( str.c_str(), &cfile );
		if( findtag != INVALID_HANDLE_VALUE )
		{
			do
			{	
				pluginName = getUserPrivateDirectory();
				pluginName += "\\AbiWord\\plugins\\";
				pluginName += getUTF8String(cfile.cFileName);
				XAP_ModuleManager::instance().loadModule( pluginName.c_str() );
			} while( FindNextFileW( findtag, &cfile ) );
			FindClose( findtag );
		}
	}
	return bSuccess;
}


// if app is NULL then we use 'this'
XAP_Frame * AP_Win32App::newFrame(void)
{
	AP_Win32Frame * pWin32Frame = new AP_Win32Frame();

	if (pWin32Frame)
		pWin32Frame->initialize();

	return pWin32Frame;
}


bool AP_Win32App::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

	delete m_prefs;
	m_prefs = NULL;

	return true;
}

bool AP_Win32App::getPrefsValueDirectory(bool bAppSpecific,
											const gchar * szKey, const gchar ** pszValue) const
{
	if (!m_prefs)
		return false;

	const gchar * psz = NULL;
	if (!m_prefs->getPrefsValue(szKey,&psz))
		return false;

	if ((*psz == '/') || (*psz == '\\'))
	{
		*pszValue = psz;
		return true;
	}

	const gchar * dir = ((bAppSpecific) ? getAbiSuiteAppDir() : getAbiSuiteLibDir());

	static gchar buf[1024];
	UT_return_val_if_fail ((strlen(dir) + strlen(psz) + 2) < sizeof(buf), false);
	
	sprintf(buf,"%s\\%s",dir,psz);
	*pszValue = buf;
	return true;
}

const char * AP_Win32App::getAbiSuiteAppDir(void) const
{
    return getAbiSuiteLibDir();
}

HICON AP_Win32App::getIcon(void)
{

	int sy = GetSystemMetrics(SM_CYICON);
	int sx = GetSystemMetrics(SM_CXICON);
	UT_DEBUGMSG(("GetIcon(): system metrics [%d %d]\n",sx,sy));
	
	if ((sx==32) && (sy==32))
		return LoadIconW(getInstance(), MAKEINTRESOURCEW(AP_RID_ICON_APPLICATION_32));
	else
		return (HICON) LoadImageW(getInstance(), MAKEINTRESOURCEW(AP_RID_ICON_APPLICATION_32), IMAGE_ICON, 0,0,0);
}

HICON AP_Win32App::getSmallIcon(void)
{

	int sy = GetSystemMetrics(SM_CYICON);
	int sx = GetSystemMetrics(SM_CXICON);
	UT_DEBUGMSG(("GetIcon(): system metrics [%d %d]\n",sx,sy));

	if ((sx==16) && (sy==16))
		return LoadIconW(getInstance(), MAKEINTRESOURCEW(AP_RID_ICON_APPLICATION_16));
	else
		return (HICON) LoadImageW(getInstance(), MAKEINTRESOURCEW(AP_RID_ICON_APPLICATION_16), IMAGE_ICON, 0,0,0);
}

const XAP_StringSet * AP_Win32App::getStringSet(void) const
{
	return m_pStringSet;
}

#ifdef COPY_ON_DEMAND
/*!
    indicate to the clipboard that we can provide data in this format
    on demand
*/
void AP_Win32App::_indicateFmtToClipboard(const char * pszFmt) const
{
	UT_return_if_fail(m_pClipboard && pszFmt);
	UINT iFmt = m_pClipboard->convertFormatString(pszFmt);

	SetClipboardDataW(iFmt, NULL);
}

bool AP_Win32App::_cacheClipboardDoc(PD_DocumentRange *pDocRange)
{
	UT_return_val_if_fail(m_pClipboard && pDocRange, false);

	UT_ByteBuf buf;
	UT_Error status;;
	UT_Byte b = 0;

	IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
	if (pExpRtf)
	{
		status = pExpRtf->copyToBuffer(pDocRange,&buf);

		if(status != UT_OK)
			return false;
			
		buf.append(&b,1);			// NULL terminate the string
		DELETEP(pExpRtf);
	}
	else
	{
		return false;
	}

	// now create a subdocument ...
	PD_Document * pDoc = new PD_Document();

	if(!pDoc)
		return false;
	
	pDoc->newDocument();
	
	PD_DocumentRange DocRange(pDoc, 2, 2);
	
	IE_Imp * pImp = 0;
	IE_Imp::constructImporter(pDoc, IE_Imp::fileTypeForSuffix(".rtf"),&pImp,0);

	if(pImp)
	{
		pImp->pasteFromBuffer(&DocRange,buf.getPointer(0),buf.getLength(),NULL);
		delete pImp;
	}
	else
	{
		return false;
	}
	
	m_pClipboard->setClipboardDoc(pDoc);
	return true;
}

/*!
    copy the required format to the clipboard on demand
    see docs on WM_RENDERFORMAT
*/
bool AP_Win32App::copyFmtToClipboardOnDemand(UINT iFmt)
{
	UT_return_val_if_fail(m_pClipboard, false);

	PD_DocumentRange DocRange;
	DocRange.m_pDoc = m_pClipboard->getClipboardDoc();
	UT_return_val_if_fail(DocRange.m_pDoc, false);

	DocRange.m_pos1 = 2;
	DocRange.m_pos2 = DocRange.m_pDoc->getLastFrag()->getPos() + DocRange.m_pDoc->getLastFrag()->getLength();
	
	if(!_copyFmtToClipboard(&DocRange, iFmt))
		return false;

	return true;
}

/*!
    copy data to the clipboard in all formats on demand (this is
    called when application is exiting and leaving data on clipboard
    that and indicating that data in addtional formats is available
    see docs on WM_RENDERALLFORMATS
*/
bool AP_Win32App::copyAllFmtsToClipboardOnDemand()
{
	UT_return_val_if_fail(m_pClipboard, false);

	// I will use NULL here, since we are about to shut down anyway ...
	if(!m_pClipboard->openClipboard(NULL))			// try to lock the clipboard
		return false;

	// need to clear clipboard in order to become its owners
	m_pClipboard->clearClipboard();
	
	// what we need is to get the data from the clipboard and convert
	// it to the format requested
	PD_DocumentRange DocRange;
	DocRange.m_pDoc = m_pClipboard->getClipboardDoc();
	UT_return_val_if_fail(DocRange.m_pDoc, false);

	DocRange.m_pos1 = 2;
	DocRange.m_pos2 = DocRange.m_pDoc->getLastFrag()->getPos() + DocRange.m_pDoc->getLastFrag()->getLength();

	_copyFmtToClipboard(&DocRange, AP_CLIPBOARD_RTF);
	_copyFmtToClipboard(&DocRange, AP_CLIPBOARD_TEXTPLAIN_UCS2);
	_copyFmtToClipboard(&DocRange, AP_CLIPBOARD_TEXTPLAIN_8BIT);

	return true;
}

#endif // COPY_ON_DEMAND

/*!
    copy data in required format to the clipboard
*/
bool AP_Win32App::_copyFmtToClipboard(PD_DocumentRange * pDocRange, UINT iFmt)
{
	UT_return_val_if_fail(m_pClipboard, false);
	const char * pszFmt = m_pClipboard->convertToFormatString(iFmt);
	UT_return_val_if_fail(pszFmt, false);

	return _copyFmtToClipboard(pDocRange, pszFmt);
}

/*!
    copy data in required format to the clipboard
*/
bool AP_Win32App::_copyFmtToClipboard(PD_DocumentRange * pDocRange, const char * pszFmt)
{
	UT_return_val_if_fail(m_pClipboard && pszFmt, false);
	
	UT_ByteBuf buf;
	UT_Error status;

	if(0 == strcmp(AP_CLIPBOARD_TEXTPLAIN_8BIT, pszFmt))
	{
		IE_Exp_Text * pExpText = new IE_Exp_Text(pDocRange->m_pDoc);
		if (pExpText)
		{
			status = pExpText->copyToBuffer(pDocRange,&buf);

			if(status != UT_OK)
				return false;
			
			UT_Byte b = 0;
			buf.append(&b,1);			// NULL terminate the string
			m_pClipboard->addData(AP_CLIPBOARD_TEXTPLAIN_8BIT,
								  (UT_Byte *)buf.getPointer(0),buf.getLength());
			DELETEP(pExpText);
			UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in TEXTPLAIN format.\n",
						 buf.getLength()));
		}
		else
		{
			return false;
		}
		
	}
	else if(0 == strcmp(AP_CLIPBOARD_TEXTPLAIN_UCS2, pszFmt))
	{
		const char *szEnc = XAP_EncodingManager::get_instance()->getNativeUnicodeEncodingName(); 
		IE_Exp_Text * pExpUnicodeText = new IE_Exp_Text(pDocRange->m_pDoc,szEnc);
		if (pExpUnicodeText)
		{
			status = pExpUnicodeText->copyToBuffer(pDocRange,&buf);

			if(status != UT_OK)
				return false;
			
			UT_Byte b[2] = {0,0};
			buf.append(b,2);			// NULL terminate the string
			m_pClipboard->addData(AP_CLIPBOARD_TEXTPLAIN_UCS2,
								  (UT_Byte *)buf.getPointer(0),buf.getLength());
			DELETEP(pExpUnicodeText);
			UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in TEXTPLAIN UNICODE format.\n",
						 buf.getLength()*2));
		}
		else
		{
			return false;
		}
	}
	else if(0 == strcmp(AP_CLIPBOARD_RTF, pszFmt))
	{
		IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
		if (pExpRtf)
		{
			status = pExpRtf->copyToBuffer(pDocRange,&buf);

			if(status != UT_OK)
				return false;

			UT_Byte b = 0;
			buf.append(&b,1);			// NULL terminate the string
			m_pClipboard->addData(AP_CLIPBOARD_RTF,(UT_Byte *)buf.getPointer(0),buf.getLength());
			DELETEP(pExpRtf);
			UT_DEBUGMSG(("CopyFmtToClipboard: copying %d bytes in RTF format.\n",
						 buf.getLength()));
		}
		else
		{
			return false;
		}
	}

	return true;
}

/*!
    copy data to the clipboard; this is what gets called when the user
    presses Ctrl+C
*/
void AP_Win32App::copyToClipboard(PD_DocumentRange * pDocRange, bool /*bUseClipboard*/)
{
	// copy the given subset of the given document to the
	// system clipboard in a variety of formats.
	// MSFT requests that we post them in the order of
	// importance to us (most preserving to most lossy).
	//
	// TODO do we need to put something in .ABW format on the clipboard ??

	AP_Win32FrameImpl * pFrameImp = static_cast<AP_Win32FrameImpl*>(getLastFocussedFrame()->getFrameImpl());
	UT_return_if_fail(pFrameImp);
	
	if (!m_pClipboard->openClipboard(pFrameImp->getHwndDocument()))
		return;

	m_pClipboard->clearClipboard(); // this also gives us the ownership

	// Be smart: always place RTF on clipboard; the
	// remaining formats we will generate on demand.
	// most of the time it will save us creating multiple importers,
	// but when the user requests other than the default format, it
	// will be a little bit more involved
	// Tomas, June 28, 2003.

#ifndef COPY_ON_DEMAND
	_copyFmtToClipboard(pDocRange, AP_CLIPBOARD_RTF);
#else
	// we need to both cache the present doc and put rtf version on
	// the clipboard, because win32 will ask for it immediately;
	// appart from that, the rtf exporter needs some layout info to
	// deal with bidi issues, which means we cannot construct the rtf
	// from the chached doc properly
	_cacheClipboardDoc(pDocRange);
	_copyFmtToClipboard(pDocRange, AP_CLIPBOARD_RTF);
#endif
	
	// TODO Should use a finer-grain technique than IsWinNT()
	// since Win98 supports unicode clipboard.
	if (UT_IsWinNT())
	{
		// put raw unicode text on the clipboard
#ifndef COPY_ON_DEMAND
		_copyFmtToClipboard(pDocRange, AP_CLIPBOARD_TEXTPLAIN_UCS2);
#else
		_indicateFmtToClipboard(AP_CLIPBOARD_TEXTPLAIN_UCS2);
#endif
	}
	else
	{
		// put raw 8bit text on the clipboard
#ifndef COPY_ON_DEMAND
		_copyFmtToClipboard(pDocRange, AP_CLIPBOARD_TEXTPLAIN_8BIT);
#else
		_indicateFmtToClipboard(AP_CLIPBOARD_TEXTPLAIN_8BIT);
#endif
	}

	m_pClipboard->closeClipboard();				// release clipboard lock
}

//
// 
//
PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{ 
	BITMAP 		bmp; 
	PBITMAPINFO 	pbmi; 
	WORD    	cClrBits; 

	// Retrieve the bitmap's color format, width, and height. 
    	if (!GetObjectW(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
		return NULL;
	
	if (bmp.bmBitsPixel==16) bmp.bmBitsPixel=24;	// 16 bit BMPs are not supported by all programs
					

	// Convert the color format to a count of bits. 
   	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    
	if (cClrBits == 1) 
		cClrBits = 1; 
	else if (cClrBits <= 4) 
		cClrBits = 4; 
	else if (cClrBits <= 8) 
		cClrBits = 8; 
	else if (cClrBits <= 16) 
		cClrBits = 16; 
	else if (cClrBits <= 24) 
		cClrBits = 24; 
	else cClrBits = 32; 
	
	// Allocate memory for the BITMAPINFO structure. (This structure 
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
	// data structures.) 
	
	if (cClrBits != 24) 
	 pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
	            sizeof(BITMAPINFOHEADER) + 
	            sizeof(RGBQUAD) * (1<< cClrBits)); 
	
	// There is no RGBQUAD array for the 24-bit-per-pixel format. 	
	else 
	 pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
	            sizeof(BITMAPINFOHEADER)); 
	
	// Initialize the fields in the BITMAPINFO structure. 
	
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
	pbmi->bmiHeader.biWidth = bmp.bmWidth; 
	pbmi->bmiHeader.biHeight = bmp.bmHeight; 
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
	if (cClrBits < 24) 
	pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 
	
	// If the bitmap is not compressed, set the BI_RGB flag. 
	pbmi->bmiHeader.biCompression = BI_RGB; 
	
	// Compute the number of bytes in the array of color 
	// indices and store the result in biSizeImage. 
	// For Windows NT/2000, the width must be DWORD aligned unless 
	// the bitmap is RLE compressed.
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
	                          * pbmi->bmiHeader.biHeight; 
	// Set biClrImportant to 0, indicating that all of the 
	// device colors are important. 
	pbmi->bmiHeader.biClrImportant = 0; 
	return pbmi; 
} 

//
//
//
void CreateBMP(HWND /*hwnd*/, UT_ByteBuf & pBB, PBITMAPINFO pbi, 
                  HBITMAP hBMP, HDC hDC) 
{ 
	BITMAPFILEHEADER hdr;       // bitmap file-header 
	PBITMAPINFOHEADER pbih;     // bitmap info-header 
	LPBYTE lpBits;              // memory pointer 	

	if (!hBMP) return;

	pbih = (PBITMAPINFOHEADER) pbi; 
	lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits) return;

	// Retrieve the color table (RGBQUAD array) and the bits 
	// (array of palette indices) from the DIB. 
	if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, 
		 DIB_RGB_COLORS)) 
	return;

	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
	// Compute the size of the entire file. 
	hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
			pbih->biSize + pbih->biClrUsed 
			* sizeof(RGBQUAD) + pbih->biSizeImage); 
	hdr.bfReserved1 = 0; 
	hdr.bfReserved2 = 0; 

	// Compute the offset to the array of color indices. 
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
	pbih->biSize + pbih->biClrUsed 
	* sizeof (RGBQUAD); 

	pBB.truncate (0);

	// Copy the BITMAPFILEHEADER into the .BMP file. 
	pBB.append ((const UT_Byte *)&hdr, sizeof(BITMAPFILEHEADER));
	pBB.append ((const UT_Byte *)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof (RGBQUAD));

	// Copy the array of color indices into the .BMP file.         
	pBB.append ((const UT_Byte *)lpBits, (int) pbih->biSizeImage);

	GlobalFree((HGLOBAL)lpBits);
}



void AP_Win32App::pasteFromClipboard(PD_DocumentRange * pDocRange, bool /*bUseClipboard*/, bool bHonorFormatting)
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
	AP_Win32FrameImpl * pFrameImp = static_cast<AP_Win32FrameImpl*>(getLastFocussedFrame()->getFrameImpl());
	UT_return_if_fail(pFrameImp);
	
	if (!m_pClipboard->openClipboard(pFrameImp->getHwndDocument())) // lock clipboard
		return;
	
	{
		// TODO Paste the most detailed version unless user overrides.
		// TODO decide if we need to support .ABW on the clipboard.
		if (!((bHonorFormatting && _pasteFormatFromClipboard(pDocRange, AP_CLIPBOARD_RTF, ".rtf", false)) ||
			_pasteFormatFromClipboard(pDocRange, AP_CLIPBOARD_TEXTPLAIN_UCS2, ".txt", true) ||
			_pasteFormatFromClipboard(pDocRange, AP_CLIPBOARD_BMP, ".bmp", false) ||
			_pasteFormatFromClipboard(pDocRange, AP_CLIPBOARD_TEXTPLAIN_8BIT, ".txt", false)))
		{
			// TODO figure out what to do with an image and other formats....
			UT_DEBUGMSG(("PasteFromClipboard: TODO support this format..."));
		}
	}

	m_pClipboard->closeClipboard();				// release clipboard lock
	return;
}

bool AP_Win32App::_pasteFormatFromClipboard(PD_DocumentRange * pDocRange, const char * szFormat,
											const char * szType, bool bWide)
{
	HANDLE	hData;
	bool 	bSuccess = false;	
  
	if (!(hData = m_pClipboard->getHandleInFormat(szFormat)))
		return bSuccess;		
 		
 	// It's a bitmap
 	if (g_ascii_strcasecmp(szFormat, AP_CLIPBOARD_BMP)==0)
	{			
 		HBITMAP					hBitmap;
 		PBITMAPINFO 			bi;
 		HWND		 			hWnd;
 		HDC 					hdc;
 		IE_ImpGraphic*			pIEG = NULL;
 		FG_ConstGraphicPtr pFG;
 		UT_Error 				errorCode;		
 		UT_ByteBuf 				byteBuf;				
 		IEGraphicFileType		iegft = IEGFT_BMP;	
 		XAP_Frame* 				pFrame;						
 		AP_FrameData* 			pFrameData;		
 		FL_DocLayout*			pDocLy;	
 		FV_View* 				pView;						
		UT_ByteBuf*				bBufBMP = new UT_ByteBuf;
 		
 		hBitmap = (HBITMAP)hData;					
 		hWnd =  GetDesktopWindow();
 		hdc = GetDC(hWnd);		
 		
 		// Create a BMP file from a BITMAP
 		bi =  CreateBitmapInfoStruct(hBitmap);						
 		CreateBMP(hWnd, *bBufBMP, bi, hBitmap,hdc);                  										
 		
 		// Since we are providing the file type, there is not need to pass the bytebuff filled up
 		errorCode = IE_ImpGraphic::constructImporter(*bBufBMP, iegft, &pIEG);				 				
		 				
 		if(errorCode != UT_OK)		
			return false;				  	
		 				 			
 		errorCode = pIEG->importGraphic(bBufBMP, pFG); 		
 		
 		if(errorCode != UT_OK || !pFG)
		{
			DELETEP(bBufBMP);
			DELETEP(pIEG);
 			return false;
		}
		// sunk in importGraphic
		bBufBMP = NULL;
 		 
 		// Insert graphic in the view
 		pFrame = getLastFocussedFrame(); 						
 		pFrameData = (AP_FrameData*) pFrame->getFrameData();		
 		pDocLy =	pFrameData->m_pDocLayout;	
 		pView =  pDocLy->getView();		
 				
 		errorCode = pView->cmdInsertGraphic(pFG);	  		  		
 	
		DELETEP(pIEG);
 		
 		bSuccess = true;
 	}
 	else	
	{
		unsigned char * pData = static_cast<unsigned char *>(GlobalLock(hData));
		UT_DEBUGMSG(("Paste: [fmt %s %s][hdata 0x%08lx][pData 0x%08lx]\n",
					 szFormat, szType,  hData, pData));
		UT_uint32 iSize = GlobalSize(hData);
		UT_uint32 iStrLen = bWide
			? wcslen(reinterpret_cast<const wchar_t *>(pData)) * 2
			: strlen(reinterpret_cast<const char *>(pData));
		UT_uint32 iLen = UT_MIN(iSize,iStrLen);

		
		IE_Imp * pImp = 0;
		IE_Imp::constructImporter(pDocRange->m_pDoc, IE_Imp::fileTypeForSuffix(szType), &pImp, 0);
		if (pImp)
		{
			const char * szEncoding = 0;
			if (bWide)
			{
				szEncoding = XAP_EncodingManager::get_instance()->getUCS2LEName();
			}
			else
			{
				; // TODO Get code page using CF_LOCALE
			}
			pImp->pasteFromBuffer(pDocRange,pData,iLen,szEncoding);
			delete pImp;
		}

		GlobalUnlock(hData);
		bSuccess = true;
	}
	return bSuccess;
}

bool AP_Win32App::canPasteFromClipboard(void)
{
	if (!getLastFocussedFrame()) 
		return false;

	AP_Win32FrameImpl * pFrameImp = static_cast<AP_Win32FrameImpl*>(getLastFocussedFrame()->getFrameImpl());
	UT_return_val_if_fail(pFrameImp, false);
	
	if (!m_pClipboard->openClipboard(pFrameImp->getHwndDocument()))
		return false;

	// TODO decide if we need to support .ABW format on the clipboard.	
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_RTF))
		goto ReturnTrue;
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_TEXTPLAIN_UCS2))
		goto ReturnTrue;
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_TEXTPLAIN_8BIT))
		goto ReturnTrue;

	// If IEGFT_BMP!=0 we have a plugin that can deal with BMP format
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_BMP) && IEGFT_BMP)
  		goto ReturnTrue;
  		
	m_pClipboard->closeClipboard();
	return false;

ReturnTrue:
	m_pClipboard->closeClipboard();
	return true;
}

/*****************************************************************/

#if defined(_DEBUG) && defined(_MSC_VER)
#define  SET_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
#endif

/*****************************************************************/

int AP_Win32App::WinMain(const char * szAppName, HINSTANCE hInstance,
						 HINSTANCE /*hPrevInstance*/, PSTR /*szCmdLine*/, int iCmdShow)
{
#if !GLIB_CHECK_VERSION(2,32,0)
	if (!g_thread_supported ())
		g_thread_init (NULL);
#endif
	
	bool bShowApp = true;
	BOOL bInitialized = FALSE; 
	
	// this is a static function and doesn't have a 'this' pointer.
	MSG msg;

#ifdef _MSC_VER
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
#endif

	// HACK: load least-common-denominator Rich Edit control
	// TODO: fix Spell dlg so we don't rely on this
	// ALT:  make it a Preview widget instead

	HINSTANCE hinstRich = LoadLibraryW(L"riched32.dll");
	if (!hinstRich)
		hinstRich = LoadLibraryW(L"riched20.dll");
	UT_return_val_if_fail (hinstRich, 1);
	
	AP_Win32App * pMyWin32App;

	// OLE Stuff
	if (SUCCEEDED(OleInitialize(NULL)))
            bInitialized = TRUE;                    
  
	
// We put this in a block to force the destruction of Args in the stack
{
	UT_Win32LocaleString scnv;
	UT_UTF8String sUTFCmdLine;

	// Load the command line into an XAP_Args class
	scnv.fromLocale(GetCommandLineW());
	sUTFCmdLine=scnv.utf8_str();
	XAP_Args XArgs = XAP_Args(sUTFCmdLine.utf8_str());

	// Step 1: Initialize our application.
	pMyWin32App = new AP_Win32App(hInstance, szAppName);
	AP_Args Args = AP_Args(&XArgs, szAppName, pMyWin32App);

	Args.parseOptions();
	pMyWin32App->initialize();
  
	// Step 2: Handle all non-window args.
	// process args (calls common arg handler, which then calls platform specific)
	// As best I understand, it returns true to continue and show window, or
	// false if no window should be shown (and thus we should simply exit).    
	bool windowlessArgsWereSuccessful = true;
	if (!Args.doWindowlessArgs(windowlessArgsWereSuccessful))
	{
		pMyWin32App->shutdown();	// properly shutdown the app 1st
		delete pMyWin32App;
		return (windowlessArgsWereSuccessful ? 0 : -1);
	}

	// Step 3: Create windows as appropriate.
	// if some args are botched, it returns false and we should
	// continue out the door.
	// We used to check for bShowApp here.  It shouldn't be needed
	// anymore, because doWindowlessArgs was supposed to bail already. -PL
	if (!pMyWin32App->openCmdLineFiles(&Args))
	{
		pMyWin32App->shutdown();	// properly shutdown the app 1st
		delete pMyWin32App;
		return 0;
	}
}
//
// This block is controlled by the Structured Exception Handle
// if any crash happens here we will recover it and save the file (cross fingers)
//	


try
{		
	UT_uint32 iHeight = 0, iWidth = 0, t_flag =0;
	UT_sint32 iPosX = 0, iPosY = 0;
		
	if (!((XAP_App::getApp()->getGeometry(&iPosX,&iPosY,&iWidth,&iHeight,&t_flag)) &&
	       ((iWidth > 0) && (iHeight > 0)))	)
		XAP_App::getApp()->getDefaultGeometry(iWidth,iHeight,t_flag);
	
	if ((t_flag & PREF_FLAG_GEOMETRY_MAXIMIZED)==PREF_FLAG_GEOMETRY_MAXIMIZED)
			iCmdShow = SW_SHOWMAXIMIZED;
	
	if (bShowApp)
	{
		// display the windows
		for(UT_sint32 i = 0; i < pMyWin32App->m_vecFrames.getItemCount(); i++)
		{
			AP_Win32Frame * curFrame = (AP_Win32Frame*)pMyWin32App->m_vecFrames[i];
			UT_continue_if_fail(curFrame);
		
			HWND hwnd = curFrame->getTopLevelWindow();
			ShowWindow(hwnd, iCmdShow);
			UpdateWindow(hwnd);
		}	

		// do dispatch loop
		while(UT_GetMessage(&msg, NULL, 0, 0))
	    {
   	      	// TranslateMessage is not called because AbiWord
	      	// has its own way of decoding keyboard accelerators
	      	if (pMyWin32App->handleModelessDialogMessage(&msg)) 
				continue;
				
			TranslateMessage(&msg);	
			UT_DispatchMessage(&msg);
	    	
			// Check for idle condition
			while( !UT_Win32Idle::_isEmpty() &&
                   !PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE) ) 
			{
				// Fire idle functions when no pending messages
		    	UT_Win32Idle::_fireall();
			}
	    }
	}
	
	// Un-init OLE		               
        if (bInitialized)
                OleUninitialize();

	FreeLibrary(hinstRich);

	// unload all loaded plugins (remove some of the memory leaks shown at shutdown :-)
	XAP_ModuleManager::instance().unloadAllPlugins();
	
	// Step 4: Destroy the App.  It should take care of deleting all frames.
	pMyWin32App->shutdown();
	delete pMyWin32App;
	
	
}// end of thes block is controlled by the Exception Handler

//
// If an exception happens, with "catch" the block
// and then the save it into disk
//
catch (...)
{
#ifdef DEBUG
	throw;
#endif

	AP_Win32App *pApp = (AP_Win32App *) XAP_App::getApp();
	
	UT_return_val_if_fail (pApp,1);

	// first of all, try to save the current prefs (so that any log entries are dumped
	// onto disk -- this allows us to save useful info for dbg purposes) we will enclose
	// this inside of a try/catch block, so that in the (unlikely) case something goes
	// badly wrong when writing the prefs file, we still get chance to save the open
	// documents

	try
	{
		if(pApp->getPrefs())
		{
			pApp->getPrefs()->savePrefsFile();
		}
	}
	catch(...)
	{
		// do nothing
	}

	pApp->saveRecoveryFiles();

	// Tell the user was has just happened
	AP_Win32Frame * curFrame = (AP_Win32Frame*)pApp->m_vecFrames[0];
	if (curFrame)
	{
		curFrame->showMessageBox(AP_STRING_ID_MSG_Exception,XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		
	}
}// end of except

	SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
	return msg.wParam;
}

/* This function takes a description and compares it all the registerd 
   importers descriptions and returns either the appropriate importer's
   IEFileType or returns IEFT_Unknown if no match was made.
*/
IEFileType AP_Win32App::_getFileTypeFromDesc(const char *desc)
{
	const char *iftDesc;
	const char *iftSuffixList;
	IEFileType ift;

	// no description given or description == 'UNKNOWN' then unknown
	if (!desc || !*desc || (g_ascii_strcasecmp(desc, "Unknown")==0)) 
		return IEFT_Unknown;  

	UT_uint32 i = 0;
	while (IE_Imp::enumerateDlgLabels(i, &iftDesc, &iftSuffixList, &ift))
	{
		// TODO: change to actually test all but suffixes, 
		// ie if iftDesc == 'Some FileType (*.sft, *.someft)' then only
            // test against 'Some FileType'
		if (g_ascii_strncasecmp(iftDesc, desc, strlen(desc)) == 0)
			return ift;
		
		// try next importer
		i++;
	}

	// if we made it here then description didn't match anything, so unknown.
	return IEFT_Unknown;
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
			hWnd = (HWND) m_IdTable[ iCounter ].pDialog->pGetWindowHandle();

			if( hWnd && IsDialogMessageW( hWnd, msg ) )
				return true;
		}
		else
			break;
	}

	return false;
}

// cmdline processing call back I reckon
void AP_Win32App::errorMsgBadArg(const char *msg)
{
	char *pszMessage;
	UT_Win32LocaleString str;

	pszMessage = g_strdup_printf ("%s\nRun with --help' to see a full list of available command line options.\n", msg);
	str.fromUTF8(pszMessage);
	MessageBoxW(NULL, str.c_str(), L"Command Line Option Error", MB_OK|MB_ICONERROR);
	g_free( pszMessage );
}

// cmdline processing call back I reckon
void AP_Win32App::errorMsgBadFile(XAP_Frame * pFrame, const char * file, 
							 UT_Error error)
{
	s_CouldNotLoadFileMessage (pFrame, file, error);
}

/*!
 * A callback for AP_Args's doWindowlessArgs call which handles
 * platform-specific windowless args.
 * return false if we should exit normally but Window should not be displayed
 */
bool AP_Win32App::doWindowlessArgs(const AP_Args *Args, bool & bSuccess)
{
	bSuccess = true;

	AP_Win32App * pMyWin32App = static_cast<AP_Win32App*>(Args->getApp());

	if (Args->m_sGeometry)
	{
		// [--geometry <X geometry string>]
		#if 0
		gint x = 0;
		gint y = 0;
		guint width = 0;
		guint height = 0;
		
		XParseGeometry(Args->m_sGeometry, &x, &y, &width, &height);

		// set the xap-level geometry for future frame use
		Args->getApp()->setGeometry(x, y, width, height, f);
		#endif

		parseAndSetGeometry(Args->m_sGeometry);
	}
	else
	if (Args->m_sPrintTo) 
	{
		if (Args->m_sFiles[0])
		{
			UT_DEBUGMSG(("DOM: Printing file %s\n", Args->m_sFiles[0]));
			AP_Convert conv ;

			if (Args->m_sMerge)
				conv.setMergeSource (Args->m_sMerge);

			if (Args->m_impProps)
				conv.setImpProps (Args->m_impProps);
			if (Args->m_expProps)
				conv.setExpProps (Args->m_expProps);
			
			UT_String s = "AbiWord: ";
			s+= Args->m_sFiles[0];
			
            UT_Win32LocaleString prn, doc;
			prn.fromASCII (Args->m_sPrintTo);
			doc.fromASCII (s.c_str());
			GR_Graphics * pG = GR_Win32Graphics::getPrinterGraphics(prn.c_str(), doc.c_str());			
            if(!pG)
			{
				// do not assert here, if the graphics creation failed, the static
				// constructor has asserted already somewhere more relevant
				return false;
			}
			
			conv.setVerbose(Args->m_iVerbose);
			conv.print (Args->m_sFiles[0], pG, Args->m_sFileExtension);
	      
			delete pG;
		}
		else
		{
			// couldn't load document
			UT_DEBUGMSG(("Error: no file to print!\n"));
			bSuccess = false;
		}

		return false;
	}

	if(Args->m_sPluginArgs)
	{
	//
	// Start a plugin rather than the main abiword application.
	//
	    const char * szName = NULL;
		XAP_Module * pModule = NULL;
		bool bFound = false;	
		if(Args->m_sPluginArgs[0])
		{
			const char * szRequest = Args->m_sPluginArgs[0];
			const UT_GenericVector<XAP_Module*> * pVec = XAP_ModuleManager::instance().enumModules ();
			UT_DEBUGMSG((" %d plugins loaded \n",pVec->getItemCount()));
			for (UT_sint32 i = 0; (i < pVec->size()) && !bFound; i++)
			{
				pModule = pVec->getNthItem (i);
				szName = pModule->getModuleInfo()->name;
				UT_DEBUGMSG(("%s\n", szName));
				if(strcmp(szName,szRequest) == 0)
				{
					bFound = true;
				}
			}
		}
		if(!bFound)
		{
			UT_DEBUGMSG(("Plugin %s not found or loaded \n",Args->m_sPluginArgs[0]));
			bSuccess = false;
			return false;
		}

//
// You must put the name of the ev_EditMethod in the usage field
// of the plugin registered information.
//
		const char * evExecute = pModule->getModuleInfo()->usage;
		EV_EditMethodContainer* pEMC = pMyWin32App->getEditMethodContainer();
		const EV_EditMethod * pInvoke = pEMC->findEditMethodByName(evExecute);
		if(!pInvoke)
		{
			UT_DEBUGMSG(("Plugin %s invoke method %s not found \n",
				   Args->m_sPluginArgs[0],evExecute));
			bSuccess = false;
			return false;
		}
		//
		// Execute the plugin, then quit
		//
		ev_EditMethod_invoke(pInvoke, UT_String("Called From App"));
		return false;
	}

	return true;
}


/*
	Get the user interface languages installed
	Caller should delete the allocated vector
*/	
UT_Vector*	AP_Win32App::getInstalledUILanguages(void)
{		
	UT_Vector* pVec = new UT_Vector(64,16);
	UT_Language lang;

	for (UT_uint32 i=0; i< lang.getCount(); i++)
	{
		const char *pLangCode = (const char*)lang.getNthLangCode(i);
		if (doesStringSetExist(pLangCode))
			pVec->addItem(g_strdup((char*)pLangCode));	
		else
		{	
			/*The en-US is the default internal string set and wont be found on disk but it should be also listed*/
			if (strcmp(pLangCode, "en-US")==0)
				pVec->addItem(g_strdup((char*)pLangCode));		
		}
		
	}		

	return pVec;
}

/*
	Does a stringSet exist on disk?
*/
bool	AP_Win32App::doesStringSetExist(const char* pLocale)
{
	HANDLE in;
	const char * szDirectory = NULL;

	UT_return_val_if_fail(pLocale, false);
	
	getPrefsValueDirectory(true,AP_PREF_KEY_StringSetDirectory,&szDirectory);
	UT_return_val_if_fail(((szDirectory) && (*szDirectory)), false);

	char *szPathname = (char*) UT_calloc(sizeof(char),strlen(szDirectory)+strlen(pLocale)+100);
	UT_return_val_if_fail(szPathname, false);
	
	char *szDest = szPathname;
	strcpy(szDest, szDirectory);
	szDest += strlen(szDest);
	if ((szDest > szPathname) && (szDest[-1]!='\\'))
		*szDest++='\\';
	lstrcpyA(szDest,pLocale);
	lstrcatA(szDest,".strings");

	UT_Win32LocaleString wsFilename;
	wsFilename.fromUTF8(szPathname);

	in = CreateFileW(wsFilename.c_str(),0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
		OPEN_EXISTING,0,NULL);
	g_free (szPathname);
	
	if (in!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(in);
		return true;
	}			
	
	return false;
}


/* From UCS4 To WinLocale */
UT_Win32LocaleString	AP_Win32App::s_fromUCS4ToWinLocale(const UT_UCS4Char * szIn)
{	
	UT_Win32LocaleString sRslt;
	sRslt.fromUCS4(szIn);
	//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return sRslt;
}

/* From WinLocale To UCS4*/
UT_UCS4String	AP_Win32App::s_fromWinLocaleToUCS4(const char* szIn)
{
	UT_UCS4Char * src = new UT_UCS4Char[strlen(szIn)+1];	
	UT_UCS4_strcpy_char(src, (char*)szIn);	
	UT_UCS4String sRslt(src);	
	delete [] src;

	return sRslt;
}

/* From  UTF8 To WinLocale */
UT_Win32LocaleString 	AP_Win32App::s_fromUTF8ToWinLocale(const char* szInUTF8)
{
	UT_UTF8String utf8(szInUTF8);	
	UT_UCS4String sUCS4(utf8.ucs4_str());
	return AP_Win32App::s_fromUCS4ToWinLocale(sUCS4.ucs4_str());
}

/* From WinLocale To UTF8*/
UT_UTF8String	AP_Win32App::s_fromWinLocaleToUTF8(const char* szIn)
{
	UT_UCS4String sUCS4 = AP_Win32App::s_fromWinLocaleToUCS4(szIn);
	UT_UTF8String sRslt(sUCS4.utf8_str());	

	return sRslt;
}

GR_Graphics * AP_Win32App::newDefaultScreenGraphics() const
{
	XAP_Frame * pFrame = findValidFrame();
	UT_return_val_if_fail( pFrame, NULL );
	
	AP_Win32FrameImpl * pFI = (AP_Win32FrameImpl *) pFrame->getFrameImpl();
	UT_return_val_if_fail( pFI, NULL );

	return pFI->createDocWndGraphics();
}

// There is not signal wrapper on Win32. It does its exception handling.
// Maybe we should move the catch stuff into here.
// 
void AP_Win32App::catchSignals(int /*sig_num*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}
