/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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


AP_Win32Dialog_PageSetup::AP_Win32Dialog_PageSetup(	XAP_DialogFactory* pDlgFactory,
													XAP_Dialog_Id id)
:	AP_Dialog_PageSetup (pDlgFactory, id)
{
}

AP_Win32Dialog_PageSetup::~AP_Win32Dialog_PageSetup()
{
}

XAP_Dialog * AP_Win32Dialog_PageSetup::static_constructor(XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
{
	AP_Win32Dialog_PageSetup* dlg = new AP_Win32Dialog_PageSetup (pDlgFactory, id);
	return dlg;
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
	UT_Bool bMarginIsInches	= getMarginUnits()	== fp_PageSize::inch;
	float pageScaleFactor	= bPageIsInches   ? 1000.0f : 100.0f;
	float marginScaleFactor	= bMarginIsInches ? 1000.0f : 100.0f;
	fp_PageSize::Unit pageUnit   = bPageIsInches ? fp_PageSize::inch : fp_PageSize::mm;

	const POINT ptPaperSize =
	{
		(LONG)(getPageSize().Width(pageUnit)	* pageScaleFactor),
		(LONG)(getPageSize().Height(pageUnit)	* pageScaleFactor)
	};
	const RECT rcMargin =
	{
		(LONG)(getMarginLeft()		* marginScaleFactor),
		(LONG)(getMarginTop()		* marginScaleFactor),
		(LONG)(getMarginRight()		* marginScaleFactor),
		(LONG)(getMarginBottom()	* marginScaleFactor)
	};

	if (psd.hDevMode)
	{
		DEVMODE* pDM = (DEVMODE*)GlobalLock(psd.hDevMode);
		UT_ASSERT(pDM);
		if (pDM->dmFields & DM_ORIENTATION)
		{
			pDM->dmOrientation = getPageOrientation() == PORTRAIT ?
									DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
		}
		if (pDM->dmFields & DM_SCALE)
		{
			pDM->dmScale = (short)getPageScale();
		}

		pDM->dmFields &= ~(DM_PAPERSIZE | DM_FORMNAME);
		pDM->dmPaperSize = 0;
		pDM->dmFormName[0] = '\0';
		const char* pszName = getPageSize().getPredefinedName();
		if (pszName)
		{
			pDM->dmFields |= DM_FORMNAME;
			lstrcpyn((char*)pDM->dmFormName, pszName, CCHFORMNAME);
		}

		// The following two are in tenths of mm
		pDM->dmPaperLength = (short)(getPageSize().Height(fp_PageSize::mm) * 10.0f);
		pDM->dmPaperWidth  = (short)(getPageSize().Width(fp_PageSize::mm) * 10.0f);

		GlobalUnlock(psd.hDevMode);
	}

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

	// Display the Page Setup dialog
	if (!PageSetupDlg(&psd))
	{
		// user don't want to make changes
		setAnswer(a_CANCEL);
		if (psd.hDevMode)
		{
			GlobalFree(psd.hDevMode);
		}
		if (psd.hDevNames)
		{
			GlobalFree(psd.hDevNames);
		}
		return;
	}

	// Set data from the PAGESETUPDLG
	bPageIsInches = (psd.Flags & PSD_INTHOUSANDTHSOFINCHES) != 0;
	pageScaleFactor = bPageIsInches ? 1000.0f : 100.0f;
	setPageUnits(bPageIsInches ? fp_PageSize::inch : fp_PageSize::mm);
	setMarginUnits(getPageUnits());
	setPageSize(fp_PageSize(float(psd.ptPaperSize.x) / pageScaleFactor,
							float(psd.ptPaperSize.y) / pageScaleFactor,
							getPageUnits()));

	setMarginLeft(psd.rtMargin.left		/ pageScaleFactor);
	setMarginTop(psd.rtMargin.top		/ pageScaleFactor);
	setMarginRight(psd.rtMargin.right	/ pageScaleFactor);
	setMarginBottom(psd.rtMargin.bottom	/ pageScaleFactor);

	if (psd.hDevMode)
	{
		DEVMODE* pDM = (DEVMODE*)GlobalLock(psd.hDevMode);
		UT_ASSERT(pDM);
		if (pDM->dmFields & DM_ORIENTATION)
		{
			setPageOrientation(pDM->dmOrientation == DMORIENT_PORTRAIT ?
													PORTRAIT : LANDSCAPE);
		}
		if (pDM->dmFields & DM_SCALE)
		{
			setPageScale((int)pDM->dmScale);
		}
		if (pDM->dmFields & DM_FORMNAME &&
			pDM->dmFormName[0] &&
			fp_PageSize::IsPredefinedName((const char*)pDM->dmFormName))
		{
			// override PAGESETUPDLG value
			setPageSize(fp_PageSize((const char*)pDM->dmFormName));
		}

		GlobalUnlock(psd.hDevMode);
		GlobalFree(psd.hDevMode);
	}
	if (psd.hDevNames)
	{
		GlobalFree(psd.hDevNames);
	}

	setAnswer(a_OK);
}
