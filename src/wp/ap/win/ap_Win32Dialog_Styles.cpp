/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Styles.h"
#include "ap_Win32Dialog_Styles.h"

#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fv_View.h"
#include "pd_Style.h"
#include "ut_string_class.h"
#include "gr_Win32Graphics.h"
#include "pt_PieceTable.h"
#include "ap_Win32App.h"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Styles::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_Styles * p = new AP_Win32Dialog_Styles(pFactory,id);
	return p;
}

AP_Win32Dialog_Styles::AP_Win32Dialog_Styles(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
:	AP_Dialog_Styles(pDlgFactory,id),
	_win32Dialog(this),
	_win32DialogNewModify(this),
	m_whichType(AP_Win32Dialog_Styles::USED_STYLES),
	m_bisNewStyle(true),
	m_selectToggle(0)
{
	m_pAbiPreviewWidget = NULL;	
}

AP_Win32Dialog_Styles::~AP_Win32Dialog_Styles(void)
{
	DELETEP(m_pParaPreviewWidget);
	DELETEP(m_pCharPreviewWidget);
}


/*
	
*/
BOOL CALLBACK AP_Win32Dialog_Styles::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_Styles * pThis;
	switch (msg)
	{
	case WM_INITDIALOG:	
		pThis = (AP_Win32Dialog_Styles *)lParam;		
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);

	case WM_COMMAND:
		
		pThis = (AP_Win32Dialog_Styles *)GetWindowLong(hWnd,DWL_USER);
		if(pThis)
			return pThis->_onCommand(hWnd,wParam,lParam);
		else		
			return 0;
			
	case WM_DRAWITEM:		
		AP_Win32Dialog_Styles * pThis;
		pThis = (AP_Win32Dialog_Styles *)lParam;				
		
		if (wParam==AP_RID_DIALOG_STYLES_NEWMODIFY_BTN_TOGGLEITEMS)
			pThis->_onDrawButton((LPDRAWITEMSTRUCT)lParam, hWnd);
		
		return 0;
	
	default:
		return 0;
	}
}


/*
	Draws the Format button with an arrow
*/
void AP_Win32Dialog_Styles::_onDrawButton(LPDRAWITEMSTRUCT lpDrawItemStruct, HWND hWnd)
{
    UINT			uiState    = lpDrawItemStruct->itemState;
    HPEN			hPen;
    HPEN			pOldPen;
	HDC				hdc = lpDrawItemStruct->hDC;
	int				nWidth;
	int				nHeight;
	int		 		x, xEnd, xStart;
	int 			y;
	POINT 			p;	
	const char* 	pText;
	HWND 			hParent;
	LONG			lData;
	
	// Get parent object and get the App object from there
	hParent = GetParent(hWnd);	
	lData = GetWindowLong(hWnd,DWL_USER);			
	AP_Win32Dialog_Styles * pParent = (AP_Win32Dialog_Styles *)lData;
	const XAP_StringSet * pSS = pParent->m_pApp->getStringSet();		
	
	pText=	pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyFormat);		
	
	nWidth = lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left;
	nHeight = lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top;
		      
    // set the pen color
    if (uiState&ODS_DISABLED)
        hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_GRAYTEXT));
    else
        hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNTEXT));
    
    pOldPen =  (HPEN) SelectObject(hdc, hPen);

    // draw the border of the button
    if(uiState&ODS_SELECTED)
        DrawFrameControl(hdc, &lpDrawItemStruct->rcItem, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED);
    else
        DrawFrameControl(hdc, &lpDrawItemStruct->rcItem, DFC_BUTTON, DFCS_BUTTONPUSH);	
	
	// Draw arrow
	y = nHeight/2;
	xStart = (nWidth/6)*5;
	for (int i=0; i<4; i++)
	{
  	   x = xStart + i;
	   xEnd = xStart + 7 - i;

	   ::MoveToEx(hdc, x, y, &p);	   
       ::LineTo(hdc, xEnd, y);
	   y++;
	 }

	ExtTextOut(hdc, (nWidth/6)*1, ((nHeight/4)), 0, NULL, pText,  strlen(pText), NULL);
		
    // Clean Up
    SelectObject(hdc, pOldPen);       
    DeleteObject(hPen);
}

