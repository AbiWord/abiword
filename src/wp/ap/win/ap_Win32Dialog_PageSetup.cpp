/* AbiWord
 * Copyright (C) 2001 Mike Nordell
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
#include <commdlg.h>
#include <commctrl.h>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32OS.h"
#include "ut_Xpm2Bmp.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Win32App.h"
#include "ap_Win32Frame.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

#include "ap_Win32Dialog_PageSetup.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"

#include "orient-vertical.xpm"
#include "orient-horizontal.xpm"

#ifdef __MINGW32__
#define LPNMUPDOWN LPNM_UPDOWN
#endif

#define BUFSIZE		128
#define SIGDIGIT	4

float mScale[] = { 25.4f, 10.0f, 1.0f };

/*****************************************************************/

XAP_Dialog* AP_Win32Dialog_PageSetup::static_constructor(XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
{
	AP_Win32Dialog_PageSetup* dlg = new AP_Win32Dialog_PageSetup (pDlgFactory, id);
	return dlg;
}

AP_Win32Dialog_PageSetup::AP_Win32Dialog_PageSetup(	XAP_DialogFactory* pDlgFactory,
													XAP_Dialog_Id id)
:	AP_Dialog_PageSetup (pDlgFactory, id),
    m_PageSize(fp_PageSize::psLetter),
	m_pWin32Frame(NULL)
{
}

AP_Win32Dialog_PageSetup::~AP_Win32Dialog_PageSetup()
{
}


void AP_Win32Dialog_PageSetup::runModal(XAP_Frame *pFrame)
{
	UT_ASSERT(pFrame);

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	m_pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == AP_DIALOG_ID_FILE_PAGESETUP);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_PAGE_SETUP);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								m_pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));

}

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32Dialog_PageSetup*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_PageSetup*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))

/*****************************************************************/

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

