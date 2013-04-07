/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifdef _MSC_VER	// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_HdrFtr.h"
#include "ap_Win32Dialog_HdrFtr.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_HdrFtr::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_HdrFtr * p = new AP_Win32Dialog_HdrFtr(pFactory,id);
	return p;
}

AP_Win32Dialog_HdrFtr::AP_Win32Dialog_HdrFtr(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_HdrFtr(pDlgFactory,id), 
      m_hThisDlg(NULL),
      _win32Dialog(this)
{

}

AP_Win32Dialog_HdrFtr::~AP_Win32Dialog_HdrFtr(void)
{
}

void AP_Win32Dialog_HdrFtr::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail (pFrame);
	_win32Dialog.runModal( pFrame,
						   AP_DIALOG_ID_HDRFTR,
                           AP_RID_DIALOG_HDRFTR,
	         		       this );
}

#define _DS(c,s)	setDlgItemText(hWnd, AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(hWnd, AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_HdrFtr::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_return_val_if_fail (app,0);

	m_hThisDlg = hWnd;
	setHandle(hWnd);
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// localize dialog title
	setDialogTitle( pSS->getValue(AP_STRING_ID_DLG_HdrFtr_Title) );

	// localize controls
	_DSX(HDRFTR_BTN_OK,				DLG_OK);
	_DSX(HDRFTR_BTN_CANCEL,			DLG_Cancel);

	_DS(HDRFTR_GBX_HDR,				DLG_HdrFtr_HeaderFrame);
	_DS(HDRFTR_CHK_HDRFACING,		DLG_HdrFtr_HeaderEven);
	_DS(HDRFTR_CHK_HDRFIRST,		DLG_HdrFtr_HeaderFirst);
	_DS(HDRFTR_CHK_HDRLAST,			DLG_HdrFtr_HeaderLast);
	_DS(HDRFTR_GBX_FTR,				DLG_HdrFtr_FooterFrame);
	_DS(HDRFTR_CHK_FTRFACING,		DLG_HdrFtr_FooterEven);
	_DS(HDRFTR_CHK_FTRFIRST,		DLG_HdrFtr_FooterFirst);
	_DS(HDRFTR_CHK_FTRLAST,			DLG_HdrFtr_FooterLast);
	_DS(HDRFTR_CHK_SECTION,			DLG_HdrFtr_RestartCheck);
	_DS(HDRFTR_LBL_SECTION,			DLG_HdrFtr_RestartNumbers);

	// set initial state
	_win32Dialog.checkButton(AP_RID_DIALOG_HDRFTR_CHK_HDRFACING, getValue(HdrEven));
	_win32Dialog.checkButton(AP_RID_DIALOG_HDRFTR_CHK_HDRFIRST,  getValue(HdrFirst));
	_win32Dialog.checkButton(AP_RID_DIALOG_HDRFTR_CHK_HDRLAST,   getValue(HdrLast));
	_win32Dialog.checkButton(AP_RID_DIALOG_HDRFTR_CHK_FTRFACING, getValue(FtrEven));
	_win32Dialog.checkButton(AP_RID_DIALOG_HDRFTR_CHK_FTRFIRST,  getValue(FtrFirst));
	_win32Dialog.checkButton(AP_RID_DIALOG_HDRFTR_CHK_FTRLAST,   getValue(FtrLast));
	_win32Dialog.setControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION, getRestartValue());

	bool bRestart = isRestart();
	_win32Dialog.checkButton(AP_RID_DIALOG_HDRFTR_CHK_SECTION, bRestart);
	_win32Dialog.enableControl(AP_RID_DIALOG_HDRFTR_LBL_SECTION, bRestart);
	_win32Dialog.enableControl(AP_RID_DIALOG_HDRFTR_EBX_SECTION, bRestart);
	_win32Dialog.enableControl(AP_RID_DIALOG_HDRFTR_SPN_SECTION, bRestart);
	
    centerDialog();
	return 1;
}

BOOL AP_Win32Dialog_HdrFtr::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
	case AP_RID_DIALOG_HDRFTR_BTN_CANCEL:
		setAnswer( a_CANCEL );
		EndDialog(hWnd,0);
		return 1;

	case AP_RID_DIALOG_HDRFTR_BTN_OK:
		{
			bool bHdrEven  = _win32Dialog.isChecked(AP_RID_DIALOG_HDRFTR_CHK_HDRFACING) != 0;
			bool bHdrFirst = _win32Dialog.isChecked(AP_RID_DIALOG_HDRFTR_CHK_HDRFIRST)  != 0;
			bool bHdrLast  = _win32Dialog.isChecked(AP_RID_DIALOG_HDRFTR_CHK_HDRLAST)   != 0;
			bool bFtrEven  = _win32Dialog.isChecked(AP_RID_DIALOG_HDRFTR_CHK_FTRFACING) != 0;
			bool bFtrFirst = _win32Dialog.isChecked(AP_RID_DIALOG_HDRFTR_CHK_FTRFIRST)  != 0;
			bool bFtrLast  = _win32Dialog.isChecked(AP_RID_DIALOG_HDRFTR_CHK_FTRLAST)   != 0;
 			bool bRestart  = _win32Dialog.isChecked(AP_RID_DIALOG_HDRFTR_CHK_SECTION)   != 0;
			UT_sint32 val  = _win32Dialog.getControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION);

			setValue( HdrEven,  bHdrEven,  bHdrEven  != getValue(HdrEven)  );
			setValue( HdrFirst, bHdrFirst, bHdrFirst != getValue(HdrFirst) );
			setValue( HdrLast,  bHdrLast,  bHdrLast  != getValue(HdrLast)  );
			setValue( FtrEven,  bFtrEven,  bFtrEven  != getValue(FtrEven)  );
			setValue( FtrFirst, bFtrFirst, bFtrFirst != getValue(FtrFirst) );
			setValue( FtrLast,  bFtrLast,  bFtrLast  != getValue(FtrLast)  );
			setRestart( bRestart, val, (bRestart != isRestart() || val != getRestartValue()) );
		}
		setAnswer( a_OK );
		EndDialog(hWnd,0);
		return 1;

	case AP_RID_DIALOG_HDRFTR_CHK_SECTION:
		_win32Dialog.enableControl(AP_RID_DIALOG_HDRFTR_LBL_SECTION, _win32Dialog.isChecked(wId)!=0);
		_win32Dialog.enableControl(AP_RID_DIALOG_HDRFTR_EBX_SECTION, _win32Dialog.isChecked(wId)!=0);
		_win32Dialog.enableControl(AP_RID_DIALOG_HDRFTR_SPN_SECTION, _win32Dialog.isChecked(wId)!=0);	
		return 1;

	case AP_RID_DIALOG_HDRFTR_EBX_SECTION:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			UT_sint32 value = _win32Dialog.getControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION);
			_win32Dialog.setControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION, value );
		}
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_HdrFtr::_onDeltaPos(NM_UPDOWN * pnmud)
{
	switch( pnmud->hdr.idFrom )
	{
	case AP_RID_DIALOG_HDRFTR_SPN_SECTION:
		{
			UT_sint32 value = _win32Dialog.getControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION);
			value -= pnmud->iDelta;
			if( value < 0 ) value = 0;
			_win32Dialog.setControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION, value );
			return 1;
		}
	default:
		return 0;
	}
}

