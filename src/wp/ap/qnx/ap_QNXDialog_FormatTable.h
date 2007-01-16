/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_QNXDIALOG_FORMATTABLE_H
#define AP_QNXDIALOG_FORMATTABLE_H

#include "ap_Dialog_FormatTable.h"
#include "gr_QNXGraphics.h"

#include <Pt.h>

class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_FormatTable: public AP_Dialog_FormatTable
{
public:
	AP_QNXDialog_FormatTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_FormatTable(void);

	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			runModeless(XAP_Frame *pFrame);
	virtual void			activate();
	virtual void			destroy();
	virtual void			setSensitivity(bool onoff);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	void				lineClicked();
	void				event_WindowDelete();
	void				event_previewExposed();
	virtual void                setBorderThicknessInGUI(UT_UTF8String & sThick) { UT_ASSERT(UT_NOT_IMPLEMENTED); };
		
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
	
private:
	PtWidget_t *	_constructWindow();
	PtWidget_t *	m_wBorderColorButton;
	PtWidget_t *	m_wBackgroundColorButton;
	PtWidget_t *	m_wApplyButton;
	PtWidget_t *	m_wLineLeft,*m_wLineRight,*m_wLineTop,*m_wLineBottom;
	PtWidget_t *	m_wPreviewArea;
	PtWidget_t *	m_mainWindow;
	GR_QNXGraphics *	m_pPreviewWidget;

};

#endif /* AP_QNXDIALOG_FORMATTABLE_H */
