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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
#include "ap_Win32Dialog_Border_Shading.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32Toolbar_Icons.h"


#define BITMAP_WITDH	15
#define BITMAP_HEIGHT	15

const char * sThicknessTable_Border_Shading[BORDER_SHADING_NUMTHICKNESS] = {"0.25pt","0.5pt",
													   "0.75pt","1.0pt",
													   "1.5pt","2.25pt","3pt",
													   "4.5pt","6.0pt"};

const char * sOffsetTable_Border_Shading[BORDER_SHADING_NUMOFFSETS] = {"0.25pt","0.5pt",
														"0.75pt","1.0pt",
														"1.5pt","2.25pt","3pt",
														"4.5pt","6.0pt"};

XAP_Dialog * AP_Win32Dialog_Border_Shading::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_Win32Dialog_Border_Shading * p = new AP_Win32Dialog_Border_Shading(pFactory,id);
	return p;
}

AP_Win32Dialog_Border_Shading::AP_Win32Dialog_Border_Shading(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_Border_Shading(pDlgFactory,id),
	m_hBitmapBottom(NULL),	
	m_hBitmapTop(NULL), 
	m_hBitmapRight(NULL),
	m_hBitmapLeft(NULL),
	m_pPreviewWidget(NULL)
{		
	UT_sint32 i = 0;
	for(i=0; i < BORDER_SHADING_NUMTHICKNESS ;i++)
		m_dThickness[i] = UT_convertToInches(sThicknessTable_Border_Shading[i]);

	for(i=0; i < BORDER_SHADING_NUMOFFSETS ;i++)
		m_dOffset[i] = UT_convertToInches(sOffsetTable_Border_Shading[i]);

}   
    
AP_Win32Dialog_Border_Shading::~AP_Win32Dialog_Border_Shading(void)
{
	if (m_pPreviewWidget) delete m_pPreviewWidget;			
	if (m_hBitmapBottom) DeleteObject(m_hBitmapBottom);		
	if (m_hBitmapTop) DeleteObject(m_hBitmapTop);		
	if (m_hBitmapRight) DeleteObject(m_hBitmapRight);		
	if (m_hBitmapLeft) DeleteObject(m_hBitmapLeft);		
}

void AP_Win32Dialog_Border_Shading::runModeless(XAP_Frame * pFrame)
{
	UT_return_if_fail (pFrame);		
    UT_return_if_fail (m_id == AP_DIALOG_ID_BORDER_SHADING);
    createModeless(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_BORDER_SHADING));
    
	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);
}

