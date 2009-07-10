// xap_Win32DialogBase.h

/* AbiWord
 * Copyright (C) 2003 AbiSource, Inc.
 *           (C) 2003 Mike Nordell
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

#ifndef XAP_Win32DialogBase_H
#define XAP_Win32DialogBase_H

// MSVC++ warns about using 'this' in initializer list.
// and the DialogHelper uses 'this' typically for its contructor
#ifdef _MSC_VER	
#pragma warning(disable: 4355)
#endif

#include <windows.h>
#include <commctrl.h>

#include "ut_types.h"
/*****************************************************************/


class XAP_Frame;
class XAP_StringSet;

class ABI_EXPORT XAP_Win32DialogBase
{
public:
	XAP_Win32DialogBase() : m_tag(magic_tag), m_hDlg(0), m_pDlg(0), m_pSS(0) {}
	// no need for user-defined destructor
		
protected:
	void createModal(XAP_Frame* pFrame, LPCTSTR dlgTemplate);
	HWND createModeless(XAP_Frame* pFrame, LPCTSTR dlgTemplate);


	// Subclasses: override this and use it as your DLGPROC
	virtual BOOL _onDlgMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL _onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam) = 0;
	virtual BOOL _onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) = 0;
	virtual BOOL _onDeltaPos(NM_UPDOWN * pnmud) = 0;
	virtual BOOL _callHelp();


	// Control Functionality
	void checkButton(UT_sint32 controlId, bool bChecked = true);
	void enableControl(UT_sint32 controlId, bool bEnabled = true);
	void destroyWindow();
	void setDialogTitle(LPCSTR p_str);
	void localizeDialogTitle(UT_uint32 stringId);
	int	 showWindow( int Mode );
	int	 showControl(UT_sint32 controlId, int Mode);
	int	 bringWindowToTop();

	// Combo boxes.

	int	 addItemToCombo(UT_sint32 controlId, LPCSTR p_str);
	void selectComboItem(UT_sint32 controlId, int index);
	int  setComboDataItem(UT_sint32 controlId, int nIndex, DWORD dwData);
	int  getComboDataItem(UT_sint32 controlId, int nIndex);
	int  getComboItemIndex(UT_sint32 controlId, LPCSTR p_str);
	int	 getComboSelectedIndex(UT_sint32 controlId) const;
	void resetComboContent(UT_sint32 controlId);

	// List boxes

	void resetContent(UT_sint32 controlId);
	int	 addItemToList(UT_sint32 controlId, LPCSTR p_str);
	int	 getListSelectedIndex(UT_sint32 controlId) const;
	int  setListDataItem(UT_sint32 controlId, int nIndex, DWORD dwData);
	int  getListDataItem(UT_sint32 controlId, int nIndex);
	void selectListItem(UT_sint32 controlId, int index);
	void getListText(UT_sint32 controlId, int index, char *p_str) const;

	// Controls
	void setControlText(UT_sint32 controlId, LPCSTR p_str);
	void localizeControlText(UT_sint32 controlId, UT_uint32 stringId);
	void setControlInt(UT_sint32 controlId, int value);
	int	 getControlInt(UT_sint32 controlId) const;
	
	void selectControlText(UT_sint32 controlId, UT_sint32 start, UT_sint32 end);
	
	int  isChecked(UT_sint32 controlId) const;
	void getControlText(UT_sint32 controlId, LPSTR p_buffer, UT_sint32 Buffer_length) const;

	bool isControlVisible(UT_sint32	controlId) const;

	void centerDialog();
	void setHandle(HWND hWnd) { m_hDlg = hWnd; };
	void setDialog(XAP_Dialog * pDlg) { m_pDlg = pDlg; };
	bool isDialogValid() const;
	
private:
	static BOOL CALLBACK s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	
	// disallow copying and assignment
	XAP_Win32DialogBase(const XAP_Win32DialogBase&);	// no impl.
	void operator=(const XAP_Win32DialogBase&);			// no impl.

	enum { // VC6 can't handle static int here, why an enum is the only way...
		magic_tag = 0x327211
	};

	int m_tag;	// all for safety
	HWND m_hDlg;
	XAP_Dialog* m_pDlg;
	const XAP_StringSet* m_pSS;
};


#endif /* XAP_Win32DialogBase_H */
