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
#include "xap_Win32PropertySheet.h"
#include "xap_Frame.h"


class UT_String;
class AP_Win32Dialog_Options;
enum PSH_PAGES {PG_TOOLBARS, PG_SPELL, PG_LANG, PG_PREF, PG_LAYOUT};

/*
	Sheet
*/
class AP_Win32Dialog_Options_Sheet: public XAP_Win32PropertySheet
{
	
public:	
		AP_Win32Dialog_Options_Sheet();				
		void _onInitDialog(HWND hwnd);
		
		void setParent(AP_Win32Dialog_Options*	pData){m_pParent=pData;};
		AP_Win32Dialog_Options* getParent(){return m_pParent;};
		int _onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
		static int CALLBACK s_sheetInit(HWND hwnd,  UINT uMsg,  LPARAM lParam);
		
private:		
	
		AP_Win32Dialog_Options*	m_pParent;
};

/*
	Toolbar page
*/
class AP_Win32Dialog_Options_Toolbars: public XAP_Win32PropertyPage
{
	
public:		
								AP_Win32Dialog_Options_Toolbars();
								~AP_Win32Dialog_Options_Toolbars();	

	void						setContainer(AP_Win32Dialog_Options*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_Options*		getContainer(){return m_pParent;};
	void						transferData();
	static int CALLBACK			s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);	
	
private:

	void						_onInitDialog();
	void						_onKillActive(){};
	void						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
		
	AP_Win32Dialog_Options*		m_pParent;	
	
};

/*
	Spelling page
*/
class AP_Win32Dialog_Options_Spelling: public XAP_Win32PropertyPage
{
	
public:		
								AP_Win32Dialog_Options_Spelling();
								~AP_Win32Dialog_Options_Spelling();	

	void						setContainer(AP_Win32Dialog_Options*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_Options*		getContainer(){return m_pParent;};
	void						transferData();
	static int CALLBACK			s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);
	
private:

	void						_onInitDialog();
	void						_onKillActive(){};
	void						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
		
	AP_Win32Dialog_Options*		m_pParent;	
	
};

/*
	Lang
*/
class AP_Win32Dialog_Options_Lang: public XAP_Win32PropertyPage
{
	
public:		
								AP_Win32Dialog_Options_Lang();
								~AP_Win32Dialog_Options_Lang();	

	void						setContainer(AP_Win32Dialog_Options*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_Options*		getContainer(){return m_pParent;};
	void						transferData();
	static int CALLBACK			s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);
	
private:

	void						_onInitDialog();
	void						_onKillActive(){};
	void						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
		
	AP_Win32Dialog_Options*		m_pParent;	
	UT_Vector*					m_pVecUILangs;
	
};

/*
	Layout page
*/
class AP_Win32Dialog_Options_Layout: public XAP_Win32PropertyPage
{
	
public:		
								AP_Win32Dialog_Options_Layout();
								~AP_Win32Dialog_Options_Layout();	

	void						setContainer(AP_Win32Dialog_Options*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_Options*		getContainer(){return m_pParent;};
	void						transferData();
	static int CALLBACK			s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);
	
private:

	void						_onInitDialog();
	void						_onKillActive(){};
	void						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
		
	AP_Win32Dialog_Options*		m_pParent;	
	
};

/*
	Preferences page
*/
class AP_Win32Dialog_Options_Pref: public XAP_Win32PropertyPage
{
	
public:	
								AP_Win32Dialog_Options_Pref();
								~AP_Win32Dialog_Options_Pref();	

	void						setContainer(AP_Win32Dialog_Options*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_Options*		getContainer(){return m_pParent;};
	void						transferData();
	static int CALLBACK			s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);
	bool						isAutoSaveInRange();
	
private:

	void						_onInitDialog();
	void						_onKillActive(){};
	void						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
		
	AP_Win32Dialog_Options*		m_pParent;	
	
};




