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
	UT_ASSERT(pFrame);	
	setFrame(pFrame);
	
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	LPCTSTR lpTemplate = NULL;
	
	UT_ASSERT(m_id == AP_DIALOG_ID_FORMAT_FOOTNOTES);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_FORMATFOOTNOTES);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
						static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
						(DLGPROC)s_dlgProc,(LPARAM)this);
						
	UT_ASSERT((result != -1));
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

typedef struct
{	
	const char*		pText;
	FootnoteType	nID;
} FT_TYPES;

FT_TYPES ft_Types[]=
{
	{"1,2,3,..", FOOTNOTE_TYPE_NUMERIC},
	{"[1],[2],[3],..", FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS},
	{"(1),(2),(3)..", FOOTNOTE_TYPE_NUMERIC_PAREN},
	{"1),2),3)..", FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN},
	{"a,b,c,..", FOOTNOTE_TYPE_LOWER},
	{"(a),(b),(c)..", FOOTNOTE_TYPE_LOWER_PAREN},
	{"a),b),c)..",  FOOTNOTE_TYPE_LOWER_OPEN_PAREN}, 
	{"A,B,C..", FOOTNOTE_TYPE_UPPER},
	{"(A),(B),(C)..", FOOTNOTE_TYPE_LOWER_PAREN},
	{"A),B),C)..", FOOTNOTE_TYPE_LOWER_OPEN_PAREN },
	{"i,ii,iii,..",  FOOTNOTE_TYPE_LOWER_ROMAN},
	{"(i),(ii),(iii),..", FOOTNOTE_TYPE_LOWER_ROMAN_PAREN},
	{"I,II,III,...", FOOTNOTE_TYPE_UPPER_ROMAN},
	{"(I),(II),(III),..", FOOTNOTE_TYPE_UPPER_ROMAN},
	{NULL, (FootnoteType)FOOTNOTE_TYPE_NUMERIC}
};

// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_FormatFootnotes::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{	
	const XAP_StringSet * pSS = m_pApp->getStringSet();	
	m_hwndDlg = hWnd;
	char szText[16];	
				
	/* localize controls  */
	_DSX(BTN_OK,			DLG_OK);
	_DSX(BTN_CANCEL,		DLG_Cancel);				
	_DS(RADIO_RSEL, 		DLG_FormatFootnotes_FootRestartSec);	
	_DS(RADIO_PAGE,			DLG_FormatFootnotes_FootRestartPage);
	_DS(STATIC_FSTYLES1,	DLG_FormatFootnotes_FootStyle);
	_DS(STATIC_FSTYLES2,	DLG_FormatFootnotes_FootStyle);
	_DS(RADIO_ENDDOC,		DLG_FormatFootnotes_EndPlaceEndDoc);	
	_DS(RADIO_ENDSEC,		DLG_FormatFootnotes_EndPlaceEndSec);	
	_DS(STATIC_ESTYLES1,	DLG_FormatFootnotes_EndStyle);
	_DS(STATIC_ESTYLES2, 	DLG_FormatFootnotes_EndStyle);
	_DS(RADIO_ERSTSEC, 		DLG_FormatFootnotes_EndRestartSec);
	
	/*Caption*/
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_FormatFootnotes_Title));
		
	setInitialValues(); /* Parent class loads data*/
	
	/* Footnote style combobox*/
	int nItem;
	FT_TYPES* ft;
	int nDef = 0;
	
	for (ft= &ft_Types[0]; ft->pText; ft++)
	{		                                                                                
		 nItem = SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE, CB_ADDSTRING, 0, (LPARAM)ft->pText);
		 SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE, CB_SETITEMDATA, nItem, (LPARAM)ft);
		 
		 if (ft->nID==getFootnoteType())
		  	nDef = nItem;			
 	}
 	
 	SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE, CB_SETCURSEL, nDef, 0);	 
 	
 	/* Endnote style combobox*/	
 	nDef = 0;
	for (ft= &ft_Types[0]; ft->pText; ft++)
	{		                                                                                
		 nItem = SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE, CB_ADDSTRING, 0, (LPARAM)ft->pText);
		 SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE, CB_SETITEMDATA, nItem, (LPARAM)ft);
		 
		 if (ft->nID==getEndnoteType())
			nDef = nItem;
 	}
 	
 	SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE, CB_SETCURSEL, nDef, 0);	 
	 	 	 	
 	
	/*Set Default Radio buttons Footnotes */                                                                                                      
	CheckRadioButton(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_RSEL, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_PAGE,
		getRestartFootnoteOnSection() ? AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_RSEL : AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_PAGE);                                                                              

	/*Set Default Radio buttons Endnotes */                                                                                                      		
	CheckRadioButton(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDDOC, AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDSEC,
		 getRestartEndnoteOnSection() ? AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDDOC: AP_RID_DIALOG_FORMATFOOTNOTES_RADIO_ENDSEC);                                                                              		
				
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
	HWND hWndCtrl = (HWND)lParam;

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
			updateDocWithValues();
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
					FT_TYPES* ft = NULL;					
					ft = (FT_TYPES*)SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_FSTYLE, CB_GETITEMDATA,   (WPARAM)nItem, 0);										
					
					if (ft)
						setFootnoteType(ft->nID);					
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
					FT_TYPES* ft = NULL;					
					ft = (FT_TYPES*)SendDlgItemMessage(hWnd, AP_RID_DIALOG_FORMATFOOTNOTES_COMBO_ESTYLE, CB_GETITEMDATA,   (WPARAM)nItem, 0);										
					
					if (ft)
						setEndnoteType(ft->nID);					
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

