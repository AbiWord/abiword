/* AbiSource Application Framework
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

#ifndef XAP_UNIXDIALOG_FILEOPENSAVEAS_H
#define XAP_UNIXDIALOG_FILEOPENSAVEAS_H

#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Strings.h"
class XAP_UnixFrame;

/*****************************************************************/

class XAP_UnixDialog_FileOpenSaveAs : public XAP_Dialog_FileOpenSaveAs
{
public:
	XAP_UnixDialog_FileOpenSaveAs(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~XAP_UnixDialog_FileOpenSaveAs(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static AP_Dialog *		static_constructor(AP_DialogFactory *, AP_Dialog_Id id);

protected:

	UT_Bool					_run_gtk_main(XAP_Frame * pFrame, void * pFSvoid, UT_Bool bCheckWritePermission);
	void 					_notifyError_OKOnly(XAP_Frame * pFrame, XAP_String_Id sid);
	void 					_notifyError_OKOnly(XAP_Frame * pFrame, XAP_String_Id sid, const char * sz1);
	UT_Bool 				_askOverwrite_YesNo(XAP_Frame * pFrame, const char * fileName);
	
	XAP_UnixFrame *			m_pUnixFrame;
};

#endif /* XAP_UNIXDIALOG_FILEOPENSAVEAS_H */