BOOL CALLBACK AP_Win32Dialog_PageSetup::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.
	// This is the dialog procedure for the top-level dialog (that contains
	// the Close button and the Tab-control).

	AP_Win32Dialog_PageSetup * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_PageSetup *)lParam;
		SWL(hWnd,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	case WM_NOTIFY:
		pThis = GWL(hWnd);
		return pThis->_onNotify(hWnd,lParam);
		
	default:
		return 0;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// the order of the tabs

#define PAGE_INDEX		0
#define MARGINS_INDEX	1

// this little struct gets passed into s_tabProc
// it's on the stack so don't rely on it to be valid later.
typedef struct _tabParam 
{
	AP_Win32Dialog_PageSetup*	pThis;
	WORD which;
} TabParam;

// As Tabbed Dialogs have problems with HotKeys, these macros have been replaced to remove &
//#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
//#define _DSX(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))
#define _DS(c,s)  { \
                    XML_Char* p = NULL; \
                    UT_XML_cloneNoAmpersands( p, pSS->getValue(AP_STRING_ID_##s));\
                    SetDlgItemText(hWnd,AP_RID_DIALOG_##c,p); \
					FREEP(p); \
                  }
#define _DSX(c,s) { \
                    XML_Char* p = NULL; \
                    UT_XML_cloneNoAmpersands( p, pSS->getValue(XAP_STRING_ID_##s));\
                    SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,p); \
					FREEP(p); \
                  }
#define _GV(s)		(pSS->getValue(AP_STRING_ID_##s))
#define _GVX(s)		(pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_PageSetup::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles the WM_INITDIALOG message for the top-level dialog.
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	m_PageSize = getPageSize();

	// localize controls
	SetWindowText( hWnd, pSS->getValue(AP_STRING_ID_DLG_PageSetup_Title) );

	SetWindowText( GetDlgItem( hWnd, 
		                       AP_RID_DIALOG_PAGE_SETUP_BTN_OK ),
				   pSS->getValue(XAP_STRING_ID_DLG_OK) );

	SetWindowText( GetDlgItem( hWnd, 
		                       AP_RID_DIALOG_PAGE_SETUP_BTN_CANCEL  ),
				   pSS->getValue(XAP_STRING_ID_DLG_Cancel) );


	// setup the tabs
	{
		TabParam tp;
		TCITEM tie; 

		XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
		HINSTANCE hinst = pWin32App->getInstance();
		DLGTEMPLATE * pTemplate = NULL;
		HWND w = NULL;

		tp.pThis = this;

		// remember the windows we're using 
		m_hwndDlg = hWnd;
		m_hwndTab = GetDlgItem(hWnd, AP_RID_DIALOG_PAGE_SETUP_TAB);

		// add a tab for each of the child dialog boxes
    
		tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM; 
		tie.iImage = -1; 

		tie.pszText = (LPSTR) _GV(DLG_PageSetup_Page); 
		tie.lParam = AP_RID_DIALOG_PAGE_SETUP_PAGE;
		TabCtrl_InsertItem(m_hwndTab, PAGE_INDEX, &tie); 

		tie.pszText = (LPSTR) _GV(DLG_PageSetup_Margin); 
		tie.lParam = AP_RID_DIALOG_PAGE_SETUP_MARGINS;
		TabCtrl_InsertItem(m_hwndTab, MARGINS_INDEX, &tie); 

		// finally, create the (modeless) child dialogs
		
		tp.which = AP_RID_DIALOG_PAGE_SETUP_PAGE;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam( hinst, 
                                       pTemplate, 
                                       m_hwndTab, 
									   (DLGPROC)s_tabProc, 
                                       (LPARAM)&tp );
		UT_ASSERT(( w
				    && ( m_vecSubDlgHWnd.getItemCount() > 0 )
				    && ( w == m_vecSubDlgHWnd.getLastItem() ) ));

		tp.which = AP_RID_DIALOG_PAGE_SETUP_MARGINS;
		pTemplate = UT_LockDlgRes(hinst, MAKEINTRESOURCE(tp.which));
		w = CreateDialogIndirectParam( hinst, 
                                       pTemplate, 
                                       m_hwndTab, 
									   (DLGPROC)s_tabProc, 
                                       (LPARAM)&tp ); 
		UT_ASSERT(( w
				    && ( m_vecSubDlgHWnd.getItemCount() > 0 )
				    && ( w == m_vecSubDlgHWnd.getLastItem() ) ));

	}

	// Initialize the preview bitmaps
	updatePreview();

	// make sure first tab is selected.
	ShowWindow((HWND)m_vecSubDlgHWnd.getNthItem(0), SW_SHOW);
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_PageSetup::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles WM_COMMAND message for the top-level dialog.
	
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_PAGE_SETUP_BTN_OK:
		if ( validatePageSettings() ) 
		{
			setAnswer (a_OK);
			setPageSize( m_PageSize );
			EndDialog(hWnd,0);
		}
		else 
		{
			// "The margins selected are too large to fit on the page."
			m_pWin32Frame->showMessageBox(AP_STRING_ID_DLG_PageSetup_ErrBigMargins, 
									 	  XAP_Dialog_MessageBox::b_O,
									 	  XAP_Dialog_MessageBox::a_OK);
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_BTN_CANCEL:
		setAnswer( a_CANCEL );
		EndDialog(hWnd,0);
		return 0;

	default:		// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;	// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_PageSetup::_onNotify(HWND hWnd, LPARAM lParam)
{
	// This handles WM_NOTIFY messages for the top-level dialog.
	LPNMHDR pNmhdr = (LPNMHDR)lParam;

	switch (pNmhdr->code)
	{
	case TCN_SELCHANGING:
		// TODO: consider validating data before leaving page
		break;

	case TCN_SELCHANGE:
		{
			UT_uint32 iTo = TabCtrl_GetCurSel(pNmhdr->hwndFrom); 
			for (UT_uint32 k=0; k<m_vecSubDlgHWnd.getItemCount(); k++)
			{
				ShowWindow((HWND)m_vecSubDlgHWnd.getNthItem(k), ((k==iTo) ? SW_SHOW : SW_HIDE));
			}
			break;
		}
		
	// Process other notifications here
	default:
		UT_DEBUGMSG(("WM_Notify for id %ld\n",pNmhdr->code));
		break;
	} 



	return 0;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

BOOL CALLBACK AP_Win32Dialog_PageSetup::s_tabProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.
	// This is a pseudo-dialog procedure for the tab-control.

	AP_Win32Dialog_PageSetup * pThis;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			TabParam * pTP = (TabParam *) lParam;
			// from now on, we can just remember pThis 
			pThis = pTP->pThis;
			SWL(hWnd,pThis);
			return pThis->_onInitTab(hWnd,wParam,lParam);
		}
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommandTab(hWnd,wParam,lParam);

	case WM_NOTIFY:
		pThis = GWL(hWnd);
		return pThis->_onNotifyTab(hWnd,lParam);

	default:
		return 0;
	}
}

BOOL AP_Win32Dialog_PageSetup::_onInitTab(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles the WM_INITDIALOG message for the tab-control

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// position ourselves w.r.t. containing tab

	RECT r;
	GetClientRect(m_hwndTab, &r);
	TabCtrl_AdjustRect(m_hwndTab, FALSE, &r);
    SetWindowPos(hWnd, HWND_TOP, r.left, r.top, 0, 0, SWP_NOSIZE); 

	m_vecSubDlgHWnd.addItem(hWnd);
	
	TabParam * pTP = (TabParam *) lParam;
	switch( pTP->which )
	{
	// Initialize Page Tab controls
	case AP_RID_DIALOG_PAGE_SETUP_PAGE:
		{
			// Localize Controls
			_DS(PAGE_SETUP_GBX_PAPER,			DLG_PageSetup_Paper);
			_DS(PAGE_SETUP_GBX_ORIENTATION,		DLG_PageSetup_Orient);
			_DS(PAGE_SETUP_GBX_SCALE,	     	DLG_PageSetup_Scale);
			_DS(PAGE_SETUP_LBL_PAPERSIZE,		DLG_PageSetup_Paper_Size);			
			_DS(PAGE_SETUP_LBL_WITDH,			DLG_PageSetup_Width);			
			_DS(PAGE_SETUP_LBL_HEIGHT,			DLG_PageSetup_Height);			
			_DS(PAGE_SETUP_LBL_UNITS,			DLG_PageSetup_Units);			
			_DS(PAGE_SETUP_RDO_PORTRAIT,		DLG_PageSetup_Portrait);			
			_DS(PAGE_SETUP_RDO_LANDSCAPE,		DLG_PageSetup_Landscape);			
			_DS(PAGE_SETUP_LBL_ADJUSTTO,		DLG_PageSetup_Adjust);			
			_DS(PAGE_SETUP_LBL_PERCENTOFSIZE,	DLG_PageSetup_Percent);			

			// Populate Combo and List Boxes
			HWND hwndPaperSize = GetDlgItem(hWnd, AP_RID_DIALOG_PAGE_SETUP_LBX_PAPERSIZE);
			for (UT_uint32 i = (UT_uint32)fp_PageSize::_first_predefined_pagesize_; i < (UT_uint32)fp_PageSize::_last_predefined_pagesize_dont_use_; i++)
			{
				SendMessage( hwndPaperSize,
                             CB_INSERTSTRING ,
							 (WPARAM) (fp_PageSize::Predefined)i,
						     (LPARAM) fp_PageSize::PredefinedToName( (fp_PageSize::Predefined)i ) );
			}
			HWND hwndUnits = GetDlgItem(hWnd, AP_RID_DIALOG_PAGE_SETUP_LBX_UNITS);
			// NB: cannot insert string at index 1 before inserting one at index 0
			SendMessage( hwndUnits, CB_INSERTSTRING , (WPARAM) DIM_IN, (LPARAM) _GVX(DLG_Unit_inch) );                                
			SendMessage( hwndUnits, CB_INSERTSTRING , (WPARAM) DIM_CM,   (LPARAM) _GVX(DLG_Unit_cm) );                                
			SendMessage( hwndUnits, CB_INSERTSTRING , (WPARAM) DIM_MM,   (LPARAM) _GVX(DLG_Unit_mm) );                                
			
			// Initialize Data
			if( getPageOrientation() == PORTRAIT )
			{
				m_PageSize.setPortrait();
			}
			else
			{
				m_PageSize.setLandscape();
			}

			char buf[BUFSIZE];
			updateWidth();
			updateHeight();
			SetDlgItemText( hWnd,
                            AP_RID_DIALOG_PAGE_SETUP_EBX_SCALE,
                            itoa( getPageScale(), buf, 10 ) );

			CheckRadioButton( hWnd,
							  AP_RID_DIALOG_PAGE_SETUP_RDO_PORTRAIT,
							  AP_RID_DIALOG_PAGE_SETUP_RDO_LANDSCAPE,
							  ( getPageOrientation() == PORTRAIT ) ?
							    AP_RID_DIALOG_PAGE_SETUP_RDO_PORTRAIT :
							    AP_RID_DIALOG_PAGE_SETUP_RDO_LANDSCAPE );

			updatePageSize();
			
			int nUnit =  getPageUnits();
			
			SendMessage( hwndUnits, CB_SETCURSEL, (WPARAM) getPageUnits(), (LPARAM) 0 );

			// Load Appropriate XPM to BMPs
			COLORREF ColorRef = GetSysColor(COLOR_BTNFACE);
			UT_RGBColor Color( GetRValue(ColorRef), GetGValue(ColorRef), GetBValue(ColorRef));
			HDC hdc = GetDC(hWnd);
			RECT rect;
			GetClientRect(GetDlgItem(hWnd, AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION), &rect);
			UT_Xpm2Bmp( rect.right - rect.left,
                        rect.bottom - rect.top,
                        (const char**) orient_vertical_xpm,
						sizeof(orient_vertical_xpm),
						hdc,
                        &Color,
						&m_bmpPortrait);
			UT_Xpm2Bmp( rect.right - rect.left,
                        rect.bottom - rect.top,
                        (const char**) orient_horizontal_xpm,
						sizeof(orient_horizontal_xpm),
						hdc,
                        &Color,
						&m_bmpLandscape);
			GetClientRect(GetDlgItem(hWnd, AP_RID_DIALOG_PAGE_SETUP_BMP_PREVIEW), &rect);
			m_bmpPreview = CreateCompatibleBitmap( hdc, rect.right - rect.left, rect.bottom - rect.top );
			ReleaseDC( hWnd, hdc );

			if( getPageOrientation() == PORTRAIT )
			{
				SendDlgItemMessage( hWnd, 
    	                            AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION, 
        	                        STM_SETIMAGE, 
            	                    IMAGE_BITMAP, 
									(LPARAM) m_bmpPortrait );
			}
			else
			{
				SendDlgItemMessage( hWnd, 
    	                            AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION, 
        	                        STM_SETIMAGE, 
            	                    IMAGE_BITMAP, 
									(LPARAM) m_bmpLandscape );
			}

			SendDlgItemMessage( hWnd, 
				                AP_RID_DIALOG_PAGE_SETUP_BMP_PREVIEW,
				                STM_SETIMAGE, 
								IMAGE_BITMAP, 
								(LPARAM)m_bmpPreview );			
		}
		break;

	// Initialize Margin Tab controls
	case AP_RID_DIALOG_PAGE_SETUP_MARGINS:
		{
			// Localize Controls
			_DS(PAGE_SETUP_LBL_UNITS,			DLG_PageSetup_Units);			
			_DS(PAGE_SETUP_LBL_TOP,				DLG_PageSetup_Top);
			_DS(PAGE_SETUP_LBL_BOTTOM,			DLG_PageSetup_Bottom);
			_DS(PAGE_SETUP_LBL_LEFT,	     	DLG_PageSetup_Left);
			_DS(PAGE_SETUP_LBL_RIGHT,			DLG_PageSetup_Right);			
			_DS(PAGE_SETUP_LBL_HEADER,			DLG_PageSetup_Header);			
			_DS(PAGE_SETUP_LBL_FOOTER,			DLG_PageSetup_Footer);			
			HWND hwndMarginUnits = GetDlgItem(hWnd, AP_RID_DIALOG_PAGE_SETUP_LBX_MARGINUNITS);
			SendMessage( hwndMarginUnits, CB_INSERTSTRING , (WPARAM) DIM_IN, (LPARAM) _GVX(DLG_Unit_inch) );                                
			SendMessage( hwndMarginUnits, CB_INSERTSTRING , (WPARAM) DIM_CM,   (LPARAM) _GVX(DLG_Unit_cm) );                                
			SendMessage( hwndMarginUnits, CB_INSERTSTRING , (WPARAM) DIM_MM,   (LPARAM) _GVX(DLG_Unit_mm) );                                

			// Initialize Data
			SendMessage( hwndMarginUnits, CB_SETCURSEL, (WPARAM) getMarginUnits(), (LPARAM) 0 );
			updateMargins();
		}
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return 1;		// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_PageSetup::_onCommandTab(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles WM_COMMAND message for all of the sub-dialogs.
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	// PAGE TAB
	case AP_RID_DIALOG_PAGE_SETUP_LBX_PAPERSIZE:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			UT_sint32 previous = (UT_sint32) fp_PageSize::NameToPredefined( m_PageSize.getPredefinedName()  );
			UT_sint32 selected = SendMessage( hWndCtrl, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0 );
			if( selected != previous )
			{
				m_PageSize.Set( (fp_PageSize::Predefined)selected );
				if( getPageUnits() != m_PageSize.getDims() )
				{
					SendDlgItemMessage( hWnd,
								 		AP_RID_DIALOG_PAGE_SETUP_LBX_UNITS,
								 		CB_SETCURSEL, 
								 		(WPARAM) m_PageSize.getDims(),
							 			(LPARAM) 0);
					setPageUnits( m_PageSize.getDims() );
				}
				updateWidth();
				updateHeight();
				updatePreview();
			}
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_LBX_UNITS:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			UT_Dimension unit = (UT_Dimension)SendMessage( hWndCtrl, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0 );
			if( unit != getPageUnits() )
			{
				setPageUnits( unit );
				updateWidth();
				updateHeight();
				updatePreview();
			}
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_WIDTH:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof(buf) >= 0.0 && atof(buf) != m_PageSize.Width(getPageUnits()) )
			{
				m_PageSize.Set( atof(buf),
					            m_PageSize.Height(getPageUnits()),
								getPageUnits() );
				updatePageSize();
				updatePreview();

			}
			updateWidth();		
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_HEIGHT:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof(buf) >= 0.0 && atof(buf) != m_PageSize.Height(getPageUnits()) )
			{
				m_PageSize.Set( m_PageSize.Width(getPageUnits()), 
								atof(buf),
								getPageUnits() );
				updatePageSize();
				updatePreview();
			}
			updateHeight();
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_SCALE:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atoi(buf) >= 0.0 && atoi(buf) != getPageScale() )
			{
				setPageScale( atoi(buf) );

			}
			SetDlgItemText( hWnd, wId, itoa( getPageScale() , buf, 10 ) );

		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_RDO_PORTRAIT:
		if( getPageOrientation() != PORTRAIT )
		{
			setPageOrientation( PORTRAIT );
			m_PageSize.setPortrait();
			SendDlgItemMessage( hWnd, 
   	                            AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION, 
       	                        STM_SETIMAGE, 
           	                    IMAGE_BITMAP, 
								(LPARAM) m_bmpPortrait );
			updateWidth();
			updateHeight();
			updatePreview();
		}
		return 0;
		
	case AP_RID_DIALOG_PAGE_SETUP_RDO_LANDSCAPE:
		if( getPageOrientation() != LANDSCAPE )
		{
			setPageOrientation( LANDSCAPE );
			m_PageSize.setLandscape();
			SendDlgItemMessage( hWnd, 
   	                            AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION, 
       	                        STM_SETIMAGE, 
           	                    IMAGE_BITMAP, 
								(LPARAM) m_bmpLandscape );
			updateWidth();
			updateHeight();
			updatePreview();
		}
		return 0;


	// MARGINS TAB	
	case AP_RID_DIALOG_PAGE_SETUP_LBX_MARGINUNITS:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			UT_sint32 selected = SendMessage( hWndCtrl, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0 );
			if( getMarginUnits() != (UT_Dimension) selected )
			{
				setMarginTop(    getMarginTop()   * mScale[getMarginUnits()] / mScale[selected] );
				setMarginBottom( getMarginBottom()* mScale[getMarginUnits()] / mScale[selected] );
				setMarginLeft(   getMarginLeft()  * mScale[getMarginUnits()] / mScale[selected] );
				setMarginRight(  getMarginRight() * mScale[getMarginUnits()] / mScale[selected] );
				setMarginHeader( getMarginHeader()* mScale[getMarginUnits()] / mScale[selected] );
				setMarginFooter( getMarginFooter()* mScale[getMarginUnits()] / mScale[selected] );
				updateMargins();
				setMarginUnits( (UT_Dimension) selected );
			}
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_TOP:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof( buf ) > 0 && atof(buf) != (double) getMarginTop() )
			{
				setMarginTop( (float) atof(buf) );
				updatePreview();
			}
			updateTopMargin();
		}
		return 0;


	case AP_RID_DIALOG_PAGE_SETUP_EBX_BOTTOM:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof( buf ) > 0 && atof(buf) != (double) getMarginBottom() )
			{
				setMarginBottom( (float) atof(buf) );
				updatePreview();
			}
			updateBottomMargin();
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_LEFT:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof( buf ) > 0 && atof(buf) != (double) getMarginLeft() )
			{
				setMarginLeft( (float) atof(buf) );
				updatePreview();
			}
			updateLeftMargin();
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_RIGHT:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof( buf ) > 0 && atof(buf) != (double) getMarginRight() )
			{
				setMarginRight( (float) atof(buf) );
				updatePreview();
			}
			updateRightMargin();
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_HEADER:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof( buf ) > 0 && atof(buf) != (double) getMarginHeader() )
			{
				setMarginHeader( (float) atof(buf) );
				updatePreview();
			}
			updateHeaderMargin();
		}
		return 0;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_FOOTER:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof( buf ) > 0 && atof(buf) != (double) getMarginFooter() )
			{
				setMarginFooter( (float) atof(buf) );
				updatePreview();
			}
			updateFooterMargin();
		}
		return 0;

	default:
		UT_DEBUGMSG(("WM_Command for id %ld for sub-dialog\n",wId));
		return 0;
	}
}

