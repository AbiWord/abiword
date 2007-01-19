/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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


#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Spell.h"
#include "ap_QNXDialog_Spell.h"
#include "ut_qnxHelper.h"


XAP_Dialog * AP_QNXDialog_Spell::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
   AP_QNXDialog_Spell * p = new AP_QNXDialog_Spell(pFactory,id);
   return p;
}

AP_QNXDialog_Spell::AP_QNXDialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : AP_Dialog_Spell(pDlgFactory,id)
{
}

AP_QNXDialog_Spell::~AP_QNXDialog_Spell(void)
{
}

/************************************************************/
void AP_QNXDialog_Spell::runModal(XAP_Frame * pFrame)
{
	// class the base class method to initialize some basic xp stuff
	AP_Dialog_Spell::runModal(pFrame);
   
	m_bCancelled = false;
	bool bRes = nextMisspelledWord();

	// To center the dialog, we need the frame of its parent.
   	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow); 

	if (bRes) { // we need to prepare the dialog
      
      	PtWidget_t * mainWindow = _constructWindow();
      	UT_ASSERT(mainWindow);
      
      	// Populate the window's data items
      	_populateWindowData();
      
		UT_QNXCenterWindow(parentWindow, mainWindow);
		UT_QNXBlockWidget(parentWindow, 1);

		PtRealizeWidget(mainWindow);
      
     	// now loop while there are still misspelled words
     	while (bRes) {
	 
	 		// show word in main window
	 		makeWordVisible();
	 
	 		// update dialog with new misspelled word info/suggestions
	 		_showMisspelledWord();
	 
			int count;
			count = PtModalStart();
			done = 0;
			while(!done) {
				PtProcessEvent();
			}
			PtModalEnd(MODAL_END_ARG(count));
		 
			_purgeSuggestions();
	 
	 		if (m_bCancelled) {
				break;
			}
	 
			// get the next unknown word
			bRes = nextMisspelledWord();
		}
      
		_storeWindowData();
      
		UT_QNXBlockWidget(parentWindow, 0);
		PtDestroyWidget(mainWindow);
	}
}

/**********************************************************/
static int s_change_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Spell *dlg = (AP_QNXDialog_Spell *)data;
   UT_ASSERT(widget && dlg);
   dlg->event_Change();
	return Pt_CONTINUE;
}

static int s_change_all_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Spell *dlg = (AP_QNXDialog_Spell *)data;
   UT_ASSERT(widget && dlg);
   dlg->event_ChangeAll();
	return Pt_CONTINUE;
}

static int s_ignore_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Spell *dlg = (AP_QNXDialog_Spell *)data;
   UT_ASSERT(widget && dlg);
   dlg->event_Ignore();
	return Pt_CONTINUE;
}

static int s_ignore_all_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Spell *dlg = (AP_QNXDialog_Spell *)data;
   UT_ASSERT(widget && dlg);
   dlg->event_IgnoreAll();
	return Pt_CONTINUE;
}

static int s_add_to_dict_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Spell *dlg = (AP_QNXDialog_Spell *)data;
   UT_ASSERT(widget && dlg);
   dlg->event_AddToDict();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Spell *dlg = (AP_QNXDialog_Spell *)data;
   UT_ASSERT(widget && dlg);
   dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_suggestion_selected(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Spell *dlg = (AP_QNXDialog_Spell *)data;
   UT_ASSERT(widget && dlg);
	PtListCallback_t   *list = (PtListCallback_t *)info->cbdata;
   dlg->event_SuggestionSelected(list->item_pos, list->item);
	return Pt_CONTINUE;
}

static int s_replacement_changed(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Spell *dlg = (AP_QNXDialog_Spell *)data;
   UT_ASSERT(widget && dlg);
   dlg->event_ReplacementChanged();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Spell *dlg = (AP_QNXDialog_Spell *)data;
   UT_ASSERT(dlg);
   dlg->event_Cancel();
	return Pt_CONTINUE;
}
/********************************************************************/

