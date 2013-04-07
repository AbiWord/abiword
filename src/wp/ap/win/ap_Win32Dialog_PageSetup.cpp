/* AbiWord
 * Copyright (C) 2001 Mike Nordell
 * Copyright (C) 2004 Mikey Cooper (mikey@bluey.com)
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
#include <math.h>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Xpm2Bmp.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

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
#define FMT_STRING "%0.2f"
#define EPSILON 1.0e-7

float mScale[] = { 25.4f, 10.0f, 1.0f };


XAP_Dialog* AP_Win32Dialog_PageSetup::static_constructor(XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
{
	AP_Win32Dialog_PageSetup* dlg = new AP_Win32Dialog_PageSetup (pDlgFactory, id);
	return dlg;
}

AP_Win32Dialog_PageSetup::AP_Win32Dialog_PageSetup(	XAP_DialogFactory* pDlgFactory,
													XAP_Dialog_Id id)
:	AP_Dialog_PageSetup (pDlgFactory, id),m_PageSize(fp_PageSize::psLetter)
{
}

AP_Win32Dialog_PageSetup::~AP_Win32Dialog_PageSetup()
{
}


void AP_Win32Dialog_PageSetup::runModal(XAP_Frame *pFrame)
{
	UT_return_if_fail (pFrame);

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	m_pFrame = pFrame;
	AP_Win32Dialog_PageSetup_Sheet	sheet;


	/* Create the property sheet and associate its pages*/
	m_page.setContainer(this);
	m_page.createPage(pWin32App, AP_RID_DIALOG_PAGE_SETUP_PAGE, AP_STRING_ID_DLG_PageSetup_Page);
	sheet.addPage(&m_page);

	m_margin.setContainer(this);
	m_margin.createPage(pWin32App, AP_RID_DIALOG_PAGE_SETUP_MARGINS, AP_STRING_ID_DLG_PageSetup_Margin);
	sheet.addPage(&m_margin);

	sheet.setParent(this);

	if (sheet.runModal(pWin32App, pFrame, AP_STRING_ID_DLG_PageSetup_Title)==IDOK)	
		m_answer = a_OK;
	else		
		m_answer = a_CANCEL;
}

#define _DS(c,s)	setDlgItemText(getHandle(),AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _GVX(s)		(pSS->getValue(XAP_STRING_ID_##s))

/*
	Sheet
*/
AP_Win32Dialog_PageSetup_Sheet::AP_Win32Dialog_PageSetup_Sheet() :
XAP_Win32PropertySheet()
{
	m_pParent = NULL;
	setCallBack((PFNPROPSHEETCALLBACK)s_sheetInit);
}

/*
	Sheet window procedure
*/
int AP_Win32Dialog_PageSetup_Sheet::_onCommand(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wID = LOWORD(wParam); 
	
	if (wID==IDOK)
	{
		if ( m_pParent->validatePageSettings() )
		{
			m_pParent->setPageSize( m_pParent->m_PageSize );
		}
		else
		{
			// "The margins selected are too large to fit on the page."
			m_pParent->m_pFrame->showMessageBox(AP_STRING_ID_DLG_PageSetup_ErrBigMargins,
												XAP_Dialog_MessageBox::b_O,
												XAP_Dialog_MessageBox::a_OK);
			return 0;
		}
		return 1;
	}
	
	return 1;	// The application did not process the message
}

INT_PTR CALLBACK AP_Win32Dialog_PageSetup_Sheet::s_sheetInit(HWND hwnd,  UINT uMsg,  LPARAM /*lParam*/)
{	
	if (uMsg==PSCB_INITIALIZED)
	{	
		/* Force the creation of all pages*/
		PropSheet_SetCurSel(hwnd, 0,0);
		PropSheet_SetCurSel(hwnd, 0,1);
	}			
	return 	0;
}

