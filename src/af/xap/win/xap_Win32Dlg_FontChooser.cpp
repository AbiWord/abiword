/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.

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
#include <stdio.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32Dlg_FontChooser.h"
#include "xap_Win32PreviewWidget.h"
#include "gr_Graphics.h"
#include "gr_Win32Graphics.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/
XAP_Dialog * XAP_Win32Dialog_FontChooser::static_constructor(XAP_DialogFactory * pFactory,
														  XAP_Dialog_Id id)
{
	XAP_Win32Dialog_FontChooser * p = new XAP_Win32Dialog_FontChooser(pFactory,id);
	return p;
}

XAP_Win32Dialog_FontChooser::XAP_Win32Dialog_FontChooser(XAP_DialogFactory * pDlgFactory,
													 XAP_Dialog_Id id)
	: XAP_Dialog_FontChooser(pDlgFactory,id),
	m_pPreviewWidget(NULL),
	m_bWin32Overline(false),
	m_bWin32Topline(false),
	m_bWin32Bottomline(false)
{
}

XAP_Win32Dialog_FontChooser::~XAP_Win32Dialog_FontChooser(void)
{
	DELETEP(m_pPreviewWidget);
}

/*****************************************************************/

void XAP_Win32Dialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);

	XAP_Win32App * pApp = static_cast<XAP_Win32App *>(pFrame->getApp());
	UT_return_if_fail(pApp);

	UT_DEBUGMSG(("FontChooserStart: Family[%s] Size[%s] Weight[%s] Style[%s] Color[%s] Underline[%d] StrikeOut[%d]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),
				 ((m_pColor) ? m_pColor : "" ),
				 (m_bUnderline),
				 (m_bStrikeout)));

	m_bWin32Overline   = m_bOverline;
	m_bWin32Topline    = m_bTopline;
	m_bWin32Bottomline = m_bBottomline;
	m_bWin32Hidden     = m_bHidden;
	m_bWin32SuperScript = m_bSuperScript;
	m_bWin32SubScript = m_bSubScript;
	

	/*
	   WARNING: any changes to this function should be closely coordinated
	   with the equivalent logic in Win32Graphics::FindFont()
	*/
	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	CHOOSEFONT cf;
	memset(&cf, 0, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS |
               CF_EFFECTS |
               CF_ENABLEHOOK |
               CF_ENABLETEMPLATE |
               CF_INITTOLOGFONTSTRUCT;
    cf.lpTemplateName = MAKEINTRESOURCE(XAP_RID_DIALOG_FONT);
    cf.lpfnHook = (LPCFHOOKPROC) s_hookProc;
	cf.lCustData = (LPARAM) this;
	cf.hInstance = pApp->getInstance();

	if (m_pFontFamily && *m_pFontFamily)
		strcpy(lf.lfFaceName,m_pFontFamily);
	else
		cf.Flags |= CF_NOFACESEL;

	if (m_pFontSize && *m_pFontSize)
	{
		UT_ASSERT(sizeof(char) == sizeof(XML_Char));
		UT_ASSERT(m_pGraphics);

		// This fixes bug 4494
		UT_uint32 ioldPer = m_pGraphics->getZoomPercentage();
		m_pGraphics->setZoomPercentage(100);
		lf.lfHeight = -(UT_convertToLayoutUnits(m_pFontSize));
		m_pGraphics->setZoomPercentage(ioldPer);
		
	}
	else
		cf.Flags |= CF_NOSIZESEL;

	if (m_pFontWeight && *m_pFontWeight)
	{
		if (UT_stricmp(m_pFontWeight,"bold") == 0)
			lf.lfWeight = 700;
		// TODO do we need any others here...
	}
	else
		cf.Flags |= CF_NOSTYLESEL;

	if (m_pFontStyle && *m_pFontStyle)
	{
		if (UT_stricmp(m_pFontStyle,"italic") == 0)
			lf.lfItalic = TRUE;
	}
	else
		cf.Flags |= CF_NOSTYLESEL;

	if (m_pColor && *m_pColor)
	{
		UT_RGBColor c;
		UT_parseColor(m_pColor,c);
		cf.rgbColors = RGB(c.m_red,c.m_grn,c.m_blu);
	}

	if (m_bUnderline)
		lf.lfUnderline = TRUE;
	if (m_bStrikeout)
		lf.lfStrikeOut = TRUE;

	// run the actual dialog...
	m_answer = (ChooseFont(&cf) ? a_OK : a_CANCEL);

	if (m_answer == a_OK)
	{
		if(m_pFontFamily)
		{
			if((UT_stricmp(lf.lfFaceName,m_pFontFamily) != 0))
			{
				m_bChangedFontFamily = true;
				CLONEP((char *&) m_pFontFamily, lf.lfFaceName);
			}
		}
		else
		{
			if(lf.lfFaceName[0])
			{
				m_bChangedFontFamily = true;
				CLONEP((char *&) m_pFontFamily, lf.lfFaceName);
			}
		}

		bool bIsSizeValid = ((cf.Flags & CF_NOSIZESEL) == 0);
		bool bWasSizeValid = (m_pFontSize && *m_pFontSize);
		char bufSize[10];
		if (bIsSizeValid)
			sprintf(bufSize,"%dpt",(cf.iPointSize/10));
		else
			bufSize[0] = 0;
		if (   (bIsSizeValid && bWasSizeValid && (UT_stricmp(bufSize,m_pFontSize) == 0))
			|| (!bIsSizeValid && !bWasSizeValid))
		{
			/* nothing changed */
		}
		else
		{
			m_bChangedFontSize = true;
			CLONEP((char *&) m_pFontSize, bufSize);
		}

		bool bIsBold = ((cf.nFontType & BOLD_FONTTYPE) != 0);
		bool bWasBold = (m_pFontWeight && *m_pFontWeight && (UT_stricmp(m_pFontWeight,"bold") == 0));
		bool bIsNormal = ((cf.nFontType & REGULAR_FONTTYPE) != 0);
		bool bWasNormal = (!m_pFontWeight
							  || !*m_pFontWeight
							  || (UT_stricmp(m_pFontWeight,"normal") != 0));
		if ((bIsBold != bWasBold) || (bIsNormal != bWasNormal))
		{
			m_bChangedFontWeight = true;
			if( bIsBold )
				CLONEP((char *&) m_pFontWeight, "bold");
			else
				CLONEP((char *&) m_pFontWeight, "normal");
		}

		bool bIsItalic = ((cf.nFontType & ITALIC_FONTTYPE) != 0);
		bool bWasItalic = (m_pFontStyle && *m_pFontStyle && (UT_stricmp(m_pFontStyle,"italic") == 0));
		if (bIsItalic != bWasItalic)
		{
			m_bChangedFontStyle = true;
			if( bIsItalic )
				CLONEP((char *&)m_pFontStyle, "italic");
			else
				CLONEP((char *&)m_pFontStyle, "normal");
		}

		char bufColor[10];
		sprintf(bufColor,"%02x%02x%02x",GetRValue(cf.rgbColors),
				GetGValue(cf.rgbColors),GetBValue(cf.rgbColors));
		bool bWasColorValid = (m_pColor && *m_pColor);
		if (   (bWasColorValid && (UT_stricmp(bufColor,m_pColor) != 0))
			|| (!bWasColorValid && (UT_stricmp(bufColor,"000000") != 0)))
		{
			m_bChangedColor = true;
			CLONEP((char *&)m_pColor, bufColor);
		}

		m_bChangedUnderline  = ((lf.lfUnderline == TRUE) != m_bUnderline);
		m_bChangedStrikeOut  = ((lf.lfStrikeOut == TRUE) != m_bStrikeout);
		m_bChangedOverline   = (m_bWin32Overline   != m_bOverline);
		m_bChangedTopline    = (m_bWin32Topline    != m_bTopline);
		m_bChangedBottomline = (m_bWin32Bottomline != m_bBottomline);
		if (m_bChangedUnderline ||
            m_bChangedStrikeOut ||
            m_bChangedOverline  ||
            m_bChangedTopline   ||
            m_bChangedBottomline)
			setFontDecoration( (lf.lfUnderline == TRUE),
                                m_bWin32Overline,
                                (lf.lfStrikeOut == TRUE),
                                m_bWin32Topline,
                                m_bWin32Bottomline );

		m_bChangedHidden = (m_bWin32Hidden != m_bHidden);
		m_bChangedSuperScript = (m_bWin32SuperScript != m_bSuperScript);
		m_bChangedSubScript = (m_bWin32SubScript != m_bSubScript);
		
		if(m_bChangedHidden)
			setHidden(m_bWin32Hidden);
			
		if(m_bChangedSuperScript)
			setSuperScript(m_bWin32SuperScript);
			
		if(m_bChangedSubScript)
			setSubScript(m_bWin32SubScript);			
	}

	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),	((m_bChangedFontFamily) ? "(chg)" : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),		((m_bChangedFontSize) ? "(chg)" : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),	((m_bChangedFontWeight) ? "(chg)" : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),		((m_bChangedFontStyle) ? "(chg)" : ""),
				 ((m_pColor) ? m_pColor : "" ),				((m_bChangedColor) ? "(chg)" : ""),
				 (m_bUnderline),							((m_bChangedUnderline) ? "(chg)" : ""),
				 (m_bStrikeout),							((m_bChangedStrikeOut) ? "(chg)" : "")));

	// the caller can get the answer from getAnswer().

	m_pWin32Frame = NULL;
}

