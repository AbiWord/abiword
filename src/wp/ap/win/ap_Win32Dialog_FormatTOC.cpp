/* AbiWord
 * Copyright (C) 2004-2005 Jordi Mas i Hern�ndez <jmas@softcatala.org>
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

#include <stdlib.h>
#include <time.h>
#include <windows.h>

#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Win32App.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatTOC.h"
#include "ap_Win32Dialog_FormatTOC.h"
#include "xap_Win32PropertySheet.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"
#include "ap_Dialog_FormatFootnotes.h"
#include "ap_Win32App.h"                                                        
#include "pt_PieceTable.h"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_FormatTOC::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_FormatTOC * p = new AP_Win32Dialog_FormatTOC(pFactory,id);
	return p;
}

AP_Win32Dialog_FormatTOC::AP_Win32Dialog_FormatTOC(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_FormatTOC(pDlgFactory,id)	
{
	m_iStartValue = 1;
}

AP_Win32Dialog_FormatTOC::~AP_Win32Dialog_FormatTOC(void)
{

}

void AP_Win32Dialog_FormatTOC::notifyActiveFrame(XAP_Frame *pFrame)
{    
	
}

void AP_Win32Dialog_FormatTOC::setTOCPropsInGUI(void)
{
	if (m_pGeneral)
		m_pGeneral->_fillGUI();
		
	if (m_pLayout)
		m_pLayout->_fillGUI();
	
}


void AP_Win32Dialog_FormatTOC::setSensitivity(bool bSensitive)
{
	if (m_pGeneral) 
		m_pGeneral->setChanged (bSensitive);
		
	if (m_pLayout)
		m_pLayout->setChanged (bSensitive);
	
}

void AP_Win32Dialog_FormatTOC::destroy(void)
{	
	
	finalize();		
	
	//Property sheet will be autodestroyed by the cancel action		
	if (m_pGeneral) {
		delete m_pGeneral;
		m_pGeneral = NULL;
	}
		
	if (m_pLayout) {
		delete m_pLayout;
		m_pLayout = NULL;
	}

	//delete m_pSheet;

}

void AP_Win32Dialog_FormatTOC::activate(void)
{	  	

	
}	



void AP_Win32Dialog_FormatTOC::runModeless(XAP_Frame * pFrame)
{

	UT_return_if_fail (pFrame);
	RECT rect;
	POINT pnt;
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());

	m_pSheet = new AP_Win32Dialog_FormatTOC_Sheet();
	m_pSheet->setContainer (this);	
	m_pSheet->setApplyButton (true);
	m_pSheet->setOkButton (true);
	m_pSheet->setCancelButton(false);
	
	m_pGeneral = new AP_Win32Dialog_FormatTOC_General();		
	m_pGeneral->setContainer (this);	
	m_pGeneral->createPage(pWin32App, AP_RID_DIALOG_FORMATTOC_GENERAL, AP_STRING_ID_DLG_FormatTOC_General);
	
	m_pLayout = new AP_Win32Dialog_FormatTOC_Layout();		
	m_pLayout->setContainer (this);	
	m_pLayout->createPage(pWin32App, AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS, AP_STRING_ID_DLG_FormatTOC_LayoutDetails);	
	
	m_pSheet->addPage(m_pGeneral);
	m_pSheet->addPage(m_pLayout);
	m_pSheet->runModeless(pWin32App, pFrame, AP_STRING_ID_DLG_FormatTOC_Title);	

	GetWindowRect(GetDlgItem(m_pSheet->getHandle() ,IDCANCEL), &rect);	
	pnt.x = rect.left;
	pnt.y = rect.top;
	ScreenToClient (m_pSheet->getHandle(), &pnt);

	HWND hwndOK = GetDlgItem(m_pSheet->getHandle(),IDOK);
	MoveWindow (hwndOK, 
		pnt.x, pnt.y, rect.right - rect.left, rect.bottom - rect.top, true);

	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	startUpdater(); 
}

void AP_Win32Dialog_FormatTOC::setStyle(HWND hWnd, int nCtrlID)
{
	UT_UTF8String sVal, str_loc;	
	UT_UTF8String sProp;
	UT_String str;
	HWND hwndCtrl = GetDlgItem (hWnd, nCtrlID);

	switch (nCtrlID) 
	{
		case AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLEVALUE:
			sProp = static_cast<char *> ("toc-heading-style");
			break;

		case AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_FILLSTYLEVALUE:
			sProp = static_cast<char *> ("toc-source-style");
			break;

		case AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_DISPLAYSTYLEVALUE:
			sProp = static_cast<char *> ("toc-dest-style");
			break;

		default:
			break;
	}

	if(g_ascii_strcasecmp("toc-heading-style",sProp.utf8_str()) != 0)
	{
		UT_String sNum =  UT_String_sprintf("%d",getMainLevel());
		sProp += sNum.c_str();
	}

	sVal = getNewStyle(sProp);	
	pt_PieceTable::s_getLocalisedStyleName (sVal.utf8_str(), str_loc);	

	SendMessage (hwndCtrl, WM_SETTEXT, 0, (LPARAM) 
		(AP_Win32App::s_fromUTF8ToWinLocale (str_loc.utf8_str())).c_str() );
		
	setTOCProperty(sProp,sVal);
	applyTOCPropsToDoc ();

}


void AP_Win32Dialog_FormatTOC::setMainLevel(UT_sint32 iLevel)
{
	AP_Dialog_FormatTOC::setMainLevel(iLevel);
	UT_UTF8String sVal, str_loc;
	UT_String str;

	sVal = getTOCPropVal("toc-dest-style",getMainLevel());

	pt_PieceTable::s_getLocalisedStyleName (sVal.utf8_str(), str_loc);
	str = AP_Win32App::s_fromUTF8ToWinLocale (str_loc.utf8_str()); 

	SendMessage (GetDlgItem (m_pGeneral->getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_DISPLAYSTYLEVALUE), 	
		WM_SETTEXT, 0, (LPARAM)str.c_str());

	sVal = getTOCPropVal("toc-source-style",getMainLevel());

	pt_PieceTable::s_getLocalisedStyleName (sVal.utf8_str(), str_loc);
	str = AP_Win32App::s_fromUTF8ToWinLocale (str_loc.utf8_str()); 

	SendMessage (GetDlgItem (m_pGeneral->getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_FILLSTYLEVALUE), 	
		WM_SETTEXT, 0, (LPARAM)str.c_str());	

	sVal = getTOCPropVal("toc-has-label",getMainLevel());
	
	CheckDlgButton(m_pGeneral->getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASLEVEL,
		(g_ascii_strcasecmp(sVal.utf8_str(),"1") == 0) ? BST_CHECKED :BST_UNCHECKED);	
}


void AP_Win32Dialog_FormatTOC::setDetailsLevel(UT_sint32 iLevel)
{
		
	UT_UTF8String sVal, str_loc;
	UT_String str;

	AP_Dialog_FormatTOC::setDetailsLevel(iLevel);

	
	/* Start at */
	sVal = getTOCPropVal("toc-label-start", getDetailsLevel());

	SetWindowText (GetDlgItem (m_pLayout->getHandle(),
		AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_STARTAT), sVal.utf8_str());

	m_pLayout->loadCtrlsValuesForDetailsLevel ();

}