void AP_Win32Dialog_Styles::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail (pFrame);
//
// Get View and Document pointers. Place them in member variables
//
	setFrame(pFrame);

	setView((FV_View *) pFrame->getCurrentView());
	UT_return_if_fail (getView());

	setDoc(getView()->getLayout()->getDocument());
	UT_return_if_fail (getDoc());


	// raise the dialog
	_win32Dialog.runModal(pFrame, AP_DIALOG_ID_STYLES, AP_RID_DIALOG_STYLES_TOP, this);
	
	
	if (m_answer == AP_Dialog_Styles::a_OK)
	{
		const char* szStyle = getCurrentStyle();
		if (szStyle)
		{
			getDoc()->updateDocForStyleChange(szStyle, true);
			getView()->getCurrentBlock()->setNeedsRedraw();
			getDoc()->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
		}
	}

}

BOOL AP_Win32Dialog_Styles::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_return_val_if_fail (app,0);
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	char szTemp[20];
	GetWindowText(hWnd, szTemp, 20 );	
			
	// Regular dialog box
	if( strncmp(szTemp, "Styles", 20) == 0 )
	{	
		SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_Styles_StylesTitle));

		// localize controls
		struct control_id_string_id {
			UT_sint32		controlId;
			XAP_String_Id	stringId;
		} static const rgMapping[] =
		{
			{AP_RID_DIALOG_STYLES_TOP_TEXT_LIST				, AP_STRING_ID_DLG_Styles_List},
			{AP_RID_DIALOG_STYLES_TOP_TEXT_PARAGRAPH_PREVIEW	, AP_STRING_ID_DLG_Styles_ParaPrev},
			{AP_RID_DIALOG_STYLES_TOP_TEXT_CHARACTER_PREVIEW	, AP_STRING_ID_DLG_Styles_CharPrev},
			{AP_RID_DIALOG_STYLES_TOP_TEXT_DESCRIPTION		, AP_STRING_ID_DLG_Styles_Description},
			{AP_RID_DIALOG_STYLES_TOP_BUTTON_DELETE			, AP_STRING_ID_DLG_Styles_Delete},
			{AP_RID_DIALOG_STYLES_TOP_BUTTON_MODIFY			, AP_STRING_ID_DLG_Styles_Modify},
			{AP_RID_DIALOG_STYLES_TOP_BUTTON_NEW				, AP_STRING_ID_DLG_Styles_New},
			{AP_RID_DIALOG_STYLES_TOP_TEXT_AVAILABLE			, AP_STRING_ID_DLG_Styles_Available},	// "Available Styles" GROUPBOX
			{AP_RID_DIALOG_STYLES_TOP_BUTTON_APPLY			, XAP_STRING_ID_DLG_Apply},
			{AP_RID_DIALOG_STYLES_TOP_BUTTON_CLOSE			, XAP_STRING_ID_DLG_Close}
		};

		for (int i = 0; i < G_N_ELEMENTS(rgMapping); ++i)
		{
			_win32Dialog.setControlText(rgMapping[i].controlId,
										pSS->getValue(rgMapping[i].stringId));
		}

		// Set the list combo.

		_win32Dialog.addItemToCombo(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST, 
									pSS->getValue (AP_STRING_ID_DLG_Styles_LBL_InUse));
		_win32Dialog.addItemToCombo(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST, 
									pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_All));
		_win32Dialog.addItemToCombo(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST,
									pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_UserDefined));
		_win32Dialog.selectComboItem(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST, (int)m_whichType);
	

		// Create a preview windows.

		HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_STYLES_TOP_TEXT_PARAGRAPH_PREVIEW);

		m_pParaPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
														  hwndChild,
														  0);
		UT_uint32 w,h;
		m_pParaPreviewWidget->getWindowSize(&w,&h);
		_createParaPreviewFromGC(m_pParaPreviewWidget->getGraphics(), w, h);
		m_pParaPreviewWidget->setPreview(m_pParaPreview);

		hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_STYLES_TOP_TEXT_CHARACTER_PREVIEW);

		m_pCharPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
														  hwndChild,
														  0);
		m_pCharPreviewWidget->getWindowSize(&w,&h);
		_createCharPreviewFromGC(m_pCharPreviewWidget->getGraphics(), w, h);
		m_pCharPreviewWidget->setPreview(m_pCharPreview);

		_populateWindowData();
	}
	// This is either the new or Modify sub dialog of styles
	else  
	{
		// Localize the controls Labels etc...
		SetWindowText(hWnd, pSS->getValue( (m_bisNewStyle) ? 
                                           AP_STRING_ID_DLG_Styles_NewTitle :
                                           AP_STRING_ID_DLG_Styles_ModifyTitle ));

		_win32DialogNewModify.setHandle(hWnd);
		
		#define _DS(c,s)  _win32DialogNewModify.setControlText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
		#define _DSX(c,s)  _win32DialogNewModify.setControlText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))
		_DS(STYLES_NEWMODIFY_LBL_NAME,			DLG_Styles_ModifyName);
		_DS(STYLES_NEWMODIFY_LBL_BASEDON,		DLG_Styles_ModifyBasedOn);
		_DS(STYLES_NEWMODIFY_LBL_TYPE,			DLG_Styles_ModifyType);
		_DS(STYLES_NEWMODIFY_LBL_FOLLOWPARA,	DLG_Styles_ModifyFollowing);
		_DS(STYLES_NEWMODIFY_LBL_REMOVE,		DLG_Styles_RemoveLab);
		_DS(STYLES_NEWMODIFY_GBX_PREVIEW,		DLG_Styles_ModifyPreview);
		_DS(STYLES_NEWMODIFY_GBX_DESC,			DLG_Styles_ModifyDescription);
		_DS(STYLES_NEWMODIFY_BTN_REMOVE,		DLG_Styles_RemoveButton);		
		_DS(STYLES_NEWMODIFY_BTN_SHORTCUT,		DLG_Styles_ModifyShortCut);
		_DSX(STYLES_NEWMODIFY_BTN_OK,			DLG_OK);
		_DSX(STYLES_NEWMODIFY_BTN_CANCEL,		DLG_Cancel);
		#undef _DSX
		#undef _DS
		
		
		// Changes basic controls based upon either New or Modify Dialog
		_win32DialogNewModify.showControl( AP_RID_DIALOG_STYLES_NEWMODIFY_EBX_TYPE , 
                                           (m_bisNewStyle) ? SW_HIDE : SW_SHOW );
		_win32DialogNewModify.showControl( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_TYPE , 
                                           (m_bisNewStyle) ? SW_SHOW : SW_HIDE );
		// Initialize the controls with appropriate data

		size_t nStyles = getDoc()->getStyleCount();
		const char * name = NULL;
		const char * pLocalised = NULL;
		const PD_Style * pcStyle = NULL;
		int nIndex;
		UT_UTF8String utf8;
		UT_String str;	

		UT_GenericVector<PD_Style*> * pStyles = NULL;
		getDoc()->enumStyles(pStyles);
		UT_return_val_if_fail( pStyles, FALSE );
		
		for (UT_uint32 i = 0; i < nStyles; i++)
		{
    		pcStyle = pStyles->getNthItem(i);
			UT_return_val_if_fail( pcStyle, FALSE );
			name = pcStyle->getName();
			
   			pt_PieceTable::s_getLocalisedStyleName(name, utf8);			
			pLocalised = utf8.utf8_str();
			str = AP_Win32App::s_fromUTF8ToWinLocale (pLocalised);
			pLocalised = str.c_str();	
			
			nIndex = _win32DialogNewModify.addItemToCombo(AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, pLocalised);				
			_win32DialogNewModify.setComboDataItem(AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, 
				nIndex, i);				
			
			nIndex = _win32DialogNewModify.addItemToCombo(AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, pLocalised);				
			_win32DialogNewModify.setComboDataItem(AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, 
				nIndex, i);				
		}

		delete pStyles;
		
		// Strings (not styles names)
		const char*	pDefCurrent = pSS->getValue(AP_STRING_ID_DLG_Styles_DefCurrent);
		const char*	pDefNone = pSS->getValue(AP_STRING_ID_DLG_Styles_DefNone);
		
		str = AP_Win32App::s_fromUTF8ToWinLocale(pDefCurrent);
		pDefCurrent = str.c_str();	
		
		_win32DialogNewModify.addItemToCombo( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, 
                                              pDefCurrent );


		str = AP_Win32App::s_fromUTF8ToWinLocale(pDefNone);
		pDefNone = str.c_str();							

		_win32DialogNewModify.addItemToCombo( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, 
                                              pDefNone);
		
		
		if( m_bisNewStyle )
		{	
			
			const char* p = pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyParagraph);
			
			_win32DialogNewModify.addItemToCombo( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_TYPE,
                                                  p );
                                                  
			p = pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyCharacter);
                                                  
			_win32DialogNewModify.addItemToCombo( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_TYPE,
                                                  p);			                         
                                                  
			// Set the Default syltes: none, default current
			UT_sint32 result;
			result = SendDlgItemMessage(hWnd, AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, CB_FINDSTRING, -1,
										(LPARAM) pSS->getValue(AP_STRING_ID_DLG_Styles_DefNone));
			_win32DialogNewModify.selectComboItem( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, result );
			
			result = SendDlgItemMessage(hWnd, AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, CB_FINDSTRING, -1,
										(LPARAM) pSS->getValue(AP_STRING_ID_DLG_Styles_DefCurrent));
			_win32DialogNewModify.selectComboItem( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, result );
			
			result = SendDlgItemMessage(hWnd, AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_TYPE, CB_FINDSTRING, -1,
										(LPARAM) pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyParagraph));
			_win32DialogNewModify.selectComboItem( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_TYPE, result );

			eventBasedOn();
			eventFollowedBy();
			eventStyleType();
			fillVecFromCurrentPoint();			
		}
		else
		{
			const char * szCurrentStyle = NULL;
			const char * szBasedOn = NULL;
			const char * szFollowedBy = NULL;
			const char * pLocalised = NULL;
			PD_Style * pStyle = NULL;
			PD_Style * pBasedOnStyle = NULL;
			PD_Style * pFollowedByStyle = NULL;
			UT_UTF8String utf8;
			UT_String str;	
			
			szCurrentStyle = m_selectedStyle.c_str();
			
			pt_PieceTable::s_getLocalisedStyleName(szCurrentStyle, utf8);						
			pLocalised = utf8.utf8_str();
			str = AP_Win32App::s_fromUTF8ToWinLocale(pLocalised);
			pLocalised = str.c_str();
	
		
			_win32DialogNewModify.setControlText( AP_RID_DIALOG_STYLES_NEWMODIFY_EBX_NAME,
                                                  pLocalised);
                                                  
			if(szCurrentStyle)
				getDoc()->getStyle(szCurrentStyle,&pStyle);
				
			if(!pStyle)
			{
				XAP_Frame * pFrame = getFrame();
				pFrame->showMessageBox( pSS->getValue(AP_STRING_ID_DLG_Styles_ErrNoStyle),
										XAP_Dialog_MessageBox::b_O,
										XAP_Dialog_MessageBox::a_OK);                                        
				m_answer = AP_Dialog_Styles::a_CANCEL;
				return false;
			}
			//
			// Valid style get the Based On and followed by values
			//
		    pBasedOnStyle = pStyle->getBasedOn();
			pFollowedByStyle = pStyle->getFollowedBy();
			
			size_t nStyles = getDoc()->getStyleCount();
			const char * name = NULL;
			const PD_Style * pcStyle = NULL;
			UT_GenericVector<PD_Style*> * pStyles = NULL;
			getDoc()->enumStyles(pStyles);
			UT_return_val_if_fail( pStyles, FALSE );
	
			for (UT_uint32 i = 0; i < nStyles; i++)
			{
				pcStyle = pStyles->getNthItem(i);
				UT_return_val_if_fail( pcStyle, FALSE );
				name = pcStyle->getName();

				if(pBasedOnStyle && pcStyle == pBasedOnStyle)
				{
					szBasedOn = name;
				}
				if(pFollowedByStyle && pcStyle == pFollowedByStyle)
				{
					szFollowedBy = name;
				}
			}

			delete pStyles;
			
			if(pBasedOnStyle != NULL)
			{

				pt_PieceTable::s_getLocalisedStyleName(szBasedOn, utf8);
				pLocalised = utf8.utf8_str();
				str = AP_Win32App::s_fromUTF8ToWinLocale(pLocalised);
				pLocalised = str.c_str();
	
				
				UT_uint32 result = SendDlgItemMessage(hWnd, AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, CB_FINDSTRING, -1,
										(LPARAM)pLocalised);
										
				_win32DialogNewModify.selectComboItem( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, result );
			}
			else
			{
				// Not a style name
				UT_uint32 result = SendDlgItemMessage(hWnd, AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, CB_FINDSTRING, -1,
										(LPARAM) pSS->getValue(AP_STRING_ID_DLG_Styles_DefNone));
										
				_win32DialogNewModify.selectComboItem( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, result );
			}

			if(pFollowedByStyle != NULL)
			{
				pt_PieceTable::s_getLocalisedStyleName(szFollowedBy, utf8);		
				pLocalised = utf8.utf8_str();
				str = AP_Win32App::s_fromUTF8ToWinLocale(pLocalised);
				pLocalised = str.c_str();
				
				UT_uint32 result = SendDlgItemMessage(hWnd, AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, CB_FINDSTRING, -1,
										(LPARAM)pLocalised);
				_win32DialogNewModify.selectComboItem( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, result );
			}
			else
			{
				pt_PieceTable::s_getLocalisedStyleName(pSS->getValue(AP_STRING_ID_DLG_Styles_DefCurrent), utf8);		
				pLocalised = utf8.utf8_str();
				str = AP_Win32App::s_fromUTF8ToWinLocale(pLocalised);
				pLocalised = str.c_str();

				
				UT_uint32 result = SendDlgItemMessage(hWnd, AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, CB_FINDSTRING, -1,
										(LPARAM) pLocalised);
				_win32DialogNewModify.selectComboItem( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, result );
			}
			
			if(strstr(getAttsVal("type"),"P") != 0)
			{
				_win32DialogNewModify.setControlText( AP_RID_DIALOG_STYLES_NEWMODIFY_EBX_TYPE, 
                                                      pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyParagraph) );
			}
			else
			{
				_win32DialogNewModify.setControlText( AP_RID_DIALOG_STYLES_NEWMODIFY_EBX_TYPE, 
                                                      pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyCharacter) );
			}

			// Disable for editing top controls in Modify Dialog
			_win32DialogNewModify.enableControl( AP_RID_DIALOG_STYLES_NEWMODIFY_EBX_NAME, false );
			_win32DialogNewModify.enableControl( AP_RID_DIALOG_STYLES_NEWMODIFY_EBX_TYPE, false ); 

			fillVecWithProps(szCurrentStyle,true);
		}

		// Generate the Preview class
		HWND hwndChild = GetDlgItem( hWnd, AP_RID_DIALOG_STYLES_NEWMODIFY_CTL_PREVIEW );

		m_pAbiPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
														  hwndChild,
														  0);
		UT_uint32 w,h;
		m_pAbiPreviewWidget->getWindowSize(&w,&h);
		_createAbiPreviewFromGC(m_pAbiPreviewWidget->getGraphics(), w, h);
		_populateAbiPreview(m_bisNewStyle);
		m_pAbiPreviewWidget->setPreview(m_pAbiPreview);

		rebuildDeleteProps();
		_populatePreviews(true);

	}
	
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Styles::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_STYLES_TOP_BUTTON_APPLY:
		{
			const gchar * szStyle = getCurrentStyle();
			if(szStyle && *szStyle)
			{
				getView()->setStyle(szStyle);
			}		
		}
		m_answer = a_OK;
		return 1;

	case AP_RID_DIALOG_STYLES_TOP_BUTTON_CLOSE:
	case IDCANCEL:
		m_answer = a_CANCEL;
		EndDialog(hWnd,0);
		return 1;

	case IDOK:
		{	
     		const XAP_StringSet * pSS = m_pApp->getStringSet ();
			// Verfiy a name value for the style
			// TODO - Verify unique name value
			_win32DialogNewModify.getControlText( AP_RID_DIALOG_STYLES_NEWMODIFY_EBX_NAME,
                                                  m_newStyleName,
	                                              MAX_EBX_LENGTH );
			

			if( !m_newStyleName || !strlen(m_newStyleName) )
			{
			    getFrame()->showMessageBox( pSS->getValue (AP_STRING_ID_DLG_Styles_ErrBlankName),
											XAP_Dialog_MessageBox::b_O,
											XAP_Dialog_MessageBox::a_OK);

			    return 1;
    		}

			strcpy (m_newStyleName, (AP_Win32App::s_fromWinLocaleToUTF8(m_newStyleName)).utf8_str());

		}
		m_answer = a_OK;
		EndDialog(hWnd,0);
		return 1;


	case AP_RID_DIALOG_STYLES_TOP_COMBO_LIST:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			switch(_win32Dialog.getComboSelectedIndex(AP_RID_DIALOG_STYLES_TOP_COMBO_LIST))
			{
			case 0:
				m_whichType = USED_STYLES;
				break;
				
			case 1:
				m_whichType = ALL_STYLES;
				break;
				
			case 2:
				m_whichType = USER_STYLES;
				break;
			}

			_populateWindowData();
		}
		return 1;

	case AP_RID_DIALOG_STYLES_TOP_LIST_STYLES:
		if (wNotifyCode == LBN_SELCHANGE)
		{
			
			UT_uint32	nData = -1;
			const char* name;
			const PD_Style * pcStyle = NULL;
			
			int row = _win32Dialog.getListSelectedIndex(AP_RID_DIALOG_STYLES_TOP_LIST_STYLES);					
			
			nData = _win32Dialog.getListDataItem( AP_RID_DIALOG_STYLES_TOP_LIST_STYLES, row);
			
			if (row!=LB_ERR)
			{
				getDoc()->enumStyles(nData, &name, &pcStyle);				
				m_selectedStyle = name; 
				
				m_nSelectedStyleIdx = nData;
							
				// refresh the previews
				_populatePreviews(false);	
			}
			
			break;			
		}
		return 1;

	case AP_RID_DIALOG_STYLES_TOP_BUTTON_DELETE:
		{
			if( m_selectedStyle != "" )
			{
				if ( !getDoc()->removeStyle(m_selectedStyle.c_str()) ) // actually remove the style
				{
					const XAP_StringSet * pSS = m_pApp->getStringSet();
					getFrame()->showMessageBox( pSS->getValue (AP_STRING_ID_DLG_Styles_ErrStyleCantDelete),
												XAP_Dialog_MessageBox::b_O,
												XAP_Dialog_MessageBox::a_OK	);
					return 1;
				}
				getFrame()->repopulateCombos();
				_populateWindowData(); // force a refresh
				getDoc()->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
				m_selectedStyle = "";
			}
    	}
		return 1;

	case AP_RID_DIALOG_STYLES_TOP_BUTTON_NEW:
		{
			m_bisNewStyle = true;
			//_win32Dialog.showWindow(SW_HIDE);
			XAP_Frame* pFrame = getFrame();
			_win32DialogNewModify.runModal(pFrame, AP_DIALOG_ID_STYLES, AP_RID_DIALOG_STYLES_NEWMODIFY, this);
			if(m_answer == AP_Dialog_Styles::a_OK)
			{
				createNewStyle((gchar *) m_newStyleName);
				_populateCList();
			}
			destroyAbiPreview();
			DELETEP(m_pAbiPreviewWidget);
			//_win32Dialog.showWindow(SW_SHOW);
		}
		return 1;

	case AP_RID_DIALOG_STYLES_TOP_BUTTON_MODIFY:
		{
			// Verify that a style is selected
			if( m_selectedStyle == "" )
			{
				XAP_Frame * pFrame = getFrame();
				const XAP_StringSet * pSS = m_pApp->getStringSet();
				pFrame->showMessageBox( pSS->getValue(AP_STRING_ID_DLG_Styles_ErrNoStyle),
										XAP_Dialog_MessageBox::b_O,
										XAP_Dialog_MessageBox::a_OK);                                        
				m_answer = AP_Dialog_Styles::a_CANCEL;
				return 1;
			}
			else
			{
				PD_Style * pStyle = NULL;
				getDoc()->getStyle(m_selectedStyle.c_str(), &pStyle);

				m_bisNewStyle = false;
				XAP_Frame* pFrame = getFrame();			
				
				XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(getApp());
			
				LPCTSTR lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_STYLES_NEWMODIFY);				
											
				int result = DialogBoxParam(pWin32App->getInstance(), lpTemplate,
									static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
									(DLGPROC)s_dlgProc, (LPARAM)this);
				
				if(m_answer == AP_Dialog_Styles::a_OK)
				{
					applyModifiedStyleToDoc();
					getDoc()->updateDocForStyleChange(getCurrentStyle(),true);
					getDoc()->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
				}

				destroyAbiPreview();
				DELETEP(m_pAbiPreviewWidget);
			}
		}
		return 1;

	case AP_RID_DIALOG_STYLES_NEWMODIFY_BTN_REMOVE:
		{
			char szTemp[128];
			_win32DialogNewModify.getControlText( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_REMOVE,
                                                  szTemp,
	                                              sizeof(szTemp) );			
			removeVecProp(szTemp);
			rebuildDeleteProps();
			updateCurrentStyle();
		}
		return 1;

	case AP_RID_DIALOG_STYLES_NEWMODIFY_BTN_TOGGLEITEMS:
	{
	    RECT 	rect;
	    HMENU 	hMenu;
	    int		x,y;	    	    
	    HWND	hWndButton;
	    
	    hWndButton = GetDlgItem(hWnd, AP_RID_DIALOG_STYLES_NEWMODIFY_BTN_TOGGLEITEMS);
	    XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);		
		const XAP_StringSet * pSS = m_pApp->getStringSet();
	    
		// Get button position
	    GetWindowRect(hWndButton, &rect);
	    x = rect.left;
	    y = rect.bottom;	               		

	    // Menu creation
	    hMenu =  CreatePopupMenu();
	    AppendMenu(hMenu, MF_ENABLED|MF_STRING, 1, (LPCTSTR)pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyParagraph));
	    AppendMenu(hMenu, MF_ENABLED|MF_STRING, 2, (LPCTSTR)pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyFont));
	    AppendMenu(hMenu, MF_ENABLED|MF_STRING, 3, (LPCTSTR)pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyTabs));
	    AppendMenu(hMenu, MF_ENABLED|MF_STRING, 4, (LPCTSTR)pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyNumbering));
	    AppendMenu(hMenu, MF_ENABLED|MF_STRING, 5, (LPCTSTR)pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyLanguage));
	    
	    // show and track the menu
    	m_selectToggle = TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,
    						x,y,0, hWndButton,  NULL);		    							    	        						 							    
	    
	    switch(m_selectToggle)
		{
		case 0:	// user has cancelled
			break;
		case 1:
			ModifyParagraph();
			break;
		case 2:
			ModifyFont();
			break;
		case 3:
			ModifyTabs();
			break;
		case 4:
			ModifyLists();
			break;
		case 5:
			ModifyLang();
			break;
		default:
			break;			
		}
		
		rebuildDeleteProps();
		updateCurrentStyle();
	    DestroyMenu(hMenu);
		return 1;
	}


	case AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			eventBasedOn();
			rebuildDeleteProps();
		}	
		return 1;

	case AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			eventFollowedBy();
		}	
		return 1;

	case AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_TYPE:
		if( wNotifyCode == CBN_SELCHANGE )
		{
			eventStyleType();
		}
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
	
	return 0;						// return zero to let windows take care of it.
}


