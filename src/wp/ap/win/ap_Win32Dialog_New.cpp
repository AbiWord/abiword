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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#include <windows.h>
#include <io.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_path.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_New.h"
#include "ap_Win32Dialog_New.h"

#include "xap_Dlg_FileOpenSaveAs.h"
#include "ie_imp.h"
#include "ie_types.h"
#include "ut_string_class.h"
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
	: AP_Dialog_New(pDlgFactory,id), /*_win32Dialog(this),*/ m_hThisDlg(NULL), m_pFrame(NULL)
{
}

AP_Win32Dialog_New::~AP_Win32Dialog_New(void)
{
}

void AP_Win32Dialog_New::runModal(XAP_Frame * pFrame)
{
	// raise the dialog
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == AP_DIALOG_ID_FILE_NEW);
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_NEW));
}

BOOL AP_Win32Dialog_New::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hThisDlg = hWnd;

	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_ASSERT(app);

	localizeDialogTitle(AP_STRING_ID_DLG_NEW_Title);

	// localize controls
	localizeControlText(AP_RID_DIALOG_NEW_BTN_OK,			XAP_STRING_ID_DLG_OK);
	localizeControlText(AP_RID_DIALOG_NEW_BTN_CANCEL,		XAP_STRING_ID_DLG_Cancel);
	localizeControlText(AP_RID_DIALOG_NEW_RDO_BLANK,		AP_STRING_ID_DLG_NEW_StartEmpty);
	localizeControlText(AP_RID_DIALOG_NEW_RDO_TEMPLATE,		AP_STRING_ID_DLG_NEW_Create);
	localizeControlText(AP_RID_DIALOG_NEW_RDO_EXISTING,		AP_STRING_ID_DLG_NEW_Open);
	localizeControlText(AP_RID_DIALOG_NEW_BTN_EXISTING,		AP_STRING_ID_DLG_NEW_Choose);

	// set initial state
	localizeControlText(AP_RID_DIALOG_NEW_EBX_EXISTING, AP_STRING_ID_DLG_NEW_NoFile);

	HWND hControl = GetDlgItem(hWnd, AP_RID_DIALOG_NEW_LBX_TEMPLATE);

	long findtag;
	struct _finddata_t cfile;
	UT_String templateName, searchDir;
	templateName = XAP_App::getApp()->getUserPrivateDirectory(); 
	searchDir = XAP_App::getApp()->getUserPrivateDirectory();
	searchDir += "\\templates\\*.awt";
	findtag = _findfirst( searchDir.c_str(), &cfile );
	if( findtag != -1 )
	{
		do
		{	
			templateName = XAP_App::getApp()->getUserPrivateDirectory();
			templateName += "\\templates\\";
			templateName += cfile.name;
			templateName = templateName.substr ( 0, templateName.size () - 4 ) ;
			UT_sint32 nIndex = SendMessage( hControl, LB_ADDSTRING, 0, (LPARAM) XAP_Win32App::getWideString(UT_basename( templateName.c_str()) ) );
			SendMessage( hControl, LB_SETITEMDATA, (WPARAM) nIndex, (LPARAM) 0 );
		} while( _findnext( findtag, &cfile ) == 0 );
	}
	_findclose( findtag );

	templateName = XAP_App::getApp()->getAbiSuiteLibDir(); 
	searchDir = XAP_App::getApp()->getAbiSuiteLibDir();
	searchDir += "\\templates\\*.awt";
	findtag = _findfirst( searchDir.c_str(), &cfile );
	if( findtag != -1 )
	{
		do
		{	
			templateName = XAP_App::getApp()->getAbiSuiteLibDir();
			templateName += "\\templates\\";
			templateName += cfile.name;
			templateName = templateName.substr ( 0, templateName.size () - 4 ) ;
			UT_sint32 nIndex = SendMessage( hControl, LB_ADDSTRING, 0, (LPARAM) XAP_Win32App::getWideString(UT_basename( templateName.c_str() )) );
			SendMessage( hControl, LB_SETITEMDATA, (WPARAM) nIndex, (LPARAM) 1 );
		} while( _findnext( findtag, &cfile ) == 0 );
	}
	_findclose( findtag );

	centerDialog();
	_updateControls();
	return 1;	// 1 == we did not call SetFocus()
}