/*

	Sheet
	
*/
AP_Win32Dialog_FormatTOC_Sheet::AP_Win32Dialog_FormatTOC_Sheet() :
XAP_Win32PropertySheet()
{
	setCallBack(s_sheetInit);	
}

int CALLBACK AP_Win32Dialog_FormatTOC_Sheet::s_sheetInit(HWND hwnd,  UINT uMsg,  LPARAM lParam)
{	
	if (uMsg==PSCB_INITIALIZED)
	{	
		/* Force the creation of all pages*/		
		PropSheet_SetCurSel(hwnd, 0,1);		
		PropSheet_SetCurSel(hwnd, 0,0);
	}			
	return 	0;
}

					  
void AP_Win32Dialog_FormatTOC_Sheet::_onOK()
{	
	getContainer()->destroy();
	destroy();
}

void AP_Win32Dialog_FormatTOC_Sheet::_onInitDialog(HWND hwnd)
{
	
	const XAP_StringSet * pSS = getContainer()->getApp()->getStringSet();		

	SendMessage(GetDlgItem(getHandle(),IDOK), WM_SETTEXT, 0,
		(LPARAM) (pSS->getValue(XAP_STRING_ID_DLG_OK)));
}


void AP_Win32Dialog_FormatTOC_Sheet::cleanup(void) 
{		
	getContainer()->modeless_cleanup ();
}


