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

#ifndef AP_QNXDIALOG_OPTIONS_H
#define AP_QNXDIALOG_OPTIONS_H

#include "ap_Dialog_Options.h"
#include <Pt.h>

class XAP_QNXFrame;

/*****************************************************************/
class AP_QNXDialog_Options: public AP_Dialog_Options
{
public:
	AP_QNXDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_Options(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
    virtual void event_OK(void);
    virtual void event_Cancel(void);
    virtual void event_Apply(void);
    virtual void event_WindowDelete(void);

 protected:

	PtWidget_t *_lookupWidget( tControl id );
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
 
	SET_GATHER			(SmartQuotesEnable, bool );
 	SET_GATHER			(DefaultPageSize,	fp_PageSize::Predefined );
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
#undef SET_GATHER

 protected:
	static int s_ok_clicked			(PtWidget_t *, void *, PtCallbackInfo_t *info);
	static int s_cancel_clicked		(PtWidget_t *, void *, PtCallbackInfo_t *info);
	static int s_apply_clicked		(PtWidget_t *, void *, PtCallbackInfo_t *info);
	static int s_delete_clicked		(PtWidget_t *, void *, PtCallbackInfo_t *info);
	static int s_ignore_reset_clicked	(PtWidget_t *, void *, PtCallbackInfo_t *info);
	static int s_ignore_edit_clicked	(PtWidget_t *, void *, PtCallbackInfo_t *info);
	static int s_dict_edit_clicked		(PtWidget_t *, void *, PtCallbackInfo_t *info);
	static int s_defaults_clicked		(PtWidget_t *, void *, PtCallbackInfo_t *info);

	static int s_checkbutton_toggle	(PtWidget_t *, void *, PtCallbackInfo_t *info);
	static int s_menu_item_activate	(PtWidget_t *, void *, PtCallbackInfo_t *info);

		
	// private construction functions
	PtWidget_t * _constructWindow(void);

	// pointers to widgets we need to query/set
	// there are a ton of them in this dialog

	PtWidget_t * m_windowMain;
	PtWidget_t * m_notebook;

	PtWidget_t * m_checkbuttonSpellCheckAsType;
	PtWidget_t * m_checkbuttonSpellHideErrors;
	PtWidget_t * m_checkbuttonSpellSuggest;
	PtWidget_t * m_checkbuttonSpellMainOnly;
	PtWidget_t * m_checkbuttonSpellUppercase;
	PtWidget_t * m_checkbuttonSpellNumbers;
	PtWidget_t * m_checkbuttonSpellInternet;
	PtWidget_t * m_listSpellDicts;
	PtWidget_t * m_listSpellDicts_menu;
	PtWidget_t * m_buttonSpellDictionary;
	PtWidget_t * m_buttonSpellIgnoreEdit;
	PtWidget_t * m_buttonSpellIgnoreReset;

	PtWidget_t * m_checkbuttonPrefsAutoSave;
	PtWidget_t * m_comboPrefsScheme;

	PtWidget_t * m_checkbuttonViewShowRuler;
	PtWidget_t * m_listViewRulerUnits;
	PtWidget_t * m_listViewRulerUnits_menu;
	PtWidget_t * m_checkbuttonViewCursorBlink;
    PtWidget_t * m_checkbuttonViewShowStandardBar;
    PtWidget_t * m_checkbuttonViewShowFormatBar;
    PtWidget_t * m_checkbuttonViewShowExtraBar;
    PtWidget_t * m_checkbuttonViewShowStatusBar;
	PtWidget_t * m_checkbuttonViewAll;
	PtWidget_t * m_checkbuttonViewHiddenText;
	PtWidget_t * m_checkbuttonViewUnprintable;

	PtWidget_t * m_checkbuttonSmartQuotesEnable;

	PtWidget_t * m_buttonSave;
	PtWidget_t * m_buttonDefaults;
	PtWidget_t * m_buttonApply;
	PtWidget_t * m_buttonOK;
	PtWidget_t * m_buttonCancel;

	int			 done;

	// FIXME: replace this with *real* gui code */
	fp_PageSize::Predefined defaultPageSize;
};

#endif /* AP_QNXDIALOG_OPTIONS_H */
