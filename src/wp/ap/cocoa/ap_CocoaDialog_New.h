/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_COCOADIALOG_NEW_H
#define AP_COCOADIALOG_NEW_H

#include <Cocoa/Cocoa.h>
#include "ap_Dialog_New.h"

class XAP_CocoaFrame;

/*****************************************************************/

class AP_CocoaDialog_New: public AP_Dialog_New
{
public:
	AP_CocoaDialog_New(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_New(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, 
											   XAP_Dialog_Id dlgid);

	void event_Ok ();
	void event_Cancel ();
	void event_ToggleUseTemplate (const char * name);
	void event_ToggleOpenExisting ();
	void event_ToggleStartNew ();
	
protected:

	virtual NSWindow * _constructWindow ();
	virtual void _constructWindowContents (NSWindow * container);

	void _connectSignals ();

private:
#if 0
	/* private ... */
	GtkWidget * m_mainWindow;
	GtkWidget * m_buttonOk;
	GtkWidget * m_buttonCancel;
#endif

	XAP_Frame * m_pFrame;
#if 0
	GtkWidget * m_entryFilename;
	GtkWidget * m_radioNew;
	GtkWidget * m_radioExisting;
	GtkWidget * m_radioEmpty;
#endif
};

#endif /* AP_COCOADIALOG_NEW_H */