void AP_Win32Dialog_PageSetup_Sheet::_onInitDialog(HWND hwnd)
{		
	const XAP_StringSet * pSS = getParent()->getApp()->getStringSet();	
	setWindowText(GetDlgItem(hwnd, IDOK), pSS->getValue(XAP_STRING_ID_DLG_OK));
	setWindowText(GetDlgItem(hwnd, IDCANCEL), pSS->getValue(XAP_STRING_ID_DLG_Cancel));	

	PropSheet_SetCurSel(hwnd, 0, 0);

	// Initialize Dialog Data
	m_pParent->updateMargins();
	m_pParent->updatePreview();
}

/*

	Page page
	
*/
AP_Win32Dialog_PageSetup_Page::AP_Win32Dialog_PageSetup_Page()
{
	setDialogProc(s_pageWndProc);	
	m_nCentered = 0;
}

AP_Win32Dialog_PageSetup_Page::~AP_Win32Dialog_PageSetup_Page()
{

}

INT_PTR CALLBACK AP_Win32Dialog_PageSetup_Page::s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
	if (msg==WM_NOTIFY)
	{
		AP_Win32Dialog_PageSetup_Page *pThis = (AP_Win32Dialog_PageSetup_Page *) GetWindowLongPtrW(hWnd, GWLP_USERDATA);					

		NMHDR* pHdr = (NMHDR*)lParam;
		LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;

		if ( lpnmud->hdr.code == UDN_DELTAPOS )
		{
			pThis->doSpinControl(lpnmud->hdr.idFrom, -lpnmud->iDelta);
		}

		if (pHdr->code==PSN_SETACTIVE)			
		{
			if (pThis->m_nCentered<2)
			{
			   	pThis->m_nCentered++;
				XAP_Win32DialogHelper::s_centerDialog(GetParent(hWnd));			
			}
		}
	}   	
	
	return XAP_Win32PropertyPage::s_pageWndProc(hWnd, msg, wParam,lParam);
}