UINT CALLBACK XAP_Win32Dialog_FontChooser::s_hookProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// static routine
	XAP_Win32Dialog_FontChooser* pThis;

	switch(msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_FontChooser *) ((CHOOSEFONT *)lParam)->lCustData;
		SetWindowLong(hDlg,DWL_USER,(LPARAM) pThis);
		return pThis->_onInitDialog(hDlg,wParam,lParam);

	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_FontChooser *)GetWindowLong(hDlg,DWL_USER);
		if (pThis)
			return pThis->_onCommand(hDlg,wParam,lParam);
		else
			return 0;

	default:
		return 0;

	}
	// Default Dialog handles all other issues
	return 0;
}

#define _DS(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_FontChooser::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND hFrame     = GetParent(hWnd);
	XAP_App*              pApp        = XAP_App::getApp();
	const XAP_StringSet*  pSS         = pApp->getStringSet();

	SetWindowText(hWnd, pSS->getValue(XAP_STRING_ID_DLG_UFS_FontTitle));

	// localize controls
	_DS(FONT_TEXT_FONT,			DLG_UFS_FontLabel);
	_DS(FONT_TEXT_FONT_STYLE,	DLG_UFS_StyleLabel);
	_DS(FONT_TEXT_SIZE,			DLG_UFS_SizeLabel);
	_DS(FONT_TEXT_EFFECTS,		DLG_UFS_EffectsFrameLabel);
	_DS(FONT_BTN_STRIKEOUT,		DLG_UFS_StrikeoutCheck);
	_DS(FONT_BTN_UNDERLINE,		DLG_UFS_UnderlineCheck);
	_DS(FONT_CHK_OVERLINE,		DLG_UFS_OverlineCheck);
	_DS(FONT_CHK_TOPLINE,		DLG_UFS_ToplineCheck);
	_DS(FONT_CHK_BOTTOMLINE,	DLG_UFS_BottomlineCheck);	
	_DS(FONT_TEXT_COLOR,		DLG_UFS_ColorLabel);
	_DS(FONT_TEXT_SCRIPT,		DLG_UFS_ScriptLabel);
	_DS(FONT_TEXT_SAMPLE,		DLG_UFS_SampleFrameLabel);
	_DS(FONT_BTN_OK,			DLG_OK);
	_DS(FONT_BTN_CANCEL,		DLG_Cancel);
	_DS(FONT_CHK_HIDDEN,        DLG_UFS_HiddenCheck);
	_DS(FONT_CHK_SUPERSCRIPT,	DLG_UFS_SuperScript);
	_DS(FONT_CHK_SUBSCRIPT,		DLG_UFS_SubScript);

	// set initial state
	if( m_bWin32Overline )
		CheckDlgButton( hWnd, XAP_RID_DIALOG_FONT_CHK_OVERLINE, BST_CHECKED );

	if( m_bWin32Topline )
		CheckDlgButton( hWnd, XAP_RID_DIALOG_FONT_CHK_TOPLINE, BST_CHECKED );

	if( m_bWin32Bottomline )
		CheckDlgButton( hWnd, XAP_RID_DIALOG_FONT_CHK_BOTTOMLINE, BST_CHECKED );

	if( m_bWin32Hidden )
		CheckDlgButton( hWnd, XAP_RID_DIALOG_FONT_CHK_HIDDEN, BST_CHECKED );
		
	if( m_bWin32SuperScript)
		CheckDlgButton(hWnd, XAP_RID_DIALOG_FONT_CHK_SUPERSCRIPT, BST_CHECKED );		
		
	if( m_bWin32SubScript)
		CheckDlgButton(hWnd, XAP_RID_DIALOG_FONT_CHK_SUBSCRIPT, BST_CHECKED );				
		
	// use the owner-draw-control dialog-item (aka window) specified in the
	// dialog resource file as a parent to the window/widget that we create
	// here and thus have complete control of.
