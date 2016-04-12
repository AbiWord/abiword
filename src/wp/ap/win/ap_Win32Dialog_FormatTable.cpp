/* AbiWord
 * Copyright (C) 2002-3 Jordi Mas i Hernàndez <jmas@softcatala.org>
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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Win32Dialog_FormatTable.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32Toolbar_Icons.h"

#define BITMAP_WITDH	15
#define BITMAP_HEIGHT	15

const char * sThicknessTable[FORMAT_TABLE_NUMTHICKNESS] = {"0.25pt","0.5pt",
													   "0.75pt","1.0pt",
													   "1.5pt","2.25pt","3pt",
													   "4.5pt","6.0pt"};


XAP_Dialog * AP_Win32Dialog_FormatTable::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_Win32Dialog_FormatTable * p = new AP_Win32Dialog_FormatTable(pFactory,id);
	return p;
}

AP_Win32Dialog_FormatTable::AP_Win32Dialog_FormatTable(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_FormatTable(pDlgFactory,id),
	m_hBitmapBottom(NULL),	
	m_hBitmapTop(NULL), 
	m_hBitmapRight(NULL),
	m_hBitmapLeft(NULL),
	m_pPreviewWidget(NULL)
{		
	UT_sint32 i = 0;
	for(i=0; i < FORMAT_TABLE_NUMTHICKNESS ;i++)
		m_dThickness[i] = UT_convertToInches(sThicknessTable[i]);
}   
    
AP_Win32Dialog_FormatTable::~AP_Win32Dialog_FormatTable(void)
{
	if (m_pPreviewWidget) delete m_pPreviewWidget;			
	if (m_hBitmapBottom) DeleteObject(m_hBitmapBottom);		
	if (m_hBitmapTop) DeleteObject(m_hBitmapTop);		
	if (m_hBitmapRight) DeleteObject(m_hBitmapRight);		
	if (m_hBitmapLeft) DeleteObject(m_hBitmapLeft);		
}

void AP_Win32Dialog_FormatTable::runModeless(XAP_Frame * pFrame)
{
	UT_return_if_fail (pFrame);		
    UT_return_if_fail (m_id == AP_DIALOG_ID_FORMAT_TABLE);
    createModeless(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_FORMATTABLE));
    
	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);
}

BOOL AP_Win32Dialog_FormatTable::_onDlgMessage(HWND /*hWnd*/, UINT msg, WPARAM /*wParam*/, LPARAM lParam)
{	
	if (msg == WM_DRAWITEM)
	{
		DRAWITEMSTRUCT* dis =  (DRAWITEMSTRUCT*)lParam;
		
		if (dis->CtlID==AP_RID_DIALOG_FORMATTABLE_BTN_BACKCOLOR)
			m_backgButton.draw(dis);			

		if (dis->CtlID==AP_RID_DIALOG_FORMATTABLE_BTN_BORDERCOLOR)
			m_borderButton.draw(dis);
			
		return TRUE;		
	}

	else if (msg == WM_DESTROY) 
    {
		finalize();
		return FALSE;
 	}
    return FALSE;
}

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_FormatTable::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{	
	UT_uint32 w,h, i;
	int nItem;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	DWORD dwColor = GetSysColor(COLOR_BTNFACE);
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));	
	
	/* Localise controls*/
	_DSX(FORMATTABLE_BTN_CANCEL,		DLG_Close);
	_DSX(FORMATTABLE_BTN_APPLY,			DLG_Apply);
	_DS(FORMATTABLE_TEXT_BACKGROUND,	DLG_FormatTable_Color);
	_DS(FORMATTABLE_TEXT_PREVIEW,		DLG_FormatTable_Preview);
	_DS(FORMATTABLE_TEXT_BORDERS,		DLG_FormatTable_Borders);
	_DS(FORMATTABLE_TEXT_BORDER, 		DLG_FormatTable_Color);
	_DS(FORMATTABLE_TEXT_BACKGROUNDS, 	DLG_FormatTable_Background);
	_DS(FORMATTABLE_TEXT_APPLYTO,	 	DLG_FormatTable_Apply_To);
	_DS(FORMATTABLE_BUTTON_SELIMAGE,	DLG_FormatTable_SelectImage);
	_DS(FORMATTABLE_BUTTON_NOIMAGE,		DLG_FormatTable_NoImageBackground);
	_DS(FORMATTABLE_TEXT_THICKNESS,		DLG_FormatTable_Thickness);
	_DS(FORMATTABLE_TEXT_IMGBACK,		DLG_FormatTable_SetImageBackground);
	
	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_FormatTableTitle));
	
	/* Load the bitmaps into the dialog box */	
	m_hBitmapBottom = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_FORMATTABLE_BMP_BOTTOM, "FT_LINEBOTTOM",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapTop = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_FORMATTABLE_BMP_TOP, "FT_LINETOP",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapRight = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_FORMATTABLE_BMP_RIGHT, "FT_LINERIGHT",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapLeft = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_FORMATTABLE_BMP_LEFT, "FT_LINELEFT",  BITMAP_WITDH, BITMAP_HEIGHT, Color);

	/* Preview*/
	HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_FORMATTABLE_STATIC_PREVIEW);	
	UT_return_val_if_fail (hwndChild,0);

	delete m_pPreviewWidget;
	m_pPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),  hwndChild, 0);	
	m_pPreviewWidget->getGraphics()->init3dColors();
	m_pPreviewWidget->getWindowSize(&w,&h);
	_createPreviewFromGC(m_pPreviewWidget->getGraphics(), w, h);	
    m_pPreviewWidget->setPreview(m_pFormatTablePreview); 
									
	startUpdater();
    setAllSensitivities();

	/* Default status for the push bottons*/
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_TOP, getTopToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_BOTTOM, getBottomToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_RIGHT, getRightToggled() ? BST_CHECKED: BST_UNCHECKED);
    CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_LEFT, getLeftToggled() ? BST_CHECKED: BST_UNCHECKED);

	/* Combo Values for Applyto*/	
	nItem = addItemToCombo (AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO, pSS->getValue(AP_STRING_ID_DLG_FormatTable_Apply_To_Selection));
	setComboDataItem (AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO, nItem, FORMAT_TABLE_SELECTION);
	
	nItem = addItemToCombo (AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO, pSS->getValue(AP_STRING_ID_DLG_FormatTable_Apply_To_Row));
	setComboDataItem (AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO, nItem, FORMAT_TABLE_ROW);
	
	nItem = addItemToCombo (AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO, pSS->getValue(AP_STRING_ID_DLG_FormatTable_Apply_To_Column));
    setComboDataItem (AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO, nItem, FORMAT_TABLE_COLUMN);
	
	nItem = addItemToCombo (AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO, pSS->getValue(AP_STRING_ID_DLG_FormatTable_Apply_To_Table));
    setComboDataItem (AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO, nItem, FORMAT_TABLE_TABLE);
			
	selectComboItem (AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO, 0);

	/* Combo Values for Thickness */	
	for(i=0; i < FORMAT_TABLE_NUMTHICKNESS ;i++)
		addItemToCombo (AP_RID_DIALOG_FORMATTABLE_COMBO_THICKNESS, sThicknessTable[i]);
	selectComboItem (AP_RID_DIALOG_FORMATTABLE_COMBO_THICKNESS, 0);
    centerDialog();
	return 1; 
}

