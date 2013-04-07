/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef XAP_COCOADIALOG_PRINT_H
#define XAP_COCOADIALOG_PRINT_H

#include "xap_Dlg_Print.h"
class GR_CocoaCairoGraphics;
class AP_CocoaFrame;
class FV_View;

/*****************************************************************/

class XAP_CocoaDialog_Print : public XAP_Dialog_Print
{
public:
	XAP_CocoaDialog_Print(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_CocoaDialog_Print(void);

	virtual void			useStart(void);
	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			runPrint(XAP_Frame * pFrame, FV_View * pPrintView,
									UT_sint32 iWidth, UT_sint32 iHeight);
	virtual void			useEnd(void);

	virtual GR_Graphics * 	getPrinterGraphicsContext(void);
	virtual void			releasePrinterGraphicsContext(GR_Graphics *);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
private:
	GR_CocoaCairoGraphics*			m_pPrintGraphics;
};

#endif /* XAP_COCOADIALOG_PRINT_H */
