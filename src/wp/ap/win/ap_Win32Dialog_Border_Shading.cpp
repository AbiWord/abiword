/* AbiWord
 * Copyright (C) 2002-3 Jordi Mas i Hernàndez <jmas@softcatala.org>
 * Copyright (C) 2010-11 Maleesh Prasan <maleesh.prasan@gmail.com>
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
#include "ap_Win32Dialog_Border_Shading.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32Toolbar_Icons.h"


#define BITMAP_WITDH	15
#define BITMAP_HEIGHT	15
#define BORDER_STYLE_BITMAP_WIDTH 180
#define BORDER_STYLE_BITMAP_HEIGHT 30
#define BORDER_STYLE_COMBO_POSITION_X 60
#define BORDER_STYLE_COMBO_POSITION_Y 125

const char * sThicknessTable_Border_Shading[BORDER_SHADING_NUMTHICKNESS] = {"0.25pt","0.5pt",
													   "0.75pt","1.0pt",
													   "1.5pt","2.25pt","3pt",
													   "4.5pt","6.0pt"};

const char * sOffsetTable_Border_Shading[BORDER_SHADING_NUMOFFSETS] = {"0.25pt","0.5pt",
														"0.75pt","1.0pt",
														"1.5pt","2.25pt","3pt",
														"4.5pt","6.0pt"};

const char * sBorderStyle_Border_Shading[BORDER_SHADING_NUMOFSTYLES] = {
														"0",	//No line
														"1",	//Solid line
														"2",	//Dashed line
														"3"};	//Dotted line

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
	m_pPreviewWidget(NULL),
	m_hwndComboEx(NULL)
{		
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

BOOL AP_Win32Dialog_Border_Shading::_onDlgMessage(HWND /*hWnd*/, UINT msg, WPARAM /*wParam*/, LPARAM lParam)
{	
	if (msg == WM_DRAWITEM)
	{
		DRAWITEMSTRUCT* dis =  (DRAWITEMSTRUCT*)lParam;
		
		if (dis->CtlID == AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR)
			m_shadingButton.draw(dis);			

		if (dis->CtlID == AP_RID_DIALOG_BORDERSHADING_BTN_BORDER_COLOR)
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

HWND AP_Win32Dialog_Border_Shading::_createComboboxEx(
														const HWND hParent,
														const HINSTANCE hInst,
														DWORD dwStyle,
														const RECT& rc,const int id)
{
	dwStyle|=WS_CHILD|WS_VISIBLE;
	return CreateWindowExW(0,
						WC_COMBOBOXEXW,
						0,    
						dwStyle, 
						rc.left,
						rc.top, 
						rc.right, 
						rc.bottom, 
						hParent,    
						reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
						hInst,     
						0);          
}

// 8/7/2010 Maleesh - Convenience constant
const UINT CBX_ITEM_MASK = CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;

int AP_Win32Dialog_Border_Shading::_insertItemToComboboxEx(
															HWND hCbx, 
															const char* txt,
															int imgIndex,
															int selectedImgIndex,
															INT_PTR index = -1,
															UINT mask = CBX_ITEM_MASK)
{
	COMBOBOXEXITEMW cbei;
	memset(&cbei, 0, sizeof(cbei));

	cbei.mask			= mask;
	cbei.iItem			= index;
	cbei.pszText		= (LPWSTR)txt;
	cbei.iImage			= imgIndex;
	cbei.iSelectedImage	= selectedImgIndex;

	return static_cast<int>(SendMessageW(hCbx, CBEM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&cbei)));
}

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_Border_Shading::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{	
	UT_uint32 w,h, i;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	DWORD dwColor = GetSysColor(COLOR_BTNFACE);
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));	
	
	/* Localise controls*/
	_DSX(BORDERSHADING_BTN_CANCEL,				DLG_Close);
	_DSX(BORDERSHADING_BTN_APPLY,				DLG_Apply);
	_DS(BORDERSHADING_TEXT_BORDER_COLOR,		DLG_BorderShading_Color);
	_DS(BORDERSHADING_TEXT_SHADING_COLOR, 		DLG_BorderShading_Color);
	_DS(BORDERSHADING_TEXT_PREVIEW,				DLG_BorderShading_Preview);
	_DS(BORDERSHADING_TEXT_BORDER,				DLG_BorderShading_Borders);
	_DS(BORDERSHADING_TEXT_SHADING, 			DLG_BorderShading_Shading);
	_DS(BORDERSHADING_BTN_SHADING_ENABLE, 		DLG_BorderShading_Use_Shading);
	_DS(BORDERSHADING_TEXT_BORDER_STYLE,		DLG_BorderShading_Border_Style);
	_DS(BORDERSHADING_TEXT_BORDER_THICKNESS,	DLG_BorderShading_Thickness);
	_DS(BORDERSHADING_TEXT_SHADING_OFFSET,		DLG_BorderShading_Offset);
	
	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_BorderShading_Title));
	
	/* Load the bitmaps into the dialog box */	
    m_hBitmapBottom = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_BORDERSHADING_BMP_BOTTOM, "FT_LINEBOTTOM",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
    m_hBitmapTop	= XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_BORDERSHADING_BMP_TOP, "FT_LINETOP",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
    m_hBitmapRight	= XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_BORDERSHADING_BMP_RIGHT, "FT_LINERIGHT",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
    m_hBitmapLeft	= XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_BORDERSHADING_BMP_LEFT, "FT_LINELEFT",  BITMAP_WITDH, BITMAP_HEIGHT, Color); 
	
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

	RECT combo_rect = {
		BORDER_STYLE_COMBO_POSITION_X, 
		BORDER_STYLE_COMBO_POSITION_Y, 
		BORDER_STYLE_BITMAP_WIDTH, 
		BORDER_STYLE_BITMAP_HEIGHT * (BORDER_SHADING_NUMOFSTYLES * 2)};
