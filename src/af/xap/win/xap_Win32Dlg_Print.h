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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_WIN32DIALOG_PRINT_H
#define XAP_WIN32DIALOG_PRINT_H

#include "xap_Dlg_Print.h"
#include "xap_Frame.h"
#include "ut_Win32LocaleString.h"

#include <commdlg.h>
/*****************************************************************/

class ABI_EXPORT XAP_Win32Dialog_Print : public XAP_Dialog_Print
{
public:
	XAP_Win32Dialog_Print(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Win32Dialog_Print(void);

	virtual void			runModal(XAP_Frame * pFrame);
	virtual GR_Graphics *		getPrinterGraphicsContext(void);
	virtual void			releasePrinterGraphicsContext(GR_Graphics *);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	PRINTDLGW *              getPrintDlg() const {return m_pPersistPrintDlg;}

	void                    setOrigPrinter(UT_uint32 i) {m_iOrigPrinter = i;}
	void                    setNewPrinter(UT_uint32 i) {m_iNewPrinter = i;}

	UT_uint32               getOrigPrinter() const {return m_iOrigPrinter;}
	UT_uint32               getNewPrinter() const {return m_iNewPrinter;}

	void                    setClosed(bool b){m_bClosed = b;}

protected:
	void					_extractResults(XAP_Frame *pFrame);

	PRINTDLGW *				m_pPersistPrintDlg;
	DOCINFOW				    m_DocInfo;
	UT_uint32               m_iOrigPrinter;
	UT_uint32               m_iNewPrinter;
	bool                    m_bClosed;
    UT_Win32LocaleString	m_docName;
	UT_Win32LocaleString	m_fileName;
};

#endif /* XAP_WIN32DIALOG_PRINT_H */