PtWidget_t * AP_QNXDialog_Spell::_constructWindow(void)
{
   PtWidget_t *windowSpell;
   
   PtWidget_t *label1;
   PtWidget_t *textWord;
   PtWidget_t *label2;
   PtWidget_t *entryChange;
   PtWidget_t *clistSuggestions;
   PtWidget_t *buttonChange;
   PtWidget_t *buttonChangeAll;
   PtWidget_t *buttonIgnore;
   PtWidget_t *buttonIgnoreAll;
   PtWidget_t *buttonAddToDict;
   PtWidget_t *buttonCancel;

   const XAP_StringSet * pSS = m_pApp->getStringSet();
   gchar * unixstr = NULL;      // used for conversions

	windowSpell = abiCreatePhabDialog("ap_QNXDialog_Spell",pSS,AP_STRING_ID_DLG_Spell_SpellTitle); 
	SetupContextHelp(windowSpell,this);
	PtAddHotkeyHandler(windowSpell,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(windowSpell, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);
   
	localizeLabel(abiPhabLocateWidget(windowSpell,"lblNotDictionary"), pSS, AP_STRING_ID_DLG_Spell_UnknownWord);

	textWord = abiPhabLocateWidget(windowSpell,"multiError");
 
	buttonIgnore = abiPhabLocateWidget(windowSpell,"btnIgnore");
	localizeLabel(buttonIgnore, pSS, AP_STRING_ID_DLG_Spell_Ignore);
	PtAddCallback(buttonIgnore, Pt_CB_ACTIVATE, s_ignore_clicked, this);
   
	buttonIgnoreAll = abiPhabLocateWidget(windowSpell,"btnIgnoreAll"); 
	localizeLabel(buttonIgnoreAll, pSS, AP_STRING_ID_DLG_Spell_IgnoreAll);
	PtAddCallback(buttonIgnoreAll, Pt_CB_ACTIVATE, s_ignore_all_clicked, this);
   
	buttonAddToDict = abiPhabLocateWidget(windowSpell,"btnAdd"); 
	localizeLabel(buttonAddToDict, pSS, AP_STRING_ID_DLG_Spell_AddToDict);
	PtAddCallback(buttonAddToDict, Pt_CB_ACTIVATE, s_add_to_dict_clicked, this);

	localizeLabel(abiPhabLocateWidget(windowSpell,"lblChange"), pSS, AP_STRING_ID_DLG_Spell_ChangeTo);

	entryChange = abiPhabLocateWidget(windowSpell,"textChange");
	PtAddCallback(entryChange, Pt_CB_TEXT_CHANGED, s_replacement_changed, this);

   clistSuggestions = abiPhabLocateWidget(windowSpell,"listSuggestions"); 
	PtAddCallback(clistSuggestions, Pt_CB_SELECTION, s_suggestion_selected, this);

   
	buttonChange = abiPhabLocateWidget(windowSpell,"btnChange"); 
	localizeLabel(buttonChange, pSS, AP_STRING_ID_DLG_Spell_Change);
	PtAddCallback(buttonChange, Pt_CB_ACTIVATE, s_change_clicked, this);
   
	buttonChangeAll = abiPhabLocateWidget(windowSpell,"btnChangeAll"); 
	localizeLabel(buttonChangeAll, pSS, AP_STRING_ID_DLG_Spell_ChangeAll);
	PtAddCallback(buttonChangeAll, Pt_CB_ACTIVATE, s_change_all_clicked, this);
   
	buttonCancel = abiPhabLocateWidget(windowSpell,"btnCancel"); 
	localizeLabel(buttonCancel, pSS, XAP_STRING_ID_DLG_Cancel);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);
   
   // update member variables with the important widgets 
   // that we'll interact with later
   
   m_buttonChange = buttonChange;
   m_buttonChangeAll = buttonChangeAll;
   m_buttonIgnore = buttonIgnore;
   m_buttonIgnoreAll = buttonIgnoreAll;
   m_buttonAddToDict = buttonAddToDict;
   m_buttonCancel = buttonCancel;
   
   m_textWord = textWord;
   m_entryChange = entryChange;
   m_clistSuggestions = clistSuggestions;
   m_windowMain = windowSpell;
   
   return windowSpell;
}