/*****************************************************************/
class AP_Win32Dialog_Options: public AP_Dialog_Options
{
public:
	AP_Win32Dialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Options(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog * 	static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
	HWND					getPage(PSH_PAGES page);
	void 					_initializeTransperentToggle(void);
	XAP_DialogFactory * 	getDialogFactory() {return	m_pDialogFactory;};
	XAP_Frame *				getFrame() {return	m_pFrame;};
	
	
 protected:
 
 	AP_Win32Dialog_Options_Toolbars		m_toolbars;
 	AP_Win32Dialog_Options_Spelling		m_spelling;
 	AP_Win32Dialog_Options_Layout		m_layout; 	
 	AP_Win32Dialog_Options_Lang			m_lang;
 	AP_Win32Dialog_Options_Pref			m_pref;
 	

	virtual void _controlEnable( tControl id, bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
						virtual void	_set##a(const t)

	SET_GATHER			(SpellCheckAsType,	bool );
	SET_GATHER			(SpellHideErrors,	bool );
	SET_GATHER			(SpellSuggest,		bool );
	SET_GATHER			(SpellMainOnly, 	bool );
	SET_GATHER			(SpellUppercase,	bool );
	SET_GATHER			(SpellNumbers,		bool );
	SET_GATHER			(SpellInternet, 	bool );

	SET_GATHER			(ShowSplash,		bool );

	SET_GATHER			(SmartQuotesEnable, bool );
	SET_GATHER			(DefaultPageSize,	fp_PageSize::Predefined );

	SET_GATHER			(PrefsAutoSave, 	bool );

	SET_GATHER			(ViewShowRuler, 	bool );
	SET_GATHER			(ViewShowStandardBar,bool );
	SET_GATHER			(ViewShowFormatBar, bool );
	SET_GATHER			(ViewShowExtraBar,	bool );
	SET_GATHER			(ViewShowStatusBar, bool );
	SET_GATHER			(ViewRulerUnits,	UT_Dimension);
	SET_GATHER			(ViewCursorBlink,	bool);

	SET_GATHER			(ViewAll,			bool );
	SET_GATHER			(ViewHiddenText,	bool );
	SET_GATHER			(ViewUnprintable,	bool );
	SET_GATHER			(AllowCustomToolbars, bool);
	SET_GATHER			(AutoLoadPlugins, bool);

	SET_GATHER			(OtherDirectionRtl, bool );
	SET_GATHER			(OtherUseContextGlyphs, bool );
	SET_GATHER			(OtherSaveContextGlyphs,bool );
	SET_GATHER			(OtherHebrewContextGlyphs,bool );

	SET_GATHER			(AutoSaveFile, bool);
	virtual void _gatherAutoSaveFilePeriod(UT_String &stRetVal);
	virtual void _setAutoSaveFilePeriod(const UT_String &stPeriod);
	virtual void _gatherAutoSaveFileExt(UT_String &stRetVal);
	virtual void _setAutoSaveFileExt(const UT_String &stExt);
	
	
	virtual void _gatherDocLanguage(UT_String &stRetVal);
	virtual void _setDocLanguage(const UT_String &stExt);
	virtual void _gatherUILanguage(UT_String &stRetVal);
	virtual void _setUILanguage(const UT_String &stExt);

	SET_GATHER			(NotebookPageNum,	int );
	SET_GATHER          (LanguageWithKeyboard, bool);
#undef SET_GATHER

 protected:
	BOOL						_onNotify(HWND hWnd, LPARAM lParam);
	BOOL						_onCommandTab(HWND hWnd, WPARAM wParam, LPARAM lParam);

	HWND						m_hwndDlg;		// parent dialog
	HWND						m_hwndTab;		// tab control in parent dialog

	int 						m_nrSubDlgs;		// number of tabs on tab control
	UT_Vector					m_vecSubDlgHWnd;	// hwnd to each sub-dialog
	UT_Vector*					m_pVecUILangs;

private:
	XAP_DialogFactory * 		m_pDialogFactory;
};

#endif /* AP_WIN32DIALOG_OPTIONS_H */
