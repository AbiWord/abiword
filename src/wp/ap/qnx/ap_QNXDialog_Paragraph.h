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

#ifndef AP_QNXDIALOG_PARAGRAPH_H
#define AP_QNXDIALOG_PARAGRAPH_H

#include "ap_Dialog_Paragraph.h"
#include <Pt.h>

class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_Paragraph: public AP_Dialog_Paragraph
{
public:
	AP_QNXDialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_Paragraph(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void event_OK(void);
	virtual void event_Cancel(void);
	virtual void event_Tabs(PtWidget_t *widget, PtCallbackInfo_t *info);
	virtual void event_WindowDelete(void);

	// all data controls are of three types in this dialog; the static
	// functions pass in widget pointers which are mapped into
	// base class "tControl" IDs for data operations.

		// menus take a "changed" event
		virtual void event_MenuChanged(PtWidget_t * widget, int value);

		// spin buttons can take "increment", "decrement", and "changed"
		virtual void event_SpinIncrement(PtWidget_t * widget);
		virtual void event_SpinDecrement(PtWidget_t * widget);
		virtual void event_SpinFocusOut(PtWidget_t * widget);
		virtual void event_SpinChanged(PtWidget_t * widget);
		
		// checks are just "toggled"
		virtual void event_CheckToggled(PtWidget_t * widget);
	
	// Preview
	virtual void event_PreviewAreaExposed(void);

 protected:

	GR_QNXGraphics	* 		m_unixGraphics;
	bool					m_bEditChanged;
	
	// private construction functions
	PtWidget_t * _constructWindow(void);
	void 		_connectCallbackSignals(void);
	
	void		_populateWindowData(void);

	virtual void	_syncControls(tControl changed, bool bAll = false);

	// pointers to widgets we need to query/set
	// there are a ton of them in this dialog
	
	PtWidget_t * m_windowMain;

	PtWidget_t * m_listAlignment;
	PtWidget_t * m_menuitemLeft;
	PtWidget_t * m_menuitemCentered;
	PtWidget_t * m_menuitemRight;
	PtWidget_t * m_menuitemJustified;

	PtWidget_t * m_spinbuttonLeft;
	PtWidget_t * m_spinbuttonRight;
	PtWidget_t * m_spinbuttonBy;

	PtWidget_t * m_listSpecial;
	PtWidget_t * m_listSpecial_menu;
	PtWidget_t * m_menuitemNone;
	PtWidget_t * m_menuitemFirstLine;
	PtWidget_t * m_menuitemHanging;	
	
	PtWidget_t * m_spinbuttonBefore;
	PtWidget_t * m_spinbuttonAfter;
	PtWidget_t * m_spinbuttonAt;

	PtWidget_t * m_listLineSpacing;
	PtWidget_t * m_listLineSpacing_menu;
	PtWidget_t * m_menuitemSingle;
	PtWidget_t * m_menuitemOneAndHalf;
	PtWidget_t * m_menuitemDouble;
	PtWidget_t * m_menuitemAtLeast;
	PtWidget_t * m_menuitemExactly;
	PtWidget_t * m_menuitemMultiple;
	PtWidget_t * m_drawingareaPreview;
	PtWidget_t * m_drawingareaPreviewGroup;

	PtWidget_t * m_checkbuttonWidowOrphan;
	PtWidget_t * m_checkbuttonKeepLines;
	PtWidget_t * m_checkbuttonPageBreak;
	PtWidget_t * m_checkbuttonSuppress;
	PtWidget_t * m_checkbuttonHyphenate;
	PtWidget_t * m_checkbuttonKeepNext;

	PtWidget_t * m_buttonOK;
	PtWidget_t * m_buttonCancel;
	PtWidget_t * m_buttonTabs;

	GR_QNXGraphics * m_qnxGraphics;

	int 		 done;
};

#endif /* XAP_QNXDIALOG_PARAGRAPH_H */