BOOL AP_Win32Dialog_Border_Shading::_onDlgMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	if (msg == WM_DRAWITEM)
	{
		DRAWITEMSTRUCT* dis =  (DRAWITEMSTRUCT*)lParam;
		
		if (dis->CtlID==AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR)
			m_backgButton.draw(dis);			

		if (dis->CtlID==AP_RID_DIALOG_BORDERSHADING_BTN_BORDER_COLOR)
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

HBITMAP AP_Win32Dialog_Border_Shading::_loadBitmap(HWND hWnd, UINT nId, char* pName, int width, int height, UT_RGBColor color)
{
	HBITMAP hBitmap = NULL;
	
    AP_Win32Toolbar_Icons::getBitmapForIcon(hWnd, width,height, &color,	pName,	&hBitmap);
	SendDlgItemMessageW(hWnd,  nId,  BM_SETIMAGE,  IMAGE_BITMAP, (LPARAM) hBitmap);	
	return hBitmap; 
}

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_Border_Shading::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{	
	UT_uint32 w,h, i;
	RECT rect;
	int nItem;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	DWORD dwColor = GetSysColor(COLOR_BTNFACE);
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));	
	
	/* Localise controls*/
	_DSX(BORDERSHADING_BTN_CANCEL,		DLG_Close);
	_DSX(BORDERSHADING_BTN_APPLY,			DLG_Apply);
	_DS(BORDERSHADING_TEXT_BORDER_COLOR,	DLG_BorderShading_Color);
	_DS(BORDERSHADING_TEXT_SHADING_COLOR, 	DLG_BorderShading_Color);
	_DS(BORDERSHADING_TEXT_PREVIEW,		DLG_BorderShading_Preview);
	_DS(BORDERSHADING_TEXT_BORDER,		DLG_BorderShading_Borders);
 	_DS(BORDERSHADING_TEXT_SHADING, 	DLG_BorderShading_Shading);
	_DS(BORDERSHADING_TEXT_BORDER_THICKNESS,	DLG_BorderShading_Thickness);
 	_DS(BORDERSHADING_TEXT_SHADING_OFFSET,		DLG_BorderShading_Offset);
	
	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_BorderShading_Title));
	
	/* Load the bitmaps into the dialog box */	
    m_hBitmapBottom = _loadBitmap(hWnd,AP_RID_DIALOG_BORDERSHADING_BMP_BOTTOM, "FT_LINEBOTTOM",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
    m_hBitmapTop = _loadBitmap(hWnd,AP_RID_DIALOG_BORDERSHADING_BMP_TOP, "FT_LINETOP",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
    m_hBitmapRight = _loadBitmap(hWnd,AP_RID_DIALOG_BORDERSHADING_BMP_RIGHT, "FT_LINERIGHT",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
    m_hBitmapLeft = _loadBitmap(hWnd,AP_RID_DIALOG_BORDERSHADING_BMP_LEFT, "FT_LINELEFT",  BITMAP_WITDH, BITMAP_HEIGHT, Color); 
    
	/* Preview*/
	HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_BORDERSHADING_STATIC_PREVIEW);	
	UT_return_val_if_fail (hwndChild,0);

	delete m_pPreviewWidget;
	m_pPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),  hwndChild, 0);	
	m_pPreviewWidget->getGraphics()->init3dColors();
	m_pPreviewWidget->getWindowSize(&w,&h);
	_createPreviewFromGC(m_pPreviewWidget->getGraphics(), w, h);	
    m_pPreviewWidget->setPreview(m_pBorderShadingPreview); 
									
	startUpdater();
    setAllSensitivities();

	/* Default status for the push bottons*/
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_TOP, getTopToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_BOTTOM, getBottomToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_RIGHT, getRightToggled() ? BST_CHECKED: BST_UNCHECKED);
    CheckDlgButton(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_LEFT, getLeftToggled() ? BST_CHECKED: BST_UNCHECKED);

	/* Combo Values for Thickness */	
	for(i=0; i < BORDER_SHADING_NUMTHICKNESS ;i++)
		addItemToCombo (AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_THICKNESS, sThicknessTable_Border_Shading[i]);
	selectComboItem (AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_THICKNESS, 0);

	/* Combo Values for Offset */	
	for(i=0; i < BORDER_SHADING_NUMOFFSETS ;i++)
		addItemToCombo (AP_RID_DIALOG_BORDERSHADING_COMBO_SHADING_OFFSET, sOffsetTable_Border_Shading[i]);
	selectComboItem (AP_RID_DIALOG_BORDERSHADING_COMBO_SHADING_OFFSET, 0);

    centerDialog();
	return 1; 
}