void AP_Win32Dialog_PageSetup_Page::doSpinControl(UT_uint32 id, UT_sint32 delta)
{
	char buf[BUFSIZE];
	int updatedData =  0;
	int pageScale   = ( m_pParent->m_PageSize.getDims() == DIM_MM ) ? 1 : 10;
	switch( id )
	{
	case AP_RID_DIALOG_PAGE_SETUP_SPN_WIDTH:
		updatedData = (int)( m_pParent->m_PageSize.Width(m_pParent->m_PageSize.getDims()) * pageScale + delta + 0.05f );
		if( updatedData >= 0 )
		{
			m_pParent->m_PageSize.Set( (m_pParent->getPageOrientation() == m_pParent->PORTRAIT) ? (double) updatedData / (double) pageScale : m_pParent->m_PageSize.Height(m_pParent->m_PageSize.getDims()),
							(m_pParent->getPageOrientation() == m_pParent->PORTRAIT) ? m_pParent->m_PageSize.Height(m_pParent->m_PageSize.getDims()) : (double) updatedData / (double) pageScale,
							m_pParent->m_PageSize.getDims() );
			m_pParent->updatePageSize();
			m_pParent->updateWidth();
			m_pParent->updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_HEIGHT:
		updatedData = (int)( m_pParent->m_PageSize.Height(m_pParent->m_PageSize.getDims()) * pageScale + delta + 0.05f );
		if( updatedData >= 0 )
		{
			m_pParent->m_PageSize.Set( (m_pParent->getPageOrientation() == m_pParent->PORTRAIT) ? m_pParent->m_PageSize.Width(m_pParent->m_PageSize.getDims()) : (double) updatedData / (double) pageScale,
							(m_pParent->getPageOrientation() == m_pParent->PORTRAIT) ? (double) updatedData / (double) pageScale : m_pParent->m_PageSize.Width(m_pParent->m_PageSize.getDims()),
							m_pParent->m_PageSize.getDims() );
			m_pParent->updatePageSize();
			m_pParent->updateHeight();
			m_pParent->updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_SCALE:
		updatedData = m_pParent->getPageScale() + delta;
		if( updatedData >= 0 )
		{
			m_pParent->setPageScale( updatedData);
			SetDlgItemText( getHandle(),
		                    AP_RID_DIALOG_PAGE_SETUP_EBX_SCALE,
			                itoa( m_pParent->getPageScale() , buf, 10 ) );
		}
		break;

	default:
		break;
	}
}

BOOL AP_Win32Dialog_PageSetup_Page::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;	

	switch (wId)
	{
	case AP_RID_DIALOG_PAGE_SETUP_LBX_PAPERSIZE:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			UT_sint32 previous = (UT_sint32) fp_PageSize::NameToPredefined( m_pParent->m_PageSize.getPredefinedName()  );
			UT_sint32 selected = SendMessage( hWndCtrl, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0 );
			if( selected != previous )
			{
				m_pParent->m_PageSize.Set( (fp_PageSize::Predefined)selected );
#if 0
				// let user select units she wants, see bug 8118
				if( m_pParent->getPageUnits() != m_pParent->m_PageSize.getDims() )
				{
					SendDlgItemMessage( hWnd,
								 		AP_RID_DIALOG_PAGE_SETUP_LBX_UNITS,
								 		CB_SETCURSEL, 
								 		(WPARAM) m_pParent->m_PageSize.getDims(),
							 			(LPARAM) 0);
					m_pParent->setPageUnits( m_pParent->m_PageSize.getDims() );
				}
#endif
				m_pParent->updateWidth();
				m_pParent->updateHeight();
				m_pParent->updatePreview();
			}
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_LBX_UNITS:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			UT_Dimension unit = (UT_Dimension)SendMessage( hWndCtrl, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0 ); 
			if( unit != m_pParent->m_PageSize.getDims() )
			{
				m_pParent->updateWidth();
				m_pParent->updateHeight();
				m_pParent->updatePreview();
			}
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_WIDTH:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof(buf) >= 0.0 && atof(buf) != m_pParent->m_PageSize.Width(m_pParent->m_PageSize.getDims()) )
			{
				m_pParent->m_PageSize.Set( (m_pParent->getPageOrientation() == m_pParent->PORTRAIT) ? atof(buf) : m_pParent->m_PageSize.Height(m_pParent->m_PageSize.getDims()),
								(m_pParent->getPageOrientation() == m_pParent->PORTRAIT) ? m_pParent->m_PageSize.Height(m_pParent->m_PageSize.getDims()) : atof(buf),
								m_pParent->m_PageSize.getDims() );
				m_pParent->updatePageSize();
				m_pParent->updatePreview();

			}
			m_pParent->updateWidth();		
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_HEIGHT:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( atof(buf) >= 0.0 && atof(buf) != m_pParent->m_PageSize.Height(m_pParent->m_PageSize.getDims()) )
			{
				m_pParent->m_PageSize.Set( (m_pParent->getPageOrientation() == m_pParent->PORTRAIT) ? m_pParent->m_PageSize.Width(m_pParent->m_PageSize.getDims()) : atof(buf), 
								(m_pParent->getPageOrientation() == m_pParent->PORTRAIT) ? atof(buf) : m_pParent->m_PageSize.Width(m_pParent->m_PageSize.getDims()),
								m_pParent->m_PageSize.getDims() );
				m_pParent->updatePageSize();
				m_pParent->updatePreview();
			}
			m_pParent->updateHeight();
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_SCALE:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( (atoi(buf) >= 1) && (atoi(buf) <= 1000) && (atoi(buf) != m_pParent->getPageScale()) )
			{
				m_pParent->setPageScale( atoi(buf) );

			}
			SetDlgItemText( hWnd, wId, itoa( m_pParent->getPageScale() , buf, 10 ) );

		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_RDO_PORTRAIT:
		if( m_pParent->getPageOrientation() != m_pParent->PORTRAIT )
		{
			m_pParent->setPageOrientation( m_pParent->PORTRAIT );
			m_pParent->m_PageSize.setPortrait();
			SendDlgItemMessage( hWnd, 
   	                            AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION, 
       	                        STM_SETIMAGE, 
           	                    IMAGE_BITMAP, 
								(LPARAM) m_pParent->m_bmpPortrait );
			m_pParent->updateWidth();
			m_pParent->updateHeight();
			m_pParent->updatePreview();
		}
		return TRUE;
		
	case AP_RID_DIALOG_PAGE_SETUP_RDO_LANDSCAPE:
		if( m_pParent->getPageOrientation() != m_pParent->LANDSCAPE )
		{
			m_pParent->setPageOrientation( m_pParent->LANDSCAPE );
			m_pParent->m_PageSize.setLandscape();
			SendDlgItemMessage( hWnd, 
   	                            AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION, 
       	                        STM_SETIMAGE, 
           	                    IMAGE_BITMAP, 
								(LPARAM) m_pParent->m_bmpLandscape );
			m_pParent->updateWidth();
			m_pParent->updateHeight();
			m_pParent->updatePreview();
		}
		return TRUE;

	default:
		UT_DEBUGMSG(("WM_Command for id %ld for Page sub-dialog\n",wId));
		return TRUE;
	}
	return FALSE;
}

