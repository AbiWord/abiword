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

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Win32App.h"
#include "ap_Win32Frame.h"

#include "ap_Strings.h"

#include "ap_Win32Dialog_PageSetup.h"
#include "ap_Win32Resources.rc2"

/*****************************************************************/

#define GWL(hwnd)		(PAGESETUPDLG*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(PAGESETUPDLG*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))

/*****************************************************************/

// fwd. decl. for documentation purposes.
static const POINT	_getPaperSizeFromDlg(const AP_Dialog_PageSetup& dlg);
static const RECT	_getMarginRectFromDlg(const AP_Dialog_PageSetup& dlg);
static void			_initDevModeFromDlg(HGLOBAL hDevMode, const AP_Dialog_PageSetup& dlg);
static void			_setDlgFromPSD(AP_Dialog_PageSetup& dlg, const PAGESETUPDLG& psd);

AP_Win32Dialog_PageSetup::AP_Win32Dialog_PageSetup(	XAP_DialogFactory* pDlgFactory,
													XAP_Dialog_Id id)
:	AP_Dialog_PageSetup (pDlgFactory, id)
{
	m_iMarginHeader 	= 0;
	m_iMarginFooter 	= 0;
	// Windows only supports mm or inches, why we only check for DIM_IN
	m_bisInches 		= (getPageUnits() == fp_PageSize::inch);
	m_fpageScaleFactor	= (m_bisInches) ? 1000.0f : 100.0f;
	for (UT_uint32 i=0;i<10;i++)
	{
		m_strCurrentBuffer[i] = NULL;
		m_strOldBuffer[i] = NULL;
		m_strOriginalBuffer[i] = NULL;
	}
}

AP_Win32Dialog_PageSetup::~AP_Win32Dialog_PageSetup()
{
}

