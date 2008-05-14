/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#ifndef AP_WIN32DIALOG_SPELL_H
#define AP_WIN32DIALOG_SPELL_H

#include "ap_Dialog_Spell.h"
#include "xap_Frame.h"


/*****************************************************************/

class ABI_EXPORT AP_Win32Dialog_Spell: public AP_Dialog_Spell
{
public:
	AP_Win32Dialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Spell(void);
   
	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);
	
protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	
	void					_toggleChangeButtons(bool bEnable) const;
	void					_showMisspelledWord(void);
	void					_suggestChange(void);
	void					_change(void);
	void					_changeAll(void);
	void					_tryAgain(void);

	HWND					m_hwndDlg;
	HWND					m_hwndSentence;		// AP_RID_DIALOG_SPELL_RICH_SENTENCE
	HWND					m_hwndChangeTo;		// AP_RID_DIALOG_SPELL_EDIT_CHANGE
	HWND					m_hwndSuggest;		// AP_RID_DIALOG_SPELL_LIST_SUGGEST

	BOOL					m_bChangingSelection;

    UT_sint32               m_iWordOffsetInSentence;
};

#endif /* AP_WIN32DIALOG_SPELL_H */