void AP_Win32Dialog_PageSetup_Page::_onInitDialog()
{				
	const XAP_StringSet * pSS = getApp()->getStringSet();
	
	// Initialize Page page's dialog items
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

	// Populate Paper Size combo box
	for (UT_uint32 i = (UT_uint32)fp_PageSize::_first_predefined_pagesize_; i < (UT_uint32)fp_PageSize::_last_predefined_pagesize_dont_use_; i++)
	{
        addItemToCombo (AP_RID_DIALOG_PAGE_SETUP_LBX_PAPERSIZE, pSS->getValue(fp_PageSize::PredefinedToLocalName((fp_PageSize::Predefined) i)));
	}

	// Populate Units combo box
	// NB: cannot insert string at index 1 before inserting one at index 0
    addItemToCombo (AP_RID_DIALOG_PAGE_SETUP_LBX_UNITS, _GVX(DLG_Unit_inch));
    addItemToCombo (AP_RID_DIALOG_PAGE_SETUP_LBX_UNITS, _GVX(DLG_Unit_cm));
    addItemToCombo (AP_RID_DIALOG_PAGE_SETUP_LBX_UNITS, _GVX(DLG_Unit_mm));
  			
	m_pParent->m_PageSize = m_pParent->getPageSize();
	if( m_pParent->getPageOrientation() == m_pParent->PORTRAIT )
	{
		m_pParent->m_PageSize.setPortrait();
	}
	else
	{
		m_pParent->m_PageSize.setLandscape();
	}

	char buf[BUFSIZE];
	m_pParent->updateWidth();
	m_pParent->updateHeight();
	SetDlgItemText( getHandle(),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_SCALE,
                    itoa( m_pParent->getPageScale(), buf, 10 ) );

	CheckRadioButton( getHandle(),
					AP_RID_DIALOG_PAGE_SETUP_RDO_PORTRAIT,
					AP_RID_DIALOG_PAGE_SETUP_RDO_LANDSCAPE,
					( m_pParent->getPageOrientation() == m_pParent->PORTRAIT ) ?
					AP_RID_DIALOG_PAGE_SETUP_RDO_PORTRAIT :
					AP_RID_DIALOG_PAGE_SETUP_RDO_LANDSCAPE );

	m_pParent->updatePageSize();

	int nUnit =  m_pParent->m_PageSize.getDims();
	selectComboItem (AP_RID_DIALOG_PAGE_SETUP_LBX_UNITS, (WPARAM) nUnit );                 

	// Load Appropriate XPM to BMPs
	COLORREF ColorRef = GetSysColor(COLOR_BTNFACE);
	UT_RGBColor Color( GetRValue(ColorRef), GetGValue(ColorRef), GetBValue(ColorRef));
	HDC hdc = GetDC(getHandle());
	RECT rect;
	GetClientRect(GetDlgItem(getHandle(), AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION), &rect);
	UT_Xpm2Bmp( rect.right - rect.left,
                rect.bottom - rect.top,
                (const char**) orient_vertical_xpm,
				sizeof(orient_vertical_xpm),
				hdc,
                &Color,
				&m_pParent->m_bmpPortrait);
	UT_Xpm2Bmp( rect.right - rect.left,
                rect.bottom - rect.top,
                (const char**) orient_horizontal_xpm,
				sizeof(orient_horizontal_xpm),
				hdc,
                &Color,
				&m_pParent->m_bmpLandscape);
	GetClientRect(GetDlgItem(getHandle(), AP_RID_DIALOG_PAGE_SETUP_BMP_PREVIEW), &rect);
	m_pParent->m_bmpPreview = CreateCompatibleBitmap( hdc, rect.right - rect.left, rect.bottom - rect.top );
	ReleaseDC( getHandle(), hdc );

	if( m_pParent->getPageOrientation() == m_pParent->PORTRAIT )
	{
		SendDlgItemMessage( getHandle(), 
    	                    AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION, 
        	                STM_SETIMAGE, 
            	            IMAGE_BITMAP, 
							(LPARAM) m_pParent->m_bmpPortrait );
	}
	else
	{
		SendDlgItemMessage( getHandle(), 
    	                    AP_RID_DIALOG_PAGE_SETUP_BMP_ORIENTATION, 
        	                STM_SETIMAGE, 
            	            IMAGE_BITMAP, 
							(LPARAM) m_pParent->m_bmpLandscape );
	}

	SendDlgItemMessage( getHandle(), 
				        AP_RID_DIALOG_PAGE_SETUP_BMP_PREVIEW,
				        STM_SETIMAGE, 
						IMAGE_BITMAP, 
						(LPARAM)m_pParent->m_bmpPreview );			


	SetWindowLongPtrW(getHandle(), GWLP_USERDATA, (LONG_PTR)this);
}