// 	StartCommonControls(ICC_USEREX_CLASSES);

	XAP_App* pApp = XAP_App::getApp();
	UT_ASSERT(pApp);
	XAP_Win32App* pWin32App = static_cast<XAP_Win32App*>(pApp);
	m_hwndComboEx = _createComboboxEx(
									m_hDlg,
									pWin32App->getInstance(), 
									CBS_DROPDOWNLIST, 
									combo_rect, 
									AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_STYLE);

	HIMAGELIST hImageList =	ImageList_Create(
											BORDER_STYLE_BITMAP_WIDTH, 
											BORDER_STYLE_BITMAP_HEIGHT, 
											ILC_MASK|ILC_COLOR32, 
											BORDER_SHADING_NUMOFSTYLES, 
											0);

	// Follow the PP_PropertyMap::TypeLineStyle order.
	HBITMAP tmp_bmp0 = XAP_Win32DialogHelper::s_loadBitmap(hWnd, 0, "BORDER_STYLE_NONE",	BORDER_STYLE_BITMAP_WIDTH, BORDER_STYLE_BITMAP_HEIGHT, Color); 
	HBITMAP tmp_bmp1 = XAP_Win32DialogHelper::s_loadBitmap(hWnd, 0, "BORDER_STYLE_SOLID",	BORDER_STYLE_BITMAP_WIDTH, BORDER_STYLE_BITMAP_HEIGHT, Color); 
	HBITMAP tmp_bmp2 = XAP_Win32DialogHelper::s_loadBitmap(hWnd, 0, "BORDER_STYLE_DOTTED",  BORDER_STYLE_BITMAP_WIDTH, BORDER_STYLE_BITMAP_HEIGHT, Color); 
	HBITMAP tmp_bmp3 = XAP_Win32DialogHelper::s_loadBitmap(hWnd, 0, "BORDER_STYLE_DASHED",  BORDER_STYLE_BITMAP_WIDTH, BORDER_STYLE_BITMAP_HEIGHT, Color); 

	ImageList_Add(hImageList, tmp_bmp0, NULL);
	ImageList_Add(hImageList, tmp_bmp1, NULL);
	ImageList_Add(hImageList, tmp_bmp2, NULL);
	ImageList_Add(hImageList, tmp_bmp3, NULL);

	SendMessage(m_hwndComboEx, CBEM_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(hImageList));

	_insertItemToComboboxEx(m_hwndComboEx, NULL, 0, 0);
	_insertItemToComboboxEx(m_hwndComboEx, NULL, 1, 1);
	_insertItemToComboboxEx(m_hwndComboEx, NULL, 2, 2);
	_insertItemToComboboxEx(m_hwndComboEx, NULL, 3, 3);

	centerDialog();
	return 1; 
}

