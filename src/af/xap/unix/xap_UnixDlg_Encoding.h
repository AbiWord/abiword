/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ut_dialogHelper.h"
#include "xap_Dialog.h"
#include "ut_Encoding.h"
#include "xap_Dlg_Encoding.h"

/********************************************************************
INSTRUCTIONS FOR DESIGN OF THE PLATFORM VERSIONS OF THIS DIALOGUE

(1)	implement runModal(); at the moment we display a single listbox

(2)	m_iEncCount will tell you how many list entries there will be; 
	the encoding strings are then in m_ppEncodings (already sorted)

(3)	use _setEncoding() to set the member variables in response
	to the user selection when the dialog is closing.
*********************************************************************/



class XAP_UnixDialog_Encoding : public XAP_Dialog_Encoding
{
public:
	XAP_UnixDialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Encoding(void);

	virtual void					runModal(XAP_Frame * pFrame);
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_Cancel(void);
	virtual void			event_DoubleClick(void);
	virtual void			event_WindowDelete(void);

protected:

	gint 		_getFromList(void);
	GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	
	GtkWidget * m_windowMain;
	GtkWidget * m_clistWindows;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;
};
#endif /* XAP_UNIXDIALOG_ENCODING_H */





