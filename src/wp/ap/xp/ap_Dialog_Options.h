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

class XAP_Frame;

class AP_Dialog_Options : public XAP_Dialog_NonPersistent
{
 public:

	AP_Dialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Options(void);

	virtual void	runModal(XAP_Frame * pFrame) = 0;

	// answer from dialog
	typedef enum { a_OK, a_CANCEL } tAnswer;
	typedef enum { unit_IN, unit_MM, unit_CM, unit_TWIPS, 
				   unit_POINTS, unit_PIXELS } tUnits;

	// control ids
	typedef enum { id_CHECK_SPELL_CHECK_AS_TYPE = 0, id_CHECK_SPELL_HIDE_ERRORS,
				   id_CHECK_SPELL_SUGGEST, id_CHECK_SPELL_MAIN_ONLY, 
				   id_CHECK_SPELL_UPPERCASE, id_CHECK_SPELL_NUMBERS, 
				   id_CHECK_SPELL_INTERNET, id_LIST_DICTIONARY,

				   id_CHECK_PREFS_AUTO_SAVE, id_COMBO_PREFS_SCHEME,

				   id_CHECK_VIEW_SHOW_RULER, id_LIST_VIEW_RULER_UNITS,
				   id_CHECK_VIEW_SHOW_TOOLBARS, id_CHECK_VIEW_ALL, 
				   id_CHECK_VIEW_HIDDEN_TEXT, id_CHECK_VIEW_UNPRINTABLE,

				   id_BUTTON_SAVE, id_BUTTON_DEFAULTS, 
				   id_BUTTON_OK, id_BUTTON_CANCEL,
			
				   id_last } tControl;


	AP_Dialog_Options::tAnswer	getAnswer(void) const;


 protected:

		// to enable/disable a control
	virtual void _controlEnable( tControl id, UT_Bool value )=0;

		// to be called when a control is toggled/changed
	void _enableDisableLogic( tControl id );

		// disable controls appropriately
	void _initEnableControls();

	void _populateWindowData(void);

	void _storeWindowData(void);	// calls the following functions to
									// lookup values to set as preferences
								// don't see any need to make virtual yet, all
								// optdlgs should as for the same preferences

#define SET_GATHER_BOOLV(a) virtual UT_Bool _gather##a(void) = 0; \
					 	    virtual void    _set##a( UT_Bool ) = 0
	SET_GATHER_BOOLV			(SpellCheckAsType);
	SET_GATHER_BOOLV			(SpellHideErrors);
	SET_GATHER_BOOLV			(SpellSuggest);
	SET_GATHER_BOOLV			(SpellMainOnly);
	SET_GATHER_BOOLV			(SpellUppercase);
	SET_GATHER_BOOLV			(SpellNumbers);
	SET_GATHER_BOOLV			(SpellInternet);

	SET_GATHER_BOOLV			(PrefsAutoSave);

	SET_GATHER_BOOLV			(ViewShowRuler);
		// have a units option
	SET_GATHER_BOOLV			(ViewShowToolbars);

	SET_GATHER_BOOLV			(ViewAll);
	SET_GATHER_BOOLV			(ViewHiddenText);
	SET_GATHER_BOOLV			(ViewUnprintable);
#undef SAVE_GATHER_BOOLV
	
 protected:
	tAnswer				m_answer;

	virtual void _event_SetDefaults(void);
};

#endif /* AP_DIALOG_PARAGRAPH_H */
