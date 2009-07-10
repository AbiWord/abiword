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

#ifndef AP_UNIXDIALOG_PARAGRAPH_H
#define AP_UNIXDIALOG_PARAGRAPH_H

#include "ap_Dialog_Paragraph.h"

class XAP_UnixFrame;
class GR_CairoGraphics;

/*****************************************************************/

class AP_UnixDialog_Paragraph: public AP_Dialog_Paragraph
{
public:
	AP_UnixDialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Paragraph(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void event_OK(void);
	virtual void event_Cancel(void);
	virtual void event_Tabs(void);

	// all data controls are of three types in this dialog; the static
	// functions pass in widget pointers which are mapped into
	// base class "tControl" IDs for data operations.

		// menus take a "changed" event
		virtual void event_MenuChanged(GtkWidget * widget);
	virtual void event_ComboBoxChanged(GtkWidget * widget);

		// spin buttons can take "increment", "decrement", and "changed"
		virtual void event_SpinIncrement(GtkWidget * widget);
		virtual void event_SpinDecrement(GtkWidget * widget);
		virtual void event_SpinFocusOut(GtkWidget * widget);
		virtual void event_SpinChanged(GtkWidget * widget);

		// checks are just "toggled"
		virtual void event_CheckToggled(GtkWidget * widget);

	// Preview
	virtual void event_PreviewAreaExposed(void);

 protected:

 	// Gtk sets up escape key to close a GtkDialog only when
	// the one of the button responses is GTK_RESPONSE_CANCEL
	enum
	  {
	    BUTTON_OK = GTK_RESPONSE_OK,
	    BUTTON_CANCEL = GTK_RESPONSE_CANCEL,
	    BUTTON_TABS = 0
	  } ResponseId;

	GR_CairoGraphics	* 		m_unixGraphics;
	bool					m_bEditChanged;

	// private construction functions
	virtual GtkWidget *  _constructWindow(void);
	GtkWidget *          _constructWindowContents(GtkWidget *);
	void                 _connectCallbackSignals(void);

	void                 _populateWindowData(void);

	virtual void         _syncControls(tControl changed, bool bAll = false);

	// pointers to widgets we need to query/set
	// there are a ton of them in this dialog

	GtkWidget * m_windowMain;
	GtkWidget * m_windowContents;

	GtkWidget * m_listAlignment;

	GtkWidget * m_spinbuttonLeft;
	GtkWidget * m_spinbuttonRight;
	GtkWidget * m_spinbuttonBy;

	GtkWidget * m_listSpecial;

	GtkWidget * m_spinbuttonBefore;
	GtkWidget * m_spinbuttonAfter;
	GtkWidget * m_spinbuttonAt;

	GtkWidget * m_listLineSpacing;

	GtkWidget * m_drawingareaPreview;

	GtkWidget * m_checkbuttonWidowOrphan;
	GtkWidget * m_checkbuttonKeepLines;
	GtkWidget * m_checkbuttonPageBreak;
	GtkWidget * m_checkbuttonSuppress;
	GtkWidget * m_checkbuttonHyphenate;
	GtkWidget * m_checkbuttonKeepNext;

	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;
	GtkWidget * m_buttonTabs;

	GtkWidget * m_checkbuttonDomDirection;
};

#endif /* XAP_UNIXDIALOG_PARAGRAPH_H */
