/* AbiWord
 * Copyright (C) 2002 Jordi Mas i Hernàndez <jmas@softcatala.org>
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
#include "xap_Win32Frame.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MetaData.h"
#include "ap_Win32Dialog_MetaData.h"
#include "xap_Win32PropertySheet.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"


/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_MetaData::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_MetaData * p = new AP_Win32Dialog_MetaData(pFactory,id);
	return p;
}

AP_Win32Dialog_MetaData::AP_Win32Dialog_MetaData(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MetaData(pDlgFactory,id)
{
}

AP_Win32Dialog_MetaData::~AP_Win32Dialog_MetaData(void)
{
}

void AP_Win32Dialog_MetaData::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);	
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(pFrame->getApp());	
		
	XAP_Win32PropertySheet				sheet;
	AP_Win32Dialog_MetaData_General		general;
	AP_Win32Dialog_MetaData_Summary		summary;
	AP_Win32Dialog_MetaData_Permissions	permissions;	
	
	general.setContainer(this);	
	general.createPage(pWin32App, AP_RID_DIALOG_META_GENERAL, AP_STRING_ID_DLG_MetaData_TAB_General);	
	sheet.addPage(&general);		
    
    summary.setContainer(this);	
    summary.createPage(pWin32App, AP_RID_DIALOG_META_SUMMARY, AP_STRING_ID_DLG_MetaData_TAB_Summary);	
	sheet.addPage(&summary);			
	
	permissions.setContainer(this);	
	permissions.createPage(pWin32App, AP_RID_DIALOG_META_PERMISSIONS, AP_STRING_ID_DLG_MetaData_TAB_Permission);	
	sheet.addPage(&permissions);				
    
	int nRslt = sheet.runModal(pWin32App, pFrame, AP_STRING_ID_DLG_MetaData_Title);
	
	if (nRslt==IDOK)
	{
		general.transferData();
		summary.transferData();
		permissions.transferData();
		setAnswer ( AP_Dialog_MetaData::a_OK);		
	}		
	else
		setAnswer ( AP_Dialog_MetaData::a_CANCEL);		
	
}

/*

	General
	
*/
AP_Win32Dialog_MetaData_General::AP_Win32Dialog_MetaData_General()
{
	setDialogProc(s_pageWndProc);
}

AP_Win32Dialog_MetaData_General::~AP_Win32Dialog_MetaData_General()
{
	
}

/*
	
*/	
int CALLBACK AP_Win32Dialog_MetaData_General::s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,
   LPARAM lParam)
{
	
	if (msg==WM_NOTIFY)
	{
		NMHDR* pHdr = (NMHDR*)lParam;

		if (pHdr->code==PSN_SETACTIVE)
			XAP_Win32DialogHelper::s_centerDialog(GetParent(hWnd));			
	}   	
	
	return XAP_Win32PropertyPage::s_pageWndProc(hWnd, msg, wParam,lParam);
}


/*
	
*/	
void AP_Win32Dialog_MetaData_General::_onInitDialog()
{				
		const XAP_StringSet * pSS = getApp()->getStringSet();		
		
		control_id_string_id rgMapping[] =
		{						 
			{AP_RID_DIALOG_META_GENERAL_TEXT_TITLE,			AP_STRING_ID_DLG_MetaData_Title_LBL},
			{AP_RID_DIALOG_META_GENERAL_TEXT_SUBJECT,		AP_STRING_ID_DLG_MetaData_Subject_LBL},
			{AP_RID_DIALOG_META_GENERAL_TEXT_AUTHOR,		AP_STRING_ID_DLG_MetaData_Author_LBL},
			{AP_RID_DIALOG_META_GENERAL_TEXT_PUBLISHER,		AP_STRING_ID_DLG_MetaData_Publisher_LBL},
			{AP_RID_DIALOG_META_GENERAL_TEXT_CONTRIBUTOR,	AP_STRING_ID_DLG_MetaData_CoAuthor_LBL},			
			{NULL,NULL}
		};		
		
		// Localise the controls
		for (int i = 0; i < rgMapping[i].controlId; i++)		
			SetDlgItemText(getHandle(), rgMapping[i].controlId, pSS->getValue(rgMapping[i].stringId));						
		
		// Setup previous text	
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_GENERAL_EDIT_TITLE,			getContainer()->getTitle().c_str());									
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_GENERAL_EDIT_SUBJECT,		getContainer()->getSubject().c_str());									
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_GENERAL_EDIT_AUTHOR,			getContainer()->getAuthor().c_str());									
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_GENERAL_EDIT_PUBLISHER,		getContainer()->getPublisher().c_str());									
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_GENERAL_EDIT_CONTRIBUTOR,	getContainer()->getCoAuthor().c_str());									
				
}

