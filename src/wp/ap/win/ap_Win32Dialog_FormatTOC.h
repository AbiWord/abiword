/* AbiWord
 * Copyright (C) 2004 Jordi Mas i Hernàndez <jmas@softcatala.org>
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

#ifndef AP_WIN32DIALOG_FORMATTOC_H
#define AP_WIN32DIALOG_FORMATTOC_H

#include "ap_Dialog_FormatTOC.h"
#include "xap_Win32PropertySheet.h"
#include "xap_Win32DialogBase.h"

class XAP_Win32Frame;
class AP_Win32Dialog_FormatTOC_General;
class AP_Win32Dialog_FormatTOC_Sheet;
class AP_Win32Dialog_FormatTOC_Layout;

class ABI_EXPORT AP_Win32Dialog_FormatTOC: public AP_Dialog_FormatTOC, XAP_Win32DialogBase
{
public:
	AP_Win32Dialog_FormatTOC(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_FormatTOC(void);
	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	virtual void            setTOCPropsInGUI(void);
   	virtual void            setSensitivity(bool bSensitive);
	virtual void            destroy(void);
	virtual void            activate(void);
	void 					setStyle(HWND hWnd, int nCtrlID);
	void 					setMainLevel(UT_sint32 iLevel);
	void					setDetailsLevel(UT_sint32 iLevel);
	UT_sint32				m_iStartValue;

private:

	AP_Win32Dialog_FormatTOC_Sheet* 	m_pSheet;
	AP_Win32Dialog_FormatTOC_General*	m_pGeneral;
	AP_Win32Dialog_FormatTOC_Layout*	m_pLayout;


};

class ABI_EXPORT AP_Win32Dialog_FormatTOC_Sheet : public XAP_Win32PropertySheet
{

public:

	AP_Win32Dialog_FormatTOC_Sheet();

	void						setContainer(AP_Win32Dialog_FormatTOC*	pData){m_pData=pData;};
	AP_Win32Dialog_FormatTOC*	getContainer(){return m_pData;};
	static int CALLBACK			s_sheetInit(HWND hwnd,  UINT uMsg,  LPARAM lParam);
	void						_onInitDialog(HWND hwnd);
	virtual void				cleanup(void);
	virtual	void				_onOK();

private:

	AP_Win32Dialog_FormatTOC*			m_pData;

};

class ABI_EXPORT AP_Win32Dialog_FormatTOC_General: public XAP_Win32PropertyPage
{
public:

	AP_Win32Dialog_FormatTOC_General();
	~AP_Win32Dialog_FormatTOC_General();

	void						setContainer(AP_Win32Dialog_FormatTOC*	pData){m_pData=pData;};
	AP_Win32Dialog_FormatTOC*	getContainer(){return m_pData;};
	void						_fillGUI(void);
	virtual BOOL				_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
private:

	virtual	void				_onInitDialog();
	virtual	void				_onApply();
	virtual	void				_onOK();

	AP_Win32Dialog_FormatTOC*			m_pData;

};

class ABI_EXPORT AP_Win32Dialog_FormatTOC_Layout: public XAP_Win32PropertyPage
{
public:

	AP_Win32Dialog_FormatTOC_Layout();
	~AP_Win32Dialog_FormatTOC_Layout();

	void						setContainer(AP_Win32Dialog_FormatTOC*	pData){m_pData=pData;};
	AP_Win32Dialog_FormatTOC*	getContainer(){return m_pData;};
	void						_fillGUI(void);
	virtual BOOL				_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	virtual void				_onNotify(LPNMHDR hdr, int iCtrlID);
	void						saveCtrlsValuesForDetailsLevel ();
	void						loadCtrlsValuesForDetailsLevel ();

private:

	virtual	void				_onInitDialog();
	virtual	void				_onApply();
	virtual	void				_onOK();
	void						getCtrlsValues ();

	AP_Win32Dialog_FormatTOC*			m_pData;

};





#endif /* AP_WIN32DIALOG_FORMATOC_H */
