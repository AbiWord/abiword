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

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32DialogHelper.h"
#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32Dlg_HTMLOptions.h"
#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_HTMLOptions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_HTMLOptions * p = new XAP_Win32Dialog_HTMLOptions(pFactory,id);
	return p;
}

XAP_Win32Dialog_HTMLOptions::XAP_Win32Dialog_HTMLOptions(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_HTMLOptions(pDlgFactory,id)
{
}

XAP_Win32Dialog_HTMLOptions::~XAP_Win32Dialog_HTMLOptions(void)
{
}

void XAP_Win32Dialog_HTMLOptions::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == XAP_DIALOG_ID_HTMLOPTIONS);

	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_HTMLOPTIONS));
}

#define _DS(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_HTMLOptions::_onInitDialog(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	localizeDialogTitle(XAP_STRING_ID_DLG_HTMLOPT_ExpTitle);

	// localize controls
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_LBL,			XAP_STRING_ID_DLG_HTMLOPT_ExpLabel);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_CHK_HTML4,	XAP_STRING_ID_DLG_HTMLOPT_ExpIs4);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_CHK_PHP,		XAP_STRING_ID_DLG_HTMLOPT_ExpAbiWebDoc);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_CHK_XML,		XAP_STRING_ID_DLG_HTMLOPT_ExpDeclareXML);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_CHK_AWML,	XAP_STRING_ID_DLG_HTMLOPT_ExpAllowAWML);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_CHK_EMBEDCSS,XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedCSS);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_CHK_URLIMAGE,XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedImages);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_CHK_PNGMATHML,XAP_STRING_ID_DLG_HTMLOPT_ExpMathMLRenderPNG);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_CHK_SPLITFILE,XAP_STRING_ID_DLG_HTMLOPT_ExpSplitDocument);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_BTN_SAVE,	XAP_STRING_ID_DLG_HTMLOPT_ExpSave);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_BTN_RESTORE,	XAP_STRING_ID_DLG_HTMLOPT_ExpRestore);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_BTN_OK,		XAP_STRING_ID_DLG_OK);
	localizeControlText(XAP_RID_DIALOG_HTMLOPTIONS_BTN_CANCEL,	XAP_STRING_ID_DLG_Cancel);

	// Set Initial conditions
	refreshStates();
	centerDialog();
	
	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_HTMLOptions::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_HTML4:
		set_HTML4( (isChecked(wId)!=0) ? true : false );
		return 1;
		
	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_PHP:
		set_PHTML( (isChecked(wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_XML:
		set_Declare_XML( (isChecked(wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_AWML:
		set_Allow_AWML( (isChecked(wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_EMBEDCSS:
		set_Embed_CSS( (isChecked(wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_URLIMAGE:
		set_Embed_Images( (isChecked(wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_PNGMATHML:
		set_MathML_Render_PNG( (isChecked(wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_SPLITFILE:
		if (can_set_Split_Document())
			set_Split_Document( (isChecked(wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_BTN_SAVE:
		saveDefaults();
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_BTN_RESTORE:
		restoreDefaults();
		refreshStates();
		return 1;
					
	case IDCANCEL:						// also XAP_RID_DIALOG_HTMLOPTIONS_BTN_CANCEL
		m_bShouldSave = false;
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also XAP_RID_DIALOG_HTMLOPTIONS_BTN_OK
		m_bShouldSave = true;
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL XAP_Win32Dialog_HTMLOptions::_onDeltaPos(NM_UPDOWN * /*pnmud*/)
{
	return FALSE;
}

void XAP_Win32Dialog_HTMLOptions::refreshStates()
{
	// determine which controls are available.
	enableControl(XAP_RID_DIALOG_HTMLOPTIONS_CHK_XML,can_set_Declare_XML());
	enableControl(XAP_RID_DIALOG_HTMLOPTIONS_CHK_AWML,can_set_Allow_AWML());
	enableControl(XAP_RID_DIALOG_HTMLOPTIONS_CHK_EMBEDCSS,can_set_Embed_CSS());
	enableControl(XAP_RID_DIALOG_HTMLOPTIONS_CHK_URLIMAGE,can_set_Embed_Images());
	enableControl(XAP_RID_DIALOG_HTMLOPTIONS_CHK_SPLITFILE,can_set_Split_Document());

	// set initial state
	checkButton(XAP_RID_DIALOG_HTMLOPTIONS_CHK_HTML4,get_HTML4());
	checkButton(XAP_RID_DIALOG_HTMLOPTIONS_CHK_PHP,get_PHTML()); 
	checkButton(XAP_RID_DIALOG_HTMLOPTIONS_CHK_XML,get_Declare_XML());
	checkButton(XAP_RID_DIALOG_HTMLOPTIONS_CHK_AWML,get_Allow_AWML());
	checkButton(XAP_RID_DIALOG_HTMLOPTIONS_CHK_EMBEDCSS,get_Embed_CSS());
	checkButton(XAP_RID_DIALOG_HTMLOPTIONS_CHK_URLIMAGE,get_Embed_Images());
	checkButton(XAP_RID_DIALOG_HTMLOPTIONS_CHK_PNGMATHML,get_MathML_Render_PNG());
	checkButton(XAP_RID_DIALOG_HTMLOPTIONS_CHK_SPLITFILE,get_Split_Document());
}