char* AP_Win32Dialog_MetaData_General::_get_text(XAP_String_Id nID, char *szBuff, int nSize)
{
	 GetDlgItemText(getHandle(), nID, szBuff, nSize); 
	 return szBuff;
}

/*

*/
void AP_Win32Dialog_MetaData_General::_onKillActive()
{	
	char szBuff[1024];

	m_sTitle = 	_get_text(AP_RID_DIALOG_META_GENERAL_EDIT_TITLE, szBuff, sizeof(szBuff));
	m_sSubject = _get_text(AP_RID_DIALOG_META_GENERAL_EDIT_SUBJECT, szBuff, sizeof(szBuff));
	m_sAuthor =  _get_text(AP_RID_DIALOG_META_GENERAL_EDIT_AUTHOR, szBuff, sizeof(szBuff));
	m_sPublisher = _get_text(AP_RID_DIALOG_META_GENERAL_EDIT_PUBLISHER, szBuff, sizeof(szBuff));
	m_sCoAuthor = _get_text(AP_RID_DIALOG_META_GENERAL_EDIT_CONTRIBUTOR, szBuff, sizeof(szBuff));	
}

/*

*/
void AP_Win32Dialog_MetaData_General::transferData()
{	
	getContainer()->setTitle(m_sTitle);
	getContainer()->setSubject(m_sSubject);
	getContainer()->setAuthor(m_sAuthor);
	getContainer()->setPublisher(m_sPublisher);
	getContainer()->setCoAuthor(m_sCoAuthor);	
}


/*

	Summary
	
*/

AP_Win32Dialog_MetaData_Summary::AP_Win32Dialog_MetaData_Summary()
{
	
}

AP_Win32Dialog_MetaData_Summary::~AP_Win32Dialog_MetaData_Summary()
{
	
}

/*
	
*/	
void AP_Win32Dialog_MetaData_Summary::_onInitDialog()
{		
		const XAP_StringSet * pSS = getApp()->getStringSet();
		
		control_id_string_id rgMapping[] =
		{						 
			{AP_RID_DIALOG_META_SUMMARY_TEXT_KEYWORDS,		AP_STRING_ID_DLG_MetaData_Keywords_LBL},
			{AP_RID_DIALOG_META_SUMMARY_TEXT_LANGUAGE,		AP_STRING_ID_DLG_MetaData_Languages_LBL},			
			{AP_RID_DIALOG_META_SUMMARY_TEXT_CATEGORY,		AP_STRING_ID_DLG_MetaData_Category_LBL},     
			{AP_RID_DIALOG_META_SUMMARY_TEXT_DESCRIPTION,	AP_STRING_ID_DLG_MetaData_Description_LBL},     
			
			{NULL,NULL}
		};		
		
		// Localise the controls
		for (int i = 0; i < rgMapping[i].controlId; i++)		
			SetDlgItemText(getHandle(), rgMapping[i].controlId, pSS->getValue(rgMapping[i].stringId));				

		// Setup previous text	
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_SUMMARY_EDIT_CATEGORY,		getContainer()->getCategory().c_str());									
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_SUMMARY_EDIT_KEYWORDS,		getContainer()->getKeywords().c_str());									
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_SUMMARY_EDIT_LANGUAGE,		getContainer()->getLanguages().c_str());									
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_SUMMARY_EDIT_DESCRIPTION,	getContainer()->getDescription().c_str());									
}

char* AP_Win32Dialog_MetaData_Summary::_get_text(XAP_String_Id nID, char *szBuff, int nSize)
{
	 GetDlgItemText(getHandle(), nID, szBuff, nSize); 
	 return szBuff;
}


