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

#ifndef AP_QNXDIALOG_SPELL_H
#define AP_QNXDIALOG_SPELL_H

#include "ap_Dialog_Spell.h"
#include <Pt.h>

class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_Spell: public AP_Dialog_Spell
{
 public:
   AP_QNXDialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
   virtual ~AP_QNXDialog_Spell(void);
   
   virtual void			runModal(XAP_Frame * pFrame);

   static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

   // callbacks can fire these events
   virtual void event_Change(void);
   virtual void event_ChangeAll(void);
   virtual void event_Ignore(void);
   virtual void event_IgnoreAll(void);
   virtual void event_AddToDict(void);
   virtual void event_Cancel(void);
   virtual void event_SuggestionSelected(int index, char *text);
   virtual void event_ReplacementChanged(void);
   
 protected:

   // private construction functions
   PtWidget_t * _constructWindow(void);
   void	    _populateWindowData(void);
   void     _storeWindowData(void);
   void 	_showMisspelledWord(void);	

   char * _convertToMB(UT_UCSChar *wword);
   UT_UCSChar * _convertFromMB(char *word);
   
   bool m_bCancelled;
   short m_iSelectedRow;
   
   // pointers to widgets we need to query/set
   PtWidget_t * m_windowMain;
   PtWidget_t * m_textWord;
   PtWidget_t * m_entryChange;
   PtWidget_t * m_clistSuggestions;
   
   PtWidget_t * m_buttonChange;
   PtWidget_t * m_buttonChangeAll;
   PtWidget_t * m_buttonIgnore;
   PtWidget_t * m_buttonIgnoreAll;
   PtWidget_t * m_buttonAddToDict;
   PtWidget_t * m_buttonCancel;

   //GdkColor m_highlight;

   int m_listHandlerID;
   int m_replaceHandlerID;
	int done;
};

#endif /* AP_QNXDIALOG_SPELL_H */