BOOL AP_Win32Dialog_Border_Shading::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
//	HWND hWndCtrl = (HWND)lParam;
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

		case AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_THICKNESS:
		{
			if (wNotifyCode == CBN_SELCHANGE)                       
			{
				int nSelected = getComboSelectedIndex (AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_THICKNESS);  

				if (nSelected != CB_ERR)
				{
					UT_Win32LocaleString thickness;
					getComboTextItem(AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_THICKNESS, nSelected, thickness);
					setBorderThickness(thickness.utf8_str().utf8_str());

					/*Force redraw*/
					InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_BORDERSHADING_BTN_BORDER_COLOR), NULL, FALSE);
					event_previewExposed();	
				}
			}
			return 1;
		}

		case AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_STYLE:            
		{
			if (wNotifyCode == CBN_SELCHANGE)                       
			{
				int nSelected = getComboSelectedIndex (AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_STYLE);

				if (nSelected != CB_ERR && nSelected >= 0 && nSelected <= BORDER_SHADING_NUMOFSTYLES)
				{
					// Kill the focus of the combobox. Because selected 
					// images of a combobox-ex doesn't look clear/good. 
					setBorderStyle(sBorderStyle_Border_Shading[nSelected]);

					SetFocus(m_hDlg); 
					event_previewExposed();	
				}
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
				m_shadingButton.setColour(cc.rgbResult);

				/*Force redraw*/
				InvalidateRect(GetDlgItem(hWnd, AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR), NULL, FALSE);
				event_previewExposed();	
			}
			return 1;
		}			

		case AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_ENABLE:
		{
			bool bChecked;			
			bChecked = (bool)(IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_ENABLE)==BST_CHECKED);							
			
			// TODO: Change this, when there are more shading patterns.
			setShadingPattern(bChecked ? BORDER_SHADING_SHADING_ENABLE : BORDER_SHADING_SHADING_DISABLE);
			setShadingEnable(bChecked);

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
					getComboTextItem(AP_RID_DIALOG_BORDERSHADING_COMBO_SHADING_OFFSET, nSelected, offset);
					setShadingOffset(offset.utf8_str().utf8_str());

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

void AP_Win32Dialog_Border_Shading::setShadingColorInGUI(const UT_RGBColor & clr)
{
	UT_DEBUGMSG(("Maleesh =============== Setup the shading color in the GUI: %d|%d|%d \n", clr.m_red, clr.m_grn, clr.m_blu));

	m_shadingButton.setColour(RGB(clr.m_red,clr.m_grn,clr.m_blu));
 	/* force redraw */
 	InvalidateRect(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR), NULL, FALSE);
}

void AP_Win32Dialog_Border_Shading::setShadingPatternInGUI(const std::string & sPattern)
{
	xxx_UT_DEBUGMSG(("Maleesh =============== Setup the shading pattern in the GUI: %s \n", sPattern.c_str()));

	// TODO: Change this, when there are more shading patterns.
	bool shading_enabled = (sPattern != BORDER_SHADING_SHADING_DISABLE);
	setShadingEnable(shading_enabled);
}

void AP_Win32Dialog_Border_Shading::setShadingOffsetInGUI(const std::string & sOffset)
{
	xxx_UT_DEBUGMSG(("Maleesh =============== Setup the shading offset in the GUI: %s \n", sOffset.c_str()));

	guint closest = _findClosestOffset(sOffset.c_str());
	SendMessageA(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_COMBO_SHADING_OFFSET), CB_SETCURSEL, WPARAM(closest), 0);
}

void AP_Win32Dialog_Border_Shading::setBorderColorInGUI(const UT_RGBColor & clr)
{
	xxx_UT_DEBUGMSG(("Maleesh =============== Setup the border color in the GUI: %d|%d|%d \n", clr.m_red, clr.m_grn, clr.m_blu));

	m_borderButton.setColour(RGB(clr.m_red,clr.m_grn,clr.m_blu));
 	/* force redraw */
 	InvalidateRect(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BTN_BORDER_COLOR), NULL, FALSE);
}

void AP_Win32Dialog_Border_Shading::setBorderThicknessInGUI(const std::string & sThick)
{
	xxx_UT_DEBUGMSG(("Maleesh =============== Setup the border thickness in the GUI: %s \n", sThick.c_str()));

	guint closest = _findClosestThickness(sThick.c_str());
	SendMessageA(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_THICKNESS), CB_SETCURSEL, WPARAM(closest), 0);
}

void AP_Win32Dialog_Border_Shading::setBorderStyleInGUI(const std::string & sStyle)
{
	xxx_UT_DEBUGMSG(("Maleesh =============== Setup the border style in the GUI: %s \n", sStyle.c_str()));

	PP_PropertyMap::TypeLineStyle style = PP_PropertyMap::linestyle_type(sStyle.c_str());
	gint index = (guint)style - 1;

	if (index >= 0)
		SendMessageA(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_COMBO_BORDER_STYLE), CB_SETCURSEL, WPARAM(index), 0);
}

void AP_Win32Dialog_Border_Shading::setShadingEnable(bool enable)
{
	CheckDlgButton(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_ENABLE, (enable ? BST_CHECKED : BST_UNCHECKED));
	m_shadingButton.setEnable(enable);
	EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_COMBO_SHADING_OFFSET), enable);
	EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR), enable);
	EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_TEXT_SHADING_COLOR), enable);
	EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_TEXT_SHADING_OFFSET), enable);

	/* force redraw */
	InvalidateRect(GetDlgItem(m_hDlg, AP_RID_DIALOG_BORDERSHADING_BTN_SHADING_COLOR), NULL, FALSE);
	event_previewExposed();	
}

void AP_Win32Dialog_Border_Shading::setSensitivity(bool /*bSens*/)
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