XAP_Dialog* AP_Win32Dialog_PageSetup::static_constructor(XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
{
	AP_Win32Dialog_PageSetup* dlg = new AP_Win32Dialog_PageSetup (pDlgFactory, id);
	return dlg;
}


//
// Free standing functions that have no business in the class' interface since
// these *can* be free-standing. Positive effect: less compile time
// dependencies and a smaller class interface. But, take note, following
// the "Interface Priciple" these *are* logically part of the
// AP_Dialog_PageSetup interface.
//

static const POINT _getPaperSizeFromDlg(const AP_Dialog_PageSetup& dlg)
{
	bool bPageIsInches	= dlg.getPageUnits() == fp_PageSize::inch;
	float pageScaleFactor	= bPageIsInches ? 1000.0f : 100.0f;
	fp_PageSize::Unit pageUnit   = bPageIsInches ? fp_PageSize::inch : fp_PageSize::mm;

	const POINT ptPaperSize =
	{
		(LONG)(dlg.getPageSize().Width(pageUnit)	* pageScaleFactor),
		(LONG)(dlg.getPageSize().Height(pageUnit)	* pageScaleFactor)
	};

	return ptPaperSize;
}

static const RECT _getMarginRectFromDlg(const AP_Dialog_PageSetup& dlg)
{
	bool bMarginIsInches	= dlg.getMarginUnits()	== fp_PageSize::inch;
	float marginScaleFactor	= bMarginIsInches ? 1000.0f : 100.0f;

	const RECT rcMargin =
	{
		(LONG)(dlg.getMarginLeft()		* marginScaleFactor),
		(LONG)(dlg.getMarginTop()		* marginScaleFactor),
		(LONG)(dlg.getMarginRight()		* marginScaleFactor),
		(LONG)(dlg.getMarginBottom()	* marginScaleFactor)
	};

	return rcMargin;
}

static void _initDevModeFromDlg(HGLOBAL hDevMode, const AP_Dialog_PageSetup& dlg)
{
	if (!hDevMode)
	{
		return;
	}

	DEVMODE* pDM = (DEVMODE*)GlobalLock(hDevMode);
	UT_ASSERT(pDM);
	if (pDM->dmFields & DM_ORIENTATION)
	{
		pDM->dmOrientation = dlg.getPageOrientation() == AP_Dialog_PageSetup::PORTRAIT ?
								DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
	}
	if (pDM->dmFields & DM_SCALE)
	{
		pDM->dmScale = (short)dlg.getPageScale();
	}

	pDM->dmFields &= ~(DM_PAPERSIZE | DM_FORMNAME);
	pDM->dmPaperSize = 0;
	pDM->dmFormName[0] = '\0';
	const char* pszName = dlg.getPageSize().getPredefinedName();
	if (pszName)
	{
		pDM->dmFields |= DM_FORMNAME;
		lstrcpyn((char*)pDM->dmFormName, pszName, CCHFORMNAME);
	}

	// The following two are in tenths of mm
	pDM->dmPaperLength = (short)(dlg.getPageSize().Height(fp_PageSize::mm) * 10.0f);
	pDM->dmPaperWidth  = (short)(dlg.getPageSize().Width(fp_PageSize::mm) * 10.0f);

	GlobalUnlock(hDevMode);
}

static void _setDlgFromPSD(AP_Dialog_PageSetup& dlg, const PAGESETUPDLG& psd)
{
	bool bPageIsInches	= dlg.getPageUnits() == fp_PageSize::inch;
//	const bool bPageIsInches = (psd.Flags & PSD_INTHOUSANDTHSOFINCHES) != 0;
	const float pageScaleFactor	= bPageIsInches ? 1000.0f : 100.0f;
	dlg.setPageUnits(bPageIsInches ? fp_PageSize::inch : fp_PageSize::mm);
	dlg.setMarginUnits(dlg.getPageUnits());
	dlg.setPageSize(fp_PageSize(float(psd.ptPaperSize.x) / pageScaleFactor,
								float(psd.ptPaperSize.y) / pageScaleFactor,
								dlg.getPageUnits()));

	dlg.setMarginLeft(psd.rtMargin.left		/ pageScaleFactor);
	dlg.setMarginTop(psd.rtMargin.top		/ pageScaleFactor);
	dlg.setMarginRight(psd.rtMargin.right	/ pageScaleFactor);
	dlg.setMarginBottom(psd.rtMargin.bottom	/ pageScaleFactor);

	if (psd.hDevMode)
	{
		DEVMODE* pDM = (DEVMODE*)GlobalLock(psd.hDevMode);
		UT_ASSERT(pDM);
		if (pDM->dmFields & DM_ORIENTATION)
		{
			dlg.setPageOrientation(pDM->dmOrientation == DMORIENT_PORTRAIT ?
														dlg.PORTRAIT : dlg.LANDSCAPE);
		}
		if (pDM->dmFields & DM_SCALE)
		{
			dlg.setPageScale((int)pDM->dmScale);
		}
		if (pDM->dmFields & DM_FORMNAME &&
			pDM->dmFormName[0] &&
			fp_PageSize::IsPredefinedName((const char*)pDM->dmFormName))
		{
			// override PAGESETUPDLG value
			dlg.setPageSize(fp_PageSize((const char*)pDM->dmFormName));
		}

		GlobalUnlock(psd.hDevMode);
	}
}

void AP_Win32Dialog_PageSetup::runModal(XAP_Frame *pFrame)
{
	
	XAP_Win32App*   pWin32App   = static_cast<XAP_Win32App*>  (pFrame->getApp());
	XAP_Win32Frame* pWin32Frame = static_cast<XAP_Win32Frame*>(pFrame);

	PAGESETUPDLG psd = { 0 };
	
	// get the DEVMODE struct so we can set Landscape/Portrait
	psd.lStructSize	= sizeof(psd);
	psd.Flags		= PSD_RETURNDEFAULT;
	PageSetupDlg(&psd);


	const POINT ptPaperSize	= _getPaperSizeFromDlg(*this);
	const RECT rcMargin		= _getMarginRectFromDlg(*this);

	_initDevModeFromDlg(psd.hDevMode, *this);


	psd.hwndOwner				= pWin32Frame->getTopLevelWindow();
	psd.Flags					= (m_bisInches ? PSD_INTHOUSANDTHSOFINCHES : PSD_INHUNDREDTHSOFMILLIMETERS) |
									PSD_MARGINS ;
	psd.Flags               	|= PSD_ENABLEPAGESETUPTEMPLATE;
	psd.Flags                	|= PSD_ENABLEPAGESETUPHOOK;
	psd.ptPaperSize				= ptPaperSize;
// Use the printer defaults as minimum margins
//	psd.rtMinMargin				= RECT();
	psd.rtMargin				= rcMargin;
	psd.hInstance				= pWin32App->getInstance();
	psd.lCustData				= (LONG)this;
	psd.lpfnPageSetupHook		= (LPPAGESETUPHOOK) s_hookProc;
	psd.lpfnPagePaintHook		= 0;
	psd.lpPageSetupTemplateName	= MAKEINTRESOURCE(AP_RID_DIALOG_PAGE_SETUP);
	psd.hPageSetupTemplate		= 0;

	const BOOL bDlgOK = PageSetupDlg(&psd);	// Display the Page Setup dialog
	if (bDlgOK)
	{
		_setDlgFromPSD(*this, psd);	// Set data from the PAGESETUPDLG
		// Set Header/Footer Margins
		setMarginHeader(m_iMarginHeader / m_fpageScaleFactor);
		setMarginFooter(m_iMarginFooter / m_fpageScaleFactor);
		setAnswer(a_OK);
	}
	else
	{
		setAnswer(a_CANCEL);
		DWORD temp = CommDlgExtendedError();
		UT_DEBUGMSG(("Dialog Error Id %ld\n",temp));
	}

	if (psd.hDevMode)
	{
		GlobalFree(psd.hDevMode);
	}
	if (psd.hDevNames)
	{
		GlobalFree(psd.hDevNames);
	}

}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))

