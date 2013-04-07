/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
 * Copyright (C) 2004 Francis James Franklin
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

#ifndef AP_DIALOG_OPTIONS_H
#define AP_DIALOG_OPTIONS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <vector>
#include <utility>

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_units.h"

#include "fp_PageSize.h"

class XAP_Frame;
class UT_String;

class ABI_EXPORT AP_Dialog_Options : public XAP_TabbedDialog_NonPersistent
{
 public:

	AP_Dialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Options(void);

	virtual void	runModal(XAP_Frame * pFrame) = 0;

	// answer from dialog
	typedef enum { a_OK, a_CANCEL, a_SAVE, a_APPLY } tAnswer;

	// control ids
	typedef enum { id_CHECK_SPELL_CHECK_AS_TYPE = 0, id_CHECK_SPELL_HIDE_ERRORS,
				   id_CHECK_SPELL_SUGGEST, id_CHECK_SPELL_MAIN_ONLY,
				   id_CHECK_SPELL_UPPERCASE, id_CHECK_SPELL_NUMBERS,
				   id_BUTTON_SPELL_AUTOREPLACE,
				   id_CHECK_GRAMMAR_CHECK,

				   id_CHECK_SMART_QUOTES_ENABLE,
				   id_CHECK_CUSTOM_SMART_QUOTES,
				   id_LIST_VIEW_OUTER_QUOTE_STYLE,
				   id_LIST_VIEW_INNER_QUOTE_STYLE,

				   id_CHECK_OTHER_DEFAULT_DIRECTION_RTL,

				   id_CHECK_AUTO_SAVE_FILE,
				   id_TEXT_AUTO_SAVE_FILE_EXT,
				   id_TEXT_AUTO_SAVE_FILE_PERIOD,
				   id_TEXT_AUTO_SAVE_FILE_PERIOD_SPIN,  // needed by Cocoa FE
				   id_CHECK_PREFS_AUTO_SAVE, id_COMBO_PREFS_SCHEME,

				   id_CHECK_VIEW_SHOW_RULER, id_LIST_VIEW_RULER_UNITS,
				   id_CHECK_VIEW_CURSOR_BLINK,
				   id_CHECK_VIEW_SHOW_STATUS_BAR,
				   id_CHECK_VIEW_ALL, id_CHECK_VIEW_HIDDEN_TEXT,
				   id_CHECK_VIEW_UNPRINTABLE,
				   id_CHECK_COLOR_FOR_TRANSPARENT_IS_WHITE,
				   id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT,
				   id_BUTTON_SAVE, id_BUTTON_DEFAULTS,
				   id_BUTTON_OK, id_BUTTON_CANCEL, id_BUTTON_APPLY,

				   id_CHECK_ENABLE_SMOOTH_SCROLLING,
                   id_CHECK_ENABLE_OVERWRITE,
				   id_CHECK_AUTO_LOAD_PLUGINS,
				   id_NOTEBOOK,
				   id_CHECK_LANG_WITH_KEYBOARD,
				   id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS,
				   id_last } tControl;

	// typedef enum { check_FALSE = 0, check_TRUE, check_INDETERMINATE } tCheckState;

	AP_Dialog_Options::tAnswer	getAnswer(void) const;

	virtual void _populateWindowData(void);
		// to be called when a control is toggled/changed
	void _enableDisableLogic( tControl id );

	virtual void _storeWindowData(void);	// calls the following functions to
						// lookup values to set as preferences
	virtual void _storeDataForControl (tControl id);	// sets preferences for a particular control
							 	// needed by instant apply and friends

	void _event_SetDefaults(void);

	//
	// Screen Color stuff
	//
	const gchar * _gatherColorForTransparent(void);
	void _setColorForTransparent(const gchar * pzsColorForTransparent);
	bool  isInitialPopulationHappenning(void)
		{ return m_bInitialPop; }