void AP_Win32Dialog_Styles::_populateWindowData(void)
{
	_populateCList();
	_populatePreviews(false);
}

void AP_Win32Dialog_Styles::_populateCList(void)
{
	const PD_Style * pStyle;
	const char * name = NULL;
	const char*	pLocalised = NULL;
	int nIndex;
	UT_UTF8String utf8;
	UT_String str;						 

	size_t nStyles = getDoc()->getStyleCount();
	xxx_UT_DEBUGMSG(("DOM: we have %d styles\n", nStyles));

	_win32Dialog.resetContent(AP_RID_DIALOG_STYLES_TOP_LIST_STYLES);
	
	UT_GenericVector<PD_Style*> * pStyles = NULL;
	getDoc()->enumStyles(pStyles);
	UT_return_if_fail( pStyles );
	
	for (UT_uint32 i = 0; i < nStyles; i++)
	{
	    const char * data[1];

	    getDoc()->enumStyles((UT_uint32)i, &name, &pStyle);
		pStyle = pStyles->getNthItem(i);
		
		// style has been deleted probably
		if (!pStyle)
			continue;

		name = pStyle->getName();

	    // all of this is safe to do... append should take a const char **
	    data[0] = name;	    

	    if ((m_whichType == ALL_STYLES) || 
			(m_whichType == USED_STYLES && pStyle->isUsed()) ||
			(m_whichType == USER_STYLES && pStyle->isUserDefined()))
		{

			pt_PieceTable::s_getLocalisedStyleName(*data, utf8);
			pLocalised = utf8.utf8_str();
			str = AP_Win32App::s_fromUTF8ToWinLocale(pLocalised);
			pLocalised = str.c_str();
			
			nIndex = _win32Dialog.addItemToList(AP_RID_DIALOG_STYLES_TOP_LIST_STYLES, pLocalised);						
			_win32Dialog.setListDataItem(AP_RID_DIALOG_STYLES_TOP_LIST_STYLES, nIndex, i);							
			
		}
	}

	delete pStyles;
}

