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

#ifndef AP_BEOSDIALOG_PRINT_H
#define AP_BEOSDIALOG_PRINT_H

#include "xap_BeOSFrame.h"
#include "xap_Dlg_Print.h"
class XAP_BeOSFrame;
class PS_Graphics;

/*****************************************************************/

class XAP_BeOSDialog_Print : public XAP_Dialog_Print
{
public:
	XAP_BeOSDialog_Print(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~XAP_BeOSDialog_Print(void);

	virtual void			useStart(void);
	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			useEnd(void);

	virtual GR_Graphics *	getPrinterGraphicsContext(void);
	virtual void			releasePrinterGraphicsContext(GR_Graphics *);

	static AP_Dialog *		static_constructor(AP_DialogFactory *, AP_Dialog_Id id);

protected:

	void					_raisePrintDialog(XAP_Frame * pFrame);
	void					_getGraphics(void);
	
	XAP_BeOSFrame *			m_pBeOSFrame;
	struct
	{
		// add various fields here to persist between uses of the dialog....
		UT_uint32	nFromPage;
		UT_uint32	nToPage;
		UT_uint32	nMinPage;
		UT_uint32	nMaxPage;
		UT_uint32	nCopies;
		UT_Bool		bDoPageRange;
		UT_Bool		bDoPrintSelection;
		UT_Bool		bDoPrintToFile;
		UT_Bool		bDoCollate;
		UT_Bool		bEnablePrintToFile;
		UT_Bool		bEnableSelection;
		UT_Bool		bEnablePageRange;
		char *		szPrintCommand;

	} m_persistPrintDlg;
	
};

#endif /* AP_BEOSDIALOG_PRINT_H */
