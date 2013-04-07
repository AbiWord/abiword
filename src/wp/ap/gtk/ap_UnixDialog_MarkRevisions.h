/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#ifndef AP_UNIXDIALOG_MARKREVISIONS_H
#define AP_UNIXDIALOG_MARKREVISIONS_H

#include <gtk/gtk.h>
#include "ap_Dialog_MarkRevisions.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_MarkRevisions: public AP_Dialog_MarkRevisions
{
 public:
	AP_UnixDialog_MarkRevisions(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_MarkRevisions(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

 protected:
	typedef enum
	{
		BUTTON_OK = GTK_RESPONSE_OK,
		BUTTON_CANCEL = GTK_RESPONSE_CANCEL
	} ResponseId ;

	virtual GtkWidget * constructWindow () ;

	static void ok_callback ( GtkWidget*, AP_UnixDialog_MarkRevisions * me)
	  {
	    me->event_OK () ;
	  }

	static void cancel_callback ( GtkWidget*, AP_UnixDialog_MarkRevisions * me)
	  {
	    me->event_OK () ;
	  }

	static void destroy_callback ( GtkWidget*, gpointer /*unused*/, AP_UnixDialog_MarkRevisions * me)
	  {
	    me->event_Cancel () ;
	  }

	static void focus_toggled_callback ( GtkWidget *, AP_UnixDialog_MarkRevisions * me )
	  {
	    me->event_FocusToggled () ;
	  }

	void event_OK () ;
	void event_Cancel () ;
	void event_FocusToggled () ;

 private:

	void constructWindowContents ( GtkWidget * container ) ;

	GtkWidget * mRadio1 ;
	GtkWidget * mRadio2 ;

	GtkWidget * mEntryLbl ;
	GtkWidget * mComment ;
	GtkWidget * mButtonOK ;
};

#endif /* AP_UNIXDIALOG_MARKREVISIONS_H */
