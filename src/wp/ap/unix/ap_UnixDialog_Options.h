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
	virtual void _controlEnable( tControl id, UT_Bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void    _set##a( t )
 
 	SET_GATHER			(SpellCheckAsType,	UT_Bool );
 	SET_GATHER			(SpellHideErrors,	UT_Bool );
 	SET_GATHER			(SpellSuggest,		UT_Bool );
 	SET_GATHER			(SpellMainOnly,		UT_Bool );
 	SET_GATHER			(SpellUppercase,	UT_Bool );
 	SET_GATHER			(SpellNumbers,		UT_Bool );
 	SET_GATHER			(SpellInternet,		UT_Bool );
 
 	SET_GATHER			(PrefsAutoSave,		UT_Bool );
 
 	SET_GATHER			(ViewShowRuler,		UT_Bool );
	SET_GATHER			(ViewRulerUnits,	UT_Dimension);		
	SET_GATHER			(ViewCursorBlink,	UT_Bool);
 	SET_GATHER			(ViewShowToolbars,	UT_Bool );
 
 	SET_GATHER			(ViewAll,			UT_Bool );
 	SET_GATHER			(ViewHiddenText,	UT_Bool );
 	SET_GATHER			(ViewUnprintable,	UT_Bool );
  
 	SET_GATHER			(NotebookPageNum,	int );
#undef SET_GATHER
	
 protected:
	
	// private construction functions
	GtkWidget * _constructWindow(void);

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

    GtkWidget * m_checkbuttonPrefsAutoSave;
	GtkWidget * m_comboPrefsScheme;

    GtkWidget * m_checkbuttonViewShowRuler;
    GtkWidget * m_listViewRulerUnits;
    GtkWidget * m_listViewRulerUnits_menu;
    GtkWidget * m_checkbuttonViewCursorBlink;
    GtkWidget * m_checkbuttonViewShowToolbars;
    GtkWidget * m_checkbuttonViewAll;
    GtkWidget * m_checkbuttonViewHiddenText;
    GtkWidget * m_checkbuttonViewUnprintable;

	GtkWidget * m_buttonSave;
	GtkWidget * m_buttonDefaults;
	GtkWidget * m_buttonApply;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;

protected:
	// Unix call back handlers
	static void s_ok_clicked			( GtkWidget *, gpointer );
	static void s_cancel_clicked		( GtkWidget *, gpointer );
	static void s_apply_clicked			( GtkWidget *, gpointer );
	static void s_delete_clicked		( GtkWidget *, gpointer );
	static void s_ignore_reset_clicked	( GtkWidget *, gpointer );
	static void s_ignore_edit_clicked	( GtkWidget *, gpointer );
	static void s_dict_edit_clicked		( GtkWidget *, gpointer );
	static void s_defaults_clicked		( GtkWidget *, gpointer );

	static void s_checkbutton_toggle	( GtkWidget *, gpointer );
	static gint s_menu_item_activate	( GtkWidget *, gpointer );

	// callbacks can fire these events
    virtual void event_OK(void);
    virtual void event_Cancel(void);
    virtual void event_Apply(void);
    virtual void event_WindowDelete(void);

};

#endif /* AP_UNIXDIALOG_OPTIONS_H */
