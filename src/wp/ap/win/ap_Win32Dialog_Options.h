/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_WIN32DIALOG_OPTIONS_H
#define AP_WIN32DIALOG_OPTIONS_H

#include "ut_vector.h"
#include "ap_Dialog_Options.h"
#include "xap_Win32PropertySheet.h"
#include "xap_Frame.h"
#include "xap_Win32DialogBase.h"


class UT_String;
class AP_Win32Dialog_Options;

enum PSH_PAGES {PG_GENERAL, PG_DOCUMENT, PG_SPELL, PG_SMARTQUOTES};

/*
	Sheet
*/
class ABI_EXPORT AP_Win32Dialog_Options_Sheet: public XAP_Win32PropertySheet
{

public:
		AP_Win32Dialog_Options_Sheet();
		void _onInitDialog(HWND hwnd);

		void setParent(AP_Win32Dialog_Options*	pData){m_pParent=pData;};
		AP_Win32Dialog_Options* getParent(){return m_pParent;};
		BOOL _onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK s_sheetInit(HWND hwnd,  UINT uMsg,  LPARAM lParam);

private:

		AP_Win32Dialog_Options*	m_pParent;

};


/*
	Spelling page
*/
class ABI_EXPORT AP_Win32Dialog_Options_Spelling: public XAP_Win32PropertyPage
{

public:
								AP_Win32Dialog_Options_Spelling();
								~AP_Win32Dialog_Options_Spelling();

	void						setContainer(AP_Win32Dialog_Options*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_Options*		getContainer(){return m_pParent;};
	void						transferData();
	static INT_PTR CALLBACK		s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);

private:

	void						_onInitDialog();
	void						_onKillActive(){};

	AP_Win32Dialog_Options*		m_pParent;

};


/*
	General page
*/
class ABI_EXPORT AP_Win32Dialog_Options_General: public XAP_Win32PropertyPage
{

public:
								AP_Win32Dialog_Options_General();
								~AP_Win32Dialog_Options_General();

	void						setContainer(AP_Win32Dialog_Options*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_Options*		getContainer(){return m_pParent;};
	void						transferData();
	static INT_PTR CALLBACK		s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);

private:

	void						_onInitDialog();
	void						_onKillActive(){};
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	AP_Win32Dialog_Options*		m_pParent;
	int							m_nCentered;
	UT_Vector*					m_pVecUILangs;

};

/*
	Document page
*/
class ABI_EXPORT AP_Win32Dialog_Options_Document: public XAP_Win32PropertyPage
{

public:
								AP_Win32Dialog_Options_Document();
								~AP_Win32Dialog_Options_Document();

	void						setContainer(AP_Win32Dialog_Options*	pParent){m_pParent=pParent;};
	AP_Win32Dialog_Options*		getContainer(){return m_pParent;};
	void						transferData();
	static INT_PTR CALLBACK		s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,   LPARAM lParam);
	bool						isAutoSaveInRange();

private:

	void						_onInitDialog();
	void						_onKillActive(){};
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	AP_Win32Dialog_Options*		m_pParent;

};

/*
	Smart Quotes page
*/
class ABI_EXPORT AP_Win32Dialog_Options_SmartQuotes: public XAP_Win32PropertyPage
{

public:
								AP_Win32Dialog_Options_SmartQuotes();
								~AP_Win32Dialog_Options_SmartQuotes();

	void						setContainer(AP_Win32Dialog_Options* pParent){m_pParent=pParent;};
	AP_Win32Dialog_Options*		getContainer(){return m_pParent;};

private:

	void						_onInitDialog();
	void						_onKillActive(){};
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	AP_Win32Dialog_Options*		m_pParent;

};