const char * AP_Win32Dialog_Styles::getCurrentStyle (void) const
{
	return m_selectedStyle.size() ? m_selectedStyle.c_str() : 0;
}

void AP_Win32Dialog_Styles::setDescription (const char * desc) const
{
	AP_Win32Dialog_Styles *p_This = (AP_Win32Dialog_Styles *)this; // Cast away const

	p_This->_win32Dialog.setControlText(AP_RID_DIALOG_STYLES_TOP_LABEL_DESCRIPTION, desc);
}

void AP_Win32Dialog_Styles::setModifyDescription (const char * desc)
{
	AP_Win32Dialog_Styles *p_This = (AP_Win32Dialog_Styles *)this; // Cast away const

	p_This->_win32DialogNewModify.setControlText(AP_RID_DIALOG_STYLES_NEWMODIFY_CTL_DESC, desc);
}


void AP_Win32Dialog_Styles::rebuildDeleteProps()
{
	AP_Win32Dialog_Styles *p_This = (AP_Win32Dialog_Styles *)this; // Cast away const
	p_This->_win32DialogNewModify.resetComboContent(AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_REMOVE);

	UT_sint32 count = m_vecAllProps.getItemCount();
	UT_sint32 i= 0;
	for(i=0; i< count; i+=2)
	{
		p_This->_win32DialogNewModify.addItemToCombo( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_REMOVE, 
                                                      (const char *) m_vecAllProps.getNthItem(i) );
	}
}

