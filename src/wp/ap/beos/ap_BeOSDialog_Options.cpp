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

#include "ut_Rehydrate.h"

/*****************************************************************/



class OptionsWin:public BWindow {
	public:
		OptionsWin(BMessage *data);
		void SetDlg(AP_BeOSDialog_Options *dlg);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
		AP_BeOSDialog_Options 	*m_DlgOptions;
				
		sem_id modalSem;
		status_t WaitForDelete(sem_id blocker);
};

status_t OptionsWin::WaitForDelete(sem_id blocker)
{
	status_t	result;
	thread_id	this_tid = find_thread(NULL);
	BLooper		*pLoop;
	BWindow		*pWin = 0;

	pLoop = BLooper::LooperForThread(this_tid);
	if (pLoop)
		pWin = dynamic_cast<BWindow*>(pLoop);

	// block until semaphore is deleted (modal is finished)
	if (pWin) {
		do {
			pWin->Unlock();
			snooze(7);
			pWin->Lock();
			
			// update the window periodically
			pWin->UpdateIfNeeded();
			result = acquire_sem_etc(blocker, 1, B_TIMEOUT, 1000);
		} while (result != B_BAD_SEM_ID);
	} else {
		do {
			// just wait for exit
			result = acquire_sem(blocker);
		} while (result != B_BAD_SEM_ID);
	}
	return result;
}

OptionsWin::OptionsWin(BMessage *data) 
	  :BWindow(data)
{

} 


struct {
	UT_Dimension  dim;
	int 		  id;
} s_aAlignUnit[] = 
{
	{ DIM_IN, XAP_STRING_ID_DLG_Unit_inch },
	{ DIM_CM, XAP_STRING_ID_DLG_Unit_cm },
	{ DIM_PT, XAP_STRING_ID_DLG_Unit_points },
	{ DIM_PI, XAP_STRING_ID_DLG_Unit_pico },
};
#define SIZE_aAlignUnit  (sizeof(s_aAlignUnit)/sizeof(s_aAlignUnit[0]))

void OptionsWin::SetDlg(AP_BeOSDialog_Options *dlg)
{
	m_DlgOptions = dlg;
	const XAP_StringSet * pSS = dlg->m_pApp->getStringSet();
	
	// This semaphore ties up the window until after it deletes..
	modalSem = create_sem(0,"OptionsModalSem");

	Show();
	Lock();
	
	BMenuField* pField;
	
	BTabView* pView = (BTabView *)FindView("TabView");
	if(pView)
		pField = (BMenuField *)pView->TabAt(2)->View()->FindView("Units");
	
	
	for( int n1 = 0; n1 < SIZE_aAlignUnit; n1++ ) 
		pField->Menu()->AddItem(new BMenuItem((char *)pSS->getValue(s_aAlignUnit[n1].id), NULL) );//SendMessage(hwndAlign, CB_ADDSTRING, 0, (LPARAM)pSS->getValue(s_aAlignUnit[n1].id));


	m_DlgOptions->_populateWindowData();
	Unlock();

// Dialog is by default canceled, until they hit OK.	
	m_DlgOptions->m_answer = AP_Dialog_Options::a_CANCEL;

	WaitForDelete(modalSem);
}

#define FIND_CONTROL(handle , chartitle) \
	handle = (BCheckBox *)newwin->FindView(chartitle); \
	if(!handle)\
	{\
		handle = (BCheckBox *)pView->TabAt(0)->View()->FindView(chartitle); \
		if(!handle) \
		{\
			handle = (BCheckBox *)pView->TabAt(1)->View()->FindView(chartitle);\
			if(!handle)\
			{\
			handle = (BCheckBox *)pView->TabAt(2)->View()->FindView(chartitle);\
			}\
		}\
	}
	
#define TOGGLE_BASED_ON_MESSAGE(charcode , controlID) \
case charcode :\
	m_DlgOptions->_enableDisableLogic(controlID);\
	break;
	
