/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_UNIXDIALOG_ZOOM_H
#define XAP_UNIXDIALOG_ZOOM_H

#include "xap_Dlg_Zoom.h"

class XAP_Frame;

/*****************************************************************/

class XAP_UnixDialog_Zoom: public XAP_Dialog_Zoom
{
public:
	XAP_UnixDialog_Zoom(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Zoom(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

protected:
	// callbacks can fire these events

	virtual void			event_Radio200Clicked(void);
	virtual void			event_Radio100Clicked(void);
	virtual void			event_Radio75Clicked(void);
	virtual void			event_RadioPageWidthClicked(void);
	virtual void			event_RadioWholePageClicked(void);

	virtual void			event_RadioPercentClicked(void);
	virtual void			event_SpinPercentChanged(void);

	static void s_radio_200_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg) ;

	static void s_radio_100_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg) ;

	static void s_radio_75_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg) ;

	static void s_radio_PageWidth_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg) ;

	static void s_radio_WholePage_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg) ;

	static void s_radio_Percent_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg) ;

	static void s_spin_Percent_changed(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg) ;

	// private construction functions
	GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	void		_enablePercentSpin(bool enable);
	void 		_storeWindowData(void);

	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;

	GtkWidget * m_radio200;
	GtkWidget * m_radio100;
	GtkWidget * m_radio75;
	GtkWidget * m_radioPageWidth;
	GtkWidget * m_radioWholePage;
	GtkWidget * m_radioPercent;
	GtkWidget * m_spinPercent;
	GtkAdjustment * m_spinAdj;

	// our "group" of radio buttons
	GSList *	m_radioGroup;
};

#endif /* XAP_UNIXDIALOG_ZOOM_H */
