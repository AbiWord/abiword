/* AbiSource Application Framework
 * Copyright (C) 2001-2002 AbiSource, Inc.
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

#ifndef XAP_UNIXDIALOG_ENCODING_H
#define XAP_UNIXDIALOG_ENCODING_H


#include "ut_types.h"
#include "ut_xml.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_UnixDialogHelper.h"
#include "xap_Dialog.h"
#include "ut_Encoding.h"
#include "xap_Dlg_Encoding.h"

class XAP_Frame;

class XAP_UnixDialog_Encoding : public XAP_Dialog_Encoding
{
public:
	XAP_UnixDialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Encoding(void);

	virtual void			runModal(XAP_Frame * pFrame);
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events

 protected:

 private:

	typedef enum
	  {
	    BUTTON_OK,
	    BUTTON_CANCEL
	  } ResponseId ;

	static void s_clist_event(GtkWidget * widget,
				  GdkEventButton * event,
				  XAP_UnixDialog_Encoding * dlg) ;

	void event_Ok (void);
	void event_Cancel (void);

	virtual void	event_DoubleClick(void);

	GtkWidget     * _constructWindow(void);
	void		_populateWindowData(void);
	gint 		_getFromList(void);
	
	GtkWidget * m_windowMain;
	GtkWidget * m_clistWindows;
};
#endif /* XAP_UNIXDIALOG_ENCODING_H */





