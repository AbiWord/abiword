/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_ENCODING_H
#define XAP_COCOADIALOG_ENCODING_H


#include "ut_types.h"
#include "ut_xml.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Dialog.h"
#include "ut_Encoding.h"
#include "xap_Dlg_Encoding.h"
#import "xap_Cocoa_NSTableUtils.h"
#import "xap_GenericListChooser_Controller.h"

/********************************************************************
INSTRUCTIONS FOR DESIGN OF THE PLATFORM VERSIONS OF THIS DIALOGUE

(1)	implement runModal(); at the moment we display a single listbox

(2)	m_iEncCount will tell you how many list entries there will be;
	the encoding strings are then in m_ppEncodings (already sorted)

(3)	use _setEncoding() to set the member variables in response
	to the user selection when the dialog is closing.
*********************************************************************/



class XAP_CocoaDialog_Encoding : public XAP_Dialog_Encoding
{
public:
	XAP_CocoaDialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_Encoding(void);

	virtual void					runModal(XAP_Frame * pFrame);
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_Cancel(void);

private:
	void		_populateWindowData(void);
	XAP_StringListDataSource*	m_dataSource;
	XAP_GenericListChooser_Controller*	m_dlg;
};
#endif /* XAP_COCOADIALOG_ENCODING_H */





