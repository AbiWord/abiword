/* AbiSource
 * 
 * Copyright (C) 2011 Urmas Dren <davian818@gmail.com>
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

#include <xap_Win32FrameImpl.h>
#include <ut_Win32LocaleString.h>
#include "ap_Win32Dialog_EpubExportOptions.h"
#include "ap_Win32Res_EpubExportOptions.rc2"

HINSTANCE AP_Win32Dialog_EpubExportOptions::m_hModule = NULL;

pt2Constructor ap_Dialog_EpubExportOptions_Constructor =
	AP_Win32Dialog_EpubExportOptions::static_constructor;

XAP_Dialog * AP_Win32Dialog_EpubExportOptions::static_constructor(
	XAP_DialogFactory* pDF, XAP_Dialog_Id id)
{
	return new AP_Win32Dialog_EpubExportOptions(pDF,id);
}

AP_Win32Dialog_EpubExportOptions::AP_Win32Dialog_EpubExportOptions(
    XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
    : AP_Dialog_EpubExportOptions(pDlgFactory,id)
{
}


AP_Win32Dialog_EpubExportOptions::~AP_Win32Dialog_EpubExportOptions()
{
}

void AP_Win32Dialog_EpubExportOptions::runModal (XAP_Frame * pFrame)
{
	if (pFrame == NULL) return;

	HWND hParent = ((XAP_Win32FrameImpl*)pFrame->getFrameImpl())->getTopLevelWindow();

	DialogBoxParamW(m_hModule,MAKEINTRESOURCEW(AP_RID_DIALOG_EPUB_EXP_OPTIONS),hParent,
		s_dlgProc,(LPARAM)this);
	
	m_bShouldSave = true;
}

INT_PTR CALLBACK AP_Win32Dialog_EpubExportOptions::s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
		{
			AP_Win32Dialog_EpubExportOptions* pThis = (AP_Win32Dialog_EpubExportOptions *) lParam;
			UT_return_val_if_fail(pThis, false);
			SetWindowLongPtrW(hWnd,DWLP_USER,lParam);
			return pThis->_onInitDialog(hWnd,wParam,lParam);
		}
		case WM_COMMAND:
		{
			AP_Win32Dialog_EpubExportOptions* pThis = (AP_Win32Dialog_EpubExportOptions *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			return pThis->_onCommand(hWnd,wParam,lParam);
		}
	}
	return FALSE;
}

INT_PTR AP_Win32Dialog_EpubExportOptions::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet ();
	UT_Win32LocaleString str;

	m_hDlg = hWnd;

	str.fromUTF8("EPUB Export Options");
	SetWindowTextW(hWnd,str.c_str());

	str.fromUTF8(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpSave));
	SetDlgItemTextW(hWnd,AP_RID_DIALOG_EPUBXOPT_SAVE,str.c_str());
	str.fromUTF8(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpRestore));
	SetDlgItemTextW(hWnd,AP_RID_DIALOG_EPUBXOPT_LOAD,str.c_str());

	str.fromUTF8("EPUB 2.0.1");
	SetDlgItemTextW(hWnd,AP_RID_DIALOG_EPUBXOPT_EPUB2,str.c_str());
	str.fromUTF8(pSS->getValue (XAP_STRING_ID_DLG_HTMLOPT_ExpSplitDocument));
	SetDlgItemTextW(hWnd,AP_RID_DIALOG_EPUBXOPT_SPLIT,str.c_str());
	str.fromUTF8("Use PNG instead of MathML");
	SetDlgItemTextW(hWnd,AP_RID_DIALOG_EPUBXOPT_PNGMATHML,str.c_str());


	setvalues();

	EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_EPUBXOPT_EPUB2),
		can_set_Epub2());
	EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_EPUBXOPT_PNGMATHML),
		can_set_RenderMathMlToPng());
	EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_EPUBXOPT_SPLIT),
		can_set_SplitDocument());
	return FALSE;
}

void AP_Win32Dialog_EpubExportOptions::setvalues()
{
	bool v;
	v = get_Epub2();
	CheckDlgButton(m_hDlg,AP_RID_DIALOG_EPUBXOPT_EPUB2,v?BST_CHECKED:BST_UNCHECKED);
	v = get_RenderMathMlToPng();
	CheckDlgButton(m_hDlg,AP_RID_DIALOG_EPUBXOPT_PNGMATHML,v?BST_CHECKED:BST_UNCHECKED);
	v = get_SplitDocument();
	CheckDlgButton(m_hDlg,AP_RID_DIALOG_EPUBXOPT_SPLIT,v?BST_CHECKED:BST_UNCHECKED);
}

void AP_Win32Dialog_EpubExportOptions::gathervalues()
{
	set_RenderMathMlToPng(IsDlgButtonChecked(m_hDlg,AP_RID_DIALOG_EPUBXOPT_PNGMATHML)==BST_CHECKED);
	set_SplitDocument(IsDlgButtonChecked(m_hDlg,AP_RID_DIALOG_EPUBXOPT_SPLIT)==BST_CHECKED);
	if (IsDlgButtonChecked(m_hDlg,AP_RID_DIALOG_EPUBXOPT_EPUB2)==BST_CHECKED)
	{
		/* mandatory for EPUB 2 */
		set_Epub2(true);
		set_RenderMathMlToPng(true);
	}
	else
	{
		set_Epub2(false);
	}
}

INT_PTR AP_Win32Dialog_EpubExportOptions::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (LOWORD(wParam)) {
		case IDOK:
			gathervalues();
			m_bShouldSave = true;
			EndDialog(hWnd,TRUE);
			break;
		case IDCANCEL:
			m_bShouldSave = false;
			EndDialog(hWnd,FALSE);
			break;
		case AP_RID_DIALOG_EPUBXOPT_LOAD:
			restoreDefaults();
			setvalues();
			break;
		case AP_RID_DIALOG_EPUBXOPT_SAVE:
			gathervalues();
			saveDefaults();
			break;
		case AP_RID_DIALOG_EPUBXOPT_EPUB2:
			{
				bool v = IsDlgButtonChecked(hWnd,AP_RID_DIALOG_EPUBXOPT_EPUB2)==BST_CHECKED;
				if (v) {
					CheckDlgButton(hWnd,AP_RID_DIALOG_EPUBXOPT_PNGMATHML,BST_CHECKED);
					EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_EPUBXOPT_PNGMATHML),
						FALSE);
				} else {
					EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_EPUBXOPT_PNGMATHML),
						TRUE);;
					CheckDlgButton(hWnd,AP_RID_DIALOG_EPUBXOPT_PNGMATHML,
						get_RenderMathMlToPng() ? BST_CHECKED:BST_UNCHECKED);
				}
			}
			break;
	}
	return TRUE;
}