void OptionsWin::DispatchMessage(BMessage *msg, BHandler *handler) 
{	
	BCheckBox* pSource;
	BCheckBox* pOther;
	
	switch(msg->what) 
	{
	case 'dbut':
		// Set defaults:
		m_DlgOptions->_event_SetDefaults();
		break;
	
	case 'obut': 
		// OK pressed.
		printf("Setting answer\n");
	
		m_DlgOptions->m_answer = AP_Dialog_Options::a_OK;
		printf("Storing data\n");
		m_DlgOptions->_storeWindowData();						// remember current settings
		printf("Quitting\n");
		PostMessage(B_QUIT_REQUESTED);

		break;
		
	case 'save':
		// Save pressed.
		m_DlgOptions->m_answer = AP_Dialog_Options::a_SAVE;
		m_DlgOptions->_storeWindowData();
//		UT_DEBUGMSG(("OptionsWin::Save todo\n"));
		break;
	
	case 'appl':
		m_DlgOptions->m_answer = AP_Dialog_Options::a_APPLY;
		m_DlgOptions->_storeWindowData();
		break;
		
	TOGGLE_BASED_ON_MESSAGE('cbsp' , m_DlgOptions->id_CHECK_SPELL_CHECK_AS_TYPE)
	TOGGLE_BASED_ON_MESSAGE('chhe' , m_DlgOptions->id_CHECK_SPELL_HIDE_ERRORS)
	TOGGLE_BASED_ON_MESSAGE('chsc' , m_DlgOptions->id_CHECK_SPELL_SUGGEST)
	TOGGLE_BASED_ON_MESSAGE('chmd' , m_DlgOptions->id_CHECK_SPELL_MAIN_ONLY)
	TOGGLE_BASED_ON_MESSAGE('cbup' , m_DlgOptions->id_CHECK_SPELL_UPPERCASE)
	TOGGLE_BASED_ON_MESSAGE('chnu' , m_DlgOptions->id_CHECK_SPELL_NUMBERS)
	TOGGLE_BASED_ON_MESSAGE('chia' , m_DlgOptions->id_CHECK_SPELL_INTERNET)
	TOGGLE_BASED_ON_MESSAGE('chas' , m_DlgOptions->id_CHECK_PREFS_AUTO_SAVE)
	TOGGLE_BASED_ON_MESSAGE('cbru' , m_DlgOptions->id_CHECK_VIEW_SHOW_RULER)
	  //TOGGLE_BASED_ON_MESSAGE('chtb' , m_DlgOptions->id_CHECK_VIEW_SHOW_TOOLBARS)
	TOGGLE_BASED_ON_MESSAGE('cbcb' , m_DlgOptions->id_CHECK_VIEW_CURSOR_BLINK)
	TOGGLE_BASED_ON_MESSAGE('cbsq' , m_DlgOptions->id_CHECK_SMART_QUOTES_ENABLE)
	TOGGLE_BASED_ON_MESSAGE('cbal' , m_DlgOptions->id_CHECK_VIEW_ALL)
	TOGGLE_BASED_ON_MESSAGE('cbht' , m_DlgOptions->id_CHECK_VIEW_HIDDEN_TEXT)
	TOGGLE_BASED_ON_MESSAGE('cbuc' , m_DlgOptions->id_CHECK_VIEW_UNPRINTABLE)
	
        TOGGLE_BASED_ON_MESSAGE('cbet' , m_DlgOptions->id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR) 
        TOGGLE_BASED_ON_MESSAGE('cbft' , m_DlgOptions->id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR) 
        TOGGLE_BASED_ON_MESSAGE('cbst' , m_DlgOptions->id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR) 
        TOGGLE_BASED_ON_MESSAGE('stat' , m_DlgOptions->id_CHECK_VIEW_SHOW_STATUS_BAR) 
#if 0
	case 'ctog':
		// Any checkbox toggled.
		if(msg->FindPointer("source" , (void **)&pSource) == B_OK)
		{
	
	case AP_RID_DIALOG_OPTIONS_CHK_SpellCheckAsType:	_enableDisableLogic(id_CHECK_SPELL_CHECK_AS_TYPE);
	case AP_RID_DIALOG_OPTIONS_CHK_SpellHideErrors:		_enableDisableLogic(id_CHECK_SPELL_HIDE_ERRORS);
	case AP_RID_DIALOG_OPTIONS_CHK_SpellSuggest:		_enableDisableLogic(id_CHECK_SPELL_SUGGEST);
	case AP_RID_DIALOG_OPTIONS_CHK_SpellMainOnly:		_enableDisableLogic(id_CHECK_SPELL_MAIN_ONLY);
	case AP_RID_DIALOG_OPTIONS_CHK_SpellUppercase:		_enableDisableLogic(id_CHECK_SPELL_UPPERCASE);
	case AP_RID_DIALOG_OPTIONS_CHK_SpellNumbers:		_enableDisableLogic(id_CHECK_SPELL_NUMBERS);
	case AP_RID_DIALOG_OPTIONS_CHK_SpellInternet:		_enableDisableLogic(id_CHECK_SPELL_INTERNET);
	case AP_RID_DIALOG_OPTIONS_CHK_PrefsAutoSave:		_enableDisableLogic(id_CHECK_PREFS_AUTO_SAVE);
	case AP_RID_DIALOG_OPTIONS_CHK_SmartQuotesEnable:	_enableDisableLogic(id_CHECK_SMART_QUOTES_ENABLE);
	case AP_RID_DIALOG_OPTIONS_CHK_ViewShowRuler:		_enableDisableLogic(id_CHECK_VIEW_SHOW_RULER);
	case AP_RID_DIALOG_OPTIONS_CHK_ViewCursorBlink:		_enableDisableLogic(id_CHECK_VIEW_CURSOR_BLINK);
	case AP_RID_DIALOG_OPTIONS_CHK_ViewShowToolbars:	_enableDisableLogic(id_CHECK_VIEW_SHOW_TOOLBARS);
	case AP_RID_DIALOG_OPTIONS_CHK_ViewAll:				_enableDisableLogic(id_CHECK_VIEW_ALL);
	case AP_RID_DIALOG_OPTIONS_CHK_ViewHiddenText:		_enableDisableLogic(id_CHECK_VIEW_HIDDEN_TEXT);
	case AP_RID_DIALOG_OPTIONS_CHK_ViewUnprintable:		_enableDisableLogic(id_CHECK_VIEW_UNPRINTABLE);

		}
#endif	
	
	default:
		BWindow::DispatchMessage(msg, handler);
	}
	
} 

