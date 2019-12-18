/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#ifndef AP_UNIXDIALOG_STYLIST_H
#define AP_UNIXDIALOG_STYLIST_H

#include "xap_UnixDialog.h"
#include "ap_Dialog_Stylist.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Stylist
  : public AP_Dialog_Stylist
  , public XAP_UnixDialog
{
public:
	AP_UnixDialog_Stylist(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Stylist(void);

	virtual void runModeless(XAP_Frame * pFrame) override;
	virtual void runModal(XAP_Frame * pFrame) override;

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	void			event_Close(void);
	void            event_Apply(void);
	void            styleClicked(UT_sint32 row, UT_sint32 col);
	virtual void destroy(void) override;
	virtual void activate(void) override;
	virtual void notifyActiveFrame(XAP_Frame * pFrame) override;
	virtual void setStyleInGUI(void) override;
private:
	GtkWidget *		_constructWindow(void);
	void			_populateWindowData(void);
	void            _connectSignals(void);
	void            _fillTree(void);

	GtkWidget * m_wStyleList;
	GtkCellRenderer * m_wRenderer;
	GtkTreeStore * m_wModel;
	GtkWidget * m_wStyleListContainer;
};

#endif /* AP_UNIXDIALOG_STYLIST_H */
