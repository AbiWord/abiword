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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_COCOADIALOG_LISTDOCUMENTS_H
#define XAP_COCOADIALOG_LISTDOCUMENTS_H

#include "xap_Dlg_ListDocuments.h"

class XAP_Frame;
@class XAP_GenericListChooser_Controller;
@class XAP_StringListDataSource;

/*****************************************************************/

class XAP_CocoaDialog_ListDocuments: public XAP_Dialog_ListDocuments
{
public:
	XAP_CocoaDialog_ListDocuments(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_ListDocuments(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	void	event_OK(void);
	void	event_Cancel(void);
protected:
private:
	void _populateWindowData(void);
	XAP_GenericListChooser_Controller*  m_dlg;
	XAP_StringListDataSource* m_dataSource;
};

#endif /* XAP_COCOADIALOG_LISTDOCUMENTS_H */
