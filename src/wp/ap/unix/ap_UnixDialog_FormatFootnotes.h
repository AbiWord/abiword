/* AbiWord
 * Copyright (C) 2003 Martin Sevior
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

#ifndef AP_UNIXDIALOG_FORMATFOOTNOTES_H
#define AP_UNIXDIALOG_FORMATFOOTNOTES_H

#include "ap_Dialog_FormatFootnotes.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_FormatFootnotes: public AP_Dialog_FormatFootnotes
{
public:
	AP_UnixDialog_FormatFootnotes(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_FormatFootnotes(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	void event_OK(void);
	void event_Cancel(void);
	void event_Delete(void);

protected:
	virtual GtkWidget *		_constructWindow(void);
	void _constructWindowContents (GtkWidget * container);

	enum
	  {
	    BUTTON_CANCEL,
	    BUTTON_OK,
	    BUTTON_DELETE
	  } ResponseId ;

	GtkWidget * m_windowMain;
};

#endif /* AP_UNIXDIALOG_FORMATFOOTNOTES_H */
