/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_LANGUAGE_H
#define XAP_COCOADIALOG_LANGUAGE_H


#include "xap_App.h"
#include "xap_Dlg_Language.h"

#import "xap_Cocoa_NSTableUtils.h"
#import "xap_GenericListChooser_Controller.h"


class XAP_CocoaFrame;

/*****************************************************************/

class XAP_CocoaDialog_Language : public XAP_Dialog_Language
{
public:
	XAP_CocoaDialog_Language(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_CocoaDialog_Language(void);

	virtual void			runModal(XAP_Frame * pFrame);

	void					okAction(void);
	void					cancelAction(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
private:
	XAP_StringListDataSource*			m_dataSource;
	XAP_GenericListChooser_Controller*	m_dlg;
};

#endif /* XAP_COCOADIALOG_LANGUAGE_H */