static char *get_text_string(PtWidget_t *w) {
	char *text;
	PtGetResource(w, Pt_ARG_TEXT_STRING, &text, 0);
	return text;
}

static void set_text_string(PtWidget_t *w, char *str) {
	PtSetResource(w, Pt_ARG_TEXT_STRING, str, 0);
}

void AP_QNXDialog_Spell::_showMisspelledWord(void)
{                                
	PtArg_t args[10];
	unsigned int 	n;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, 0, 0);
	PtSetResources(m_textWord, n, args);
   
	const UT_UCSChar *p;
	char 		*str;
	int		   len;

	// insert start of sentence
	UT_sint32 iLength;
	PtMultiTextAttributes_t attrs;

    p = m_pWordIterator->getPreWord(iLength);
	if (0 < iLength)
	{
		str = _convertToMB(p);
		len = mbstrlen(str, 0, NULL);
		PtMultiTextAttributes_t attrs;
		memset(&attrs, 0, sizeof(attrs));
		attrs.text_color = Pg_BLACK;
		PtMultiTextModifyText(m_textWord, 	/* Widget */
							  0, 0,			/* start, end positions */
							  -1,			/* insert position (end of buffer) */
							  str,	/* text to insert */
							  len,			/* length of text */
							  &attrs,			/* Attributes (use default) */
							  Pt_MT_TEXT_COLOR);		    /* Flags on which attrs to use */				
		FREEP(str);
	}
   
	// insert misspelled word (in highlight color)
    p = m_pWordIterator->getCurrentWord(iLength);
	if (0 < iLength)
	{
		str = _convertToMB(p);
		len = mbstrlen(str, 0, NULL);
		attrs.text_color = Pg_RED;
		PtMultiTextModifyText(m_textWord, 	/* Widget */
							  0, 0,			/* start, end positions */
							  -1,			/* insert position (end of buffer) */
							  str,			/* text to insert */
							  len,			/* length of text */
							  &attrs,		/* Attributes */
							  Pt_MT_TEXT_COLOR);   /* Flags on which attrs to use */				
   
		FREEP(str);
	}

	// insert end of sentence
    p = m_pWordIterator->getPostWord(iLength);
	if (0 < iLength)
	{
		str = _convertToMB(p);
		len = mbstrlen(str, 0, NULL);
		attrs.text_color = Pg_BLACK;
		PtMultiTextModifyText(m_textWord, 	/* Widget */
							  0, 0,			/* start, end positions */
							  -1,			/* insert position (end of buffer) */
							  str,			/* text to insert */
							  len,			/* length of text */
							  &attrs,		/* Attributes */
							  Pt_MT_TEXT_COLOR);   /* Flags on which attrs to use */				
		FREEP(str);
    }
	// TODO: set scroll position so misspelled word is centered

	char *suggest[2] = {NULL, NULL};

	PtListDeleteAllItems(m_clistSuggestions); 
	for (unsigned int i = 0; i < m_Suggestions->getItemCount(); i++) {
		suggest[0] = _convertToMB((UT_UCSChar *)m_Suggestions->getNthItem(i));
		PtListAddItems(m_clistSuggestions, (const char **)suggest, 1, 0);
		if (i==0) {
			set_text_string(m_entryChange, suggest[0]);
		}
		FREEP(suggest[0]);
	}
  
   if (!m_Suggestions->getItemCount()) {
		const XAP_StringSet * pSS = m_pApp->getStringSet();
		UT_UTF8String s;
		pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_NoSuggestions,s);
		UT_XML_cloneNoAmpersands(suggest[0], s.utf8_str());


		PtListAddItems(m_clistSuggestions, (const char **)&suggest[0], 1, 0);
		FREEP(suggest[0]);
	  	n = 0;
	  	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_BLOCKED, Pt_BLOCKED);
		PtSetResources(m_clistSuggestions, n, args);

		n = 0;
      	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, NULL, 0);
		PtSetResources(m_entryChange, n, args);

	} else {
      	// select first on the list; signal handler should update our entry box
		PtListSelectPos(m_clistSuggestions, 1);
	}
	m_iSelectedRow = -1;
}

