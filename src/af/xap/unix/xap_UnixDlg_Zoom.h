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

#ifndef XAP_UNIXDIALOG_ZOOM_H
#define XAP_UNIXDIALOG_ZOOM_H

#include "xap_Dlg_Zoom.h"
#include "xap_UnixFontManager.h"

class XAP_UnixFrame;

/*****************************************************************/

class XAP_UnixDialog_Zoom: public XAP_Dialog_Zoom
{
public:
	XAP_UnixDialog_Zoom(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Zoom(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_Cancel(void);
	virtual void			event_WindowDelete(void);

	virtual void			event_Radio200Clicked(void);
	virtual void			event_Radio100Clicked(void);
	virtual void			event_Radio75Clicked(void);
	virtual void			event_RadioPageWidthClicked(void);
	virtual void			event_RadioWholePageClicked(void);

	virtual void			event_RadioPercentClicked(void);
	virtual void			event_SpinPercentChanged(void);

	virtual void			event_PreviewAreaExposed(void);
	
protected:

	GR_UnixGraphics	* 		m_unixGraphics;
	
	// private construction functions
	GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	void		_enablePercentSpin(UT_Bool enable);
	void 		_storeWindowData(void);

	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;

	GtkWidget * m_previewFrame;
	GtkWidget * m_previewArea;
	
	GtkWidget * m_radio200;
	GtkWidget * m_radio100;
	GtkWidget * m_radio75;
	GtkWidget * m_radioPageWidth;
	GtkWidget * m_radioWholePage;
	GtkWidget * m_radioPercent;
	GtkWidget * m_spinPercent;

	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;

	// our "group" of radio buttons
	GSList *	m_radioGroup;
};

#endif /* XAP_UNIXDIALOG_ZOOM_H */
