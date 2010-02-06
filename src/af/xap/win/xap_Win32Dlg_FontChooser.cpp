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
#include "xap_EncodingManager.h"
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
	  m_iColorIndx(0),
	  m_iColorCount(0)
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

	XAP_Win32App * pApp = static_cast<XAP_Win32App *>(XAP_App::getApp());
	UT_return_if_fail(pApp);
	const XAP_EncodingManager *pEncMan = pApp->getEncodingManager();
	UT_return_if_fail(pEncMan);

	UT_DEBUGMSG(("FontChooserStart: Family[%s] Size[%s] Weight[%s] Style[%s] Color[%s] Underline[%d] StrikeOut[%d]\n",
				 m_sFontFamily.c_str(),
				 m_sFontSize.c_str(),
				 m_sFontWeight.c_str(),
				 m_sFontStyle.c_str(),
				 m_sColor.c_str(),
				 m_bUnderline,
				 m_bStrikeout));

	m_bWin32Overline   = m_bOverline;
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

	if (!m_sFontFamily.empty())
		strcpy(lf.lfFaceName,pEncMan->strToNative(m_sFontFamily.c_str(), "UTF-8"));
	else
		cf.Flags |= CF_NOFACESEL;

	if (!m_sFontSize.empty())
	{
		UT_ASSERT(sizeof(char) == sizeof(gchar));
		lf.lfHeight = (long) -(UT_convertToPoints(m_sFontSize.c_str()))*4/3;
	}
	else
		cf.Flags |= CF_NOSIZESEL;

	if (!m_sFontWeight.empty())
	{
		if (g_ascii_strcasecmp(m_sFontWeight.c_str(),"bold") == 0)
			lf.lfWeight = 700;
		// TODO do we need any others here...
	}
	else
		cf.Flags |= CF_NOSTYLESEL;

	if (!m_sFontStyle.empty())
	{
		if (g_ascii_strcasecmp(m_sFontStyle.c_str(),"italic") == 0)
			lf.lfItalic = TRUE;
	}
	else
		cf.Flags |= CF_NOSTYLESEL;

	if (!m_sColor.empty())
	{
		UT_RGBColor c;
		UT_parseColor(m_sColor.c_str(),c);
		cf.rgbColors = RGB(c.m_red,c.m_grn,c.m_blu);
	}

	if (m_bUnderline)
		lf.lfUnderline = TRUE;
	if (m_bStrikeout)
		lf.lfStrikeOut = TRUE;

	// run the actual dialog...
	m_answer = (ChooseFont(&cf) ? a_OK : a_CANCEL);
	// Convert the font name returned by the Windows Font Chooser
	// to UTF-8.
	// (strToNative() puts the string in a static buffer).
	const char *szFontFamily = pEncMan->strToNative(lf.lfFaceName, "UTF-8", true);

	if (m_answer == a_OK)
	{
		if(!m_sFontFamily.empty())
		{
			if((g_ascii_strcasecmp(szFontFamily, m_sFontFamily.c_str()) != 0))
			{
				m_bChangedFontFamily = true;
				m_sFontFamily = szFontFamily;
			}
		}
		else
		{
			if(szFontFamily[0])
			{
				m_bChangedFontFamily = true;
				m_sFontFamily = szFontFamily;
			}
		}

		bool bIsSizeValid = ((cf.Flags & CF_NOSIZESEL) == 0);
		bool bWasSizeValid = !m_sFontSize.empty();
		char bufSize[10];
		if (bIsSizeValid)
			sprintf(bufSize,"%dpt",(cf.iPointSize/10));
		else
			bufSize[0] = 0;

		if (bIsSizeValid && bWasSizeValid && (g_ascii_strcasecmp(bufSize,m_sFontSize.c_str()) != 0))			
		{
			m_bChangedFontSize = true;
			m_sFontSize = bufSize;
		}
		else
		{
			/* nothing changed */			
		}

		bool bIsBold = ((cf.nFontType & BOLD_FONTTYPE) != 0);
		bool bWasBold = (g_ascii_strcasecmp(m_sFontWeight.c_str(),"bold") == 0);
		bool bIsNormal = ((cf.nFontType & REGULAR_FONTTYPE) != 0);
		bool bWasNormal = (m_sFontWeight.empty() || (g_ascii_strcasecmp(m_sFontWeight.c_str(),"normal") != 0));
		if ((bIsBold != bWasBold) || (bIsNormal != bWasNormal))
		{
			m_bChangedFontWeight = true;
			if( bIsBold )
				m_sFontWeight = "bold";
			else
				m_sFontWeight = "normal";
		}

		bool bIsItalic = ((cf.nFontType & ITALIC_FONTTYPE) != 0);
		bool bWasItalic = (g_ascii_strcasecmp(m_sFontStyle.c_str(),"italic") == 0);
		if (bIsItalic != bWasItalic)
		{
			m_bChangedFontStyle = true;
			if( bIsItalic )
				m_sFontStyle = "italic";
			else
				m_sFontStyle = "normal";
		}

		char bufColor[10];
		sprintf(bufColor,"%02x%02x%02x",GetRValue(cf.rgbColors),
				GetGValue(cf.rgbColors),GetBValue(cf.rgbColors));
		bool bWasColorValid = !m_sColor.empty();

		if ( m_bChangedColor &&  ((bWasColorValid && (g_ascii_strcasecmp(bufColor,m_sColor.c_str()) != 0))
								  || (!bWasColorValid && (g_ascii_strcasecmp(bufColor,"000000") != 0))))
		{
			m_sColor = bufColor;
		}

		m_bChangedUnderline  = ((lf.lfUnderline == TRUE) != m_bUnderline);
		m_bChangedStrikeOut  = ((lf.lfStrikeOut == TRUE) != m_bStrikeout);
		m_bChangedOverline   = (m_bWin32Overline   != m_bOverline);
		if (m_bChangedUnderline ||
            m_bChangedStrikeOut ||
            m_bChangedOverline)
			setFontDecoration( (lf.lfUnderline == TRUE),
                                m_bWin32Overline,
                                (lf.lfStrikeOut == TRUE), NULL, NULL);

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
				 m_sFontFamily.c_str(),	((m_bChangedFontFamily) ? "(chg)" : ""),
				 m_sFontSize.c_str(),	((m_bChangedFontSize) ? "(chg)" : ""),
				 m_sFontWeight.c_str(),	((m_bChangedFontWeight) ? "(chg)" : ""),
				 m_sFontStyle.c_str(),	((m_bChangedFontStyle) ? "(chg)" : ""),
				 m_sColor.c_str(),		((m_bChangedColor) ? "(chg)" : ""),
				 m_bUnderline,			((m_bChangedUnderline) ? "(chg)" : ""),
				 m_bStrikeout,			((m_bChangedStrikeOut) ? "(chg)" : "")));

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
		
	case WM_HELP:
		pThis = (XAP_Win32Dialog_FontChooser *)GetWindowLong(hDlg,DWL_USER);
		if (pThis)
			return pThis->_callHelp();
		else
			return 0;
		
	default:
		return 0;

	}
	// Default Dialog handles all other issues
	return 0;
}

