/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_FIELD_H
#define AP_COCOADIALOG_FIELD_H

#include "ap_Dialog_Field.h"

class XAP_CocoaFrame;


/*****************************************************************/

class AP_CocoaDialog_Field: public AP_Dialog_Field
{
public:
	AP_CocoaDialog_Field(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *
pFactory , XAP_Dialog_Id id);
	virtual ~AP_CocoaDialog_Field(void);

	virtual void runModal(XAP_Frame * pFrame);

	void event_OK(void);
	void event_Cancel(void);
	void event_WindowDelete(void);
	void types_changed(int row);
	void setTypesList(void);
	void setFieldsList(void);

protected:
#if 0
	virtual GtkWidget *		_constructWindow(void);
	GtkWidget *				_constructWindowContents (void);
	void					_populateCatogries(void);
	void					_connectSignals (void);

	GtkWidget * m_windowMain;

	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;

	GtkWidget * m_listTypes;
	GtkWidget * m_listFields;
	GtkWidget * m_entryParam;
#endif
};

#endif /* AP_COCOADIALOG_FIELD_H */






