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

#ifndef AP_UNIXDIALOG_OPTIONS_H
#define AP_UNIXDIALOG_OPTIONS_H

#include "xap_UnixFontManager.h"
#include "ap_Dialog_Options.h"

class XAP_UnixFrame;

/*****************************************************************/
class AP_UnixDialog_Options: public AP_Dialog_Options
{
public:
	AP_UnixDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Options(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	//void initializeTransperentToggle(void);
	void event_ChooseTransparentColor(void);
	void event_AllowTransparentColor(void);

 protected:

	GtkWidget *_lookupWidget( tControl id );
	virtual void _controlEnable( tControl id, bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void _set##a(t)

 	SET_GATHER			(SpellCheckAsType,	bool );
 	SET_GATHER			(SpellHideErrors,	bool );
 	SET_GATHER			(SpellSuggest,		bool );
 	SET_GATHER			(SpellMainOnly,		bool );
 	SET_GATHER			(SpellUppercase,	bool );
 	SET_GATHER			(SpellNumbers,		bool );
 	SET_GATHER			(SpellInternet,		bool );

 	SET_GATHER			(ShowSplash,	bool);

	SET_GATHER			(SmartQuotesEnable,	bool );

 	SET_GATHER			(PrefsAutoSave,		bool );

 	SET_GATHER			(ViewShowRuler,		bool );

	virtual bool _gatherViewShowToolbar(UT_uint32 t);
	virtual void _setViewShowToolbar(UT_uint32 row, bool b);

 	SET_GATHER			(ViewShowStatusBar,	bool );

	SET_GATHER			(ViewRulerUnits,	UT_Dimension);
	SET_GATHER			(ViewCursorBlink,	bool);


 	SET_GATHER			(ViewAll,			bool );
 	SET_GATHER			(ViewHiddenText,	bool );
 	SET_GATHER			(ViewUnprintable,	bool );
    SET_GATHER          (AllowCustomToolbars, bool);
    SET_GATHER          (EnableSmoothScrolling, bool);
    SET_GATHER          (AutoLoadPlugins,    bool);
 	SET_GATHER			(NotebookPageNum,	int);

	SET_GATHER			(OtherDirectionRtl, bool);
	SET_GATHER			(OtherSaveContextGlyphs, bool);
	SET_GATHER			(OtherHebrewContextGlyphs, bool);

	SET_GATHER			(AutoSaveFile, bool);
	virtual void _gatherAutoSaveFilePeriod(UT_String &stRetVal);
	virtual void _setAutoSaveFilePeriod(const UT_String &stPeriod);
	virtual void _gatherAutoSaveFileExt(UT_String &stRetVal);
	virtual void _setAutoSaveFileExt(const UT_String &stExt);

#undef SET_GATHER

 protected:

	// private construction functions
	GtkWidget * _constructWindow(void);
	GtkWidget *         _constructWindowContents(GtkWidget *);

	// pointers to widgets we need to query/set
	// there are a ton of them in this dialog

	GtkWidget * m_windowMain;
	GtkWidget * m_notebook;

    GtkWidget * m_checkbuttonSpellCheckAsType;
    GtkWidget * m_checkbuttonSpellHideErrors;
    GtkWidget * m_checkbuttonSpellSuggest;
    GtkWidget * m_checkbuttonSpellMainOnly;
    GtkWidget * m_checkbuttonSpellUppercase;
    GtkWidget * m_checkbuttonSpellNumbers;
    GtkWidget * m_checkbuttonSpellInternet;
	GtkWidget * m_listSpellDicts;
	GtkWidget * m_listSpellDicts_menu;
	GtkWidget * m_buttonSpellDictionary;
	GtkWidget * m_buttonSpellIgnoreEdit;
	GtkWidget * m_buttonSpellIgnoreReset;

    GtkWidget * m_checkbuttonSmartQuotesEnable;

    GtkWidget * m_checkbuttonPrefsAutoSave;
	GtkWidget * m_comboPrefsScheme;

    GtkWidget * m_checkbuttonViewShowRuler;
    GtkWidget * m_listViewRulerUnits;
    GtkWidget * m_listViewRulerUnits_menu;
    GtkWidget * m_checkbuttonViewCursorBlink;
    GtkWidget * m_checkbuttonViewShowStatusBar;

	GtkWidget * m_checkbuttonTransparentIsWhite;
	GtkWidget * m_pushbuttonNewTransparentColor;

	GtkWidget * m_checkbuttonAllowCustomToolbars;
	GtkWidget * m_checkbuttonEnableSmoothScrolling;
	GtkWidget * m_checkbuttonAutoLoadPlugins;

    GtkWidget * m_checkbuttonViewShowTB;
    GtkWidget * m_checkbuttonViewHideTB;
    GtkWidget * m_toolbarClist;

    GtkWidget * m_checkbuttonViewAll;
    GtkWidget * m_checkbuttonViewHiddenText;
    GtkWidget * m_checkbuttonViewUnprintable;

    GtkWidget * m_checkbuttonOtherDirectionRtl;
    GtkWidget * m_checkbuttonOtherSaveContextGlyphs;
    GtkWidget * m_checkbuttonOtherHebrewContextGlyphs;

	GtkWidget * m_checkbuttonAutoSaveFile;
	GtkWidget * m_textAutoSaveFilePeriod;
	GtkWidget * m_textAutoSaveFileExt;
	GtkWidget * m_checkbuttonShowSplash;
	GtkWidget * m_buttonDefaults;
	GtkWidget * m_buttonApply;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;

private:
	// Unix call back handlers
	static void s_toolbars_toggled		( GtkWidget *, gpointer );
	static void s_apply_clicked			( GtkWidget *, gpointer );
	static void s_defaults_clicked		( GtkWidget *, gpointer );
	static void s_chooseTransparentColor( GtkWidget *, gpointer );
	static void s_allowTransparentColor ( GtkWidget *, gpointer );
	static void s_color_changed(GtkWidget * csel,  AP_UnixDialog_Options * dlg);
	static void s_clist_clicked (GtkWidget *, gint, gint, GdkEvent *, gpointer);

	static void s_checkbutton_toggle	( GtkWidget *, gpointer );
	static gint s_menu_item_activate	( GtkWidget *, gpointer );

	// callbacks can fire these events
    void event_OK(void);
    void event_Cancel(void);
    void event_Apply(void);
    void event_clistClicked (int row, int col);
    void _saveUnixOnlyPrefs();
    void _initUnixOnlyPrefs();
    virtual void _storeWindowData(void);
};

#endif /* AP_UNIXDIALOG_OPTIONS_H */