/*

	Margin page
	
*/
AP_Win32Dialog_PageSetup_Margin::AP_Win32Dialog_PageSetup_Margin()
{
	setDialogProc(s_pageWndProc);	
}

AP_Win32Dialog_PageSetup_Margin::~AP_Win32Dialog_PageSetup_Margin()
{

}

BOOL AP_Win32Dialog_PageSetup_Margin::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;	

	switch (wId)
	{
	case AP_RID_DIALOG_PAGE_SETUP_LBX_MARGINUNITS:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			UT_sint32 selected = SendMessage( hWndCtrl, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0 );
			if( m_pParent->getMarginUnits() != (UT_Dimension) selected )
			{
				m_pParent->setMarginTop(    m_pParent->getMarginTop()   * mScale[m_pParent->getMarginUnits()] / mScale[selected] );
				m_pParent->setMarginBottom( m_pParent->getMarginBottom()* mScale[m_pParent->getMarginUnits()] / mScale[selected] );
				m_pParent->setMarginLeft(   m_pParent->getMarginLeft()  * mScale[m_pParent->getMarginUnits()] / mScale[selected] );
				m_pParent->setMarginRight(  m_pParent->getMarginRight() * mScale[m_pParent->getMarginUnits()] / mScale[selected] );
				m_pParent->setMarginHeader( m_pParent->getMarginHeader()* mScale[m_pParent->getMarginUnits()] / mScale[selected] );
				m_pParent->setMarginFooter( m_pParent->getMarginFooter()* mScale[m_pParent->getMarginUnits()] / mScale[selected] );
				
				m_pParent->updateMargins();
				m_pParent->setMarginUnits( (UT_Dimension) selected );
			}
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_TOP:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( (atof( buf ) > -EPSILON) && (fabs(atof(buf) - (double) m_pParent->getMarginTop()) > EPSILON ) )
			{
				m_pParent->setMarginTop( (float) atof(buf) );
				m_pParent->updatePreview();
			}
			m_pParent->updateTopMargin();
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_BOTTOM:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( (atof( buf ) > -EPSILON) && (fabs(atof(buf) - (double) m_pParent->getMarginBottom()) > EPSILON ) )
			{
				m_pParent->setMarginBottom( (float) atof(buf) );
				m_pParent->updatePreview();
			}
			m_pParent->updateBottomMargin();
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_LEFT:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( (atof( buf ) > -EPSILON) && (fabs(atof(buf) - (double) m_pParent->getMarginLeft()) > EPSILON ) )
			{
				m_pParent->setMarginLeft( (float) atof(buf) );
				m_pParent->updatePreview();
			}
			m_pParent->updateLeftMargin();
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_RIGHT:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( (atof( buf ) > -EPSILON) && (fabs(atof(buf) - (double) m_pParent->getMarginRight()) > EPSILON ) )
			{
				m_pParent->setMarginRight( (float) atof(buf) );
				m_pParent->updatePreview();
			}
			m_pParent->updateRightMargin();
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_HEADER:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( (atof( buf ) > -EPSILON) && (fabs(atof(buf) - (double) m_pParent->getMarginHeader()) > EPSILON ) )
			{
				m_pParent->setMarginHeader( (float) atof(buf) );
				m_pParent->updatePreview();
			}
			m_pParent->updateHeaderMargin();
		}
		return TRUE;

	case AP_RID_DIALOG_PAGE_SETUP_EBX_FOOTER:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemText( hWnd, wId, buf, BUFSIZE );
			if( (atof( buf ) > -EPSILON) && (fabs(atof(buf) - (double) m_pParent->getMarginFooter()) > EPSILON ) )
			{
				m_pParent->setMarginFooter( (float) atof(buf) );
				m_pParent->updatePreview();
			}
			m_pParent->updateFooterMargin();
		}
		return TRUE;

	default:
		UT_DEBUGMSG(("WM_Command for id %ld for Page sub-dialog\n",wId));
		return TRUE;
	}
	return FALSE;
}

