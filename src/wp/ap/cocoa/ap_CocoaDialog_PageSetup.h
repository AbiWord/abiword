/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef AP_CocoaDialog_PageSetup_H
#define AP_CocoaDialog_PageSetup_H

#import <Cocoa/Cocoa.h>
#include "ap_Dialog_PageSetup.h"

class AP_CocoaDialog_PageSetup : public AP_Dialog_PageSetup
{
public:
	AP_CocoaDialog_PageSetup (XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_PageSetup (void);

	virtual void runModal (XAP_Frame *pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	virtual void event_OK (void);
	virtual void event_Cancel (void);
	virtual void event_WindowDelete (void);

	virtual void event_PageSizeChanged (fp_PageSize::Predefined pageSize);
	virtual void event_PageUnitsChanged ();
	virtual void event_MarginUnitsChanged ();

 protected:
	// construction functions
	virtual NSWindow * _constructWindow (void);
	virtual void        _constructWindowContents (NSWindow *container);
	virtual void        _connectSignals (void);


 private:
#if 0
	// pointers to all the action items
	GtkWidget * m_window;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;
	// pointers to widgets that we may need to query
	GtkWidget * m_optionPageSize;
	GtkWidget * m_entryPageWidth;
	GtkWidget * m_entryPageHeight;
	GtkWidget * m_optionPageUnits;
	GtkWidget * m_radioPagePortrait;
	GtkWidget * m_radioPageLandscape;
	GtkWidget * m_spinPageScale;

	GtkWidget * m_optionMarginUnits;
	GtkWidget * m_spinMarginTop;
	GtkWidget * m_spinMarginBottom;
	GtkWidget * m_spinMarginLeft;
	GtkWidget * m_spinMarginRight;
	GtkWidget * m_spinMarginHeader;
	GtkWidget * m_spinMarginFooter;
#endif

    XAP_Frame * m_pFrame;
};

#endif // AP_CocoaDialog_PageSetup_H
