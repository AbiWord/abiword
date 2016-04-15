/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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


#include <windows.h>
#include <io.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_path.h"
#include "ut_Win32LocaleString.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_New.h"
#include "ap_Win32Dialog_New.h"
#include "ap_Win32App.h"

#include "xap_Dlg_FileOpenSaveAs.h"
#include "ie_imp.h"
#include "ie_types.h"
#include "ut_string_class.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_New::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_New * p = new AP_Win32Dialog_New(pFactory,id);
	return p;
}

#ifdef _MSC_VER	// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

AP_Win32Dialog_New::AP_Win32Dialog_New(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_New(pDlgFactory,id), m_hThisDlg(NULL), _win32Dialog(this), m_pFrame(NULL)
{
}

AP_Win32Dialog_New::~AP_Win32Dialog_New(void)
{
}

void AP_Win32Dialog_New::runModal(XAP_Frame * pFrame)
{

	UT_ASSERT_HARMLESS(pFrame);
	m_pFrame = pFrame;

	_win32Dialog.runModal( pFrame, 
                           AP_DIALOG_ID_FILE_NEW, 
                           AP_RID_DIALOG_NEW, 
                           this );
}

#define _DS(c,s)	setDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_New::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_hThisDlg = hWnd;

	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_return_val_if_fail (app,1);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	_win32Dialog.setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_NEW_Title));

	// localize controls
	_DSX(NEW_BTN_OK,		DLG_OK);
	_DSX(NEW_BTN_CANCEL,	DLG_Cancel);
	_DS(NEW_RDO_TEMPLATE,	DLG_NEW_Create);
	_DS(NEW_RDO_EXISTING,	DLG_NEW_Open);
    _DS(NEW_BTN_EXISTING,	DLG_NEW_Choose);

	// set initial state
	_win32Dialog.setControlText(AP_RID_DIALOG_NEW_EBX_EXISTING, 
  								pSS->getValue(AP_STRING_ID_DLG_NEW_NoFile));

	HWND hControl = GetDlgItem(hWnd, AP_RID_DIALOG_NEW_LBX_TEMPLATE);

	HANDLE findtag;
	WIN32_FIND_DATAW cfile;
	UT_String templateName, searchDir;
	UT_String dirName[2];

	dirName[0] = XAP_App::getApp()->getUserPrivateDirectory();
	dirName[1] = XAP_App::getApp()->getAbiSuiteLibDir();

	for (int i=0; i<2; i++) {
		templateName = dirName[i]; 
		searchDir = templateName;
		searchDir += "\\templates\\*.awt";
		UT_Win32LocaleString str;
		str.fromUTF8 (searchDir.c_str());
		findtag = FindFirstFileW( str.c_str(), &cfile );
		if( findtag != INVALID_HANDLE_VALUE )
		{
			do
			{	
				str.fromLocale(cfile.cFileName);
				if(!wcsstr(str.c_str(), L"normal.awt-")) // don't truncate localized template names
					str = str.substr ( 0, str.size () - 4 ) ;

				UT_sint32 nIndex = SendMessageW( hControl, LB_ADDSTRING, 0, (LPARAM)str.c_str());
				SendMessageW( hControl, LB_SETITEMDATA, (WPARAM) nIndex, (LPARAM) i );

			} while( FindNextFileW( findtag, &cfile ) );
		}
		FindClose( findtag );
	}

	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
	_updateControls();
	return 1;	// 1 == we did not call SetFocus()
}


BOOL AP_Win32Dialog_New::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_NEW_BTN_CANCEL
		setAnswer (AP_Dialog_New::a_CANCEL);
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also AP_RID_DIALOG_NEW_BTN_OK
		setAnswer (AP_Dialog_New::a_OK);
		EndDialog(hWnd,0);
		return 1;

	case AP_RID_DIALOG_NEW_LBX_TEMPLATE:
		switch (HIWORD(wParam))
		{
		case LBN_SELCHANGE:
			UT_sint32 nIndex = _win32Dialog.getListSelectedIndex(wId);
			_setFileName( nIndex );
			return 1;
		}
		return 0;

	case AP_RID_DIALOG_NEW_BTN_EXISTING:
		_doChoose();
		return 1;

	case AP_RID_DIALOG_NEW_RDO_TEMPLATE:
		setOpenType(AP_Dialog_New::open_Template);
		{
			int nIndex = _win32Dialog.getListSelectedIndex(AP_RID_DIALOG_NEW_LBX_TEMPLATE);
			if( nIndex == LB_ERR )
			{
				HWND hControl = GetDlgItem(hWnd, AP_RID_DIALOG_NEW_LBX_TEMPLATE);
				nIndex = SendMessageW( hControl, LB_FINDSTRING , (WPARAM) -1, (LPARAM) "Normal" );
				_win32Dialog.selectListItem(AP_RID_DIALOG_NEW_LBX_TEMPLATE, nIndex);
				_setFileName( nIndex );
			}
		}
		_updateControls();
		return 1;

	case AP_RID_DIALOG_NEW_RDO_EXISTING:
		setOpenType(AP_Dialog_New::open_Existing);
		_updateControls();
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_New::_onDeltaPos(NM_UPDOWN * /*pnmud*/)
{
	return 0;
}