UINT AP_Win32Dialog_PageSetup::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles the WM_INITDIALOG message for the dialog.
	//
	// Initalizae Variables
	HWND hFrame						= GetParent(hWnd);
	XAP_Win32Frame* pWin32Frame		= (XAP_Win32Frame *) ( GetWindowLong(hFrame,GWL_USERDATA) );
	XAP_App* pApp					= pWin32Frame->getApp();
	const XAP_StringSet * pSS		= pApp->getStringSet();
	
	// localize controls
	_DS(PAGE_SETUP_TEXT_HEADER,		DLG_PageSetup_Header);
	_DS(PAGE_SETUP_TEXT_FOOTER,		DLG_PageSetup_Footer);

	// Initialize Controls
	char buf[10];

	HWND hHeader = GetDlgItem( hWnd, AP_RID_DIALOG_PAGE_SETUP_EDIT_HEADER );
	loadData(hWnd,HEADER,buf);
	SetDlgItemText(hWnd,AP_RID_DIALOG_PAGE_SETUP_EDIT_HEADER,buf);
	SendMessage( hHeader, EM_LIMITTEXT, 6, 0 );

	HWND hFooter = GetDlgItem( hWnd, AP_RID_DIALOG_PAGE_SETUP_EDIT_FOOTER );
	loadData(hWnd,FOOTER,buf);
	SetDlgItemText(hWnd,AP_RID_DIALOG_PAGE_SETUP_EDIT_FOOTER,buf);
	SendMessage( hFooter, EM_LIMITTEXT, 6, 0 );
	
	return 0;							
}

UINT AP_Win32Dialog_PageSetup::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// This handles WM_COMMAND message for the top-level dialog.

	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	UT_uint32 type=0;
	
	if ( wId == AP_RID_DIALOG_PAGE_SETUP_EDIT_HEADER ||
		 wId == AP_RID_DIALOG_PAGE_SETUP_EDIT_FOOTER )
	{
		switch( wNotifyCode )
		{
		case EN_UPDATE:
			SendMessage(hWndCtrl, WM_GETTEXT, 6, (LPARAM)m_strCurrentBuffer );
			if ( isKeyStrokeValid() )
			{
				strncpy(m_strOldBuffer, m_strCurrentBuffer, 6);
			}
			else
			{
				MessageBeep(MB_OK);
				SendMessage(hWndCtrl, WM_SETTEXT, 0, (LPARAM) m_strOldBuffer);
			}
			break;
			
		case EN_SETFOCUS:
			SendMessage(hWndCtrl, WM_GETTEXT, 6, (LPARAM)m_strOriginalBuffer);
			strncpy(m_strOldBuffer, m_strOriginalBuffer, 6);
			break;

		case EN_KILLFOCUS:
			// Verify if data is valid
			SendMessage(hWndCtrl, WM_GETTEXT, 6, (LPARAM)m_strCurrentBuffer);
			type = ( wId == AP_RID_DIALOG_PAGE_SETUP_EDIT_HEADER ) ? HEADER : FOOTER;
			if ( isInputValid(hWnd, type) )
			{
				SendMessage(hWndCtrl, WM_SETTEXT, 0, (LPARAM) m_strCurrentBuffer);
			}
			else
			{
				SendMessage(hWndCtrl, WM_SETTEXT, 0, (LPARAM) m_strOriginalBuffer);
			}
			break;
			
		default:
			break;
		}
	}

	return 0;

}

