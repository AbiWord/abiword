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

#ifndef AP_COCOADIALOG_HDRFTR_H
#define AP_COCOADIALOG_HDRFTR_H

#include <Cocoa/Cocoa.h>
#include "ap_Dialog_HdrFtr.h"

class XAP_CocoaFrame;

/*****************************************************************/

class AP_CocoaDialog_HdrFtr: public AP_Dialog_HdrFtr
{
public:
	AP_CocoaDialog_HdrFtr(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_HdrFtr(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	virtual void eventOk(void);
	virtual void eventCancel(void);
	void         RestartChanged(void);
	void         RestartSpinChanged(void);
	void         CheckChanged(HdrFtr_Control which);

 protected:
	virtual NSWindow * _constructWindow (void);
	virtual void _constructWindowContents (NSWindow * parent);
	virtual void _connectSignals(void);

#if 0
	GtkWidget * m_wHdrFtrCheck[6];
	GtkWidget * m_wRestartButton;
	GtkWidget * m_wRestartLabel;
	GtkObject * m_oSpinAdj;
	GtkWidget * m_wSpin;
	GtkWidget * m_wButtonOK;
	GtkWidget * m_wButtonCancel;
	GtkWidget * m_wHdrFtrDialog;
#endif
};

#endif /* AP_COCOADIALOG_HDRFTR_H */


