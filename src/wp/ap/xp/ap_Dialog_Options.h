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

#ifndef AP_DIALOG_OPTIONS_H
#define AP_DIALOG_OPTIONS_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_units.h"

#include "fp_PageSize.h"

class XAP_Frame;
class UT_String;

class AP_Dialog_Options : public XAP_Dialog_NonPersistent
{
 public:

	AP_Dialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Options(void);

	virtual void	runModal(XAP_Frame * pFrame) = 0;

	virtual void setInitialPageNum ( int which ) { m_pageNum = which; }
	virtual int getInitialPageNum () { return m_pageNum; }

	// answer from dialog
	typedef enum { a_OK, a_CANCEL, a_SAVE, a_APPLY } tAnswer;

	// control ids
	typedef enum { id_CHECK_SPELL_CHECK_AS_TYPE = 0, id_CHECK_SPELL_HIDE_ERRORS,
				   id_CHECK_SPELL_SUGGEST, id_CHECK_SPELL_MAIN_ONLY,
				   id_CHECK_SPELL_UPPERCASE, id_CHECK_SPELL_NUMBERS,
				   id_CHECK_SPELL_INTERNET, id_LIST_DICTIONARY,
				   id_BUTTON_DICTIONARY_EDIT, id_BUTTON_IGNORE_RESET,
				   id_BUTTON_IGNORE_EDIT, id_BUTTON_SPELL_AUTOREPLACE,

				   id_CHECK_SMART_QUOTES_ENABLE, id_LIST_DEFAULT_PAGE_SIZE,

				   id_CHECK_OTHER_DEFAULT_DIRECTION_RTL,
				   id_CHECK_OTHER_USE_CONTEXT_GLYPHS,
				   id_CHECK_OTHER_SAVE_CONTEXT_GLYPHS,
				   id_CHECK_OTHER_HEBREW_CONTEXT_GLYPHS,

				   id_CHECK_AUTO_SAVE_FILE,
				   id_TEXT_AUTO_SAVE_FILE_EXT,
				   id_TEXT_AUTO_SAVE_FILE_PERIOD,
				   id_CHECK_PREFS_AUTO_SAVE, id_COMBO_PREFS_SCHEME,

				   id_CHECK_VIEW_SHOW_RULER, id_LIST_VIEW_RULER_UNITS,
				   id_CHECK_VIEW_CURSOR_BLINK,
				   id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR,
				   id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR,
				   id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR,
				   id_CHECK_VIEW_SHOW_STATUS_BAR,
				   id_CHECK_VIEW_ALL, id_CHECK_VIEW_HIDDEN_TEXT,
				   id_CHECK_VIEW_UNPRINTABLE,
				   id_CHECK_COLOR_FOR_TRANSPARENT_IS_WHITE,
				   id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT,
				   id_BUTTON_SAVE, id_BUTTON_DEFAULTS,
				   id_BUTTON_OK, id_BUTTON_CANCEL, id_BUTTON_APPLY,
				   id_SHOWSPLASH,
				   id_UNIXFONTWARNING,
				   id_CHECK_ALLOW_CUSTOM_TOOLBARS,
				   id_CHECK_AUTO_LOAD_PLUGINS,
				   id_LIST_VIEW_TOOLBARS,		// this is needed for the Cocoa front-end to fetch the control
				   id_NOTEBOOK,
				   id_last } tControl;

	// typedef enum { check_FALSE = 0, check_TRUE, check_INDETERMINATE } tCheckState;

	AP_Dialog_Options::tAnswer	getAnswer(void) const;

 protected:

		// to enable/disable a control
	virtual void _controlEnable( tControl id, bool value )=0;

		// to be called when a control is toggled/changed
	void _enableDisableLogic( tControl id );

		// disable controls appropriately
	void _initEnableControls();

	void _populateWindowData(void);
	void _eventSave(void);

	virtual void _storeWindowData(void);	// calls the following functions to
									// lookup values to set as preferences

#define SET_GATHER(a,u) virtual u _gather##a(void) = 0; \
						virtual void	_set##a(const u) = 0
	SET_GATHER			(SpellCheckAsType,	bool);
	SET_GATHER			(SpellHideErrors,	bool);
	SET_GATHER			(SpellSuggest,		bool);
	SET_GATHER			(SpellMainOnly, 	bool);
	SET_GATHER			(SpellUppercase,	bool);
	SET_GATHER			(SpellNumbers,		bool);
	SET_GATHER			(SpellInternet, 	bool);
#if 0
	SET_GATHER			(SpellAutoReplace,	bool);
#endif

	SET_GATHER			(ShowSplash,bool);
	SET_GATHER			(SmartQuotesEnable, bool);
	SET_GATHER			(DefaultPageSize,	fp_PageSize::Predefined);

	SET_GATHER			(PrefsAutoSave, 	bool);

	SET_GATHER			(ViewShowRuler, 	bool);
	SET_GATHER			(ViewShowStandardBar,	bool);
	SET_GATHER			(ViewShowFormatBar, bool);
	SET_GATHER			(ViewShowExtraBar,	bool);
	SET_GATHER			(ViewShowStatusBar, bool);
	SET_GATHER			(ViewRulerUnits,	UT_Dimension);
	SET_GATHER			(ViewCursorBlink,	bool);

	SET_GATHER			(ViewAll,			bool);
	SET_GATHER			(ViewHiddenText,	bool);
	SET_GATHER			(ViewUnprintable,	bool);
	SET_GATHER			(AllowCustomToolbars, bool);
	SET_GATHER			(AutoLoadPlugins, bool);

	SET_GATHER			(OtherDirectionRtl, bool);
	SET_GATHER			(OtherUseContextGlyphs, bool);
	SET_GATHER			(OtherSaveContextGlyphs, bool);
	SET_GATHER			(OtherHebrewContextGlyphs, bool);

	SET_GATHER			(AutoSaveFile,		bool);
	virtual void _gatherAutoSaveFilePeriod(UT_String &stRetVal) = 0;
	virtual void _setAutoSaveFilePeriod(const UT_String &stPeriod) = 0;
	virtual void _gatherAutoSaveFileExt(UT_String &stRetVal) = 0;
	virtual void _setAutoSaveFileExt(const UT_String &stExt) = 0;
	
	// Jordi: For now this is just implemented in win32, we should make it
	// an abstract member if we decide to implemented in all platforms
	virtual void _gatherDocLanguage(UT_String &stRetVal){};
	virtual void _setDocLanguage(const UT_String &stExt) {};
	virtual void _gatherUILanguage(UT_String &stRetVal){};
	virtual void _setUILanguage(const UT_String &stExt) {};

	// so we can save and restore to the same page - must be able to return
	// the current page and reset it later (i.e., don't use a handle, but a
	// page index)
	SET_GATHER			(NotebookPageNum,	int );
#undef SET_GATHER

 protected:


//
// Screen Color stuff
	const XML_Char * _gatherColorForTransparent(void);
	void _setColorForTransparent(const XML_Char * pzsColorForTransparent);


	tAnswer 			m_answer;
	XAP_Frame * 		m_pFrame;
	XML_Char			m_CurrentTransparentColor[10];

	int m_pageNum;

	// AP level handlers
	void _event_SetDefaults(void);
	void _event_IgnoreReset(void);
	void _event_IgnoreEdit(void);
	void _event_DictionaryEdit(void);
};

#endif /* AP_DIALOG_PARAGRAPH_H */













