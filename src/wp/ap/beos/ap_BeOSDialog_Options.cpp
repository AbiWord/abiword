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

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_BeOSGraphics.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_BeOSDialog_Options.h"

/*****************************************************************/
/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Options::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id id)
{
    AP_BeOSDialog_Options * p = new AP_BeOSDialog_Options(pFactory,id);
    return p;
}

AP_BeOSDialog_Options::AP_BeOSDialog_Options(XAP_DialogFactory * pDlgFactory,
                                                 XAP_Dialog_Id id)
    : AP_Dialog_Options(pDlgFactory,id)
{
}

AP_BeOSDialog_Options::~AP_BeOSDialog_Options(void)
{
}

/*****************************************************************/

// this function will allow multiple widget to tie into the same logic
// function (at the AP level) to enable/disable stuff
#if 0	
	// TODO: rewrite for win32
void s_checkbutton_toggle( GtkWidget *w, AP_BeOSDialog_Options *dlg )
{ 
	UT_ASSERT(dlg); 
	UT_ASSERT( w && GTK_IS_WIDGET(w));
	int i = (int) gtk_object_get_data( GTK_OBJECT(w), "tControl" );
	dlg->_enableDisableLogic( (AP_Dialog_Options::tControl) i );
}
#endif
/*****************************************************************/

void AP_BeOSDialog_Options::runModal(XAP_Frame * pFrame)
{
}

// might need in windows
#if 0
GtkWidget *AP_BeOSDialog_Options::_lookupWidget ( tControl id )
{
	switch (id)
	{
	case id_CHECK_SPELL_CHECK_AS_TYPE:
		return m_checkbuttonSpellCheckAsType;
		break;

	case id_CHECK_SPELL_HIDE_ERRORS:
		return m_checkbuttonSpellHideErrors;
		break;

	case id_CHECK_SPELL_SUGGEST:
		return m_checkbuttonSpellSuggest;
		break;

	case id_CHECK_SPELL_MAIN_ONLY:
		return m_checkbuttonSpellMainOnly;
		break;

	case id_CHECK_SPELL_UPPERCASE:
		return m_checkbuttonSpellUppercase;
		break;

	case id_CHECK_SPELL_NUMBERS:
		return m_checkbuttonSpellNumbers;
		break;

	case id_CHECK_SPELL_INTERNET:
		return m_checkbuttonSpellInternet;
		break;

	case id_LIST_DICTIONARY:
		return m_listSpellDicts;
		break;

	case id_CHECK_PREFS_AUTO_SAVE:
		return m_checkbuttonPrefsAutoSave;
		break;

	case id_COMBO_PREFS_SCHEME:
		return m_comboPrefsScheme;
		break;

	case id_CHECK_VIEW_SHOW_RULER:
		return m_checkbuttonViewShowRuler;
		break;

	case id_LIST_VIEW_RULER_UNITS:
		return m_listViewRulerUnits;
		break;

	case id_CHECK_VIEW_SHOW_TOOLBARS:
		return m_checkbuttonViewShowToolbars;
		break;

	case id_CHECK_VIEW_ALL:
		return m_checkbuttonViewAll;
		break;

	case id_CHECK_VIEW_HIDDEN_TEXT:
		return m_checkbuttonViewHiddenText;
		break;

	case id_CHECK_VIEW_UNPRINTABLE:
		return m_checkbuttonViewUnprintable;
		break;

	case id_BUTTON_SAVE:
		return m_buttonSave;
		break;

	case id_BUTTON_DEFAULTS:
		return m_buttonDefaults;
		break;

	case id_BUTTON_OK:
		return m_buttonOK;
		break;

	case id_BUTTON_CANCEL:
		return m_buttonCancel;
		break;

	default:
		UT_ASSERT("Unknown Widget");
		return 0;
		break;
	}
}
#endif

void AP_BeOSDialog_Options::_controlEnable( tControl id, UT_Bool value )
{
	// TODO - change for win32
#if 0
	GtkWidget *w = _lookupWidget(id);
	UT_ASSERT( w && GTK_IS_WIDGET(w) );
	gtk_widget_set_sensitive( w, value );
#endif
}


// TODO - change for 

/*
#define DEFINE_GET_SET_BOOL(button) \
UT_Bool     AP_BeOSDialog_Options::_gather##button(void) {				\
	UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
	return gtk_toggle_button_get_active(								\
				GTK_TOGGLE_BUTTON(m_checkbutton##button) ); }			\
void        AP_BeOSDialog_Options::_set##button(UT_Bool b) {	\
	UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
	gtk_toggle_button_set_active (										\
				GTK_TOGGLE_BUTTON(m_checkbutton##button), b ); }
*/

#define DEFINE_GET_SET_BOOL(button) \
UT_Bool     AP_BeOSDialog_Options::_gather##button(void) {	return UT_FALSE; } \
void        AP_BeOSDialog_Options::_set##button(UT_Bool b) { }

DEFINE_GET_SET_BOOL(SpellCheckAsType);
DEFINE_GET_SET_BOOL(SpellHideErrors);
DEFINE_GET_SET_BOOL(SpellSuggest);
DEFINE_GET_SET_BOOL(SpellMainOnly);
DEFINE_GET_SET_BOOL(SpellUppercase);
DEFINE_GET_SET_BOOL(SpellNumbers);
DEFINE_GET_SET_BOOL(SpellInternet);

DEFINE_GET_SET_BOOL(PrefsAutoSave);

DEFINE_GET_SET_BOOL	(ViewShowRuler);
	// have a units option
DEFINE_GET_SET_BOOL	(ViewCursorBlink);
DEFINE_GET_SET_BOOL	(ViewShowToolbars);

DEFINE_GET_SET_BOOL	(ViewAll);
DEFINE_GET_SET_BOOL	(ViewHiddenText);
DEFINE_GET_SET_BOOL	(ViewUnprintable);

#undef DEFINE_GET_SET_BOOL

UT_Dimension AP_BeOSDialog_Options::_gatherViewRulerUnits(void) 
{
	return DIM_IN;
}

void    AP_BeOSDialog_Options::_setViewRulerUnits(UT_Dimension dim) 
{
}

int AP_BeOSDialog_Options::_gatherNotebookPageNum(void) 
{				
	return 0;
}			

void    AP_BeOSDialog_Options::_setNotebookPageNum(int pn) 
{	
}
