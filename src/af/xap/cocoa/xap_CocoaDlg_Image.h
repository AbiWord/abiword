/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
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

#ifndef XAP_COCOADIALOG_IMAGE_H
#define XAP_COCOADIALOG_IMAGE_H

#include "xap_Dlg_Image.h"

class XAP_CocoaFrame;

/*****************************************************************/

class XAP_CocoaDialog_Image: public XAP_Dialog_Image
{
 public:
	XAP_CocoaDialog_Image(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_CocoaDialog_Image(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
	void event_Ok ();
	void event_Cancel ();

 protected:
#if 0
	void _constructWindowContents (GtkWidget * container);
	virtual GtkWidget * _constructWindow ();
	void _connectSignals ();

	GtkWidget * mMainWindow;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;

 private:
	GtkWidget *height_spin;
	GtkWidget *width_spin;
#endif
};

#endif /* XAP_COCOADIALOG_IMAGE_H */