/*

	General

*/
AP_Win32Dialog_FormatTOC_General::AP_Win32Dialog_FormatTOC_General()
{

}

AP_Win32Dialog_FormatTOC_General::~AP_Win32Dialog_FormatTOC_General()
{

}

void AP_Win32Dialog_FormatTOC_General::_fillGUI()
{														 	
	
	UT_UTF8String sVal, str_loc;
	UT_String str;

	sVal = getContainer()->getTOCPropVal("toc-has-heading"); 
	if(g_ascii_strcasecmp(sVal.utf8_str(),"1") == 0)
	{
		CheckDlgButton(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASHEADING, BST_CHECKED);
		EnableWindow(GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_HEADINGSTYLE), true);
		EnableWindow(GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLEVALUE), true);
	}
	else
	{
		CheckDlgButton(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASHEADING, BST_UNCHECKED);
		EnableWindow(GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_HEADINGSTYLE), false);
		EnableWindow(GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLEVALUE), false);
	}
	
	sVal = getContainer()->getTOCPropVal("toc-has-label", getContainer()->getMainLevel()); 	 
	CheckDlgButton(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASLEVEL,
		(g_ascii_strcasecmp(sVal.utf8_str(),"1") == 0) ? BST_CHECKED :BST_UNCHECKED);

	sVal = getContainer()->getTOCPropVal("toc-heading-style");
	pt_PieceTable::s_getLocalisedStyleName (sVal.utf8_str(), str_loc);
	str = AP_Win32App::s_fromUTF8ToWinLocale (str_loc.utf8_str()); 

	SendMessage (GetDlgItem (getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLEVALUE), 	
		WM_SETTEXT, 0, (LPARAM)str.c_str());	

	// Set MainLevel for the Combobox	
	SendMessage(GetDlgItem(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_COMBO_LEVEL), CB_SETCURSEL, getContainer()->getMainLevel() - 1, 0);	
	getContainer()->setMainLevel(getContainer()->getMainLevel());

	sVal = getContainer()->getTOCPropVal("toc-heading");
	pt_PieceTable::s_getLocalisedStyleName (sVal.utf8_str(), str_loc);
	str = AP_Win32App::s_fromUTF8ToWinLocale (str_loc.utf8_str()); 

	SetWindowText (GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_GENERAL_EDIT_HEADINGTEXT), str.c_str());
	
	
}

