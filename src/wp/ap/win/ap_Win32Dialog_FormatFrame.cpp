/* AbiWord
 * Copyright (C) 2002-4 Jordi Mas i Hernàndez <jmas@softcatala.org>
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
#include "ut_locale.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Win32Dialog_FormatFrame.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32Toolbar_Icons.h"

#define BITMAP_WITDH	15
#define BITMAP_HEIGHT	15

const char * sThickness[FORMAT_FRAME_NUMTHICKNESS] = {"0.25pt","0.5pt",
													   "0.75pt","1.0pt",
													   "1.5pt","2.25pt","3pt",
													   "4.5pt","6.0pt"};
	

XAP_Dialog * AP_Win32Dialog_FormatFrame::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_Win32Dialog_FormatFrame * p = new AP_Win32Dialog_FormatFrame(pFactory,id);
	return p;
}

AP_Win32Dialog_FormatFrame::AP_Win32Dialog_FormatFrame(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_FormatFrame(pDlgFactory,id),
	m_hBitmapBottom(NULL),	
	m_hBitmapTop(NULL), 
	m_hBitmapRight(NULL),
	m_hBitmapLeft(NULL),
	m_pPreviewWidget(NULL)
{
	UT_sint32 i = 0;
	for(i=0; i < FORMAT_FRAME_NUMTHICKNESS ;i++)
		m_dThickness[i] = UT_convertToInches(sThickness[i]);	 
}   
    
AP_Win32Dialog_FormatFrame::~AP_Win32Dialog_FormatFrame(void)
{
	if (m_pPreviewWidget) delete m_pPreviewWidget;			
	if (m_hBitmapBottom) DeleteObject(m_hBitmapBottom);		
	if (m_hBitmapTop) DeleteObject(m_hBitmapTop);		
	if (m_hBitmapRight) DeleteObject(m_hBitmapRight);		
	if (m_hBitmapLeft) DeleteObject(m_hBitmapLeft);		
}

void AP_Win32Dialog_FormatFrame::runModeless(XAP_Frame * pFrame)
{
	UT_return_if_fail (pFrame);		

	UT_return_if_fail (m_id == AP_DIALOG_ID_FORMAT_FRAME);	

	 createModeless(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_FORMATFRAME));

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	ShowWindow(m_hDlg, SW_SHOW);
	BringWindowToTop(m_hDlg);	
		
}

BOOL AP_Win32Dialog_FormatFrame::_onDlgMessage(HWND /*hWnd*/, UINT msg, WPARAM /*wParam*/, LPARAM lParam)
{	
	if (msg == WM_DRAWITEM)
 	{
	DRAWITEMSTRUCT* dis =  (DRAWITEMSTRUCT*)lParam;
	
	if (dis->CtlID==AP_RID_DIALOG_FORMATFRAME_BTN_BACKCOLOR)		
           m_backgButton.draw(dis);
		
	if (dis->CtlID==AP_RID_DIALOG_FORMATFRAME_BTN_BORDERCOLOR)							    
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
BOOL AP_Win32Dialog_FormatFrame::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{	
//	HDC hdc;
//	int x, y;	
	UT_uint32 w,h;
	UT_sint32 i = 0;
	RECT rect;	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	DWORD dwColor = GetSysColor(COLOR_BTNFACE);	
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
	
	/* The four items are the same size */
	GetClientRect(GetDlgItem(hWnd, AP_RID_DIALOG_FORMATFRAME_BMP_BOTTOM), &rect);					
//	hdc = GetDC(hWnd);
//	x = rect.right - rect.left;
//	y = rect.bottom - rect.top;
	
	/* Localise controls*/
	_DSX(FORMATFRAME_BTN_CANCEL,		DLG_Close);
	_DSX(FORMATFRAME_BTN_APPLY,			DLG_Apply);
	_DS(FORMATFRAME_TEXT_BACKGROUND,	DLG_FormatFrame_Color);
	_DS(FORMATFRAME_TEXT_PREVIEW,		DLG_FormatFrame_Preview);
	_DS(FORMATFRAME_TEXT_BORDERS,		DLG_FormatFrame_Border_Color);
	_DS(FORMATFRAME_TEXT_BORDER, 		DLG_FormatFrame_Color);
	_DS(FORMATFRAME_TEXT_BACKGROUNDS, 	DLG_FormatFrame_Background);
	_DS(FORMATFRAME_TEXT_TEXTWRAPS,		DLG_FormatFrame_TextWrapping);
	_DS(FORMATFRAME_CHK_TEXTWRAP,		DLG_FormatFrame_SetTextWrapping);
	_DS(FORMATFRAME_TEXT_POSTEXTPOS,	DLG_FormatFrame_PositionTo);
	_DS(FORMATFRAME_RADIO_PARA,		 	DLG_FormatFrame_SetToParagraph);
	_DS(FORMATFRAME_RADIO_COLUMN,		DLG_FormatFrame_SetToColumn);
	_DS(FORMATFRAME_RADIO_PAGE,			DLG_FormatFrame_SetToPage);
	_DS(FORMATFRAME_BUTTON_SELIMAGE,	DLG_FormatFrame_SelectImage);
	_DS(FORMATFRAME_BUTTON_NOIMAGE,		DLG_FormatFrame_NoImageBackground);
	_DS(FORMATFRAME_TEXT_THICKNESS,		DLG_FormatTable_Thickness);
	_DS(FORMATFRAME_TEXT_IMGBACK,		DLG_FormatFrame_SetImageBackground);


	setDialogTitle (pSS->getValue(AP_STRING_ID_DLG_FormatFrameTitle));	
	
	
	/* Load the bitmaps into the dialog box */
	m_hBitmapBottom = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_FORMATFRAME_BMP_BOTTOM, "FT_LINEBOTTOM",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapTop = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_FORMATFRAME_BMP_TOP, "FT_LINETOP",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapRight = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_FORMATFRAME_BMP_RIGHT, "FT_LINERIGHT",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapLeft = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_FORMATFRAME_BMP_LEFT, "FT_LINELEFT",  BITMAP_WITDH, BITMAP_HEIGHT, Color);

	/* Preview*/
	HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_FORMATFRAME_STATIC_PREVIEW);	
	UT_return_val_if_fail (hwndChild,1);

	m_pPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),  hwndChild, 0);	
	m_pPreviewWidget->getGraphics()->init3dColors();
	m_pPreviewWidget->getWindowSize(&w,&h);
	_createPreviewFromGC(m_pPreviewWidget->getGraphics(), w, h);	
	m_pPreviewWidget->setPreview(m_pFormatFramePreview); 	
								
	startUpdater();
	setAllSensitivities();

	/* Default status for the dialog controls */
	CheckDlgButton(hWnd, AP_RID_DIALOG_FORMATFRAME_BMP_TOP, getTopToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(hWnd, AP_RID_DIALOG_FORMATFRAME_BMP_BOTTOM, getBottomToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(hWnd, AP_RID_DIALOG_FORMATFRAME_BMP_RIGHT, getRightToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(hWnd, AP_RID_DIALOG_FORMATFRAME_BMP_LEFT, getLeftToggled() ? BST_CHECKED: BST_UNCHECKED);

	/* Position to radio buttons */
	if(positionMode() == FL_FRAME_POSITIONED_TO_BLOCK)
	{
		CheckRadioButton(hWnd, AP_RID_DIALOG_FORMATFRAME_RADIO_PARA, AP_RID_DIALOG_FORMATFRAME_RADIO_PAGE, AP_RID_DIALOG_FORMATFRAME_RADIO_PARA);
	}
	else if(positionMode() == FL_FRAME_POSITIONED_TO_COLUMN)
	{
		CheckRadioButton(hWnd, AP_RID_DIALOG_FORMATFRAME_RADIO_PARA, AP_RID_DIALOG_FORMATFRAME_RADIO_PAGE, AP_RID_DIALOG_FORMATFRAME_RADIO_COLUMN);
	}
	else if(positionMode() == FL_FRAME_POSITIONED_TO_PAGE)
	{
		CheckRadioButton(hWnd, AP_RID_DIALOG_FORMATFRAME_RADIO_PARA, AP_RID_DIALOG_FORMATFRAME_RADIO_PAGE, AP_RID_DIALOG_FORMATFRAME_RADIO_PAGE);
	}

	/* Wrapping checkbox */
	CheckDlgButton(hWnd, AP_RID_DIALOG_FORMATFRAME_CHK_TEXTWRAP, getWrapping() ? BST_CHECKED: BST_UNCHECKED);

	/* Combo Values for Thickness */
	
	for(i=0; i < FORMAT_FRAME_NUMTHICKNESS ;i++)
		addItemToCombo (AP_RID_DIALOG_FORMATFRAME_COMBO_THICKNESS, sThickness[i]);

	selectComboItem (AP_RID_DIALOG_FORMATFRAME_COMBO_THICKNESS, 0);

	centerDialog();
	return 1; 
}


BOOL AP_Win32Dialog_FormatFrame::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);

	switch (wId)
	{			
		case AP_RID_DIALOG_FORMATFRAME_BMP_BOTTOM:		
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_FORMATFRAME_BMP_BOTTOM)==BST_CHECKED);							
			toggleLineType(AP_Dialog_FormatFrame::toggle_bottom, bChecked);				
			event_previewExposed();			
			return 1;
		}			
		
		case AP_RID_DIALOG_FORMATFRAME_BMP_TOP:		
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_FORMATFRAME_BMP_TOP)==BST_CHECKED);							
			toggleLineType(AP_Dialog_FormatFrame::toggle_top, bChecked);				
			event_previewExposed();			
			return 1;
		}	
		
		case AP_RID_DIALOG_FORMATFRAME_BMP_RIGHT:		
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_FORMATFRAME_BMP_RIGHT)==BST_CHECKED);							
			toggleLineType(AP_Dialog_FormatFrame::toggle_right, bChecked);				
			event_previewExposed();			
			return 1;
		}			
		
		case AP_RID_DIALOG_FORMATFRAME_BMP_LEFT:		
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_FORMATFRAME_BMP_LEFT)==BST_CHECKED);							
			toggleLineType(AP_Dialog_FormatFrame::toggle_left, bChecked);				
			event_previewExposed();			
			return 1;
		}	
		 
		 
		case AP_RID_DIALOG_FORMATFRAME_BTN_BORDERCOLOR:		
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
				InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_FORMATFRAME_BTN_BORDERCOLOR), NULL, FALSE);
				event_previewExposed();	
			}

			return 1;
		}	
		
		
		case AP_RID_DIALOG_FORMATFRAME_BTN_BACKCOLOR:		
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
				setBGColor(UT_RGBColor(GetRValue( cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult)));						
				m_backgButton.setColour(cc.rgbResult);

				/*Force redraw*/
				InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_FORMATFRAME_BTN_BACKCOLOR), NULL, FALSE);
				event_previewExposed();	
			}

			return 1;
		}			

		case AP_RID_DIALOG_FORMATFRAME_CHK_TEXTWRAP:
		{
			bool bChecked;
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_FORMATFRAME_CHK_TEXTWRAP)==BST_CHECKED);

			setWrapping(bChecked);

			// Not necessary now, but we may some day show
			// text wrapping in the preview.
			event_previewExposed();
			return 1;
		}

			
		case AP_RID_DIALOG_FORMATFRAME_BTN_CANCEL:			
			m_answer = AP_Dialog_FormatFrame::a_CLOSE;
			destroy();
			event_Close();
			EndDialog(hWnd,0);
			return 1;

		case AP_RID_DIALOG_FORMATFRAME_COMBO_THICKNESS:
		{
			if (wNotifyCode == CBN_SELCHANGE)                       
			{
				int nSelected = getComboSelectedIndex (AP_RID_DIALOG_FORMATFRAME_COMBO_THICKNESS);

				if (nSelected != CB_ERR)
				{
					UT_LocaleTransactor t(LC_NUMERIC, "C");					
					UT_Win32LocaleString thickness;
					UT_UTF8String thickness_utf8 = thickness.utf8_str ();
					getComboTextItem(AP_RID_DIALOG_FORMATFRAME_COMBO_THICKNESS, nSelected, thickness);
					setBorderThicknessAll(thickness_utf8);					
					event_previewExposed();
				}
			}
			return 1;
		}

		case AP_RID_DIALOG_FORMATFRAME_BUTTON_SELIMAGE:
				askForGraphicPathName();
				return 1;

		case AP_RID_DIALOG_FORMATFRAME_BUTTON_NOIMAGE:
				clearImage();
				return 1;

		case AP_RID_DIALOG_FORMATFRAME_RADIO_PARA:
				setPositionMode(FL_FRAME_POSITIONED_TO_BLOCK);
				return 1;

		case AP_RID_DIALOG_FORMATFRAME_RADIO_COLUMN:
				setPositionMode(FL_FRAME_POSITIONED_TO_COLUMN);
				return 1;

		case AP_RID_DIALOG_FORMATFRAME_RADIO_PAGE:
				setPositionMode(FL_FRAME_POSITIONED_TO_PAGE);
				return 1;

		case AP_RID_DIALOG_FORMATFRAME_BTN_APPLY:
				applyChanges();
				return 1;

		
			
		default:							// we did not handle this notification 
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}