void AP_Win32Dialog_Styles::eventBasedOn()
{
	char szTemp[128];
	int	nSel;
	int	nData;
	const PD_Style * pStyle = NULL;
	const char* pText = szTemp;
		
	nSel = _win32DialogNewModify.getComboSelectedIndex(AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON);		
				
	if (nSel==CB_ERR) return;				
	
	nData= _win32DialogNewModify.getComboDataItem(AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_BASEDON, nSel);
	
	getDoc()->enumStyles((UT_uint32)nData,&pText, &pStyle);				
	
	strcpy (szTemp, (AP_Win32App::s_fromWinLocaleToUTF8(pText)).utf8_str());
	addOrReplaceVecAttribs("basedon",pText);
	fillVecWithProps(pText,false);
	updateCurrentStyle();
}

void AP_Win32Dialog_Styles::eventFollowedBy()
{
	char szTemp[128]; 
	int	nSel;
	int	nData;
	const PD_Style * pStyle = NULL;
	const char* pText = szTemp;
		
	nSel = _win32DialogNewModify.getComboSelectedIndex(AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA);		
					
	if (nSel==CB_ERR) return;				
	
	nData= _win32DialogNewModify.getComboDataItem(AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_FOLLOWPARA, nSel);
					
	getDoc()->enumStyles((UT_uint32)nData,&pText, &pStyle);			
	
	strcpy (szTemp, (AP_Win32App::s_fromWinLocaleToUTF8(pText)).utf8_str());
	addOrReplaceVecAttribs("followedby",pText);
}

void AP_Win32Dialog_Styles::eventStyleType()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();	
	const gchar * pszSt = "P";
	char szTemp[128];
	_win32DialogNewModify.getControlText( AP_RID_DIALOG_STYLES_NEWMODIFY_CBX_TYPE,
                                          szTemp,
                                          sizeof(szTemp));			
	if(strstr(szTemp, pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyCharacter)) != 0)
		pszSt = "C";
	
	addOrReplaceVecAttribs("type",pszSt);
}

