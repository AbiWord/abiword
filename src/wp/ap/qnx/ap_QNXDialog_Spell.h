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
   virtual void event_SuggestionSelected(int row, int column);
   virtual void event_ReplacementChanged(void);
   
 protected:

#if 0
   // private construction functions
   GtkWidget * _constructWindow(void);
   void	    _populateWindowData(void);
   void 	    _storeWindowData(void);
   void _showMisspelledWord(void);	
#endif

   char * _convertToMB(UT_UCSChar *wword);
   UT_UCSChar * _convertFromMB(char *word);
   
   UT_Bool m_bCancelled;
   short m_iSelectedRow;
   
   // pointers to widgets we need to query/set
   void * m_windowMain;
   void * m_textWord;
   void * m_entryChange;
   void * m_clistSuggestions;
   
   void * m_buttonChange;
   void * m_buttonChangeAll;
   void * m_buttonIgnore;
   void * m_buttonIgnoreAll;
   void * m_buttonAddToDict;
   void * m_buttonCancel;

   //GdkColor m_highlight;

   int m_listHandlerID;
   int m_replaceHandlerID;
};

#endif /* AP_QNXDIALOG_SPELL_H */