BOOL AP_Win32Dialog_FormatTable::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
//	HWND hWndCtrl = (HWND)lParam;
	switch (wId)
	{			
		case AP_RID_DIALOG_FORMATTABLE_BMP_BOTTOM:		
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_BOTTOM)==BST_CHECKED);							
			toggleLineType(AP_Dialog_FormatTable::toggle_bottom, bChecked);				
			event_previewExposed();			
			return 1;
		}		
		case AP_RID_DIALOG_FORMATTABLE_BMP_TOP:		
		{
			bool bChecked;			
	    	bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_TOP)==BST_CHECKED);								
			toggleLineType(AP_Dialog_FormatTable::toggle_top, bChecked);				
			event_previewExposed();			
			return 1;
		}	
		case AP_RID_DIALOG_FORMATTABLE_BMP_RIGHT:		
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_RIGHT)==BST_CHECKED);
    		toggleLineType(AP_Dialog_FormatTable::toggle_right, bChecked);
			event_previewExposed();
			return 1;
		}		
		case AP_RID_DIALOG_FORMATTABLE_BMP_LEFT:
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_LEFT)==BST_CHECKED);								
			toggleLineType(AP_Dialog_FormatTable::toggle_left, bChecked);				
			event_previewExposed();			
			return 1;
		}			 
		case AP_RID_DIALOG_FORMATTABLE_BTN_BORDERCOLOR:		
		{	
			CHOOSECOLORW cc;                
			static COLORREF acrCustClr[16];
			/* Initialize CHOOSECOLOR */
			ZeroMemory(&cc, sizeof(CHOOSECOLORW));
			cc.lStructSize = sizeof(CHOOSECOLORW);
			cc.hwndOwner = m_hDlg;
			cc.lpCustColors = (LPDWORD) acrCustClr;
			cc.rgbResult = 0;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;
			if(ChooseColorW(&cc))			
			{
				setBorderColor(UT_RGBColor(GetRValue( cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult)));		
				m_borderButton.setColour(cc.rgbResult);
				/*Force redraw*/
				InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_FORMATTABLE_BTN_BORDERCOLOR), NULL, FALSE);
				event_previewExposed();	
			}
			return 1;
		}
		
        case AP_RID_DIALOG_FORMATTABLE_BTN_BACKCOLOR:		
		{	
			CHOOSECOLORW cc;               
			static COLORREF acrCustClr2[16];
			/* Initialize CHOOSECOLOR */
			ZeroMemory(&cc, sizeof(CHOOSECOLORW));
			cc.lStructSize = sizeof(CHOOSECOLORW);
			cc.hwndOwner = m_hDlg;
			cc.lpCustColors = (LPDWORD) acrCustClr2;
			cc.rgbResult = 0;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;
			if(ChooseColorW(&cc))			
			{
				setBackgroundColor(UT_RGBColor(GetRValue( cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult)));						
				m_backgButton.setColour(cc.rgbResult);
				/*Force redraw*/
				InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_FORMATTABLE_BTN_BACKCOLOR), NULL, FALSE);
				event_previewExposed();	
			}
			return 1;
		}			

		case AP_RID_DIALOG_FORMATTABLE_COMBO_THICKNESS:             //TODO: CHECK
		{
			if (wNotifyCode == CBN_SELCHANGE)                       
			{
              int nSelected = getComboSelectedIndex (AP_RID_DIALOG_FORMATTABLE_COMBO_THICKNESS);  
                
				if (nSelected != CB_ERR)
				{
                    UT_Win32LocaleString thickness;
                    UT_UTF8String thickness_utf8 = thickness.utf8_str ();
					getComboTextItem(AP_RID_DIALOG_FORMATTABLE_COMBO_THICKNESS, nSelected, thickness);
                    setBorderThickness(thickness_utf8);                                        
                    /*Force redraw*/
					InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_FORMATTABLE_BTN_BACKCOLOR), NULL, FALSE);
					event_previewExposed();	
				}
			}
			return 1;
		}

		case AP_RID_DIALOG_FORMATTABLE_BTN_CANCEL:			
			m_answer = AP_Dialog_FormatTable::a_CLOSE;
			destroy();
			EndDialog(hWnd,0);
			return 1;

		case AP_RID_DIALOG_FORMATTABLE_BTN_APPLY:
		{
			int nSelected, nData = FORMAT_TABLE_SELECTION;
			HWND hCombo = GetDlgItem(hWnd, AP_RID_DIALOG_FORMATTABLE_COMBO_APPLYTO);

			nSelected = SendMessageW(hCombo, CB_GETCURSEL, 0, 0);					

			if (nSelected!=CB_ERR)			
				nData  = SendMessageW(hCombo, CB_GETITEMDATA, nSelected, 0);

			setApplyFormatTo((FormatTable) nData);

			m_answer = AP_Dialog_FormatTable::a_OK;
			applyChanges();			
			return 1;
		}							  					

		case AP_RID_DIALOG_FORMATTABLE_BUTTON_SELIMAGE:
				askForGraphicPathName();
				return 1;

		case AP_RID_DIALOG_FORMATTABLE_BUTTON_NOIMAGE:
				clearImage();
				return 1;

		default:							// we did not handle this notification 
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}