void AP_Win32Dialog_FormatTOC_General::_onInitDialog()
{

	const XAP_StringSet * pSS = getApp()->getStringSet();
	int i;

	struct control_id_string_id {
		UT_sint32		controlId;
		XAP_String_Id	stringId;
	} static const rgMapping[] =
	{
		{AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASHEADING, 		AP_STRING_ID_DLG_FormatTOC_HasHeading},
		{AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGTEXT,		AP_STRING_ID_DLG_FormatTOC_HeadingText},
		{AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLE,		AP_STRING_ID_DLG_FormatTOC_HeadingStyle},
		{AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_HEADINGSTYLE,	AP_STRING_ID_DLG_FormatTOC_ChangeStyle},
		{AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_MAINPROPERTIES,	AP_STRING_ID_DLG_FormatTOC_LevelDefs},
		{AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASLEVEL,		AP_STRING_ID_DLG_FormatTOC_HasLabel},
		{AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_FILLSTYLE,		AP_STRING_ID_DLG_FormatTOC_FillStyle},
		{AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_FILLSTYLE,		AP_STRING_ID_DLG_FormatTOC_ChangeStyle},
		{AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_DISPLAYSTYLE,		AP_STRING_ID_DLG_FormatTOC_DispStyle},
		{AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_DISPLAYSTYLE,	AP_STRING_ID_DLG_FormatTOC_ChangeStyle},
		{NULL,NULL}
	};

	// Localise the controls
	for (i = 0; i < rgMapping[i].controlId; i++)
		SetDlgItemText(getHandle(), rgMapping[i].controlId, pSS->getValue(rgMapping[i].stringId));

	/* Levels */
	int item;
	int nID [4] = {AP_STRING_ID_DLG_FormatTOC_Level1, AP_STRING_ID_DLG_FormatTOC_Level2,
	AP_STRING_ID_DLG_FormatTOC_Level3, AP_STRING_ID_DLG_FormatTOC_Level4};
	for (i = 0; i < 4; i++)
	{	
		item = SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_COMBO_LEVEL, CB_ADDSTRING, 0, 
			(LPARAM) pSS->getValue(nID[i]));

		SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_COMBO_LEVEL, CB_SETITEMDATA, item, i+1);
	}	 

	SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_COMBO_LEVEL, CB_SETCURSEL, 0, 0);

	getContainer()->setTOCPropsInGUI();	
}


void AP_Win32Dialog_FormatTOC_General::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	AP_Win32Dialog_FormatTOC*	 pParent=  (AP_Win32Dialog_FormatTOC*)getContainer();	
	
	switch (wId)
	{		
		case AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASHEADING:
		{
			UT_UTF8String sProp = static_cast<char *> ("toc-has-heading");
			UT_UTF8String sVal = "1";

			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASHEADING) != BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_HEADINGSTYLE), false);
				EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLEVALUE), false);
				sVal = "0";		
			}
			else
			{
				EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_HEADINGSTYLE), true);
				EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLEVALUE), true);			
			}
			getContainer()->setTOCProperty(sProp,sVal);			
			break;
		}
				
		case AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_HEADINGSTYLE:						   
			getContainer()->setStyle(hWnd, AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLEVALUE);
			break;

		case AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_FILLSTYLE:						   
			getContainer()->setStyle(hWnd, AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_FILLSTYLEVALUE);
			break;

		case AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_DISPLAYSTYLE:						   
			getContainer()->setStyle(hWnd, AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_DISPLAYSTYLEVALUE);
			break;

		case AP_RID_DIALOG_FORMATTOC_GENERAL_COMBO_LEVEL:
		{
			if (wNotifyCode == CBN_SELCHANGE)
			{
				int nSelected, nLevel;
				HWND hCombo = GetDlgItem(hWnd, AP_RID_DIALOG_FORMATTOC_GENERAL_COMBO_LEVEL);

				nSelected = SendMessage(hCombo, CB_GETCURSEL, 0, 0);					

				if (nSelected!=CB_ERR) 
				{		
					nLevel  = SendMessage(hCombo, CB_GETITEMDATA, nSelected, 0);
					getContainer()->setMainLevel (nLevel);
				}
			}
			break;
			
		}

		case AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASLEVEL:
		{

			UT_UTF8String sProp = static_cast<char *> ("toc-has-label");
			UT_UTF8String sVal = "1";
			UT_String sNum =  UT_String_sprintf("%d",getContainer()->getMainLevel());

			/* Has label */
			sVal = getContainer()->getTOCPropVal(sProp.utf8_str(), getContainer()->getMainLevel());
			sVal = "1";

			if (IsDlgButtonChecked(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASLEVEL) != BST_CHECKED)
				sVal = "0";	
				
			sProp += sNum.c_str();			
			getContainer()->setTOCProperty(sProp,sVal);	
			break;
		}
			
		default:
			break;
	}
}

