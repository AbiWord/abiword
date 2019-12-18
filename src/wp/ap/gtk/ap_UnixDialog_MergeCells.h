/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2019 Hubert Figuière
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

#ifndef AP_UNIXDIALOG_MERGECELLS_H
#define AP_UNIXDIALOG_MERGECELLS_H

#include "xap_UnixDialog.h"
#include "ap_Dialog_MergeCells.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_MergeCells
  : public AP_Dialog_MergeCells
  , public XAP_UnixDialog
{
public:
	AP_UnixDialog_MergeCells(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_MergeCells(void);

	virtual void runModeless(XAP_Frame * pFrame) override;

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void			event_Close(void);
	virtual void setSensitivity(AP_Dialog_MergeCells::mergeWithCell mergeThis, bool bsens) override;
	virtual void destroy(void) override;
	virtual void activate(void) override;
	virtual void notifyActiveFrame(XAP_Frame * pFrame) override;
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
	GtkWidget * m_wContents;

	GtkWidget * m_wMergeLeft;
	GtkWidget * m_wMergeRight;
	GtkWidget * m_wMergeAbove;
	GtkWidget * m_wMergeBelow;
	GtkWidget * m_lwMergeLeft;
	GtkWidget * m_lwMergeRight;
	GtkWidget * m_lwMergeAbove;
	GtkWidget * m_lwMergeBelow;

};

#endif /* AP_UNIXDIALOG_MERGECELLS_H */
