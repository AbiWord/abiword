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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_UNIXDIALOG_SPELL_H
#define AP_UNIXDIALOG_SPELL_H

#include "ap_Dialog_Spell.h"


class XAP_Frame;


class AP_UnixDialog_Spell : public AP_Dialog_Spell
{
public:

	AP_UnixDialog_Spell (XAP_DialogFactory * pDlgFactory,
						 XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Spell (void);

	static XAP_Dialog *	static_constructor (XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void runModal (XAP_Frame * pFrame);

	// callbacks can fire these events
	virtual void onChangeClicked	  (void);
	virtual void onChangeAllClicked	  (void);
	virtual void onIgnoreClicked	  (void);
	virtual void onIgnoreAllClicked	  (void);
	virtual void onAddClicked		  (void);
	virtual void onSuggestionSelected (void);
	virtual void onSuggestionChanged  (void);

	const GtkWidget * getWindow (void) const { return m_wDialog; }

protected:

   virtual GtkWidget * _constructWindow	   (void);
   void				   _populateWindowData (void);
   void 			   _updateWindow 	   (void);

private:

   char 	  * _convertToMB   (const UT_UCSChar *wword);
   char 	  * _convertToMB   (const UT_UCSChar *wword,
								UT_sint32 iLength);
   UT_UCSChar * _convertFromMB (const char *word);

   // pointers to widgets we need to query/set
   GtkWidget * m_wDialog;
   GtkWidget * m_txWrong;
   GtkWidget * m_eChange;
   GtkWidget * m_lvSuggestions;

   GdkColor m_highlight;

   guint m_listHandlerID;
   guint m_replaceHandlerID;
};

#endif /* AP_UNIXDIALOG_SPELL_H */