BOOL AP_Win32Dialog_New::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

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
			UT_sint32 nIndex = getListSelectedIndex(wId);
			_setFileName( nIndex );
			return 1;
		}
		return 0;

	case AP_RID_DIALOG_NEW_BTN_EXISTING:
		_doChoose();
		return 1;

	case AP_RID_DIALOG_NEW_RDO_BLANK:
		setOpenType(AP_Dialog_New::open_New);
		_updateControls();
		return 1;

	case AP_RID_DIALOG_NEW_RDO_TEMPLATE:
		setOpenType(AP_Dialog_New::open_Template);
		{
			int nIndex = getListSelectedIndex(AP_RID_DIALOG_NEW_LBX_TEMPLATE);
			if( nIndex == LB_ERR )
			{
				HWND hControl = GetDlgItem(hWnd, AP_RID_DIALOG_NEW_LBX_TEMPLATE);
				nIndex = SendMessage( hControl, LB_FINDSTRING , (WPARAM) -1, (LPARAM) L"Normal" );
				selectListItem(AP_RID_DIALOG_NEW_LBX_TEMPLATE, nIndex);
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

void AP_Win32Dialog_New::_doChoose()
{

	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) m_pFrame->getDialogFactory();

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname(0);
	pDialog->setSuggestFilename(false);

	UT_uint32 filterCount = IE_Imp::getImporterCount();
	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1,
												   sizeof(IEFileType));
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
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
		{
			// update the entry box
			setControlText( AP_RID_DIALOG_NEW_EBX_EXISTING, 
			                             szResultPathname);
			setFileName (szResultPathname);
		}
	}
}

void AP_Win32Dialog_New::_updateControls()
{
	switch( getOpenType() )
	{
	case AP_Dialog_New::open_New:
		enableControl( AP_RID_DIALOG_NEW_EBX_EXISTING, false );
		enableControl( AP_RID_DIALOG_NEW_BTN_EXISTING, false );
		enableControl( AP_RID_DIALOG_NEW_LBX_TEMPLATE, false );
		checkButton( AP_RID_DIALOG_NEW_RDO_EXISTING, false );
		checkButton( AP_RID_DIALOG_NEW_RDO_TEMPLATE, false );
		checkButton( AP_RID_DIALOG_NEW_RDO_BLANK, true );
		break;
	case AP_Dialog_New::open_Template:
		enableControl( AP_RID_DIALOG_NEW_EBX_EXISTING, false );
		enableControl( AP_RID_DIALOG_NEW_BTN_EXISTING, false );
		enableControl( AP_RID_DIALOG_NEW_LBX_TEMPLATE, true );
		checkButton( AP_RID_DIALOG_NEW_RDO_EXISTING, false );
		checkButton( AP_RID_DIALOG_NEW_RDO_TEMPLATE, true );
		checkButton( AP_RID_DIALOG_NEW_RDO_BLANK, false );
		break;
	case AP_Dialog_New::open_Existing:
		enableControl( AP_RID_DIALOG_NEW_EBX_EXISTING, true );
		enableControl( AP_RID_DIALOG_NEW_BTN_EXISTING, true );
		enableControl( AP_RID_DIALOG_NEW_LBX_TEMPLATE, false );
		checkButton( AP_RID_DIALOG_NEW_RDO_EXISTING, true );
		checkButton( AP_RID_DIALOG_NEW_RDO_TEMPLATE, false );
		checkButton( AP_RID_DIALOG_NEW_RDO_BLANK, false );
		break;
	}
}

void AP_Win32Dialog_New::_setFileName( UT_sint32 nIndex )
{
	HWND hControl = GetDlgItem(m_hThisDlg, AP_RID_DIALOG_NEW_LBX_TEMPLATE);
	if( nIndex != LB_ERR )
	{
		char buf[_MAX_PATH];
		getListText( AP_RID_DIALOG_NEW_LBX_TEMPLATE, nIndex, buf, _MAX_PATH );
		UT_String templateName; 
		switch ( SendMessage( hControl, LB_GETITEMDATA, nIndex, 0 ) )
		{
		case 0:
			templateName = XAP_App::getApp()->getUserPrivateDirectory();
			break;
		case 1:
			templateName = XAP_App::getApp()->getAbiSuiteLibDir();
			break;
		default:
			UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
			break;
		}
		templateName += "\\templates\\";
		templateName += buf;
		templateName += ".awt";
		setFileName(templateName.c_str());
	}
}