//	m_pPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
//												  GetDlgItem(hWnd, XAP_RID_DIALOG_FONT_PREVIEW),
//												  0);

	// instantiate the XP preview object using the win32 preview widget (window)
	// we just created.  we seem to have a mish-mash of terms here, sorry.

//	UT_uint32 w,h;
//	m_pPreviewWidget->getWindowSize(&w,&h);
//	_createPreviewFromGC(m_pPreviewWidget->getGraphics(),w,h);
//	m_pPreviewWidget->setPreview(); // we need this to call draw() on WM_PAINTs
//	_updatePreviewZoomPercent(getZoomPercent());

	return 1;		// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_FontChooser::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case XAP_RID_DIALOG_FONT_CHK_OVERLINE:
		m_bWin32Overline = (IsDlgButtonChecked(hWnd,XAP_RID_DIALOG_FONT_CHK_OVERLINE)==BST_CHECKED);
		return 1;

	case XAP_RID_DIALOG_FONT_CHK_TOPLINE:
		m_bWin32Topline = (IsDlgButtonChecked(hWnd,XAP_RID_DIALOG_FONT_CHK_TOPLINE)==BST_CHECKED);
		return 1;

	case XAP_RID_DIALOG_FONT_CHK_BOTTOMLINE:
		m_bWin32Bottomline = (IsDlgButtonChecked(hWnd,XAP_RID_DIALOG_FONT_CHK_BOTTOMLINE)==BST_CHECKED);

	case XAP_RID_DIALOG_FONT_CHK_HIDDEN:
		m_bWin32Hidden = (IsDlgButtonChecked(hWnd,XAP_RID_DIALOG_FONT_CHK_HIDDEN)==BST_CHECKED);
		return 1;

	case XAP_RID_DIALOG_FONT_CHK_SUPERSCRIPT:		
		m_bWin32SuperScript = (IsDlgButtonChecked(hWnd,XAP_RID_DIALOG_FONT_CHK_SUPERSCRIPT)==BST_CHECKED);		
		/* It makes no sense to have subscript and superscript at the same time, Word behaves this way also*/
		if (m_bWin32SuperScript) CheckDlgButton(hWnd, XAP_RID_DIALOG_FONT_CHK_SUBSCRIPT, BST_UNCHECKED);		
		return 1;
		
	case XAP_RID_DIALOG_FONT_CHK_SUBSCRIPT:
		m_bWin32SubScript = (IsDlgButtonChecked(hWnd,XAP_RID_DIALOG_FONT_CHK_SUBSCRIPT)==BST_CHECKED);
		/* It makes no sense to have subscript and superscript at the same time, Word behaves this way also*/
		if (m_bWin32SubScript) CheckDlgButton(hWnd, XAP_RID_DIALOG_FONT_CHK_SUPERSCRIPT, BST_UNCHECKED);
		return 1;		

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