void AP_Win32Dialog_New::_doChoose()
{

	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) m_pFrame->getDialogFactory();

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_return_if_fail (pDialog);

	pDialog->setCurrentPathname(0);
	pDialog->setSuggestFilename(false);

	UT_uint32 filterCount = IE_Imp::getImporterCount();
	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
													  sizeof(char *));
	UT_return_if_fail(szDescList);
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
														sizeof(char *));
	if(!szSuffixList)
	{
		UT_ASSERT_HARMLESS(szSuffixList);
		FREEP(szDescList);
		return;
	}
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1,
												   sizeof(IEFileType));
	if(!nTypeList)
	{
		UT_ASSERT_HARMLESS(nTypeList);
		FREEP(szDescList);
		FREEP(szSuffixList);
		return;
	}
	UT_uint32 k = 0;

	while (IE_Imp::enumerateDlgLabels(k, &szDescList[k], 
									  &szSuffixList[k], &nTypeList[k]))
			k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);

	pDialog->setDefaultFileType(IE_Imp::fileTypeForSuffix(".abw"));

	pDialog->runModal(m_pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const std::string resultPathname = pDialog->getPathname();
		if (!resultPathname.empty())
		{
			// update the entry box
			char *pFilename = UT_go_filename_from_uri(resultPathname.c_str());
			_win32Dialog.setControlText( AP_RID_DIALOG_NEW_EBX_EXISTING, 
			                             pFilename);
			FREEP(pFilename);
			setFileName (resultPathname.c_str());
		}
	}
}

void AP_Win32Dialog_New::_updateControls()
{
	switch( getOpenType() )
	{
	case AP_Dialog_New::open_New:
		_win32Dialog.enableControl( AP_RID_DIALOG_NEW_EBX_EXISTING, false );
		_win32Dialog.enableControl( AP_RID_DIALOG_NEW_BTN_EXISTING, false );
		_win32Dialog.enableControl( AP_RID_DIALOG_NEW_LBX_TEMPLATE, false );
		_win32Dialog.checkButton( AP_RID_DIALOG_NEW_RDO_EXISTING, false );
		_win32Dialog.checkButton( AP_RID_DIALOG_NEW_RDO_TEMPLATE, false );
		break;
	case AP_Dialog_New::open_Template:
		_win32Dialog.enableControl( AP_RID_DIALOG_NEW_EBX_EXISTING, false );
		_win32Dialog.enableControl( AP_RID_DIALOG_NEW_BTN_EXISTING, false );
		_win32Dialog.enableControl( AP_RID_DIALOG_NEW_LBX_TEMPLATE, true );
		_win32Dialog.checkButton( AP_RID_DIALOG_NEW_RDO_EXISTING, false );
		_win32Dialog.checkButton( AP_RID_DIALOG_NEW_RDO_TEMPLATE, true );
		break;
	case AP_Dialog_New::open_Existing:
		_win32Dialog.enableControl( AP_RID_DIALOG_NEW_EBX_EXISTING, true );
		_win32Dialog.enableControl( AP_RID_DIALOG_NEW_BTN_EXISTING, true );
		_win32Dialog.enableControl( AP_RID_DIALOG_NEW_LBX_TEMPLATE, false );
		_win32Dialog.checkButton( AP_RID_DIALOG_NEW_RDO_EXISTING, true );
		_win32Dialog.checkButton( AP_RID_DIALOG_NEW_RDO_TEMPLATE, false );
		break;
	}
}

void AP_Win32Dialog_New::_setFileName( UT_sint32 nIndex )
{
	HWND hControl = GetDlgItem(m_hThisDlg, AP_RID_DIALOG_NEW_LBX_TEMPLATE);
	if( nIndex != LB_ERR )
	{
		WCHAR buf[PATH_MAX];
		int l = SendMessageW(hControl, LB_GETTEXTLEN, nIndex, 0);
		UT_return_if_fail(l<PATH_MAX);

		_win32Dialog.getListText( AP_RID_DIALOG_NEW_LBX_TEMPLATE, nIndex, (char*)buf );
		UT_String templateName; 
		switch ( SendMessageW( hControl, LB_GETITEMDATA, nIndex, 0 ) )
		{
		case 0:
			templateName = XAP_App::getApp()->getUserPrivateDirectory();
			break;
		case 1:
			templateName = XAP_App::getApp()->getAbiSuiteLibDir();
			break;
		default:
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			break;
		}

		UT_Win32LocaleString str;
		UT_UTF8String u_str;
		str.fromLocale(buf);
		u_str=str.utf8_str();

		templateName += "\\templates\\";
		templateName += u_str.utf8_str();
		if(!strstr(u_str.utf8_str(), "normal.awt-")) // don't append awt to localized templates
			templateName += ".awt";

		char *uri = UT_go_filename_to_uri(templateName.c_str());
		UT_return_if_fail(uri);

		setFileName(uri);
		g_free(uri);
	}
}