	typedef std::vector<std::pair<std::string, int> >	UnitMenuContent;
	void _getUnitMenuContent(const XAP_StringSet *pSS, UnitMenuContent & content);

 protected:

	// to enable/disable a control
	virtual void _controlEnable( tControl id, bool value )=0;



	// disable controls appropriately
	void _initEnableControls();

	// called by _initEnableControls() just before it returns
	// its purpose is to allow overriding of the enable logic on
	// platform basis
	virtual void _initEnableControlsPlatformSpecific(){};


	void _eventSave(void);


#define SET_GATHER(a,u) virtual u _gather##a(void) = 0; \
						virtual void	_set##a(const u) = 0
	SET_GATHER			(SpellCheckAsType,	bool);
	SET_GATHER			(SpellHideErrors,	bool);
	SET_GATHER			(SpellSuggest,		bool);
	SET_GATHER			(SpellMainOnly, 	bool);
	SET_GATHER			(SpellUppercase,	bool);
	SET_GATHER			(SpellNumbers,		bool);
	SET_GATHER			(GrammarCheck,		bool);
	SET_GATHER			(CustomSmartQuotes,		bool);
	SET_GATHER			(SmartQuotes,		bool);
	SET_GATHER			(PrefsAutoSave, 	bool);
    SET_GATHER          (EnableOverwrite,   bool);

#if !defined (TOOLKIT_GTK_ALL) && !defined(TOOLKIT_COCOA)
	SET_GATHER			(ViewShowRuler, 	bool);
	virtual bool _gatherViewShowToolbar(UT_uint32 t) = 0;
	virtual void _setViewShowToolbar(UT_uint32 row, bool b) = 0;
	SET_GATHER			(ViewShowStatusBar, bool);
#endif
	SET_GATHER			(ViewRulerUnits,	UT_Dimension);
	SET_GATHER			(OuterQuoteStyle,	gint);
	SET_GATHER			(InnerQuoteStyle,	gint);
	SET_GATHER			(ViewCursorBlink,	bool);

	SET_GATHER			(ViewAll,			bool);
	SET_GATHER			(ViewHiddenText,	bool);
	SET_GATHER			(ViewUnprintable,	bool);
#if defined(TOOLKIT_GTK_ALL)
	SET_GATHER			(EnableSmoothScrolling, bool);
#endif
	SET_GATHER			(AutoLoadPlugins, bool);

	SET_GATHER			(OtherDirectionRtl, bool);

	SET_GATHER			(AutoSaveFile,		bool);
	virtual void _gatherAutoSaveFilePeriod(UT_String &stRetVal) = 0;
	virtual void _setAutoSaveFilePeriod(const UT_String &stPeriod) = 0;
	virtual void _gatherAutoSaveFileExt(UT_String &stRetVal) = 0;
	virtual void _setAutoSaveFileExt(const UT_String &stExt) = 0;

	// Jordi: For now this is just implemented in win32, we should make it
	// an abstract member if we decide to implemented in all platforms
	virtual void _gatherUILanguage(UT_String &stRetVal){stRetVal.clear();};
	virtual void _setUILanguage(const UT_String & /*stExt*/) {};
	virtual bool _gatherLanguageWithKeyboard() {return false;}
	virtual void _setLanguageWithKeyboard(const bool) {}
	virtual bool _gatherDirMarkerAfterClosingParenthesis(){return false;}
	virtual void _setDirMarkerAfterClosingParenthesis(const bool){}

	// so we can save and restore to the same page - must be able to return
	// the current page and reset it later (i.e., don't use a handle, but a
	// page index)
	SET_GATHER			(NotebookPageNum,	int );

#undef SET_GATHER
 protected:


	tAnswer 			m_answer;
	XAP_Frame * 		m_pFrame;
	gchar			m_CurrentTransparentColor[10];

private:
	bool                m_bInitialPop;

};


#endif /* AP_DIALOG_PARAGRAPH_H */
