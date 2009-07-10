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

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatFootnotes.h"
#include "ap_Win32Dialog_FormatFootnotes.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32LabelledSeparator.h"

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_FORMATFOOTNOTES_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_FORMATFOOTNOTES_##c,pSS->getValue(XAP_STRING_ID_##s))

#define GWL(hwnd)		(AP_Win32Dialog_FormatFootnotes*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_FormatFootnotes*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_FormatFootnotes::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_FormatFootnotes * p = new AP_Win32Dialog_FormatFootnotes(pFactory,id);
	return p;
}

AP_Win32Dialog_FormatFootnotes::AP_Win32Dialog_FormatFootnotes(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_FormatFootnotes(pDlgFactory,id)
{
}

AP_Win32Dialog_FormatFootnotes::~AP_Win32Dialog_FormatFootnotes(void)
{
}

void AP_Win32Dialog_FormatFootnotes::runModal(XAP_Frame * pFrame)
{	
	UT_return_if_fail (pFrame);	
	setFrame(pFrame);
	
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);

	LPCTSTR lpTemplate = NULL;
	
	UT_return_if_fail (m_id == AP_DIALOG_ID_FORMAT_FOOTNOTES);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_FORMATFOOTNOTES);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
						static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
						(DLGPROC)s_dlgProc,(LPARAM)this);
						
	UT_ASSERT_HARMLESS((result != -1));
}

BOOL CALLBACK AP_Win32Dialog_FormatFootnotes::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
		
	AP_Win32Dialog_FormatFootnotes * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_FormatFootnotes *)lParam;
		SWL(hWnd,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommand(hWnd,wParam,lParam);
	default:
		return 0;
	}
}

// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_FormatFootnotes::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{	
	const XAP_StringSet * pSS = m_pApp->getStringSet();	
	m_hwndDlg = hWnd;
	char szText[16];	
				
	/* localize controls  */
	_DSX(BTN_OK,			DLG_OK);
	_DSX(BTN_CANCEL,		DLG_Cancel);				
	_DS(RADIO_RSEL, 		DLG_FormatFootnotes_FootRestartSec);	
	_DS(RADIO_PAGE,			DLG_FormatFootnotes_FootRestartPage);
	_DS(RADIO_DONOT,		DLG_FormatFootnotes_FootRestartNone);
	_DS(STATIC_INITFOOTVAL,	DLG_FormatFootnotes_FootInitialVal);
	_DS(STATIC_FSTYLES1,	DLG_FormatFootnotes_FootStyle);
	_DS(STATIC_FSTYLES2,	DLG_FormatFootnotes_FootStyle);
	_DS(RADIO_ENDDOC,		DLG_FormatFootnotes_EndPlaceEndDoc);	
	_DS(RADIO_ENDSEC,		DLG_FormatFootnotes_EndPlaceEndSec);	
	_DS(STATIC_INITENDVAL,	DLG_FormatFootnotes_EndInitialVal);
	_DS(STATIC_ESTYLES1,	DLG_FormatFootnotes_EndStyle);
	_DS(STATIC_ESTYLES2, 	DLG_FormatFootnotes_EndStyle);
	_DS(RADIO_ERSTSEC, 		DLG_FormatFootnotes_EndRestartSec);
	_DS(STATIC_PLACEMENT, 	DLG_FormatFootnotes_EndPlacement);
	_DS(STATIC_NUMBERING, 	DLG_FormatFootnotes_FootnoteRestart);

	
	/*Caption*/
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_FormatFootnotes_Title));
		
	setInitialValues(); /* Parent class loads data*/
	
	/* Footnote style combobox*/
	int nItem;
	int nDefF = 0;
	int nDefE = 0;
	const FootnoteTypeDesc * footnoteTypeList = getFootnoteTypeLabelList();

	for(UT_uint32 i = 0; footnoteTypeList->n !=  _FOOTNOTE_TYPE_INVALID; footnoteTypeList++, i++)
	{
		 nItem = SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE, CB_ADDSTRING, 0,
									(LPARAM)footnoteTypeList->label);
		 SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE, CB_SETITEMDATA, nItem, (LPARAM)i);

		 if (i==(UT_uint32)getFootnoteType())
		  	nDefF = i;			

		 nItem = SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE, CB_ADDSTRING, 0,
									(LPARAM)footnoteTypeList->label);
		 SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE, CB_SETITEMDATA, nItem, (LPARAM)i);
		 
		 if (i==(UT_uint32)getEndnoteType())
		  	nDefE = i;			
	}

 	SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE, CB_SETCURSEL, nDefF, 0);
 	SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE, CB_SETCURSEL, nDefE, 0);
	 	 	
 	
	/*Set Default Radio buttons Footnotes */                                                                                                      
	if (getRestartFootnoteOnSection() || getRestartFootnoteOnPage())
		CheckRadioButton(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_RSEL, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_DONOT,
			getRestartFootnoteOnSection() ? AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_RSEL : AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_PAGE);                                                                              
	else
		CheckRadioButton(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_RSEL, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_DONOT,
			AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_DONOT);                                                                              


	/*Set Default Radio buttons Endnotes */                                                                                                      		
	CheckRadioButton(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDDOC, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDSEC,
		 getPlaceAtDocEnd() ? AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDDOC: AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDSEC);                                                                              		
				
	/* Set Footnotes Spin*/ 
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_FORMATFOOTNOTES_SPIN_FSTYLE),UDM_SETRANGE,(WPARAM)1,(WPARAM)9999);	
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_FORMATFOOTNOTES_TEXT_FSTYLE),EM_LIMITTEXT,(WPARAM)4,(WPARAM)0);	
	sprintf (szText, "%u", getFootnoteVal());	
	SetDlgItemText(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_TEXT_FSTYLE, szText);
	
	/* Set Endnotes Spin*/ 
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_FORMATFOOTNOTES_SPIN_ESTYLE),UDM_SETRANGE,(WPARAM)1,(WPARAM)9999);	
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_FORMATFOOTNOTES_TEXT_ESTYLE),EM_LIMITTEXT,(WPARAM)4,(WPARAM)0);		
	sprintf (szText, "%u", getEndnoteVal());	
	SetDlgItemText(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_TEXT_ESTYLE, szText);
	
	
	CheckDlgButton(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ERSTSEC, getRestartEndnoteOnSection());
	      
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
	
	return 0; // 0 because we called SetFocus

}

