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

#ifndef XAP_QNXDIALOG_PRINT_H
#define XAP_QNXDIALOG_PRINT_H

#include "xap_QNXFrame.h"
#include "xap_Dlg_Print.h"
#include <Pt.h>

class XAP_QNXFrame;

/*****************************************************************/

class XAP_QNXDialog_Print : public XAP_Dialog_Print
{
public:
	XAP_QNXDialog_Print(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_QNXDialog_Print(void);

	virtual void			useStart(void);
	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			useEnd(void);

	virtual GR_Graphics *	getPrinterGraphicsContext(void);
	virtual void			releasePrinterGraphicsContext(GR_Graphics *);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

protected:

	XAP_QNXFrame *			m_pQNXFrame;
	PpPrintContext_t  		*m_pPrintContext;

	void					_raisePrintDialog(XAP_Frame * pFrame);

	struct {
		// add various fields here to persist between uses of the dialog....
		UT_uint32	nFromPage;
		UT_uint32	nToPage;
		UT_uint32	nMinPage;
		UT_uint32	nMaxPage;
		UT_uint32	nCopies;
		bool		bDoPageRange;
		bool		bDoPrintSelection;
		bool		bDoPrintToFile;
		bool		bDoCollate;
		bool		bEnablePrintToFile;
		bool		bEnableSelection;
		bool		bEnablePageRange;

	} m_persistPrintDlg;
};

#endif /* XAP_QNXDIALOG_PRINT_H */
