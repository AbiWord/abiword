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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
      m_hThisDlg(NULL)
{

}

AP_Win32Dialog_HdrFtr::~AP_Win32Dialog_HdrFtr(void)
{
}

void AP_Win32Dialog_HdrFtr::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == AP_DIALOG_ID_HDRFTR);
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_HDRFTR));
}

BOOL AP_Win32Dialog_HdrFtr::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_ASSERT(app);

	m_hThisDlg = hWnd;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// localize dialog title
	localizeDialogTitle(AP_STRING_ID_DLG_HdrFtr_Title);

	// localize controls
	localizeControlText(AP_RID_DIALOG_HDRFTR_BTN_OK, 	XAP_STRING_ID_DLG_OK);
	localizeControlText(AP_RID_DIALOG_HDRFTR_BTN_CANCEL, 	XAP_STRING_ID_DLG_Cancel);
	
	localizeControlText(AP_RID_DIALOG_HDRFTR_GBX_HDR, 	AP_STRING_ID_DLG_HdrFtr_HeaderFrame);
	localizeControlText(AP_RID_DIALOG_HDRFTR_CHK_HDRFACING, AP_STRING_ID_DLG_HdrFtr_HeaderEven);
	localizeControlText(AP_RID_DIALOG_HDRFTR_CHK_HDRFIRST, 	AP_STRING_ID_DLG_HdrFtr_HeaderFirst);
	localizeControlText(AP_RID_DIALOG_HDRFTR_CHK_HDRLAST, 	AP_STRING_ID_DLG_HdrFtr_HeaderLast);
	localizeControlText(AP_RID_DIALOG_HDRFTR_GBX_FTR, 	AP_STRING_ID_DLG_HdrFtr_FooterFrame);
	localizeControlText(AP_RID_DIALOG_HDRFTR_CHK_FTRFACING, AP_STRING_ID_DLG_HdrFtr_FooterEven);
	localizeControlText(AP_RID_DIALOG_HDRFTR_CHK_FTRFIRST, 	AP_STRING_ID_DLG_HdrFtr_FooterFirst);
	localizeControlText(AP_RID_DIALOG_HDRFTR_CHK_FTRLAST, 	AP_STRING_ID_DLG_HdrFtr_FooterLast);
	localizeControlText(AP_RID_DIALOG_HDRFTR_CHK_SECTION, 	AP_STRING_ID_DLG_HdrFtr_RestartCheck);
	localizeControlText(AP_RID_DIALOG_HDRFTR_LBL_SECTION, 	AP_STRING_ID_DLG_HdrFtr_RestartNumbers);

	// set initial state
	checkButton(AP_RID_DIALOG_HDRFTR_CHK_HDRFACING, getValue(HdrEven));
	checkButton(AP_RID_DIALOG_HDRFTR_CHK_HDRFIRST,  getValue(HdrFirst));
	checkButton(AP_RID_DIALOG_HDRFTR_CHK_HDRLAST,   getValue(HdrLast));
	checkButton(AP_RID_DIALOG_HDRFTR_CHK_FTRFACING, getValue(FtrEven));
	checkButton(AP_RID_DIALOG_HDRFTR_CHK_FTRFIRST,  getValue(FtrFirst));
	checkButton(AP_RID_DIALOG_HDRFTR_CHK_FTRLAST,   getValue(FtrLast));
	setControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION, getRestartValue());

	bool bRestart = isRestart();
	checkButton(AP_RID_DIALOG_HDRFTR_CHK_SECTION, bRestart);
	enableControl(AP_RID_DIALOG_HDRFTR_LBL_SECTION, bRestart);
	enableControl(AP_RID_DIALOG_HDRFTR_EBX_SECTION, bRestart);
	enableControl(AP_RID_DIALOG_HDRFTR_SPN_SECTION, bRestart);
	
	centerDialog();			
	return 1;
}

BOOL AP_Win32Dialog_HdrFtr::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_HDRFTR_BTN_CANCEL:
		setAnswer( a_CANCEL );
		EndDialog(hWnd,0);
		return 1;

	case AP_RID_DIALOG_HDRFTR_BTN_OK:
		{
			bool bHdrEven  = isChecked(AP_RID_DIALOG_HDRFTR_CHK_HDRFACING) != 0;
			bool bHdrFirst = isChecked(AP_RID_DIALOG_HDRFTR_CHK_HDRFIRST)  != 0;
			bool bHdrLast  = isChecked(AP_RID_DIALOG_HDRFTR_CHK_HDRLAST)   != 0;
			bool bFtrEven  = isChecked(AP_RID_DIALOG_HDRFTR_CHK_FTRFACING) != 0;
			bool bFtrFirst = isChecked(AP_RID_DIALOG_HDRFTR_CHK_FTRFIRST)  != 0;
			bool bFtrLast  = isChecked(AP_RID_DIALOG_HDRFTR_CHK_FTRLAST)   != 0;
 			bool bRestart  = isChecked(AP_RID_DIALOG_HDRFTR_CHK_SECTION)   != 0;
			UT_sint32 val  = getControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION);

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
		enableControl(AP_RID_DIALOG_HDRFTR_LBL_SECTION, isChecked(wId)!=0);
		enableControl(AP_RID_DIALOG_HDRFTR_EBX_SECTION, isChecked(wId)!=0);
		enableControl(AP_RID_DIALOG_HDRFTR_SPN_SECTION, isChecked(wId)!=0);	
		return 1;

	case AP_RID_DIALOG_HDRFTR_EBX_SECTION:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			UT_sint32 value = getControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION);
			setControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION, value );
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
			UT_sint32 value = getControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION);
			value -= pnmud->iDelta;
			if( value < 0 ) value = 0;
			setControlInt(AP_RID_DIALOG_HDRFTR_EBX_SECTION, value );
			return 1;
		}
	default:
		return 0;
	}
}

