/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2004 Mikey Cooper (mikey@bluey.com)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_WIN32DIALOG_PAGESETUP_H
#define AP_WIN32DIALOG_PAGESETUP_H

#include "ap_Dialog_PageSetup.h"
#include "xap_Win32PropertySheet.h"
#include "xap_Frame.h"
#include "xap_Win32DialogBase.h"

class AP_Win32Dialog_PageSetup;
/*
	Sheet
*/
class ABI_EXPORT AP_Win32Dialog_PageSetup_Sheet: public XAP_Win32PropertySheet, XAP_Win32DialogBase
{
public:
		AP_Win32Dialog_PageSetup_Sheet();
		void _onInitDialog(HWND hwnd);

		void setParent(AP_Win32Dialog_PageSetup*	pData){m_pParent=pData;};
		AP_Win32Dialog_PageSetup* getParent(){return m_pParent;};
		int _onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK s_sheetInit(HWND hwnd,  UINT uMsg,  LPARAM lParam);

private:

		AP_Win32Dialog_PageSetup*	m_pParent;
};

/*
	Page page
*/
class ABI_EXPORT AP_Win32Dialog_PageSetup_Page: public XAP_Win32PropertyPage
{
public:
								AP_Win32Dialog_PageSetup_Page();
								~AP_Win32Dialog_PageSetup_Page();

	void						setContainer(AP_Win32Dialog_PageSetup*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_PageSetup*	getContainer(){return m_pParent;};
	static INT_PTR CALLBACK		s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);

protected:
	void						doSpinControl(UT_uint32 id, UT_sint32 delta);

private:

	void						_onInitDialog();
	void						_onKillActive(){};
	BOOL						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	AP_Win32Dialog_PageSetup*	m_pParent;
    int							m_nCentered;
};

/*
	Margin page
*/
class ABI_EXPORT AP_Win32Dialog_PageSetup_Margin: public XAP_Win32PropertyPage
{
public:
								AP_Win32Dialog_PageSetup_Margin();
								~AP_Win32Dialog_PageSetup_Margin();

	void						setContainer(AP_Win32Dialog_PageSetup*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_PageSetup*	getContainer(){return m_pParent;};
	static INT_PTR CALLBACK		s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);

protected:
	void						doSpinControl(UT_uint32 id, UT_sint32 delta);

private:

	void						_onInitDialog();
	void						_onKillActive(){};
	BOOL						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	AP_Win32Dialog_PageSetup*	m_pParent;

};




/*****************************************************************/
class ABI_EXPORT AP_Win32Dialog_PageSetup: public AP_Dialog_PageSetup
{
friend class AP_Win32Dialog_PageSetup_Sheet;
friend class AP_Win32Dialog_PageSetup_Page;
friend class AP_Win32Dialog_PageSetup_Margin;

public:
	AP_Win32Dialog_PageSetup(XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_PageSetup (void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	XAP_DialogFactory * 	getDialogFactory() {return	m_pDialogFactory;};
	XAP_Frame *				getFrame() {return	m_pFrame;};

protected:

	AP_Win32Dialog_PageSetup_Page		m_page;
 	AP_Win32Dialog_PageSetup_Margin		m_margin;
	//UT_sint32					m_curSelection;

	void						updatePageSize();
	void						updateWidth();
	void						updateHeight();
	void						updateMargins();
	void						updateTopMargin();
	void						updateBottomMargin();
	void						updateLeftMargin();
	void						updateRightMargin();
	void						updateHeaderMargin();
	void						updateFooterMargin();
	void						updatePreview();

	//HWND						m_hwndDlg;		// parent dialog
	//HWND						m_hwndTab;		// tab control in parent dialog

	//int							m_nrSubDlgs;		// number of tabs on tab control
	//UT_Vector					m_vecSubDlgHWnd;	// hwnd to each sub-dialog

	HBITMAP					m_bmpLandscape;
	HBITMAP					m_bmpPortrait;
	HBITMAP					m_bmpPreview;

	fp_PageSize					m_PageSize;

private:
	XAP_DialogFactory * 		m_pDialogFactory;
};

#endif
