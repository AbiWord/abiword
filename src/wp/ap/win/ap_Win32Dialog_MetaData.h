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

#ifndef AP_WIN32DIALOG_METADATA_H
#define AP_WIN32DIALOG_METADATA_H

#include "ap_Dialog_MetaData.h"
#include "xap_Win32PropertySheet.h"

// localise controls
typedef struct
{
	UT_sint32		controlId;
	XAP_String_Id	stringId;
} control_id_string_id;

typedef struct
{
	UT_sint32		controlId;
	const char		*string;
} control_text;

typedef struct
{
	UT_sint32		controlId;
	UT_String		*string;
} control_var;


class XAP_Win32Frame;

/*****************************************************************/

class AP_Win32Dialog_MetaData: public AP_Dialog_MetaData
{
public:
	AP_Win32Dialog_MetaData(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_MetaData(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
protected:

};

/*
	Sheet
*/
class AP_Win32Dialog_MetaData_Sheet: public XAP_Win32PropertySheet
{

public:	
		AP_Win32Dialog_MetaData_Sheet() : XAP_Win32PropertySheet() {};				
		void _onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam) {};		
		int _onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {return 1;};
};


class AP_Win32Dialog_MetaData_General: public XAP_Win32PropertyPage, XAP_Win32DialogBase
{
	
public:		
								AP_Win32Dialog_MetaData_General();
								~AP_Win32Dialog_MetaData_General();	

	void						setContainer(AP_Win32Dialog_MetaData*	pData){m_pData=pData;};
	AP_Win32Dialog_MetaData*	getContainer(){return m_pData;};
	void						transferData();	
	
private:

	virtual	BOOL				_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	virtual BOOL				_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {return false;};
	virtual	void				_onOK(); 	
	char* 						_get_text(XAP_String_Id nID, char *szBuff, int nSize);
	
	UT_UTF8String					m_sTitle;
	UT_UTF8String					m_sSubject;
	UT_UTF8String					m_sAuthor;
	UT_UTF8String					m_sPublisher;
	UT_UTF8String					m_sCoAuthor;
	AP_Win32Dialog_MetaData*	m_pData;	
	
};

class AP_Win32Dialog_MetaData_Summary: public XAP_Win32PropertyPage, XAP_Win32DialogBase
{
	public:		
								AP_Win32Dialog_MetaData_Summary();
								~AP_Win32Dialog_MetaData_Summary();
								
	void						setContainer(AP_Win32Dialog_MetaData*	pData){m_pData=pData;};
	AP_Win32Dialog_MetaData*	getContainer(){return m_pData;};
	void						transferData();

private:
						
	virtual	void				_onOK(); 	
	char* 						_get_text(XAP_String_Id nID, char *szBuff, int nSize);	
	virtual	BOOL				_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	virtual BOOL				_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {return false;};	
	
	AP_Win32Dialog_MetaData*	m_pData;	
	UT_UTF8String					m_sCategory;
	UT_UTF8String					m_sKeywords;
	UT_UTF8String					m_sLanguages;
	UT_UTF8String					m_sDescription;
	
};

class AP_Win32Dialog_MetaData_Permissions: public XAP_Win32PropertyPage, XAP_Win32DialogBase
{
public:		
								AP_Win32Dialog_MetaData_Permissions();
								~AP_Win32Dialog_MetaData_Permissions();	
	
	void						setContainer(AP_Win32Dialog_MetaData*	pData){m_pData=pData;};
	AP_Win32Dialog_MetaData*	getContainer(){return m_pData;};
	void						transferData();
		
private:	

	char* 						_get_text(XAP_String_Id nID, char *szBuff, int nSize);
	virtual	BOOL				_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	virtual BOOL				_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {return false;};			
	virtual	void				_onOK(); 	
	
	AP_Win32Dialog_MetaData*	m_pData;	
	UT_UTF8String					m_sSource;
	UT_UTF8String					m_sRelation;
	UT_UTF8String					m_sCoverage;
	UT_UTF8String					m_sRights;	
};

#endif /* AP_WIN32DIALOG_METADATA_H */