void AP_Win32Dialog_FormatFrame::event_previewExposed(void)
{
	if(m_pFormatFramePreview)
		m_pFormatFramePreview->draw();
}


void AP_Win32Dialog_FormatFrame::setSensitivity(bool /*bSens*/)
{
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATFRAME_BMP_TOP, getTopToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATFRAME_BMP_BOTTOM, getBottomToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATFRAME_BMP_RIGHT, getRightToggled() ? BST_CHECKED: BST_UNCHECKED);
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATFRAME_BMP_LEFT, getLeftToggled() ? BST_CHECKED: BST_UNCHECKED);	
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_FORMATFRAME_CHK_TEXTWRAP, getWrapping()?  BST_CHECKED: BST_UNCHECKED);
}

void AP_Win32Dialog_FormatFrame::destroy(void) 
{
	finalize();	
	DestroyWindow(m_hDlg);	
}


void AP_Win32Dialog_FormatFrame::activate(void)
{	        
	ConstructWindowName();
	setAllSensitivities();	
	
	ShowWindow( m_hDlg, SW_SHOW );
	BringWindowToTop( m_hDlg );
	
}

void AP_Win32Dialog_FormatFrame::notifyActiveFrame(XAP_Frame *pFrame)
{ 	
	setAllSensitivities();
	
	if((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) != static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		// Update the caption
		ConstructWindowName();
		setDialogTitle (m_WindowName);

		SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, (LONG_PTR)static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow());
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	}
}


void AP_Win32Dialog_FormatFrame::event_Close(void)
{
	m_answer = AP_Dialog_FormatFrame::a_CLOSE;
	destroy();
}