void AP_Win32Dialog_PageSetup::loadData(HWND hWnd, UT_uint32 type, char* buf)
{
	UT_uint32 iValue;

	switch(type)
	{
	case HEADER:
		iValue = (UT_uint32) (getMarginHeader()*m_fpageScaleFactor);
		break;
	case FOOTER:
		iValue = (UT_uint32) (getMarginFooter()*m_fpageScaleFactor);
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	iValue = verifyMarginValue(hWnd, type, iValue);
	convertInteger(iValue, buf);
}

bool AP_Win32Dialog_PageSetup::isKeyStrokeValid()
{
	const char* strValidChar = (m_bisInches) ?	"1234567890.\"\0" : "1234567890.m\0" ;
	UT_uint32 i = 0;

	while (i<5 && m_strCurrentBuffer[i]) 
	{
		if ( strchr(strValidChar, m_strCurrentBuffer[i]) == NULL )
		{
			return false;
		}
		i++;
	}
	return true;
}

bool AP_Win32Dialog_PageSetup::isInputValid(HWND hWnd, UT_uint32 type)
{
	//Input Validation
	const char* strValidChar = (m_bisInches) ? "1234567890.\"\0" : "1234567890.m\0" ;
	const char* endMarker    = (m_bisInches) ? "\"" : "mm" ;
	
	//String Termination
	UT_uint32 i = 0;
	while ( m_strCurrentBuffer[i] && (strchr(strValidChar, m_strCurrentBuffer[i])!=NULL) ) i++;
	m_strCurrentBuffer[i+1] = '\0';
	
	// Check data for endmarker and strip and add end of string NULL
	if (m_bisInches)
	{
		if (m_strCurrentBuffer[i] == '"') m_strCurrentBuffer[i-1] = '\0';
	}
	else
	{
		if ( m_strCurrentBuffer[i] == 'm' &&
		     m_strCurrentBuffer[i-1] == 'm' ) m_strCurrentBuffer[i] = '\0';
	}

	// Check data for conversion to float if not float invalidate
	float fMarginValue = 0.0f;
	if ( (fMarginValue = (float) atof(m_strCurrentBuffer)) == NULL) return false;
			
	// Convert float to integer for storage by scaling,
	UT_uint32 iMarginValue=0;
	iMarginValue = (UT_uint32) (fMarginValue * m_fpageScaleFactor);
	iMarginValue = verifyMarginValue(hWnd, type,iMarginValue);
	// Reset the CurrentBuffer
	convertInteger(iMarginValue, m_strCurrentBuffer);
	// Load local variable
	switch(type)
	{
	case HEADER:
		m_iMarginHeader = iMarginValue;
		break;
	case FOOTER:
		m_iMarginFooter = iMarginValue;
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return true;
}

UT_uint32 AP_Win32Dialog_PageSetup::verifyMarginValue(HWND hWnd, UT_uint32 type, UT_uint32 margin)
{
	PAGESETUPDLG* psd = GWL(hWnd);
	UT_sint32 value=0;
	UT_sint32 tempMarg  = (UT_sint32)margin;
	switch(type)
	{
	case HEADER:
		if      ( tempMarg < psd->rtMinMargin.top ) value = psd->rtMinMargin.top;
		else if ( tempMarg > psd->rtMargin.top )    value = psd->rtMargin.top;
		else                                        value = tempMarg;
		break;
	case FOOTER:
		if      ( tempMarg < psd->rtMinMargin.bottom ) value = psd->rtMinMargin.bottom;
		else if ( tempMarg > psd->rtMargin.bottom )    value = psd->rtMargin.bottom;
		else                                           value = tempMarg;
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	return ( (UT_uint32) value);
}

void AP_Win32Dialog_PageSetup::convertInteger(UT_uint32 margin, char* buf)
{
	char strIntValue[20];
	UT_uint32 iDecPt = m_bisInches ? 3 : 2;
	
	// Trim trailing Zeros
	while (  margin && ((margin % 10) == 0) ) 
	{
		iDecPt--;
		margin /= 10;
	}
	UT_ASSERT( iDecPt >= 0);
	
	itoa(margin,strIntValue,10);
	UT_uint32 iLength = strlen(strIntValue);
	
	UT_ASSERT( iLength < 10 ); // 6 characters as input + 3 for scale
	
	if (margin == 0)
	{
		buf[0] 	= '0';
		buf[1] 	= '\0';
	}
	else if ( iLength <= iDecPt)
	{
		strncpy(buf,"0.\0",3);
		for (UT_uint32 i=0; i < (iDecPt-iLength); i++) strncat(buf,"0",1);
		strncat(buf,strIntValue,iLength);
	}
	else
	{
		strncpy(buf,strIntValue,iLength-iDecPt);
		buf[iLength-iDecPt] = '\0';
		strncat(buf,".",1);
		strncat(buf, strIntValue+(iLength-iDecPt) ,iDecPt);
	}

	//Trim Tailing decimal point
	iLength = strlen(buf);
	if ( buf[iLength-1] == '.') buf[iLength-1] = '\0';
	
	if (m_bisInches) 
	{
		strcat(buf,"\"");
	}
	else
	{
		strcat(buf,"mm");
	}
}

UINT CALLBACK AP_Win32Dialog_PageSetup::s_hookProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.
	PAGESETUPDLG* pPSD = { 0 };	
	AP_Win32Dialog_PageSetup * pThis;

	switch (msg)
	{
	case WM_INITDIALOG:
		pPSD = (PAGESETUPDLG *)lParam;
		SWL(hWnd,lParam);
		pThis = (AP_Win32Dialog_PageSetup *)pPSD->lCustData;
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pPSD = GWL(hWnd);
		if (pPSD)
		{
			pThis = (AP_Win32Dialog_PageSetup *)pPSD->lCustData;
		}
		else
		{
			pThis = NULL;
		}
		return pThis->_onCommand(hWnd,wParam,lParam);
	
	default:
		return 0;
	}
}