void AP_Win32Dialog_FormatTOC_General::_onApply()
{
	char szText[1024];
	UT_UTF8String sUTF8;

	GetWindowText(GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_GENERAL_EDIT_HEADINGTEXT), szText, 1024);

	sUTF8 = AP_Win32App::s_fromWinLocaleToUTF8 (szText); 
	
	getContainer()->setTOCProperty("toc-heading", sUTF8.utf8_str());
	getContainer()->Apply();
}

void AP_Win32Dialog_FormatTOC_General::_onOK()
{
	_onApply();
}


/*

	Layout Details

*/
AP_Win32Dialog_FormatTOC_Layout::AP_Win32Dialog_FormatTOC_Layout()
{

}

AP_Win32Dialog_FormatTOC_Layout::~AP_Win32Dialog_FormatTOC_Layout()
{

}

void AP_Win32Dialog_FormatTOC_Layout::_fillGUI()
{														 	
	UT_UTF8String sVal, str_loc;
	UT_String str;

	/* Text Before */
	sVal = getContainer()->getTOCPropVal("toc-label-before", getContainer()->getDetailsLevel());
	pt_PieceTable::s_getLocalisedStyleName (sVal.utf8_str(), str_loc);
	str = AP_Win32App::s_fromUTF8ToWinLocale (str_loc.utf8_str()); 

	SetWindowText (GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_TEXTBEFORE), str.c_str());	

	/* Text After */	
	sVal = getContainer()->getTOCPropVal("toc-label-after", getContainer()->getDetailsLevel());
	pt_PieceTable::s_getLocalisedStyleName (sVal.utf8_str(), str_loc);
	str = AP_Win32App::s_fromUTF8ToWinLocale (str_loc.utf8_str()); 

	SetWindowText (GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_TEXTAFTER), str.c_str());	

	/* Start at */
	sVal = getContainer()->getTOCPropVal("toc-label-start", getContainer()->getDetailsLevel());

	SetWindowText (GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_STARTAT), sVal.utf8_str());

}