void AP_Win32Dialog_PageSetup_Margin::_onInitDialog()
{
		const XAP_StringSet * pSS = getApp()->getStringSet();	
	
		// Localize Controls
		_DS(PAGE_SETUP_LBL_UNITS,			DLG_PageSetup_Units);			
		_DS(PAGE_SETUP_LBL_TOP,				DLG_PageSetup_Top);
		_DS(PAGE_SETUP_LBL_BOTTOM,			DLG_PageSetup_Bottom);
		_DS(PAGE_SETUP_LBL_LEFT,	     	DLG_PageSetup_Left);
		_DS(PAGE_SETUP_LBL_RIGHT,			DLG_PageSetup_Right);			
		_DS(PAGE_SETUP_LBL_HEADER,			DLG_PageSetup_Header);			
		_DS(PAGE_SETUP_LBL_FOOTER,			DLG_PageSetup_Footer);			

		// Populate Margin Units combo box                      
                               
        addItemToCombo (AP_RID_DIALOG_PAGE_SETUP_LBX_MARGINUNITS, _GVX(DLG_Unit_inch));
        addItemToCombo (AP_RID_DIALOG_PAGE_SETUP_LBX_MARGINUNITS, _GVX(DLG_Unit_cm));
        addItemToCombo (AP_RID_DIALOG_PAGE_SETUP_LBX_MARGINUNITS, _GVX(DLG_Unit_mm));
		// Initialize Data
        selectComboItem (AP_RID_DIALOG_PAGE_SETUP_LBX_MARGINUNITS, (WPARAM) m_pParent->getMarginUnits() );   		
		SetWindowLongPtrW(getHandle(), GWLP_USERDATA, (LONG_PTR)this);
}

INT_PTR CALLBACK AP_Win32Dialog_PageSetup_Margin::s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg==WM_NOTIFY)
	{
		AP_Win32Dialog_PageSetup_Margin *pThis = (AP_Win32Dialog_PageSetup_Margin *) GetWindowLongPtrW(hWnd, GWLP_USERDATA);					

		LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;

		if ( lpnmud->hdr.code == UDN_DELTAPOS )
		{
			pThis->doSpinControl(lpnmud->hdr.idFrom, -lpnmud->iDelta);
		}
	}   	
	
	return XAP_Win32PropertyPage::s_pageWndProc(hWnd, msg, wParam,lParam);
}

