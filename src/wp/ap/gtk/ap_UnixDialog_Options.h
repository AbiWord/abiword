/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef AP_UNIXDIALOG_OPTIONS_H
#define AP_UNIXDIALOG_OPTIONS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xap_Gtk2Compat.h"
#include "ap_Dialog_Options.h"

class XAP_UnixFrame;

/*****************************************************************/
class AP_UnixDialog_Options : public AP_Dialog_Options,
							  public XAP_NotebookDialog
{
public:
	AP_UnixDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Options(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	void event_ChooseTransparentColor(void);

	// tabbed dialog interface
	virtual void addPage (const XAP_NotebookDialog::Page *page);

 protected:

	GtkWidget *_lookupWidget( tControl id );
	virtual void _controlEnable( tControl id, bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void _set##a(t)


 	SET_GATHER			(NotebookPageNum,		int);

//	// Tabs
//		// Categories
//			// Subordinate Controls

	// General

		// User Interface

		SET_GATHER (ViewRulerUnits,	 UT_Dimension);

	// not implemented
	virtual bool _gatherViewCursorBlink(void) { return true; }
	virtual void _setViewCursorBlink(const bool) {}

		// Application Startup

		SET_GATHER (AutoLoadPlugins,	 bool);

	// Documents

		// AutoSave

		SET_GATHER (AutoSaveFile,	      bool);
			virtual void _gatherAutoSaveFilePeriod (      UT_String &stRetVal);
			virtual void _setAutoSaveFilePeriod    (const UT_String &stPeriod);
			virtual void _gatherAutoSaveFileExt    (      UT_String &stRetVal);
			virtual void _setAutoSaveFileExt       (const UT_String &stExt);

		// RTL Text Layout

		SET_GATHER (OtherDirectionRtl,	      bool);

	// Spell Checking

		// General

	 	SET_GATHER (SpellCheckAsType, bool);
 		SET_GATHER (SpellHideErrors,  bool);

		// Ignore Words

	 	SET_GATHER (SpellUppercase,   bool);
	 	SET_GATHER (SpellNumbers,     bool);

		// Dictionaries

 		SET_GATHER (SpellSuggest,     bool);
	 	SET_GATHER (SpellMainOnly,    bool);

		// Grammar Check

	 	SET_GATHER (GrammarCheck,    bool);

	// Smart Quotes

	 	SET_GATHER (SmartQuotes,   bool);
	 	SET_GATHER (CustomSmartQuotes,   bool);
		SET_GATHER (OuterQuoteStyle,	gint);
		SET_GATHER (InnerQuoteStyle,	gint);

	// unimplemented UI-wise. We need dummy implementations to satisfy the XP framework, though

	SET_GATHER			(PrefsAutoSave,			bool);
	SET_GATHER			(ViewShowRuler,			bool);
	SET_GATHER			(ViewShowStatusBar,		bool);
	SET_GATHER			(ViewAll,			bool);
	SET_GATHER			(ViewHiddenText,		bool);
	SET_GATHER			(ViewUnprintable,		bool);
	SET_GATHER			(EnableSmoothScrolling,		bool);
    SET_GATHER          (EnableOverwrite,       bool);
#undef SET_GATHER

 protected:

	// private construction functions
	void	    _setupUnitMenu(GtkWidget *optionmenu, const XAP_StringSet *pSS);
	void	    _constructWindowContents(GtkBuilder *builder);
	GtkWidget * _constructWindow(void);

	// pointers to widgets we need to query/set
	// there are a ton of them in this dialog

	GtkWidget * m_windowMain;
	GtkWidget * m_notebook;
	GtkWidget * m_buttonDefaults;
	GtkWidget * m_buttonClose;

//	// Tabs
//		// Categories
//			// Subordinate Controls

	// General

		// User Interface

		GtkWidget *m_pushbuttonNewTransparentColor;
		GtkWidget *m_menuUnits;

			// used inside the color selector

			GtkWidget *m_buttonColSel_Defaults;

		// Application Startup

		GtkWidget *m_checkbuttonAutoLoadPlugins;

	// Documents

		// General

		GtkWidget *m_checkbuttonAutoSaveFile;
			GtkWidget *m_tableAutoSaveFile;
				GtkWidget *m_textAutoSaveFilePeriod;
				GtkWidget *m_textAutoSaveFileExt;

		// RTL Text Layout

		GtkWidget * m_checkbuttonOtherDirectionRtl;

	// Spell Checking

		// General

		GtkWidget *m_checkbuttonSpellCheckAsType;
		GtkWidget *m_checkbuttonSpellHideErrors;

		// Ignore Words

		GtkWidget *m_checkbuttonSpellUppercase;
		GtkWidget *m_checkbuttonSpellNumbers;

		// Dictionaries

		GtkWidget *m_checkbuttonSpellSuggest;
		GtkWidget *m_checkbuttonSpellMainOnly;

		// Grammar Checking

		GtkWidget *m_checkbuttonGrammarCheck;

		// Smart Quotes

		GtkWidget *m_checkbuttonSmartQuotes;
		GtkWidget *m_checkbuttonCustomSmartQuotes;
		GtkWidget *m_omOuterQuoteStyle;
		GtkWidget *m_omInnerQuoteStyle;

    GtkWidget *m_checkbuttonEnableOverwrite;

    		// Dummy
		bool m_boolEnableSmoothScrolling;
		bool m_boolPrefsAutoSave;
		bool m_boolViewAll;
		bool m_boolViewHiddenText;
		bool m_boolViewShowRuler;
		bool m_boolViewShowStatusBar;
		bool m_boolViewUnprintable;
private:
	// Unix call back handlers
	static void s_control_changed	     (GtkWidget *,	   gpointer);
	static void s_apply_clicked	     (GtkWidget *,         gpointer);
	static void s_defaults_clicked	     (GtkWidget *,         gpointer);
	static void s_chooseTransparentColor (GtkWidget *,         gpointer);
    static void s_real_color_changed(GdkRGBA & gdkcolor, AP_UnixDialog_Options * dlg);
	static void s_color_changed	     (GtkColorChooser *, GdkRGBA*, gpointer);
	static void s_auto_save_toggled	     (GtkToggleButton *,   gpointer);
	static void s_checkbutton_toggle     (GtkWidget *,	   gpointer);
	static gint s_menu_item_activate     (GtkWidget *,	   gpointer);
	void	    _setupSmartQuotesCombos( GtkWidget *optionmenu );

	// callbacks can fire these events
	virtual void _storeWindowData(void);

	GSList	*m_extraPages;
};

#endif /* AP_UNIXDIALOG_OPTIONS_H */