void AP_Win32Dialog_FormatTOC_Layout::_onInitDialog()
{
	UT_UTF8String sVal, str_loc;
	UT_String str;
	const XAP_StringSet * pSS = getApp()->getStringSet();
	int i;

	struct control_id_string_id {
		UT_sint32		controlId;
		XAP_String_Id	stringId;
	} static const rgMapping[] =
	{
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_TEXT_STARTAT,		AP_STRING_ID_DLG_FormatTOC_StartAt},
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_TEXT_TEXTBEFORE,		AP_STRING_ID_DLG_FormatTOC_TextBefore},
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_TEXT_TABSPAGENUM,	AP_STRING_ID_DLG_FormatTOC_DetailsTabPage},
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_TEXT_LABELDEF,		AP_STRING_ID_DLG_FormatTOC_DetailsTop},
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_TEXT_NUMTYPE,		AP_STRING_ID_DLG_FormatTOC_NumberingType},
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_TEXT_TEXTAFTER,		AP_STRING_ID_DLG_FormatTOC_TextAfter},
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_CHECK_INHERITLABEL,	AP_STRING_ID_DLG_FormatTOC_InheritLabel},
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_TEXT_TABLEADER,		AP_STRING_ID_DLG_FormatTOC_TabLeader},
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_TEXT_PAGENUMERING,	AP_STRING_ID_DLG_FormatTOC_PageNumbering},
		{AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_TEXT_IDENT,			AP_STRING_ID_DLG_FormatTOC_Indent},
		{NULL,NULL}
	};

	// Localise the controls
	for (i = 0; i < rgMapping[i].controlId; i++)
		SetDlgItemText(getHandle(), rgMapping[i].controlId, pSS->getValue(rgMapping[i].stringId));

	/* Spin controls */
	SendMessage(GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_SPIN_STARTAT),
		UDM_SETBUDDY, (WPARAM) GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_STARTAT),0);	

	SendMessage(GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_SPIN_STARTAT),UDM_SETRANGE,
		0,(WPARAM)9999);

	SendMessage(GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_SPIN_IDENT),
		UDM_SETBUDDY, (WPARAM) GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_IDENT),0);	

	SendMessage(GetDlgItem(getHandle(),AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_SPIN_IDENT),UDM_SETRANGE,
		0,(WPARAM)9999);

	/* Levels */
	int item;
	int nID [4] = {AP_STRING_ID_DLG_FormatTOC_Level1, AP_STRING_ID_DLG_FormatTOC_Level2,
	AP_STRING_ID_DLG_FormatTOC_Level3, AP_STRING_ID_DLG_FormatTOC_Level4};
	for (i = 0; i < 4; i++)
	{	
		item = SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_LEVEL, CB_ADDSTRING, 0, 
			(LPARAM) pSS->getValue(nID[i]));

		SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_LEVEL, CB_SETITEMDATA, item, i+1);
	}	 

	SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_LEVEL, CB_SETCURSEL, 0, 0);	
	

	/* Now the Page Numbering style */
	const FootnoteTypeDesc * vecTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();
	UT_UTF8String * sProp = NULL;	
	UT_UTF8String  val;
	
	sProp = new UT_UTF8String("toc-page-type");		
	for (; vecTypeList->n !=  _FOOTNOTE_TYPE_INVALID; vecTypeList++)
	{
		const char * szVal = vecTypeList->label;
		item = SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_NUMTYPE, 
			CB_ADDSTRING, 0, (LPARAM) szVal);
			
		item = SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_PAGENUMERING, 
			CB_ADDSTRING, 0, (LPARAM) szVal);
	}	
	
	/*FV_View * pView = static_cast<FV_View *>(getContainer()->getActiveFrame()->getCurrentView());
	val = getContainer()->getTOCPropVal("toc-page-type", getContainer()->getDetailsLevel());	
	UT_sint32 iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(val.utf8_str()));
		
	SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_NUMTYPE, CB_SETCURSEL, iHist, 0);	*/

	/* Tab Type styles */
	const UT_GenericVector<const gchar*> * vecLabels = getContainer()->getVecTABLeadersLabel();
	const UT_GenericVector<const gchar*> * vecProps = getContainer()->getVecTABLeadersProp();
	UT_sint32 nTypes = vecLabels->getItemCount();
	sProp = new UT_UTF8String("toc-tab-leader");	

	for(UT_sint32 j=0; j< nTypes; j++)
	{
		const char * szLab = static_cast<const char *>(vecLabels->getNthItem(j));
		UT_DEBUGMSG(("Got label %s for item %d \n",szLab,j));		

		item = SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_TABLEADER, 
			CB_ADDSTRING, 0, (LPARAM) szLab);
	}
	
	SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_TABLEADER, 
			CB_SETCURSEL, 0, 0);	

	loadCtrlsValuesForDetailsLevel ();	

}


void AP_Win32Dialog_FormatTOC_Layout::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	AP_Win32Dialog_FormatTOC_Layout* pParent=  (AP_Win32Dialog_FormatTOC_Layout*)getContainer();	
	
	
	switch (wId)
	{		
		case AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_LEVEL:
		{
			if (wNotifyCode == CBN_SELCHANGE)
			{
				int nSelected, nLevel;
				HWND hCombo = GetDlgItem(hWnd, AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_LEVEL);

				nSelected = SendMessage(hCombo, CB_GETCURSEL, 0, 0);					

				if (nSelected!=CB_ERR) 
				{					
					saveCtrlsValuesForDetailsLevel (); // Save current level settings

					nLevel = SendMessage(hCombo, CB_GETITEMDATA, nSelected, 0);
					getContainer()->setDetailsLevel (nLevel);
				}
			}

			break;			
		}					
		default:
			break;
	}
}