/*

*/
void AP_Win32Dialog_MetaData_Summary::_onKillActive()
{	
	char szBuff[4096];	// description can be long

	m_sCategory = 	_get_text(AP_RID_DIALOG_META_SUMMARY_EDIT_CATEGORY, szBuff, sizeof(szBuff));
	m_sKeywords = _get_text(AP_RID_DIALOG_META_SUMMARY_EDIT_KEYWORDS, szBuff, sizeof(szBuff));
	m_sLanguages =  _get_text(AP_RID_DIALOG_META_SUMMARY_EDIT_LANGUAGE, szBuff, sizeof(szBuff));
	m_sDescription = _get_text(AP_RID_DIALOG_META_SUMMARY_EDIT_DESCRIPTION, szBuff, sizeof(szBuff));
}

/*

*/
void AP_Win32Dialog_MetaData_Summary::transferData()
{		
	getContainer()->setCategory(m_sCategory);
	getContainer()->setKeywords(m_sKeywords);
	getContainer()->setLanguages(m_sLanguages);
	getContainer()->setDescription(m_sDescription);	
}

/*

	Permissions
	
*/

AP_Win32Dialog_MetaData_Permissions::AP_Win32Dialog_MetaData_Permissions()
{
	
}

AP_Win32Dialog_MetaData_Permissions::~AP_Win32Dialog_MetaData_Permissions()
{
	
}

/*
	
*/	
void AP_Win32Dialog_MetaData_Permissions::_onInitDialog()
{				
		const XAP_StringSet * pSS = getApp()->getStringSet();
		
		control_id_string_id rgMapping[] =
		{	
			{AP_RID_DIALOG_META_PERMISSIONS_TEXT_SOURCE,	AP_STRING_ID_DLG_MetaData_Source_LBL},
			{AP_RID_DIALOG_META_PERMISSIONS_TEXT_RELATION,	AP_STRING_ID_DLG_MetaData_Relation_LBL},
			{AP_RID_DIALOG_META_PERMISSIONS_TEXT_COVERAGE,	AP_STRING_ID_DLG_MetaData_Coverage_LBL},
			{AP_RID_DIALOG_META_PERMISSIONS_TEXT_RIGHTS,	AP_STRING_ID_DLG_MetaData_Rights_LBL},     			
			{NULL,NULL}
		};		
		
		// Localise the controls
		for (int i = 0; i < rgMapping[i].controlId; i++)		
			SetDlgItemText(getHandle(), rgMapping[i].controlId, pSS->getValue(rgMapping[i].stringId));				

		// Setup previous text	
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_PERMISSIONS_EDIT_SOURCE,		getContainer()->getSource().c_str());											
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_PERMISSIONS_EDIT_RELATION,	getContainer()->getRelation().c_str());											
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_PERMISSIONS_EDIT_COVERAGE,	getContainer()->getCoverage().c_str());											
		SetDlgItemText(getHandle(), AP_RID_DIALOG_META_PERMISSIONS_EDIT_RIGHTS,		getContainer()->getRights().c_str());									
}


/*

*/
void AP_Win32Dialog_MetaData_Permissions::_onKillActive()
{	
	char szBuff[1024];
	
	m_sSource = _get_text(AP_RID_DIALOG_META_PERMISSIONS_EDIT_SOURCE, szBuff, sizeof(szBuff));
	m_sRelation = _get_text(AP_RID_DIALOG_META_PERMISSIONS_EDIT_RELATION, szBuff, sizeof(szBuff));
	m_sCoverage =  _get_text(AP_RID_DIALOG_META_PERMISSIONS_EDIT_COVERAGE, szBuff, sizeof(szBuff));
	m_sRights = _get_text(AP_RID_DIALOG_META_PERMISSIONS_EDIT_RIGHTS, szBuff, sizeof(szBuff));
}

/*

*/
void AP_Win32Dialog_MetaData_Permissions::transferData()
{			
	getContainer()->setSource(m_sSource);
	getContainer()->setRelation(m_sRelation);
	getContainer()->setCoverage(m_sCoverage);
	getContainer()->setRights(m_sRights);	
}

char* AP_Win32Dialog_MetaData_Permissions::_get_text(XAP_String_Id nID, char *szBuff, int nSize)
{
	 GetDlgItemText(getHandle(), nID, szBuff, nSize); 
	 return szBuff;
}
