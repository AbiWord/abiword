/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef AP_WIN32DIALOG_OPTIONS_H
#define AP_WIN32DIALOG_OPTIONS_H

#include "ut_vector.h"
#include "ap_Dialog_Options.h"

class XAP_Win32Frame;
class UT_String;

/*****************************************************************/
class AP_Win32Dialog_Options: public AP_Dialog_Options
{
public:
	AP_Win32Dialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Options(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);

 protected:

	virtual void _controlEnable( tControl id, bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void    _set##a(const t)
 
 	SET_GATHER			(SpellCheckAsType,	bool );
 	SET_GATHER			(SpellHideErrors,	bool );
 	SET_GATHER			(SpellSuggest,		bool );
 	SET_GATHER			(SpellMainOnly,		bool );
 	SET_GATHER			(SpellUppercase,	bool );
 	SET_GATHER			(SpellNumbers,		bool );
 	SET_GATHER			(SpellInternet,		bool );
 
	SET_GATHER			(SmartQuotesEnable,	bool ); 
	SET_GATHER			(DefaultPageSize,  	fp_PageSize::Predefined ); 

 	SET_GATHER			(PrefsAutoSave,		bool );
 
 	SET_GATHER			(ViewShowRuler,		bool );
	SET_GATHER			(ViewShowStandardBar,bool );      
	SET_GATHER			(ViewShowFormatBar,	bool );      
	SET_GATHER			(ViewShowExtraBar,	bool );
	SET_GATHER			(ViewShowStatusBar,	bool );
	SET_GATHER			(ViewRulerUnits,	UT_Dimension);		
	SET_GATHER			(ViewCursorBlink,	bool);
 
 	SET_GATHER			(ViewAll,			bool );
 	SET_GATHER			(ViewHiddenText,	bool );
 	SET_GATHER			(ViewUnprintable,	bool );

#ifdef BIDI_ENABLED
	SET_GATHER			(OtherDirectionRtl,	bool );
#endif  

	SET_GATHER			(AutoSaveFile, bool);
	virtual void _gatherAutoSaveFilePeriod(UT_String &stRetVal);
	virtual void _setAutoSaveFilePeriod(const UT_String &stPeriod);
	virtual void _gatherAutoSaveFileExt(UT_String &stRetVal);
	virtual void _setAutoSaveFileExt(const UT_String &stExt);

 	SET_GATHER			(NotebookPageNum,	int );
#undef SET_GATHER
	
 protected:
	BOOL						_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);   
	static BOOL CALLBACK		s_tabProc(HWND,UINT,WPARAM,LPARAM);
	BOOL						_onInitTab(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL						_onNotify(HWND hWnd, LPARAM lParam);
	BOOL						_onCommandTab(HWND hWnd, WPARAM wParam, LPARAM lParam);

	HWND						m_hwndDlg;		// parent dialog
	HWND						m_hwndTab;		// tab control in parent dialog

	int							m_nrSubDlgs;		// number of tabs on tab control
	UT_Vector					m_vecSubDlgHWnd;	// hwnd to each sub-dialog
	
	// FIXME: replace this with *real* gui code */
	fp_PageSize::Predefined		defaultPageSize;
	
};

#endif /* AP_WIN32DIALOG_OPTIONS_H */
