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

#ifndef AP_UNIXDIALOG_PRINT_H
#define AP_UNIXDIALOG_PRINT_H

#include "ap_Dialog_Print.h"
class AP_UnixFrame;
class PS_Graphics;

/*****************************************************************/

class AP_UnixDialog_Print : public AP_Dialog_Print
{
public:
	AP_UnixDialog_Print(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_UnixDialog_Print(void);

	virtual void			useStart(void);
	virtual void			runModal(AP_Frame * pFrame);
	virtual void			useEnd(void);

	virtual DG_Graphics *	getPrinterGraphicsContext(void);
	virtual void			releasePrinterGraphicsContext(DG_Graphics *);

	static AP_Dialog *		static_constructor(AP_DialogFactory *, AP_Dialog_Id id);

protected:
	void					_raisePrintDialog(void);
	void					_getGraphics(void);
	
	AP_UnixFrame *			m_pUnixFrame;
	PS_Graphics *			m_pPSGraphics;
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

	} m_persistPrintDlg;
	
};

#endif /* AP_UNIXDIALOG_PRINT_H */
