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

#ifndef XAP_Win32PropertySheet_H
#define XAP_Win32PropertySheet_H

#include <commctrl.h>
#include "ut_vector.h"
#include "xap_Frame.h"

/*****************************************************************/

class XAP_Win32PropertySheet;

// HACK: forward declarations for subclassed controls
#ifdef STRICT   
#define WHICHPROC	WNDPROC
#else   
#define WHICHPROC	FARPROC
#endif

#define ID_APPLY_NOW                    0x3021


class XAP_Win32PropertyPage
{
public:
	
	XAP_Win32PropertyPage();	
	
	void 						createPage(XAP_Win32App* pWin32App, WORD wRscID, XAP_String_Id	nID = 0);	
	PROPSHEETPAGE*				getStruct(){return &m_page;}
	XAP_Win32App*				getApp(){return m_pWin32App;}
	HWND						getHandle(){return m_hWnd;}
	XAP_Win32PropertySheet*		getParent(){return m_pParent;}
	void						setDialogProc(DLGPROC pfnDlgProc){m_pfnDlgProc=pfnDlgProc;};	
	virtual	void				_onInitDialog(){};
	virtual	void				_onKillActive(){}; 
	virtual void				_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam){};
	static int CALLBACK			s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);
	
	
	
private:
		
	PROPSHEETPAGE				m_page;
	HPROPSHEETPAGE	 			m_hdle;	
	HWND						m_hWnd;
	XAP_Win32App*				m_pWin32App;
	XAP_Win32PropertySheet*		m_pParent;
	DLGPROC						m_pfnDlgProc;
		
};


class XAP_Win32PropertySheet
{
public:
	XAP_Win32PropertySheet();		
	
public:

	int							runModal(XAP_Win32App* pWin32App,XAP_Frame*	pFrame, XAP_String_Id	nID = 0);							
	void 						addPage(XAP_Win32PropertyPage* pPage);
	PROPSHEETPAGE* 				_buildPageArray(void);	
	static int CALLBACK			s_sheetWndProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);	
	virtual	void				_onInitDialog(HWND hwnd){};	
	void						setDialogProc(DLGPROC pfnDlgProc){m_pfnDlgProc=pfnDlgProc;};	
	virtual int					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam){return 1;};
	void						setApplyButton(bool b){m_bApplyButton=b;};	
	void 						destroy(void);
	void						setCallBack(PFNPROPSHEETCALLBACK pCallback) {m_pCallback=pCallback;};
	
	int							m_nRslt;
private:	
	
	HWND						m_hWnd;
	UT_Vector					m_vecPages;
	PROPSHEETHEADER				m_psh;	
	PFNPROPSHEETCALLBACK		m_pCallback;
	DLGPROC						m_pfnDlgProc;
	WHICHPROC 					m_lpfnDefSheet; 
	bool						m_bApplyButton;
	
};

#endif /* XAP_Win32PropertySheet_H */