void AP_QNXDialog_Spell::_populateWindowData(void)
{
   // TODO: initialize list of user dictionaries
}

void AP_QNXDialog_Spell::_storeWindowData(void)
{
   // TODO: anything to store?
}

/*************************************************************/

void AP_QNXDialog_Spell::event_Change()
{
	UT_UCSChar * replace = NULL;

   	if (m_iSelectedRow != -1) {
		replace = (UT_UCSChar*) m_Suggestions->getNthItem(m_iSelectedRow);
		changeWordWith(replace);
	}
	else {
		char *text = get_text_string(m_entryChange);
		replace = _convertFromMB(text);
		if (!UT_UCS4_strlen(replace)) {
		   FREEP(replace);
		   return;
		}
		changeWordWith(replace);
		FREEP(replace);
   	}
	done = 1;
}

void AP_QNXDialog_Spell::event_ChangeAll()
{
	UT_UCSChar * replace = NULL;

	if (m_iSelectedRow != -1) {
		replace = (UT_UCSChar*) m_Suggestions->getNthItem(m_iSelectedRow);
		addChangeAll(replace);
		changeWordWith(replace);
	}
	else {
		char *text = get_text_string(m_entryChange);
		replace = _convertFromMB(text);
		if (!UT_UCS4_strlen(replace)) {
			FREEP(replace);
			return;
		}
		addChangeAll(replace);
		changeWordWith(replace);
		FREEP(replace);
	}
	done = 1;
}

void AP_QNXDialog_Spell::event_Ignore()
{
   	ignoreWord();
	done = 1;
}

void AP_QNXDialog_Spell::event_IgnoreAll()
{
   	addIgnoreAll();
   	ignoreWord();
	done = 1;
}

void AP_QNXDialog_Spell::event_AddToDict()
{
	addToDict();
	ignoreWord();
	done = 1;
}

void AP_QNXDialog_Spell::event_Cancel()
{
   	if (!done++) {
		m_bCancelled = true;
	}
}

//This is called when the user clicks an item in the list ...
void AP_QNXDialog_Spell::event_SuggestionSelected(int index, char *newreplacement)
{

   if (!newreplacement) return;

	m_iSelectedRow = index - 1;
	set_text_string(m_entryChange, newreplacement); 

	//Set the suggestion text ....
   UT_ASSERT(newreplacement);
}

void AP_QNXDialog_Spell::event_ReplacementChanged()
{
	printf("Replacement changed \n");

	char * modtext = get_text_string(m_entryChange);
	UT_ASSERT(modtext);

	int index = PtListItemPos(m_clistSuggestions, modtext);
  	if (index != 0) {
		m_iSelectedRow = index -1;
		PtListSelectPos(m_clistSuggestions, index); 
		//PtListGotoPos(m_clistSuggestions, index);
	}
	else {
		if (m_iSelectedRow > 0) {
			PtListUnselectPos(m_clistSuggestions, m_iSelectedRow + 1);
		}
		//PtListGotoPos(m_clistSuggestions, 0);
   		m_iSelectedRow = -1;
	}
}


// Photon uses multibyte strings for i18n
// these functions convert from wide (UCS) to MB and back

// TODO: these are probably generally useful functions,
// TODO: but I don't know about xp support for them.

// make a multibyte encoded version of a string
char * AP_QNXDialog_Spell::_convertToMB(const UT_UCS4Char *wword)
{
    UT_UCS4String ucs4(wword);
    return g_strdup(ucs4.utf8_str());
}

// make a multibyte encoded version of a string
char * AP_QNXDialog_Spell::_convertToMB(const UT_UCS4Char *wword, UT_sint32 iLength)
{
   UT_UCS4String ucs4(wword, iLength);
   return g_strdup(ucs4.utf8_str());
}

// make a wide string from a multibyte string
UT_UCSChar * AP_QNXDialog_Spell::_convertFromMB(const char *word)
{
    UT_UCS4Char * str = 0;
    UT_UCS4String ucs4(word);
    UT_UCS4_cloneString(&str, ucs4.ucs4_str());
    return str;
}
