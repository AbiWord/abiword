/* AbiWord
 * Copyright (C) 2002 Jordi Mas i Hern�ndez <jmas@softcatala.org>
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
#include "ap_Win32App.h"
#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MetaData.h"
#include "ap_Win32Dialog_MetaData.h"
#include "xap_Win32PropertySheet.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"
#include "xap_EncodingManager.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_stringbuf.h"

/*
	Helpers
*/
#if 0
static UT_String 	sRslt;
static UT_UTF8String  sRsltUTF8;

const char* fromUTF8toAnsi(const char* szIn)
{
	sRslt =	AP_Win32App::s_fromUTF8ToAnsi(szIn);
	return sRslt.utf8_str();
}

const char* fromAnsitoUTF8(const char* szIn)
{
	sRsltUTF8 = AP_Win32App::s_fromAnsiToUTF8(szIn);				
	return sRsltUTF8.utf8_str();
}
#endif


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
		
	AP_Win32Dialog_MetaData_Sheet	sheet;
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
BOOL AP_Win32Dialog_MetaData_General::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{				
	const XAP_StringSet * pSS = getApp()->getStringSet();
	setStringSet(pSS);
	setHandle(getHandle());		
	
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
		localizeControlText(rgMapping[i].controlId, rgMapping[i].stringId);						
	
	// Setup previous text	
	setControlText(AP_RID_DIALOG_META_GENERAL_EDIT_TITLE,				getContainer()->getTitle().c_str());									
	setControlText(AP_RID_DIALOG_META_GENERAL_EDIT_SUBJECT,		getContainer()->getSubject().c_str());									
	setControlText(AP_RID_DIALOG_META_GENERAL_EDIT_AUTHOR,		getContainer()->getAuthor().c_str());									
	setControlText(AP_RID_DIALOG_META_GENERAL_EDIT_PUBLISHER,		getContainer()->getPublisher().c_str());									
	setControlText(AP_RID_DIALOG_META_GENERAL_EDIT_CONTRIBUTOR,	getContainer()->getCoAuthor().c_str());									
	
	
	HWND hParent = GetParent(getHandle());						
	SetDlgItemText(hParent, IDOK, XAP_Win32App::getWideString(pSS->getValue(XAP_STRING_ID_DLG_OK)));
	SetDlgItemText(hParent, IDCANCEL, XAP_Win32App::getWideString(pSS->getValue(XAP_STRING_ID_DLG_Cancel)));	
	
	centerDialog();	
	return true;						
}

char* AP_Win32Dialog_MetaData_General::_get_text(XAP_String_Id nID, char *szBuff, int nSize)
{
	*szBuff=0;
	 getControlText(nID, szBuff, nSize); 
	 //strcpy (szBuff, (AP_Win32App::s_fromAnsiToUTF8(szBuff)).utf8_str());
	 return szBuff;
}

/*

*/
void AP_Win32Dialog_MetaData_General::_onOK()
{	
	char szBuff[1024];

	m_sTitle = _get_text(AP_RID_DIALOG_META_GENERAL_EDIT_TITLE, szBuff, sizeof(szBuff));
	m_sSubject = _get_text(AP_RID_DIALOG_META_GENERAL_EDIT_SUBJECT, szBuff, sizeof(szBuff));
	m_sAuthor =  _get_text(AP_RID_DIALOG_META_GENERAL_EDIT_AUTHOR, szBuff, sizeof(szBuff));
	m_sPublisher = _get_text(AP_RID_DIALOG_META_GENERAL_EDIT_PUBLISHER, szBuff, sizeof(szBuff));
	m_sCoAuthor = _get_text(AP_RID_DIALOG_META_GENERAL_EDIT_CONTRIBUTOR, szBuff, sizeof(szBuff));	
}