BOOL AP_Win32Dialog_FormatFootnotes::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case AP_RID_DIALOG_FORMATFOOTNOTES_BTN_CANCEL:									
			setAnswer(AP_Dialog_FormatFootnotes::a_CANCEL);
			EndDialog(hWnd,0);
			return 1;
			
		case AP_RID_DIALOG_FORMATFOOTNOTES_BTN_OK:				
		{
			char szValue[16];
			
			if (GetDlgItemText(m_hwndDlg, AP_RID_DIALOG_FORMATFOOTNOTES_TEXT_FSTYLE, szValue, 16))	
				setFootnoteVal(atoi(szValue));
				
			if (GetDlgItemText(m_hwndDlg, AP_RID_DIALOG_FORMATFOOTNOTES_TEXT_ESTYLE, szValue, 16))	
				setEndnoteVal(atoi(szValue));
		
			setAnswer(AP_Dialog_FormatFootnotes::a_OK);
			EndDialog(hWnd,0);
			return 1;
		}
		/*Footnote*/	
		case AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_RSEL:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_RSEL))
			{
				setRestartFootnoteOnPage(false);
				setRestartFootnoteOnSection(true);
			}
			break;
		}
		
		case AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_PAGE:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_PAGE))
			{
				setRestartFootnoteOnPage(true);
				setRestartFootnoteOnSection(false);
			}
			break;	
		}

		case AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_DONOT:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_DONOT))
			{
				setRestartFootnoteOnPage(false);
				setRestartFootnoteOnSection(false);
			}
			break;	
		}
		
		/*Endnote*/	
		case AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDDOC:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDDOC))
			{
				setPlaceAtSecEnd(false);
				setPlaceAtDocEnd(true);
			}
			break;
		}
		
		case AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDSEC:
		{
			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDSEC))
			{
				setPlaceAtSecEnd(true);
				setPlaceAtDocEnd(false);
			}
			break;	
		}
		
		
		/*Checkbox*/
		case AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ERSTSEC:
		{
			bool bChecked;
			
			bChecked = (IsDlgButtonChecked( hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ERSTSEC) == BST_CHECKED);
			setRestartEndnoteOnSection(bChecked);
			break;	
		}
	
			
		case AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE:  
		{
			if (wNotifyCode == CBN_SELCHANGE)                       
			{
				int nItem = SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE, CB_GETCURSEL, 0, 0);				
				
				if (nItem!=CB_ERR)
				{
					UT_uint32 n = (UT_uint32)SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE,
																CB_GETITEMDATA,   (WPARAM)nItem, 0);										
					
					setFootnoteType((FootnoteType)n);					
				}
			}
			
			break;
		}
		
		case AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE:  
		{
			if (wNotifyCode == CBN_SELCHANGE)                       
			{
				int nItem = SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE, CB_GETCURSEL, 0, 0);				
				
				if (nItem!=CB_ERR)
				{
					UT_uint32 n = (UT_uint32)SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE,
																CB_GETITEMDATA,   (WPARAM)nItem, 0);										
					
						setEndnoteType((FootnoteType)n);
				}
			}
			
			break;
		}

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}

	return 0;
}