void AP_Win32Dialog_FormatTable::event_previewExposed(void)
{
	if(m_pFormatTablePreview)
		m_pFormatTablePreview->draw();
}

void AP_Win32Dialog_FormatTable::setBackgroundColorInGUI(UT_RGBColor clr)
{
	m_backgButton.setColour(RGB(clr.m_red,clr.m_grn,clr.m_blu));
	/* force redraw */
	InvalidateRect(GetDlgItem(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BTN_BACKCOLOR), NULL, FALSE);
}

void AP_Win32Dialog_FormatTable::setBorderThicknessInGUI(UT_UTF8String & /*sThick*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

void AP_Win32Dialog_FormatTable::setSensitivity(bool /*bSens*/)
{
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_TOP, getTopToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_BOTTOM, getBottomToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_RIGHT, getRightToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATTABLE_BMP_LEFT, getLeftToggled() ? BST_CHECKED: BST_UNCHECKED);	
}

void AP_Win32Dialog_FormatTable::destroy(void) 
{
	finalize();	
	DestroyWindow(m_hDlg);	
}

void AP_Win32Dialog_FormatTable::activate(void)
{	        
	ConstructWindowName();
	setAllSensitivities();	

	showWindow(SW_SHOW);
	bringWindowToTop();
}

void AP_Win32Dialog_FormatTable::notifyActiveFrame(XAP_Frame *pFrame)
{ 	
	setAllSensitivities();
	
	if((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) != static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		// Update the caption
		ConstructWindowName();
		setDialogTitle(m_WindowName);

		SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, (LONG_PTR)static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow());
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void AP_Win32Dialog_FormatTable::event_Close(void)
{
	m_answer = AP_Dialog_FormatTable::a_CLOSE;
	destroy();
}
