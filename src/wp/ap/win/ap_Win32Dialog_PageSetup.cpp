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

#include "ap_Win32Dialog_PageSetup.h"
#include "xap_Win32Frame.h"
#include <windows.h>
#include <commdlg.h>


// fwd. decl. for documentation purposes.
static const POINT	_getPaperSizeFromDlg(const AP_Dialog_PageSetup& dlg);
static const RECT	_getMarginRectFromDlg(const AP_Dialog_PageSetup& dlg);
static void			_initDevModeFromDlg(HGLOBAL hDevMode, const AP_Dialog_PageSetup& dlg);
static void			_setDlgFromPSD(AP_Dialog_PageSetup& dlg, const PAGESETUPDLG& psd);


AP_Win32Dialog_PageSetup::AP_Win32Dialog_PageSetup(	XAP_DialogFactory* pDlgFactory,
													XAP_Dialog_Id id)
:	AP_Dialog_PageSetup (pDlgFactory, id)
{
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
	UT_Bool bPageIsInches	= dlg.getPageUnits() == fp_PageSize::inch;
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
	UT_Bool bMarginIsInches	= dlg.getMarginUnits()	== fp_PageSize::inch;
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
	const UT_Bool bPageIsInches = (psd.Flags & PSD_INTHOUSANDTHSOFINCHES) != 0;
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
	XAP_Win32Frame* pWin32Frame = static_cast<XAP_Win32Frame*>(pFrame);
	PAGESETUPDLG psd = { 0 };

	// get the DEVMODE struct so we can set Landscape/Portrait
	psd.lStructSize	= sizeof(psd);
	psd.Flags		= PSD_RETURNDEFAULT;
	PageSetupDlg(&psd);

	// Windows only supports mm or inches, why we only check for DIM_IN
	UT_Bool bPageIsInches	= getPageUnits()	== fp_PageSize::inch;

	const POINT ptPaperSize	= _getPaperSizeFromDlg(*this);
	const RECT rcMargin		= _getMarginRectFromDlg(*this);

	_initDevModeFromDlg(psd.hDevMode, *this);

	psd.hwndOwner				= pWin32Frame->getTopLevelWindow();
	psd.Flags					= (bPageIsInches ? PSD_INTHOUSANDTHSOFINCHES : PSD_INHUNDREDTHSOFMILLIMETERS) |
									PSD_MARGINS;
	psd.ptPaperSize				= ptPaperSize;
	psd.rtMinMargin				= RECT();
	psd.rtMargin				= rcMargin;
	psd.hInstance				= 0;
	psd.lCustData				= 0;
	psd.lpfnPageSetupHook		= 0;
	psd.lpfnPagePaintHook		= 0;
	psd.lpPageSetupTemplateName	= 0;
	psd.hPageSetupTemplate		= 0;

	const BOOL bDlgOK = PageSetupDlg(&psd);	// Display the Page Setup dialog
	if (bDlgOK)
	{
		_setDlgFromPSD(*this, psd);	// Set data from the PAGESETUPDLG
		setAnswer(a_OK);
	}
	else
	{
		setAnswer(a_CANCEL);
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
