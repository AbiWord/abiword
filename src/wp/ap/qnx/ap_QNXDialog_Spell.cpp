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


// TODO: still getting some artifacts when doing highligh/replacements

#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

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
   
	m_bCancelled = UT_FALSE;
	UT_Bool bRes = nextMisspelledWord();

	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
   	UT_ASSERT(pQNXFrame);
      
   	// Get the Window of the parent frame
   	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
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
	 
			int count = PtModalStart();
			done = 0;
			while(!done) {
				PtProcessEvent();
			}
			PtModalEnd(MODAL_END_ARG(count));
		 
			for (int i = 0; i < m_Suggestions.count; i++) {
				FREEP(m_Suggestions.word[i]);
			}
	 		FREEP(m_Suggestions.word);
	 		FREEP(m_Suggestions.score);
	 		m_Suggestions.count = 0;
	 
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
   PtWidget_t *tableMain;
   
   PtWidget_t *label1;
   PtWidget_t *scroll2;
   PtWidget_t *textWord;
   PtWidget_t *label2;
   PtWidget_t *entryChange;
   PtWidget_t *scroll1;
   PtWidget_t *clistSuggestions;
   PtWidget_t *buttonChange;
   PtWidget_t *buttonChangeAll;
   PtWidget_t *buttonIgnore;
   PtWidget_t *buttonIgnoreAll;
   PtWidget_t *buttonAddToDict;
   PtWidget_t *buttonCancel;
	PtArg_t  args[10];
	int    n;
	PhArea_t area;

   const XAP_StringSet * pSS = m_pApp->getStringSet();
   XML_Char * unixstr = NULL;      // used for conversions

	n = 0;
    PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(AP_STRING_ID_DLG_Spell_SpellTitle), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowSpell = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowSpell, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);
   
#define BUTTON_WIDTH 80
#define TEXT_WIDTH   200
#define LABEL_HEIGHT  10
#define TEXTBOX_HEIGHT  100
#define H_SPACER     15
#define V_SPACER   	 15


	n = 0;
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_UnknownWord));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = area.pos.y = V_SPACER;
	area.size.w = TEXT_WIDTH; area.size.h = LABEL_HEIGHT;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
   	label1 = PtCreateWidget(PtLabel, windowSpell, n, args);

	//Add the multi-text snippet around the word we are writing ....
	n = 0;
	area.pos.y += area.size.h + V_SPACER;
	area.size.h = TEXTBOX_HEIGHT;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE);
    textWord = PtCreateWidget(PtMultiText, windowSpell, n, args);

   // ignore button set
	n = 0;
	area.pos.x += area.size.w + H_SPACER;
	area.size.w = BUTTON_WIDTH;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
			 Pt_GROUP_EQUAL_SIZE_HORIZONTAL, 
			 Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
   PtWidget_t * vboxIgnoreButtons = PtCreateWidget(PtGroup, windowSpell, n, args);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_Ignore));
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
   buttonIgnore = PtCreateWidget(PtButton, vboxIgnoreButtons, n, args);
	PtAddCallback(buttonIgnore, Pt_CB_ACTIVATE, s_ignore_clicked, this);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_IgnoreAll));
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
   buttonIgnoreAll = PtCreateWidget(PtButton, vboxIgnoreButtons, n, args);
	PtAddCallback(buttonIgnoreAll, Pt_CB_ACTIVATE, s_ignore_all_clicked, this);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_AddToDict));
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
   buttonAddToDict = PtCreateWidget(PtButton, vboxIgnoreButtons, n, args);
	PtAddCallback(buttonAddToDict, Pt_CB_ACTIVATE, s_add_to_dict_clicked, this);

   // suggestion half
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_ChangeTo));
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = H_SPACER; 
	area.pos.y = V_SPACER + LABEL_HEIGHT + V_SPACER + TEXTBOX_HEIGHT + V_SPACER; 
	area.size.w = TEXT_WIDTH / 3;
	area.size.h = LABEL_HEIGHT;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
   label2 = PtCreateWidget(PtLabel, windowSpell, n, args);

	n = 0;
	area.pos.x += area.size.w + H_SPACER;
	area.size.w = (TEXT_WIDTH / 3) * 2 - H_SPACER;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
   entryChange = PtCreateWidget(PtText, windowSpell, n, args);
	PtAddCallback(entryChange, Pt_CB_TEXT_CHANGED, s_replacement_changed, this);
	//We should call change when we hit return
	//PtAddCallback(entryChange, Pt_CB_ACTIVATE, s_change_clicked, this);

	n = 0;
	area.pos.x = H_SPACER;
	area.pos.y += area.size.h + V_SPACER;
	area.size.w = TEXT_WIDTH;
	area.size.h = TEXTBOX_HEIGHT;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
   clistSuggestions = PtCreateWidget(PtList, windowSpell, n, args);
	//We should call the "activate" on double click
	//PtAddCallback(clistSuggestions, Pt_CB_ACTIVATE, s_change_clicked, this);
	PtAddCallback(clistSuggestions, Pt_CB_SELECTION, s_suggestion_selected, this);

   
   // change buttons
	n = 0;
	area.pos.x += area.size.w + H_SPACER;
	area.size.w = BUTTON_WIDTH;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
			 Pt_GROUP_EQUAL_SIZE_HORIZONTAL, 
			 Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
   PtWidget_t * vboxChangeButtons = PtCreateWidget(PtGroup, windowSpell, n, args);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_Change));
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
   buttonChange = PtCreateWidget(PtButton, vboxChangeButtons, n, args);
	PtAddCallback(buttonChange, Pt_CB_ACTIVATE, s_change_clicked, this);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_ChangeAll));
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
   buttonChangeAll = PtCreateWidget(PtButton, vboxChangeButtons, n, args);
	PtAddCallback(buttonChangeAll, Pt_CB_ACTIVATE, s_change_all_clicked, this);
   
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
   buttonCancel = PtCreateWidget(PtButton, vboxChangeButtons, n, args);
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
	PtArg_t arg;
	char *text;
	PtSetArg(&arg, Pt_ARG_TEXT_STRING, &text, 0);
	PtGetResources(w, 1, &arg);
	return text;
}