BOOL AP_Win32Dialog_Border_Shading::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	switch (wId)
	{			
		case AP_RID_DIALOG_BORDERSHADING_BMP_BOTTOM:		
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_BOTTOM)==BST_CHECKED);							
			toggleLineType(AP_Dialog_Border_Shading::toggle_bottom, bChecked);				
			event_previewExposed();			
			return 1;
		}		
		case AP_RID_DIALOG_BORDERSHADING_BMP_TOP:		
		{
			bool bChecked;			
	    	bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_TOP)==BST_CHECKED);								
			toggleLineType(AP_Dialog_Border_Shading::toggle_top, bChecked);				
			event_previewExposed();			
			return 1;
		}	
		case AP_RID_DIALOG_BORDERSHADING_BMP_RIGHT:		
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_RIGHT)==BST_CHECKED);
    		toggleLineType(AP_Dialog_Border_Shading::toggle_right, bChecked);
			event_previewExposed();
			return 1;
		}		
		case AP_RID_DIALOG_BORDERSHADING_BMP_LEFT:
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_LEFT)==BST_CHECKED);								
			toggleLineType(AP_Dialog_Border_Shading::toggle_left, bChecked);				
			event_previewExposed();			
			return 1;
		}			 
		case AP_RID_DIALOG_BORDERSHADING_BTN_BORDER_COLOR:		
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
				InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_BORDERSHADING_BTN_BORDER_COLOR), NULL, FALSE);
				event_previewExposed();	
			}
			return 1;
		}
		
        case AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR:		
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
				setShadingColor(UT_RGBColor(GetRValue( cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult)));						
				m_backgButton.setColour(cc.rgbResult);
				/*Force redraw*/
				InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR), NULL, FALSE);
				event_previewExposed();	
			}
			return 1;
		}			

		case AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_THICKNESS:             //TODO: CHECK
		{
			if (wNotifyCode == CBN_SELCHANGE)                       
			{
              int nSelected = getComboSelectedIndex (AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_THICKNESS);  
                
				if (nSelected != CB_ERR)
				{
                    UT_Win32LocaleString thickness;
                    UT_UTF8String thickness_utf8 = thickness.utf8_str ();
					getComboTextItem(AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_THICKNESS, nSelected, thickness);
                    setBorderThickness(thickness_utf8);                                        
                    /*Force redraw*/
					InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_BORDERSHADING_BTN_BORDER_COLOR), NULL, FALSE);
					event_previewExposed();	
				}
			}
			return 1;
		}

		case AP_RID_DIALOG_BORDERSHADING_COMBO_SHADING_OFFSET:      
			{
				if (wNotifyCode == CBN_SELCHANGE)                       
				{
					int nSelected = getComboSelectedIndex (AP_RID_DIALOG_BORDERSHADING_COMBO_SHADING_OFFSET);  

					if (nSelected != CB_ERR)
					{
						UT_Win32LocaleString offset;
						UT_UTF8String offset_utf8 = offset.utf8_str ();
						
						getComboTextItem(AP_RID_DIALOG_BORDERSHADING_COMBO_SHADING_OFFSET, nSelected, offset);
						
						//	Maleesh 6/14/2010 -  TODO:Replace this with the correct function.
// 						setBorderThickness(offset_utf8);                                        
						/*Force redraw*/
						InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR), NULL, FALSE);
						event_previewExposed();	
					}
				}
				return 1;
			}
		case AP_RID_DIALOG_BORDERSHADING_BTN_CANCEL:			
			m_answer = AP_Dialog_Border_Shading::a_CLOSE;
			destroy();
			EndDialog(hWnd,0);
			return 1;

		case AP_RID_DIALOG_BORDERSHADING_BTN_APPLY:
		{
			int nSelected, nData = FORMAT_TABLE_SELECTION;
			//	Maleesh 6/10/2010 -  TODO:
// 			HWND hCombo = GetDlgItem(hWnd, AP_RID_DIALOG_BORDERSHADING_COMBO_APPLYTO);
// 
// 			nSelected = SendMessageW(hCombo, CB_GETCURSEL, 0, 0);					
// 
// 			if (nSelected!=CB_ERR)			
// 				nData  = SendMessageW(hCombo, CB_GETITEMDATA, nSelected, 0);
// 
// 			setApplyFormatTo((_FormatTable) nData);

			m_answer = AP_Dialog_Border_Shading::a_OK;
			applyChanges();			
			return 1;
		}							  					
		default:							// we did not handle this notification 
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}

void AP_Win32Dialog_Border_Shading::event_previewExposed(void)
{
	if(m_pBorderShadingPreview)
		m_pBorderShadingPreview->draw();
}

void AP_Win32Dialog_Border_Shading::setBackgroundColorInGUI(UT_RGBColor clr)
{
	m_backgButton.setColour(RGB(clr.m_red,clr.m_grn,clr.m_blu));
	/* force redraw */
	InvalidateRect(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR), NULL, FALSE);
}

void AP_Win32Dialog_Border_Shading::setBorderThicknessInGUI(UT_UTF8String & sThick)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

void AP_Win32Dialog_Border_Shading::setSensitivity(bool bSens)
{
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_TOP, getTopToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_BOTTOM, getBottomToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_RIGHT, getRightToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BMP_LEFT, getLeftToggled() ? BST_CHECKED: BST_UNCHECKED);	
}

void AP_Win32Dialog_Border_Shading::destroy(void) 
{
	finalize();	
	DestroyWindow(m_hDlg);	
}

void AP_Win32Dialog_Border_Shading::activate(void)
{	        
	ConstructWindowName();
	setAllSensitivities();	

	showWindow(SW_SHOW);
	bringWindowToTop();
}

void AP_Win32Dialog_Border_Shading::notifyActiveFrame(XAP_Frame *pFrame)
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

void AP_Win32Dialog_Border_Shading::event_Close(void)
{
	m_answer = AP_Dialog_Border_Shading::a_CLOSE;
	destroy();
}
