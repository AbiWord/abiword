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

 protected:

	GtkWidget *_lookupWidget( tControl id );
	virtual void _controlEnable( tControl id, bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void    _set##a( t )

 	SET_GATHER			(SpellCheckAsType,	bool );
 	SET_GATHER			(SpellHideErrors,	bool );
 	SET_GATHER			(SpellSuggest,		bool );
 	SET_GATHER			(SpellMainOnly,		bool );
 	SET_GATHER			(SpellUppercase,	bool );
 	SET_GATHER			(SpellNumbers,		bool );
 	SET_GATHER			(SpellInternet,		bool );
 
 	SET_GATHER			(SmartQuotesEnable,	bool );
	SET_GATHER			(DefaultPageSize,  fp_PageSize::Predefined);

 	SET_GATHER			(PrefsAutoSave,		bool );
 
 	SET_GATHER			(ViewShowRuler,		bool );

 	SET_GATHER			(ViewShowStandardBar,	bool );
 	SET_GATHER			(ViewShowFormatBar,	bool );
 	SET_GATHER			(ViewShowExtraBar,	bool );

 	SET_GATHER			(ViewShowStatusBar,	bool );

	SET_GATHER			(ViewRulerUnits,	UT_Dimension);		
	SET_GATHER			(ViewCursorBlink,	bool);
 
 	SET_GATHER			(ViewAll,			bool );
 	SET_GATHER			(ViewHiddenText,	bool );
 	SET_GATHER			(ViewUnprintable,	bool );
  
 	SET_GATHER			(NotebookPageNum,	int );

#ifdef BIDI_ENABLED
	SET_GATHER			(OtherUseUnicodeDirection, bool);
	SET_GATHER			(OtherDirectionRtl, bool);
#endif
#undef SET_GATHER
	
 protected:
	
	// private construction functions
	virtual GtkWidget * _constructWindow(void);
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
    GtkWidget * m_listDefaultPageSize;

    GtkWidget * m_checkbuttonPrefsAutoSave;
	GtkWidget * m_comboPrefsScheme;

    GtkWidget * m_checkbuttonViewShowRuler;
    GtkWidget * m_listViewRulerUnits;
    GtkWidget * m_listViewRulerUnits_menu;
    GtkWidget * m_checkbuttonViewCursorBlink;
    GtkWidget * m_checkbuttonViewShowStatusBar;

    GtkWidget * m_checkbuttonViewShowTB;
    GtkWidget * m_checkbuttonViewHideTB;
    GtkWidget * m_toolbarClist;

    GtkWidget * m_checkbuttonViewAll;
    GtkWidget * m_checkbuttonViewHiddenText;
    GtkWidget * m_checkbuttonViewUnprintable;

#ifdef BIDI_ENABLED
    GtkWidget * m_checkbuttonOtherUseUnicodeDirection;
    GtkWidget * m_checkbuttonOtherDirectionRtl;
#endif
	
	GtkWidget * m_buttonDefaults;
	GtkWidget * m_buttonApply;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;

protected:
	// Unix call back handlers
	static void s_ok_clicked			( GtkWidget *, gpointer );
	static void s_cancel_clicked		( GtkWidget *, gpointer );
	static void s_apply_clicked			( GtkWidget *, gpointer );
	static void s_delete_clicked		( GtkWidget *, GdkEvent *, gpointer );
	static void s_ignore_reset_clicked	( GtkWidget *, gpointer );
	static void s_ignore_edit_clicked	( GtkWidget *, gpointer );
	static void s_dict_edit_clicked		( GtkWidget *, gpointer );
	static void s_defaults_clicked		( GtkWidget *, gpointer );

	static void s_clist_clicked (GtkWidget *, gint, gint, GdkEvent *, gpointer);

	static void s_checkbutton_toggle	( GtkWidget *, gpointer );
	static gint s_menu_item_activate	( GtkWidget *, gpointer );

	// callbacks can fire these events
    virtual void event_OK(void);
    virtual void event_Cancel(void);
    virtual void event_Apply(void);
    virtual void event_WindowDelete(void);
    virtual void event_clistClicked (int row, int col);
};

#endif /* AP_UNIXDIALOG_OPTIONS_H */
