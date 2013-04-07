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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_UNIXDIALOG_SPLITCELLS_H
#define AP_UNIXDIALOG_SPLITCELLS_H

#include "ap_Dialog_SplitCells.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_SplitCells: public AP_Dialog_SplitCells
{
public:
	AP_UnixDialog_SplitCells(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_SplitCells(void);

	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void			event_Close(void);
	virtual void            setSensitivity(AP_CellSplitType splitThis, bool bsens);
	virtual void            destroy(void);
	virtual void            activate(void);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
protected:
	typedef enum
	{
	    BUTTON_CLOSE = GTK_RESPONSE_CLOSE
	} ResponseId ;


	virtual GtkWidget *		_constructWindow(void);
	virtual GtkWidget *		_constructWindowContents(void);
	void					_populateWindowData(void);
	void					_storeWindowData(void);
	void                     _connectSignals(void);
	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;
	GtkWidget * m_wContents;

	GtkWidget * m_wSplitLeft;
	GtkWidget * m_wSplitHoriMid;
	GtkWidget * m_wSplitRight;
	GtkWidget * m_wSplitAbove;
	GtkWidget * m_wSplitVertMid;
	GtkWidget * m_wSplitBelow;
	GtkWidget * m_lwSplitLeft;
	GtkWidget * m_lwSplitHoriMid;
	GtkWidget * m_lwSplitRight;
	GtkWidget * m_lwSplitAbove;
	GtkWidget * m_lwSplitVertMid;
	GtkWidget * m_lwSplitBelow;

};

#endif /* AP_UNIXDIALOG_SPLITCELLS_H */