// Spin control notification
void AP_Win32Dialog_FormatTOC_Layout::_onNotify(LPNMHDR pNMHDR, int iCtrlID)
{

	if (pNMHDR->code != UDN_DELTAPOS)
			return;	

	NMUPDOWN* up = (NMUPDOWN* ) pNMHDR;
	
	/* Start at*/
	UT_sint32 iNew = up->iPos + up->iDelta;
	bool bInc = true;
	if(iNew != getContainer()->m_iStartValue)
	{	
		UT_UTF8String sVal;

		if(iNew < getContainer()->m_iStartValue)
			bInc = false;
		
		getContainer()->m_iStartValue = iNew;
		getContainer()->incrementStartAt(getContainer()->getDetailsLevel(),bInc);
		sVal = getContainer()->getTOCPropVal("toc-label-start",getContainer()->getDetailsLevel());
		SetWindowText (GetDlgItem (getHandle(),
			AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_STARTAT), sVal.utf8_str());
	}

}

void AP_Win32Dialog_FormatTOC_Layout::loadCtrlsValuesForDetailsLevel ()
{
	UT_UTF8String sVal, str_loc;
	UT_String str;	
	UT_sint32 iHist;

	FV_View * pView = static_cast<FV_View *>(getContainer()->getActiveFrame()->getCurrentView());
	sVal = getContainer()->getTOCPropVal("toc-page-type", getContainer()->getDetailsLevel());
	iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(sVal.utf8_str()));
	SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_NUMTYPE, 
			CB_SETCURSEL, iHist, 0);

	/* Text Before */
	sVal = getContainer()->getTOCPropVal("toc-label-before", getContainer()->getDetailsLevel());
	pt_PieceTable::s_getLocalisedStyleName (sVal.utf8_str(), str_loc);
	str = AP_Win32App::s_fromUTF8ToWinLocale (str_loc.utf8_str()); 

	SetWindowText (GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_TEXTBEFORE), str.c_str());	

	/* Text After */	
	sVal = getContainer()->getTOCPropVal("toc-label-after", getContainer()->getDetailsLevel());
	pt_PieceTable::s_getLocalisedStyleName (sVal.utf8_str(), str_loc);
	str = AP_Win32App::s_fromUTF8ToWinLocale (str_loc.utf8_str()); 

	SetWindowText (GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_TEXTAFTER), str.c_str());	

}

void AP_Win32Dialog_FormatTOC_Layout::saveCtrlsValuesForDetailsLevel ()
{
		
	UT_UTF8String sProp, sVal;
	UT_String sNum;
	char szText[1024];
	int nSelected;

	
	/* Text Before */
	GetWindowText(GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_TEXTBEFORE), szText, 1024);

	sVal = AP_Win32App::s_fromWinLocaleToUTF8 (szText);	
	
	sNum =  UT_String_sprintf("%d",getContainer()->getDetailsLevel());	
	sProp = "toc-label-before";
	sProp += sNum.c_str();
	getContainer()->setTOCProperty(sProp, sVal);

	/* Text After */
	GetWindowText(GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_EDIT_TEXTAFTER), szText, 1024);

	sVal = AP_Win32App::s_fromWinLocaleToUTF8 (szText);	
	sProp = "toc-label-after";
	sProp += sNum.c_str();
	getContainer()->setTOCProperty(sProp, sVal);

	/* Numering type */
	HWND hCombo = GetDlgItem(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_NUMTYPE);
	nSelected = SendMessage(hCombo, CB_GETCURSEL, 0, 0);					

	if (nSelected!=CB_ERR) 
	{	
		UT_String sNum;		

		const FootnoteTypeDesc * footnoteTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();
		const char * szVal = footnoteTypeList[nSelected].prop;
		sProp = static_cast<char *> ("toc-page-type");
		sVal = static_cast<const char *> (szVal);
		
		sNum = UT_String_sprintf("%d",getContainer()->getDetailsLevel());
		sProp += sNum.c_str();
		getContainer()->setTOCProperty(sProp,sVal);				
		
	}


}

void AP_Win32Dialog_FormatTOC_Layout::_onApply()
{

	saveCtrlsValuesForDetailsLevel ();	
	getContainer()->Apply(); /* Apply */
	
}


void AP_Win32Dialog_FormatTOC_Layout::_onOK()
{
	_onApply();
}