static void set_text_string(PtWidget_t *w, char *str) {
	PtArg_t arg;
	PtSetArg(&arg, Pt_ARG_TEXT_STRING, str, 0);
	PtSetResources(w, 1, &arg);
}


void AP_QNXDialog_Spell::_showMisspelledWord(void)
{                                
	PtArg_t args[10];
	int 	n;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, 0, 0);
	PtSetResources(m_textWord, n, args);
   
   UT_UCSChar *p;
   char 		*str;
   int		   len;

   // insert start of sentence
   p = _getPreWord();
   str = _convertToMB(p);
   len = mbstrlen(str, 0, NULL);
   FREEP(p);
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
   
   // insert misspelled word (in highlight color)
   p = _getCurrentWord();
   str = _convertToMB(p);
   len = mbstrlen(str, 0, NULL);
   FREEP(p);
	attrs.text_color = Pg_RED;
  	PtMultiTextModifyText(m_textWord, 	/* Widget */
						  0, 0,			/* start, end positions */
						  -1,			/* insert position (end of buffer) */
						  str,			/* text to insert */
						  len,			/* length of text */
						  &attrs,		/* Attributes */
						  Pt_MT_TEXT_COLOR);   /* Flags on which attrs to use */				
   
   FREEP(str);

   // insert end of sentence
   p = _getPostWord();
   str = _convertToMB(p);
   len = mbstrlen(str, 0, NULL);
   FREEP(p);
	attrs.text_color = Pg_BLACK;
  	PtMultiTextModifyText(m_textWord, 	/* Widget */
						  0, 0,			/* start, end positions */
						  -1,			/* insert position (end of buffer) */
						  str,			/* text to insert */
						  len,			/* length of text */
						  &attrs,		/* Attributes */
						  Pt_MT_TEXT_COLOR);   /* Flags on which attrs to use */				
   FREEP(str);
    
   // TODO: set scroll position so misspelled word is centered


   char *suggest[2] = {NULL, NULL};

   PtListDeleteAllItems(m_clistSuggestions); 
   for (int i = 0; i < m_Suggestions.count; i++) {
	  suggest[0] = _convertToMB((UT_UCSChar *)m_Suggestions.word[i]);
      PtListAddItems(m_clistSuggestions, (const char **)suggest, 1, 0);
	  if (i==0) {
		set_text_string(m_entryChange, suggest[0]);
	  }
	  FREEP(suggest[0]);
   }
  
   if (!m_Suggestions.count) {

      const XAP_StringSet * pSS = m_pApp->getStringSet();
      UT_XML_cloneNoAmpersands(suggest[0], pSS->getValue(AP_STRING_ID_DLG_Spell_NoSuggestions));

      PtListAddItems(m_clistSuggestions, (const char **)&suggest[0], 1, 0);
      FREEP(suggest[0]);
	  	n = 0;
	  	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_BLOCKED, Pt_BLOCKED);
		PtSetResources(m_clistSuggestions, n, args);

		n = 0;
      	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, NULL, 0);
		PtSetResources(m_entryChange, n, args);

      	m_iSelectedRow = -1;
   } else {
      	// select first on the list; signal handler should update our entry box
		PtListSelectPos(m_clistSuggestions, 1);
   }
   
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
		replace = (UT_UCSChar*) m_Suggestions.word[m_iSelectedRow];
		changeWordWith(replace);
	}
	else {
		char *text = get_text_string(m_entryChange);
		replace = _convertFromMB(text);
		if (!UT_UCS_strlen(replace)) {
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
		replace = (UT_UCSChar*) m_Suggestions.word[m_iSelectedRow];
		addChangeAll(replace);
		changeWordWith(replace);
	}
	else {
		char *text = get_text_string(m_entryChange);
		replace = _convertFromMB(text);
		if (!UT_UCS_strlen(replace)) {
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
   	m_bCancelled = UT_TRUE;
	done = 1;
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


// GTK+ uses multibyte strings for i18n
// these functions convert from wide (UCS) to MB and back

// TODO: these are probably generally useful functions,
// TODO: but I don't know about xp support for them.

// make a multibyte encoded version of a string
char * AP_QNXDialog_Spell::_convertToMB(UT_UCSChar *wword)
{
	int    len;
	char   *buffer;

	len = UT_UCS_strlen(wword);
	buffer = new char[(len + 1) * 3];	//Be very pessimistic about size
	wcstombs(buffer, (wchar_t *)wword, len*3);	
	return buffer;

#if 0
	int len, offset;
	char *buffer;

	len = offset = 0;
	for (buffer = NULL; wword[offset]; offset++) {
		if (offset >= len) {
			len += 10;
			buffer = (char *)realloc(buffer, len);
		}
		buffer[offset] = (char)wword[offset];
	}
	buffer[offset] = '\0';
	return buffer;
#endif

#if 0
   int mbindex = 0, wcindex = 0;
   int mblength = 0;
   int wclength = UT_UCS_strlen(wword);
   while (wcindex < wclength) {
      int len = wctomb(NULL, (wchar_t)(wword[wcindex]));
      UT_ASSERT(len >= 0);
      mblength += len;
      wcindex++;
   }
   wcindex = 0;
   char * word = (char*) calloc(mblength + 1, sizeof(char));
   if (word == NULL) return NULL;
     
   while (mbindex < mblength) {
      int len = wctomb(word+mbindex, (wchar_t)(wword[wcindex]));
      UT_ASSERT(len >= 0);
      mbindex += len;
      wcindex++;
   }
   word[mblength] = 0;

//   UT_DEBUGMSG(("wc2mb: wc %i/%i - mb %i/%i", wcindex, wclength, mbindex, mblength));
   UT_ASSERT(mblength >= mbindex);
   
   return word;
#endif
	return(NULL);
}

// make a wide string from a multibyte string
UT_UCSChar * AP_QNXDialog_Spell::_convertFromMB(char *word)
{
	int 		len;
	UT_UCSChar *buffer;

	len = mbstrlen(word, 0, NULL);
	buffer = new UT_UCSChar[(len + 1) * 2];
	mbstowcs((wchar_t *)buffer, word, len);	
	return buffer;

#if 0
	int len, offset;
	UT_UCSChar *buffer;

	len = offset = 0;
	for (buffer = NULL; word[offset]; offset++) {
		if (offset >= len) {
			len += 10;
			buffer = (UT_UCSChar *)realloc(buffer, len*sizeof(UT_UCSChar));
		}
		buffer[offset] = (UT_UCSChar)word[offset];
	}
	buffer[offset] = '\0';
	return buffer;
#endif

#if 0
   int wcindex = 0, mbindex = 0;
   int wclength = 0;
   int mblength = strlen(word);
   while (mbindex < mblength) {
      int len = mblen(word+mbindex, mblength-mbindex);
      UT_ASSERT(len >= 0);
      mbindex += len;
      wclength++;
   }
   mbindex = 0;

   wchar_t wch; // we only support two bytes, so this is a temp Unicode char holder
   UT_UCSChar * wword = (UT_UCSChar*) calloc(wclength + 1, sizeof(UT_UCSChar));
   if (wword == NULL) return NULL;
     
   while (wcindex < wclength) {
      int len = mbtowc(&wch, word+mbindex, mblength-mbindex);
      UT_ASSERT(len >= 0);
      mbindex += len;
      wword[wcindex++] = (UT_UCSChar) wch;
   }
   wword[wclength] = 0;
   
//   UT_DEBUGMSG(("mb2wc: mb %i/%i - wc %i/%i", mbindex, mblength, wcindex, wclength));
   UT_ASSERT(wclength >= wcindex);
   
   return (UT_UCSChar*)wword;
#endif
	return(NULL);
}