void AP_Win32Dialog_PageSetup_Margin::doSpinControl(UT_uint32 id, UT_sint32 delta)
{
	int updatedData =  0;
	int marginScale = ( m_pParent->getMarginUnits() == DIM_MM ) ? 1 : 10;

	switch( id )
	{
	case AP_RID_DIALOG_PAGE_SETUP_SPN_TOP:
		updatedData = (int)( m_pParent->getMarginTop()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			m_pParent->setMarginTop( (float) updatedData/marginScale );
			m_pParent->updateTopMargin();
			m_pParent->updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_BOTTOM:
		updatedData = (int)( m_pParent->getMarginBottom()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			m_pParent->setMarginBottom( (float) updatedData/marginScale );
			m_pParent->updateBottomMargin();
			m_pParent->updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_LEFT:
		updatedData = (int)( m_pParent->getMarginLeft()*marginScale + delta + 0.05f );
		if( updatedData >= 0 )
		{
			m_pParent->setMarginLeft( (float) updatedData/marginScale );
			m_pParent->updateLeftMargin();
			m_pParent->updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_RIGHT:
		updatedData = (int)( m_pParent->getMarginRight()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			m_pParent->setMarginRight( (float) updatedData/marginScale );
			m_pParent->updateRightMargin();
			m_pParent->updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_HEADER:
		updatedData = (int)( m_pParent->getMarginHeader()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			m_pParent->setMarginHeader( (float) updatedData/marginScale );
			m_pParent->updateHeaderMargin();
			m_pParent->updatePreview();
		}
		break;

	case AP_RID_DIALOG_PAGE_SETUP_SPN_FOOTER:
		updatedData = (int)( m_pParent->getMarginFooter()*marginScale + delta + 0.05f );
		if( updatedData >= 0 ) 
		{
			m_pParent->setMarginFooter( (float) updatedData/marginScale );
			m_pParent->updateFooterMargin();
			m_pParent->updatePreview();
		}
		break;

	default:
		break;
	}
}


void AP_Win32Dialog_PageSetup::updatePageSize()
{
	SendDlgItemMessage( m_page.getHandle(),
						AP_RID_DIALOG_PAGE_SETUP_LBX_PAPERSIZE,
						CB_SETCURSEL,
						(WPARAM) fp_PageSize::NameToPredefined( m_PageSize.getPredefinedName()  ),
						(LPARAM) 0 );
}

void AP_Win32Dialog_PageSetup::updateWidth()
{

	char buf[BUFSIZE];

	sprintf (buf, FMT_STRING, m_PageSize.Width (m_PageSize.getDims()));
	SetDlgItemText( m_page.getHandle(),
			AP_RID_DIALOG_PAGE_SETUP_EBX_WIDTH, buf);
	
}

void AP_Win32Dialog_PageSetup::updateHeight()
{
	char buf[BUFSIZE];
	sprintf (buf, FMT_STRING, m_PageSize.Height(m_PageSize.getDims()));
	SetDlgItemText( m_page.getHandle(),
   	                AP_RID_DIALOG_PAGE_SETUP_EBX_HEIGHT, buf);
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
	SetDlgItemText( m_margin.getHandle(),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_TOP,
                    gcvt( (double)getMarginTop(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateBottomMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( m_margin.getHandle(),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_BOTTOM,
                    gcvt( (double)getMarginBottom(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateLeftMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( m_margin.getHandle(),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_LEFT,
                    gcvt( (double)getMarginLeft(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateRightMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( m_margin.getHandle(),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_RIGHT,
                    gcvt( (double)getMarginRight(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateHeaderMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( m_margin.getHandle(),
                    AP_RID_DIALOG_PAGE_SETUP_EBX_HEADER,
                    gcvt( (double)getMarginHeader(), SIGDIGIT, buf ) );
}

void AP_Win32Dialog_PageSetup::updateFooterMargin()
{
	char buf[BUFSIZE];
	SetDlgItemText( m_margin.getHandle(),
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

	HWND hwndPreview = GetDlgItem( m_page.getHandle(),
		                            AP_RID_DIALOG_PAGE_SETUP_BMP_PREVIEW );

	HWND hwndMargin = GetDlgItem( m_margin.getHandle(),
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
