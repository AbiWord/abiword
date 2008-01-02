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

#ifndef XAP_UNIXDIALOG_PRINT_H
#define XAP_UNIXDIALOG_PRINT_H

#include "xap_Dlg_Print.h"
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>

class XAP_UnixGnomePrintGraphics;

/*****************************************************************/

class XAP_UnixDialog_Print : public XAP_Dialog_Print
{
public:
	XAP_UnixDialog_Print(XAP_DialogFactory * pDlgFactory, 
							  XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Print(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			useStart(void);
	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			useEnd(void);

	virtual GR_Graphics *	getPrinterGraphicsContext(void);
	virtual void			releasePrinterGraphicsContext(GR_Graphics *);

protected:
	virtual void			_raisePrintDialog(XAP_Frame * pFrame);
	virtual void            _getGraphics(void);

	GR_Graphics  *                m_pPrintGraphics;
	GR_Graphics::ColorSpace		  colorSpace;
	GnomePrintJob                *m_gpm;
	bool                          m_bIsPreview;
	bool                          m_bPdfWorkAround;
};

#endif /* XAP_UNIXDIALOG_PRINT_H */