#define _DS(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_FontChooser::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
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

	// get the initial offset in the text color dlg so that we can tell if the user
	// changed color
	m_iColorIndx = SendDlgItemMessage(hWnd, 1139, CB_GETCURSEL, 0, 0);
	m_iColorCount = SendDlgItemMessage(hWnd, 1139, CB_GETCOUNT, 0, 0);

	if(m_iColorIndx == 0 && !m_sColor.empty() && strcmp(m_sColor.c_str(), "000000") != 0)
	{
		// the first item of the list was selected either becase becase the color we
		// passed to the dlg is not one of those it supports -- we simply add it to the end
		// of the list

		// we have no name for this color, so left it empty
		SendDlgItemMessage(hWnd, 1139, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"");

		// set the color for the entry
		UT_RGBColor c;
		UT_parseColor(m_sColor.c_str(),c);
		DWORD dColor = RGB(c.m_red,c.m_grn,c.m_blu);
		SendDlgItemMessage(hWnd, 1139, CB_SETITEMDATA, m_iColorCount, (LPARAM)(DWORD)dColor);

		// make sure this worked and select the color
		int iOldCount = m_iColorCount;
		m_iColorCount = SendDlgItemMessage(hWnd, 1139, CB_GETCOUNT, 0, 0);
		if(iOldCount + 1 == m_iColorCount)
		{
			SendDlgItemMessage(hWnd, 1139, CB_SETCURSEL, m_iColorCount-1, 0);
			m_iColorIndx = m_iColorCount-1;
		}
	}
	
	return 1;		// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_FontChooser::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case XAP_RID_DIALOG_FONT_CHK_OVERLINE:
			m_bWin32Overline = (IsDlgButtonChecked(hWnd,XAP_RID_DIALOG_FONT_CHK_OVERLINE)==BST_CHECKED);
			return 1;

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

		case 1139: // this is hardcoded in the dlg
			if(wNotifyCode == CBN_SELCHANGE)
			{
				int iIndx = SendDlgItemMessage(hWnd, 1139, CB_GETCURSEL, 0, 0);
				if(iIndx != m_iColorIndx)
					m_bChangedColor = true;

				// return 0 in order for windows to do its thing, i.e., update the preview
				return 0;
			}
			return 0;

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}

extern bool helpLocalizeAndOpenURL(const char* pathBeforeLang, const char* pathAfterLang, const char *remoteURLbase);

BOOL XAP_Win32Dialog_FontChooser::_callHelp()
{
	if ( getHelpUrl().size () > 0 )
    {
		helpLocalizeAndOpenURL ("help", getHelpUrl().c_str(), "http://www.abisource.com/help/" );
    }
	else
    {
		// TODO: warn no help on this topic
		UT_DEBUGMSG(("NO HELP FOR THIS TOPIC!!\n"));
    }

	return TRUE;
}

