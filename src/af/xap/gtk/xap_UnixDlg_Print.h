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

#ifndef XAP_UNIXDIALOG_PRINT_H
#define XAP_UNIXDIALOG_PRINT_H

#include "xap_Dlg_Print.h"
#include <gtk/gtk.h>

class FV_View;
class FL_DocLayout;
class XAP_Frane;

/*****************************************************************/

class XAP_UnixDialog_Print : public XAP_Dialog_Print
{
public:
	XAP_UnixDialog_Print(XAP_DialogFactory * pDlgFactory,
							  XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Print(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			runModal(XAP_Frame * pFrame);

	virtual GR_Graphics *	getPrinterGraphicsContext(void);
	virtual void		releasePrinterGraphicsContext(GR_Graphics *);
	void                    setupPrint(void);
	void                    BeginPrint(GtkPrintContext * context);
	void                    PrintPage(gint iPage);
	virtual	void            setPreview(bool b);
	virtual void            PrintDirectly(XAP_Frame * pFrame, const char * szFilename, const char * szPrinter);
	void                    cleanup(void);
protected:
	GR_Graphics  *                m_pPrintGraphics;
	GR_Graphics::ColorSpace		  colorSpace;
	bool                          m_bIsPreview;
	GtkPageSetup *                m_pPageSetup;
	GtkPaperSize *                m_pGtkPageSize;
	GtkPrintOperation *           m_pPO;
	FV_View *                     m_pView;
	gint                          m_iNumberPages;
	gint                          m_iCurrentPage;
	FL_DocLayout *                m_pDL;
	FV_View *                     m_pPrintView;
	FL_DocLayout *                m_pPrintLayout;
	bool                          m_bDidQuickPrint;
	bool                          m_bShowParagraphs;
	XAP_Frame *                   m_pFrame;
};

#endif /* XAP_UNIXDIALOG_PRINT_H */