//Behave like a good citizen
bool OptionsWin::QuitRequested()
{
	delete_sem(modalSem);
	return(true);
}

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
	m_pFrame = pFrame;
	BMessage msg;

        if (RehydrateWindow("OptionsWindow", &msg)) {
                newwin = new OptionsWin(&msg);
                newwin->SetDlg(this);
                //Take the information here ...
//                newwin->Lock();
  //              newwin->Close(); 
        }                
}
		
void AP_BeOSDialog_Options::_controlEnable( tControl id, UT_Bool value )
{
	newwin->Lock();
	
	BTabView* pView = (BTabView *)newwin->FindView("TabView"); \
	BCheckBox* control = NULL;
	
	switch(id)
	{
	
	case id_CHECK_SPELL_CHECK_AS_TYPE:
		FIND_CONTROL(control,"SpellCheckAsType")
		control->SetValue(value);
		break;

	case id_CHECK_SPELL_HIDE_ERRORS:
		FIND_CONTROL(control,"SpellHideErrors")
		control->SetValue(value);
		break;

	case id_CHECK_SPELL_SUGGEST:
		FIND_CONTROL(control,"SpellSuggest")
		control->SetValue(value);
		break;

	case id_CHECK_SPELL_MAIN_ONLY:
		FIND_CONTROL(control,"SpellMainOnly")
		control->SetValue(value);
		break;

	case id_CHECK_SPELL_UPPERCASE:
		FIND_CONTROL(control,"SpellUppercase")
		control->SetValue(value);
		break;

	case id_CHECK_SPELL_NUMBERS:
		FIND_CONTROL(control,"SpellNumbers")
		control->SetValue(value);	
		break;

	case id_CHECK_SPELL_INTERNET:
		FIND_CONTROL(control,"SpellInternet")
		control->SetValue(value);		
		break;

	case id_CHECK_PREFS_AUTO_SAVE:
		FIND_CONTROL(control,"AutoSavePrefs")
		control->SetValue(value);
		break;

	case id_CHECK_VIEW_SHOW_RULER:
		FIND_CONTROL(control,"RulerEnable")
		control->SetValue(value);
		break;

	case id_CHECK_VIEW_CURSOR_BLINK:
		FIND_CONTROL(control, "chkCursorBlinkEnable")
		control->SetValue(value);
		break;
	
/*TF: Deprecated with specific toolbar toggling
	case id_CHECK_VIEW_SHOW_TOOLBARS:
		FIND_CONTROL(control,"chkToolbarsEnable")
		control->SetValue(value);
		break;
*/

	case id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR:
		FIND_CONTROL(control,"chkStandardBarEnable")
		control->SetValue(value);
		break;

	case id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR:
		FIND_CONTROL(control,"chkFormatBarEnable")
		control->SetValue(value);
		break;

	case id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR:
		FIND_CONTROL(control,"chkExtraBarEnable")
		control->SetValue(value);
		break;

	case id_CHECK_VIEW_SHOW_STATUS_BAR:
		FIND_CONTROL(control,"chkStatusEnable")
		control->SetValue(value);
		break;


	case id_CHECK_VIEW_ALL:
		FIND_CONTROL(control,"chkViewAll")
		control->SetValue(value);
		break;

	case id_CHECK_VIEW_HIDDEN_TEXT:
		FIND_CONTROL(control,"chkViewHiddenText")
		control->SetValue(value);
		break;

	case id_CHECK_VIEW_UNPRINTABLE:
		FIND_CONTROL(control,"chkViewUnprintable")
		control->SetValue(value);
		break;
		
	case id_CHECK_SMART_QUOTES_ENABLE:
		FIND_CONTROL(control,"SmartQuotes")
		control->SetValue(value);
		break;
	}
	
	newwin->Unlock();
	
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

// Couldn't figure out a way to make a char string from the button name..
#include <stdio.h>

// Okay, the tab view will only report the active tab as a child view, so we store
// which tab is the active one and set the active one to 0, search, 1 , search , 2 , search , restore.

#define DEFINE_GET_SET_BOOL(button , chartitle) \
UT_Bool     AP_BeOSDialog_Options::_gather##button(void) \
{\
	BCheckBox* pBox;	\
	BTabView* pView = (BTabView *)newwin->FindView("TabView"); \
	pBox = (BCheckBox *)newwin->FindView(chartitle); \
	if(pBox) \
		return (pBox->Value() == B_CONTROL_ON); \
	pBox = (BCheckBox *)pView->TabAt(0)->View()->FindView(chartitle); \
	if(pBox) \
		return (pBox->Value() == B_CONTROL_ON);\
\
	pBox = (BCheckBox *)pView->TabAt(1)->View()->FindView(chartitle);\
	if(pBox) \
		return (pBox->Value() == B_CONTROL_ON);\
\
	pBox = (BCheckBox *)pView->TabAt(2)->View()->FindView(chartitle);\
	if(pBox) \
		return (pBox->Value() == B_CONTROL_ON);\
	\
	return UT_FALSE;\
} \
void        AP_BeOSDialog_Options::_set##button(UT_Bool b) \
{ \
	BCheckBox* pBox;\
	BTabView* pView = (BTabView *)newwin->FindView("TabView"); \
	pBox = (BCheckBox *)newwin->FindView(chartitle); \
	if(pBox) \
		return pBox->SetValue( (b == UT_TRUE )); \
	pBox = (BCheckBox *)pView->TabAt(0)->View()->FindView(chartitle); \
	if(pBox) \
		return pBox->SetValue( (b == UT_TRUE )); \
\
	pBox = (BCheckBox *)pView->TabAt(1)->View()->FindView(chartitle);\
	if(pBox) \
		return pBox->SetValue( (b == UT_TRUE )); \
\
	pBox = (BCheckBox *)pView->TabAt(2)->View()->FindView(chartitle);\
	if(pBox) \
		return pBox->SetValue( (b == UT_TRUE )); \
} 

DEFINE_GET_SET_BOOL(SpellCheckAsType , "SpellCheckAsType");
DEFINE_GET_SET_BOOL(SpellHideErrors , "SpellHideErrors");
DEFINE_GET_SET_BOOL(SpellSuggest , "SpellSuggest");
DEFINE_GET_SET_BOOL(SpellMainOnly , "SpellMainOnly");
DEFINE_GET_SET_BOOL(SpellUppercase , "SpellUppercase");
DEFINE_GET_SET_BOOL(SpellNumbers , "SpellNumbers");
DEFINE_GET_SET_BOOL(SpellInternet , "SpellInternet");
DEFINE_GET_SET_BOOL(PrefsAutoSave , "AutoSavePrefs");
DEFINE_GET_SET_BOOL	(ViewShowRuler , "RulerEnable");

//TF: Deprecated with specific toolbar toggling
//DEFINE_GET_SET_BOOL	(ViewShowToolbars , "chkToolbarsEnable");
DEFINE_GET_SET_BOOL	(ViewShowStandardBar , "chkStandardBarEnable");
DEFINE_GET_SET_BOOL	(ViewShowFormatBar , "chkFormatBarEnable");
DEFINE_GET_SET_BOOL	(ViewShowExtraBar , "chkExtraBarEnable");
DEFINE_GET_SET_BOOL	(ViewShowStatusBar , "chkStatusEnable");

DEFINE_GET_SET_BOOL	(ViewCursorBlink , "chkCursorBlinkEnable");
DEFINE_GET_SET_BOOL	(ViewAll , "chkViewAll");
DEFINE_GET_SET_BOOL	(ViewHiddenText , "chkViewHiddenText");
DEFINE_GET_SET_BOOL	(ViewUnprintable , "chkViewUnprintable" );
DEFINE_GET_SET_BOOL	(SmartQuotesEnable , "SmartQuotes");
#undef DEFINE_GET_SET_BOOL

UT_Dimension AP_BeOSDialog_Options::_gatherViewRulerUnits(void) 
{
	int selID = 0;
	BTabView* tabView = (BTabView *)newwin->FindView("TabView");
	BMenuField* units;

	units = (BMenuField *) (tabView->TabAt(2)->View()->FindView("Units"));
	if(units) 
	{
		for(selID = 0; selID < units->Menu()->CountItems(); selID++)
		{
			if(units->Menu()->FindMarked() == units->Menu()->ItemAt(selID))
				break;
		}
		
		if(selID != units->Menu()->CountItems())
			return s_aAlignUnit[selID].dim;
	}
	
	return DIM_IN;
}

void    AP_BeOSDialog_Options::_setViewRulerUnits(UT_Dimension dim) 
{
	int n1;
	for( n1 = 0; n1 < SIZE_aAlignUnit; n1++ ) 
		if( s_aAlignUnit[n1].dim == dim )
			break;
	if( n1 == SIZE_aAlignUnit )
		n1 = 0;

	BTabView* tabView = (BTabView *)newwin->FindView("TabView");
	BMenuField* units;

	units = (BMenuField *) (tabView->TabAt(2)->View()->FindView("Units"));
	units->Menu()->ItemAt(n1)->SetMarked(true);//Select(n1);
}

fp_PageSize::Predefined AP_BeOSDialog_Options::_gatherDefaultPageSize(void)
{
	// FIXME: replace this with *real* gui code
	return defaultPaperSize;
}

void	AP_BeOSDialog_Options::_setDefaultPageSize(fp_PageSize::Predefined pre)
{
	// FIXME: replace this with *real* gui code
	defaultPageSize = pre;
}

void 

int AP_BeOSDialog_Options::_gatherNotebookPageNum(void) 
{				
	return 0;
}			

void    AP_BeOSDialog_Options::_setNotebookPageNum(int pn) 
{	
}
