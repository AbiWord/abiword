/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef XAP_UNIXDIALOG_ABOUT_H
#define XAP_UNIXDIALOG_ABOUT_H

#include "xap_Dlg_About.h"
#include "gr_UnixGraphics.h"
#include "gr_UnixImage.h"

/*****************************************************************/

class XAP_UnixDialog_About: public XAP_Dialog_About
{
public:
	XAP_UnixDialog_About(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_About(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			runModal(XAP_Frame * pFrame);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_URL(void);
	virtual void			event_WindowDelete(void);
	virtual void			event_DrawingAreaExpose(void);
	
 protected:

	GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	void		_preparePicture(void);
	
	GtkWidget * m_windowMain;
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonURL;
	GtkWidget * m_drawingareaGraphic;
	
	GR_UnixGraphics * m_gc;

	GR_UnixImage * m_pGrImageSidebar;
	XAP_UnixFrame * m_pFrame;
};

#endif /* XAP_UNIXDIALOG_ABOUT_H */
