/* AbiWord
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

#ifndef AP_UNIXDIALOG_FORMATTABLE_H
#define AP_UNIXDIALOG_FORMATTABLE_H

#include "ap_Dialog_FormatTable.h"
#include "gr_UnixGraphics.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_FormatTable: public AP_Dialog_FormatTable
{
public:
	AP_UnixDialog_FormatTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_FormatTable(void);

	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void			event_Close(void);
	void					event_previewExposed(void);
	virtual void            setSensitivity(bool bsens);
	virtual void            destroy(void);
	virtual void            activate(void);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
protected:
	typedef enum
	{
		BUTTON_APPLY,
	    BUTTON_CLOSE
	} ResponseId ;
		
	virtual GtkWidget *		_constructWindow(void);
	virtual GtkWidget *		_constructWindowContents(void);
	void					_populateWindowData(void);
	void					_storeWindowData(void);
	void                     _connectSignals(void);
	
	GR_UnixGraphics	* 		m_pPreviewWidget;	
	
	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;
	GtkWidget * m_wContents;
	GtkWidget * m_wApplyButton;
	GtkWidget * m_wCloseButton;

	GtkWidget * m_wBorderColorButton;
	GtkWidget * m_wLineLeft;
	GtkWidget * m_wLineRight;
	GtkWidget * m_wLineTop;
	GtkWidget * m_wLineBottom;
	
	GtkWidget * m_wpreviewArea;
};

#endif /* AP_UNIXDIALOG_FORMATTABLE_H */
