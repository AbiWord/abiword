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

#ifndef XAP_QNXDIALOG_ZOOM_H
#define XAP_QNXDIALOG_ZOOM_H

#include <Pt.h>
#include "xap_Dlg_Zoom.h"

class XAP_QNXFrame;

/*****************************************************************/

class XAP_QNXDialog_Zoom: public XAP_Dialog_Zoom
{
public:
	XAP_QNXDialog_Zoom(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_QNXDialog_Zoom(void);

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

	GR_QNXGraphics	* 		m_qnxGraphics;
	
	// private construction functions
	PtWidget_t * _constructWindow(void);
	void		_populateWindowData(void);
	void		_enablePercentSpin(bool enable);
	void 		_storeWindowData(void);

	// pointers to widgets we need to query/set
	PtWidget_t * m_windowMain;

	PtWidget_t * m_previewFrame;
	PtWidget_t * m_previewArea;
	
	PtWidget_t * m_radio200;
	PtWidget_t * m_radio100;
	PtWidget_t * m_radio75;
	PtWidget_t * m_radioPageWidth;
	PtWidget_t * m_radioWholePage;
	PtWidget_t * m_radioPercent;
	PtWidget_t * m_spinPercent;

	PtWidget_t * m_buttonOK;
	PtWidget_t * m_buttonCancel;

	// our "group" of radio buttons
	PtWidget_t * m_radioGroup;
	int 		 done;
};

#endif /* XAP_QNXDIALOG_ZOOM_H */