/*

*/
void AP_Win32Dialog_MetaData_General::transferData()
{	
	getContainer()->setTitle(m_sTitle.utf8_str());
	getContainer()->setSubject(m_sSubject.utf8_str());
	getContainer()->setAuthor(m_sAuthor.utf8_str());
	getContainer()->setPublisher(m_sPublisher.utf8_str());
	getContainer()->setCoAuthor(m_sCoAuthor.utf8_str());	
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
BOOL AP_Win32Dialog_MetaData_Summary::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{		
	const XAP_StringSet * pSS = getApp()->getStringSet();
	setStringSet(pSS);
	setHandle(getHandle());
	
	control_id_string_id rgMapping[] =
	{						 
		{AP_RID_DIALOG_META_SUMMARY_TEXT_KEYWORDS,		AP_STRING_ID_DLG_MetaData_Keywords_LBL},
		{AP_RID_DIALOG_META_SUMMARY_TEXT_LANGUAGE,		AP_STRING_ID_DLG_MetaData_Languages_LBL},			
		{AP_RID_DIALOG_META_SUMMARY_TEXT_CATEGORY,		AP_STRING_ID_DLG_MetaData_Category_LBL},     
		{AP_RID_DIALOG_META_SUMMARY_TEXT_DESCRIPTION,		AP_STRING_ID_DLG_MetaData_Description_LBL},     
		
		{NULL,NULL}
	};		
	
	// Localise the controls
	for (int i = 0; i < rgMapping[i].controlId; i++)		
		localizeControlText(rgMapping[i].controlId, rgMapping[i].stringId);				

	// Setup previous text	
	setControlText(AP_RID_DIALOG_META_SUMMARY_EDIT_CATEGORY,	getContainer()->getCategory().c_str());									
	setControlText(AP_RID_DIALOG_META_SUMMARY_EDIT_KEYWORDS,	getContainer()->getKeywords().c_str());									
	setControlText(AP_RID_DIALOG_META_SUMMARY_EDIT_LANGUAGE,	getContainer()->getLanguages().c_str());									
	setControlText(AP_RID_DIALOG_META_SUMMARY_EDIT_DESCRIPTION,	getContainer()->getDescription().c_str());									

	return true;
}

char* AP_Win32Dialog_MetaData_Summary::_get_text(XAP_String_Id nID, char *szBuff, int nSize)
{
	*szBuff=0;
	 getControlText(nID, szBuff, nSize); 
	 //strcpy (szBuff, (AP_Win32App::s_fromAnsiToUTF8(szBuff)).utf8_str());
	 return szBuff;
}


/*

*/
void AP_Win32Dialog_MetaData_Summary::_onOK()
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
	getContainer()->setCategory(m_sCategory.utf8_str());
	getContainer()->setKeywords(m_sKeywords.utf8_str());
	getContainer()->setLanguages(m_sLanguages.utf8_str());
	getContainer()->setDescription(m_sDescription.utf8_str());	
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
BOOL AP_Win32Dialog_MetaData_Permissions::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{				
	const XAP_StringSet * pSS = getApp()->getStringSet();
	setStringSet(pSS);
	setHandle(getHandle());
	
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
		localizeControlText(rgMapping[i].controlId, rgMapping[i].stringId);				

	// Setup previous text	
	setControlText(AP_RID_DIALOG_META_PERMISSIONS_EDIT_SOURCE,	getContainer()->getSource().c_str());											
	setControlText(AP_RID_DIALOG_META_PERMISSIONS_EDIT_RELATION,	getContainer()->getRelation().c_str());											
	setControlText(AP_RID_DIALOG_META_PERMISSIONS_EDIT_COVERAGE,	getContainer()->getCoverage().c_str());											
	setControlText(AP_RID_DIALOG_META_PERMISSIONS_EDIT_RIGHTS,	getContainer()->getRights().c_str());

	return true;									

}												

	

/*

*/
void AP_Win32Dialog_MetaData_Permissions::_onOK()
{	
	char szBuff[1024];
	
	m_sSource = _get_text(AP_RID_DIALOG_META_PERMISSIONS_EDIT_SOURCE, szBuff, sizeof(szBuff));
	m_sRelation = _get_text(AP_RID_DIALOG_META_PERMISSIONS_EDIT_RELATION, szBuff, sizeof(szBuff));
	m_sCoverage = _get_text(AP_RID_DIALOG_META_PERMISSIONS_EDIT_COVERAGE, szBuff, sizeof(szBuff));
	m_sRights = _get_text(AP_RID_DIALOG_META_PERMISSIONS_EDIT_RIGHTS, szBuff, sizeof(szBuff));

}

/*

*/
void AP_Win32Dialog_MetaData_Permissions::transferData()
{			
	getContainer()->setSource(m_sSource.utf8_str());
	getContainer()->setRelation(m_sRelation.utf8_str());
	getContainer()->setCoverage(m_sCoverage.utf8_str());
	getContainer()->setRights(m_sRights.utf8_str());	
}

char* AP_Win32Dialog_MetaData_Permissions::_get_text(XAP_String_Id nID, char *szBuff, int nSize)
{
	 *szBuff=0;
	 getControlText(nID, szBuff, nSize); 
	 //strcpy (szBuff, (AP_Win32App::s_fromAnsiToUTF8(szBuff)).utf8_str());
	 return szBuff;
}
