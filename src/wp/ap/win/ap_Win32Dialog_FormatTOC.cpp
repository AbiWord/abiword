/* AbiWord
 * Copyright (C) 2004 Jordi Mas i Hern�ndez <jmas@softcatala.org>
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
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(pFrame->getApp());

	m_pSheet = new AP_Win32Dialog_FormatTOC_Sheet();
	m_pSheet->setContainer (this);	
	m_pSheet->setApplyButton (true);
	m_pSheet->setOkButton (false);	
	
	m_pGeneral = new AP_Win32Dialog_FormatTOC_General();		
	m_pGeneral->setContainer (this);	
	m_pGeneral->createPage(pWin32App, AP_RID_DIALOG_FORMATTOC_GENERAL, AP_STRING_ID_DLG_FormatTOC_General);
	
	m_pLayout = new AP_Win32Dialog_FormatTOC_Layout();		
	m_pLayout->setContainer (this);	
	m_pLayout->createPage(pWin32App, AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS, AP_STRING_ID_DLG_FormatTOC_LayoutDetails);	
	
	m_pSheet->addPage(m_pGeneral);
	m_pSheet->addPage(m_pLayout);
	m_pSheet->runModeless(pWin32App, pFrame, AP_STRING_ID_DLG_FormatTOC_Title);	

	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	startUpdater(); 
}

void AP_Win32Dialog_FormatTOC::setStyle(HWND hwndCtrl)
{
	UT_UTF8String sVal;	
	UT_UTF8String sProp = static_cast<char *> ("toc-prop");
	if(UT_stricmp("toc-heading-style",sProp.utf8_str()) != 0)
	{
		UT_String sNum =  UT_String_sprintf("%d",getMainLevel());
		sProp += sNum.c_str();
	}
	sVal = getNewStyle(sProp);
	SendMessage (hwndCtrl, 	WM_SETTEXT, 0,  (LPARAM)sVal.utf8_str());
	setTOCProperty(sProp,sVal);
}


void AP_Win32Dialog_FormatTOC::setMainLevel(UT_sint32 iLevel)
{
	AP_Dialog_FormatTOC::setMainLevel(iLevel);
	UT_UTF8String sVal;
	sVal = getTOCPropVal("toc-dest-style",getMainLevel());

	SendMessage (GetDlgItem (m_pGeneral->getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_DISPLAYSTYLEVALUE), 	
		WM_SETTEXT, 0, (LPARAM)sVal.utf8_str());

	sVal = getTOCPropVal("toc-source-style",getMainLevel());
	SendMessage (GetDlgItem (m_pGeneral->getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_FILLSTYLEVALUE), 	
		WM_SETTEXT, 0, (LPARAM)sVal.utf8_str());	

	sVal = getTOCPropVal("toc-has-label",getMainLevel());
	CheckDlgButton(m_pGeneral->getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASHEADING,
		(UT_stricmp(sVal.utf8_str(),"1") == 0) ? BST_CHECKED :BST_UNCHECKED);	
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

					  
void AP_Win32Dialog_FormatTOC_Sheet::_onCancel()
{
	getContainer()->destroy();
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
	
	UT_UTF8String sVal; 

	sVal = getContainer()->getTOCPropVal("toc-has-heading"); 
	CheckDlgButton(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASHEADING,
		(UT_stricmp(sVal.utf8_str(),"1") == 0) ? BST_CHECKED :BST_UNCHECKED);

	sVal = getContainer()->getTOCPropVal("toc-has-label"); 	 
	CheckDlgButton(getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASLEVEL,
		(UT_stricmp(sVal.utf8_str(),"1") == 0) ? BST_CHECKED :BST_UNCHECKED);

	sVal = getContainer()->getTOCPropVal("toc-heading-style");
	SendMessage (GetDlgItem (getHandle(), AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLEVALUE), 	
		WM_SETTEXT, 0, (LPARAM)sVal.utf8_str());

	getContainer()->setMainLevel(getContainer()->getMainLevel());

	sVal = getContainer()->getTOCPropVal("toc-heading");
	SetWindowText (GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_GENERAL_EDIT_HEADINGTEXT), sVal.utf8_str());
	
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
			UT_UTF8String sProp = static_cast<char *> ("toc-prop");
			UT_UTF8String sVal = "1";

			if (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_FORMATTOC_GENERAL_CHECK_HASHEADING) != BST_CHECKED)
				sVal = "0";		
			
			if(UT_stricmp("toc-has-heading",sProp.utf8_str()) != 0)
			{
				UT_String sNum =  UT_String_sprintf("%d",getContainer()->getMainLevel());
				sProp += sNum.c_str();
			} 

			getContainer()->setTOCProperty(sProp,sVal);			
			break;
		}
				
		case AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_HEADINGSTYLE:						   
			getContainer()->setStyle(GetDlgItem (hWnd, AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_HEADINGSTYLEVALUE));
			break;

		case AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_FILLSTYLE:						   
			getContainer()->setStyle(GetDlgItem (hWnd, AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_FILLSTYLEVALUE));
			break;

		case AP_RID_DIALOG_FORMATTOC_GENERAL_BUTTON_DISPLAYSTYLE:						   
			getContainer()->setStyle(GetDlgItem (hWnd, AP_RID_DIALOG_FORMATTOC_GENERAL_TEXT_DISPLAYSTYLEVALUE));
			break;

		case AP_RID_DIALOG_FORMATTOC_GENERAL_COMBO_LEVEL:
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
			
		default:
			break;
	}
}

void AP_Win32Dialog_FormatTOC_General::_onApply()
{
	char szText[1024];	

	GetWindowText(GetDlgItem (getHandle(),
		AP_RID_DIALOG_FORMATTOC_GENERAL_EDIT_HEADINGTEXT), szText, 1024);

	getContainer()->setTOCProperty("toc-heading", szText);
	getContainer()->Apply();
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
		
}

void AP_Win32Dialog_FormatTOC_Layout::_onInitDialog()
{

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
	const UT_GenericVector<const XML_Char*> * vecTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();
	const UT_GenericVector<const XML_Char*> * vecPropList = getContainer()->getVecLabelPropValue();
	UT_sint32 nTypes = vecTypeList->getItemCount();
	UT_UTF8String * sProp = NULL;
	//UT_UTF8String * sVal = NULL;
	UT_UTF8String  val;
	int j;
	
	sProp = new UT_UTF8String("toc-page-type");	
	//m_vecAllPropVals.addItem(sProp);
	for (j=0; j< nTypes; j++)
	{
		//sVal = new UT_UTF8String(vecTypeList->getNthItem(j));		
		
		const char * szVal = static_cast<const char *>(vecTypeList->getNthItem(j));
		item = SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_NUMTYPE, 
			CB_ADDSTRING, 0, (LPARAM) szVal);
			
		item = SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_PAGENUMERING, 
			CB_ADDSTRING, 0, (LPARAM) szVal);
	}	
	
	FV_View * pView = static_cast<FV_View *>(getContainer()->getActiveFrame()->getCurrentView());
	val = getContainer()->getTOCPropVal("toc-page-type", getContainer()->getDetailsLevel());	
	UT_sint32 iHist = static_cast<UT_sint32>(pView->getLayout()->FootnoteTypeFromString(val.utf8_str()));
		
	SendDlgItemMessage(getHandle(), AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_NUMTYPE, CB_SETCURSEL, iHist, 0);	
	
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
			int nSelected, nLevel;
			HWND hCombo = GetDlgItem(hWnd, AP_RID_DIALOG_FORMATTOC_LAYOUTDETAILS_COMBO_LEVEL);

			nSelected = SendMessage(hCombo, CB_GETCURSEL, 0, 0);					

			if (nSelected!=CB_ERR) 
			{		
				nLevel  = SendMessage(hCombo, CB_GETITEMDATA, nSelected, 0);
				getContainer()->setDetailsLevel (nLevel);
			}
			
		}
			
		default:
			break;
	}
}

void AP_Win32Dialog_FormatTOC_Layout::_onApply()
{
	
}