BOOL AP_Win32Dialog_PageSetup::_onNotifyTab(HWND hWnd, LPARAM lParam)
{
	// This handles WM_NOTIFY message for all of the sub-dialogs.
	LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;

	switch( lpnmud->hdr.code )
	{
	case UDN_DELTAPOS:
		doSpinControl( lpnmud->hdr.idFrom, -lpnmud->iDelta );
		return 0;
	default:
		UT_DEBUGMSG(("WM_Nofify for id %ld for sub-dialog\n", lpnmud->hdr.code));
		return 0;
	}
}

void AP_Win32Dialog_PageSetup::doSpinControl(UT_uint32 id, UT_sint32 delta)
{
	char buf[BUFSIZE];
	int updatedData =  0;
	int pageScale   = ( getPageUnits()   == DIM_MM ) ? 1 : 10;
	int marginScale = ( getMarginUnits() == DIM_MM ) ? 1 : 10;

	switch( id )
	{
	case AP_RID_DIALOG_PAGE_SETUP_SPN_WIDTH:
		updatedData = (int)( m_PageSize.Width(getPageUnits()) * pageScale + delta + 0.05f );
		if( updatedData >= 0 )
		{
			m_PageSize.Set( (double) updatedData/pageScale,
				             m_PageSize.Height(getPageUnits()),
							 getPageUnits() );
			updatePageSize();
			updateWidth();
			updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_HEIGHT:
		updatedData = (int)( m_PageSize.Height(getPageUnits()) * pageScale + delta + 0.05f );
		if( updatedData >= 0 )
		{
			m_PageSize.Set( m_PageSize.Width(getPageUnits()),
							(double) updatedData/pageScale,
							getPageUnits() );
			updatePageSize();
			updateHeight();
			updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_TOP:
		updatedData = (int)( getMarginTop()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			setMarginTop( (float) updatedData/marginScale );
			updateTopMargin();
			updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_BOTTOM:
		updatedData = (int)( getMarginBottom()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			setMarginBottom( (float) updatedData/marginScale );
			updateBottomMargin();
			updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_LEFT:
		updatedData = (int)( getMarginLeft()*marginScale + delta + 0.05f );
		if( updatedData >= 0 )
		{
			setMarginLeft( (float) updatedData/marginScale );
			updateLeftMargin();
			updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_RIGHT:
		updatedData = (int)( getMarginRight()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			setMarginRight( (float) updatedData/marginScale );
			updateRightMargin();
			updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_HEADER:
		updatedData = (int)( getMarginHeader()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			setMarginHeader( (float) updatedData/marginScale );
			updateHeaderMargin();
			updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_FOOTER:
		updatedData = (int)( getMarginFooter()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			setMarginFooter( (float) updatedData/marginScale );
			updateFooterMargin();
			updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_SCALE:
		updatedData = getPageScale() + delta;
		if( updatedData >= 0 )
		{
			setPageScale( updatedData);
			SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(PAGE_INDEX),
		                    AP_RID_DIALOG_PAGE_SETUP_EBX_SCALE,
			                itoa( getPageScale() , buf, 10 ) );
		}
		break;

	default:
		break;
	}
}


void AP_Win32Dialog_PageSetup::updatePageSize()
{
	SendDlgItemMessage( (HWND)m_vecSubDlgHWnd.getNthItem(PAGE_INDEX),
						AP_RID_DIALOG_PAGE_SETUP_LBX_PAPERSIZE,
						CB_SETCURSEL,
						(WPARAM) fp_PageSize::NameToPredefined( m_PageSize.getPredefinedName()  ),
						(LPARAM) 0 );
}

void AP_Win32Dialog_PageSetup::updateWidth()
{
	char buf[BUFSIZE];
	SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(PAGE_INDEX),
   	                AP_RID_DIALOG_PAGE_SETUP_EBX_WIDTH,
       	            gcvt( m_PageSize.Width(getPageUnits()), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateHeight()
{
	char buf[BUFSIZE];
	SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(PAGE_INDEX),
   	                AP_RID_DIALOG_PAGE_SETUP_EBX_HEIGHT,
       	            gcvt( m_PageSize.Height(getPageUnits()), SIGDIGIT, buf ) );
}


void AP_Win32Dialog_PageSetup::updateMargins()
{
	updateTopMargin();
	updateBottomMargin();
	updateLeftMargin();
	updateRightMargin();
	updateHeaderMargin();
	updateFooterMargin();
}

void AP_Win32Dialog_PageSetup::updateTopMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(MARGINS_INDEX),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_TOP,
                    gcvt( (double)getMarginTop(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateBottomMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(MARGINS_INDEX),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_BOTTOM,
                    gcvt( (double)getMarginBottom(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateLeftMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(MARGINS_INDEX),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_LEFT,
                    gcvt( (double)getMarginLeft(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateRightMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(MARGINS_INDEX),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_RIGHT,
                    gcvt( (double)getMarginRight(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateHeaderMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(MARGINS_INDEX),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_HEADER,
                    gcvt( (double)getMarginHeader(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateFooterMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( (HWND)m_vecSubDlgHWnd.getNthItem(MARGINS_INDEX),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_FOOTER,
                    gcvt( (double)getMarginFooter(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updatePreview()
{
	UT_uint16 offset = 5;

	RECT rectBorder;
	RECT rectPage;
	RECT rectShadow;
	RECT rectMargin;

	COLORREF colorBackground = GetSysColor(COLOR_BTNFACE);
	COLORREF colorWhite(0x00FFFFFF);
	COLORREF colorBlack(0x00000000);
	HBRUSH brushBackground = CreateSolidBrush( colorBackground );
	HBRUSH brushWhite = CreateSolidBrush( colorWhite );
	HBRUSH brushBlack = CreateSolidBrush( colorBlack );

	HWND hwndPreview = GetDlgItem( (HWND)m_vecSubDlgHWnd.getNthItem(PAGE_INDEX),
		                            AP_RID_DIALOG_PAGE_SETUP_BMP_PREVIEW );

	HWND hwndMargin = GetDlgItem( (HWND)m_vecSubDlgHWnd.getNthItem(MARGINS_INDEX),
								  AP_RID_DIALOG_PAGE_SETUP_BMP_MARGINPREVIEW );
	// Calculate Rectangles
	GetClientRect(hwndPreview, &rectBorder);
	
	UT_uint16 borderWidth  = rectBorder.right  - rectBorder.left - offset;
	UT_uint16 borderHeight = rectBorder.bottom - rectBorder.top  - offset;

	UT_uint16 pageWidth  = (UT_uint16) m_PageSize.Width( DIM_MM );  // in mm
	UT_uint16 pageHeight = (UT_uint16) m_PageSize.Height( DIM_MM ); // in mm
	UT_uint16 marginTop  = (UT_uint16) (getMarginTop() * mScale[getMarginUnits()] );
	UT_uint16 marginLeft = (UT_uint16) (getMarginLeft() * mScale[getMarginUnits()] );
	UT_uint16 marginRight= (UT_uint16) (getMarginRight() * mScale[getMarginUnits()] );
	UT_uint16 marginBottom  = (UT_uint16) (getMarginBottom() * mScale[getMarginUnits()] );

	double scale = ( (double)pageWidth/borderWidth > (double)pageHeight/borderHeight ) ?
					(double)pageWidth / borderWidth :
					(double)pageHeight / borderHeight ;

	rectPage.left   = rectBorder.left   + ( borderWidth  - (int)(pageWidth/scale) ) / 2;
	rectPage.right  = rectBorder.right  - ( borderWidth  - (int)(pageWidth/scale) ) / 2 - offset;
	rectPage.top    = rectBorder.top    + ( borderHeight - (int)(pageHeight/scale) ) / 2;
	rectPage.bottom = rectBorder.bottom - ( borderHeight - (int)(pageHeight/scale) ) / 2 - offset;

	CopyRect( &rectShadow, &rectPage );
	OffsetRect( &rectShadow, offset, offset );

	rectMargin.left   = rectPage.left  + (int)(marginLeft/scale);
	rectMargin.right  = rectPage.right - (int)(marginRight/scale);
	rectMargin.top    = rectPage.top   + (int)(marginTop/scale);
	rectMargin.bottom = rectPage.bottom - (int)(marginBottom/scale);

	HDC hDC = CreateCompatibleDC(NULL);

	SelectObject( hDC, m_bmpPreview );
	FillRect( hDC, &rectBorder, brushBackground );
	FillRect( hDC, &rectShadow, brushBlack );
	FillRect( hDC, &rectPage, brushWhite );
	FrameRect( hDC, &rectMargin, brushBlack );

	SendMessage( hwndPreview, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_bmpPreview );
	InvalidateRgn( hwndPreview, NULL, FALSE );

	if( hwndMargin )
	{
		SendMessage( hwndMargin,  STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_bmpPreview );
		InvalidateRgn( hwndMargin,  NULL, FALSE );
	}
	
	DeleteDC( hDC );
}