/*****************************************************************/
class ABI_EXPORT AP_Win32Dialog_Options: public AP_Dialog_Options, public XAP_Win32DialogBase
{
public:
	AP_Win32Dialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Options(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog * 	static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	HWND					getPage(PSH_PAGES page);
	XAP_DialogFactory * 	getDialogFactory() {return	m_pDialogFactory;};
	XAP_Frame *				getFrame() {return	m_pFrame;};
	void					checkLanguageChange();
	HFONT					getBoldFontHandle () {return m_hFont;}

 protected:

	AP_Win32Dialog_Options_General		m_general;
	AP_Win32Dialog_Options_Document		m_document;
 	AP_Win32Dialog_Options_Spelling		m_spelling;
 	AP_Win32Dialog_Options_SmartQuotes	m_smartquotes;
	UT_String							m_curLang;
	BOOL								m_langchanged;
	HFONT								m_hFont;

	virtual void _controlEnable( tControl id, bool value );
	virtual void _initEnableControlsPlatformSpecific();

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
						virtual void	_set##a(const t)

	SET_GATHER			(SpellCheckAsType,	bool );
	SET_GATHER			(SpellHideErrors,	bool );
	SET_GATHER			(SpellSuggest,		bool );
	SET_GATHER			(SpellMainOnly, 	bool );
	SET_GATHER			(SpellUppercase,	bool );
	SET_GATHER			(SpellNumbers,		bool );
	SET_GATHER			(GrammarCheck,		bool);
	SET_GATHER			(ViewRulerUnits,	UT_Dimension);
	SET_GATHER			(AutoLoadPlugins, bool);
	SET_GATHER			(OtherDirectionRtl, bool );
	SET_GATHER			(AutoSaveFile, bool);
 	SET_GATHER			(SmartQuotes,   		bool);
 	SET_GATHER			(CustomSmartQuotes,		bool);
	SET_GATHER			(EnableOverwrite,		bool);

	virtual bool _gatherViewShowToolbar(UT_uint32 /*t*/) { UT_ASSERT(UT_SHOULD_NOT_HAPPEN); return true;}
	virtual void _setViewShowToolbar(UT_uint32 /*row*/, bool /*b*/) {}


	// unimplemented UI-wise. We need dummy implementations to satisfy the XP framework, though

	SET_GATHER			(ViewCursorBlink,		bool);
	SET_GATHER			(PrefsAutoSave,			bool);
	SET_GATHER			(ViewShowRuler,			bool);
	SET_GATHER			(ViewShowStatusBar,		bool);
	SET_GATHER			(ViewAll,				bool);
	SET_GATHER			(ViewHiddenText,		bool);
	SET_GATHER			(ViewUnprintable,		bool);
	SET_GATHER			(EnableSmoothScrolling,	bool);

	virtual void _gatherAutoSaveFilePeriod(UT_String &stRetVal);
	virtual void _setAutoSaveFilePeriod(const UT_String &stPeriod);
	virtual void _gatherAutoSaveFileExt(UT_String &stRetVal);
	virtual void _setAutoSaveFileExt(const UT_String &stExt);
	virtual void _gatherUILanguage(UT_String &stRetVal);
	virtual void _setUILanguage(const UT_String &stExt);
	virtual gint _gatherOuterQuoteStyle();
	virtual gint _gatherInnerQuoteStyle();
	virtual void _setOuterQuoteStyle(const gint index);
	virtual void _setInnerQuoteStyle(const gint index);

	SET_GATHER			(NotebookPageNum,	int );
	SET_GATHER          (LanguageWithKeyboard, bool);

	// Dummy
	bool			m_boolEnableSmoothScrolling;
	bool			m_boolPrefsAutoSave;
	bool			m_boolViewAll;
	bool			m_boolViewHiddenText;
	bool			m_boolViewShowRuler;
	bool			m_boolViewShowStatusBar;
	bool			m_boolViewUnprintable;
	bool			m_boolViewCursorBlink;

#undef SET_GATHER

 protected:
	BOOL						_onNotify(HWND hWnd, LPARAM lParam);
	BOOL						_onCommandTab(HWND hWnd, WPARAM wParam, LPARAM lParam);

	HWND						m_hwndTab;		// tab control in parent dialog

	int 					        m_nrSubDlgs;		// number of tabs on tab control
	UT_Vector				m_vecSubDlgHWnd;	// hwnd to each sub-dialog

private:
	XAP_DialogFactory * 		m_pDialogFactory;
};

#endif /* AP_WIN32DIALOG_OPTIONS_H */
