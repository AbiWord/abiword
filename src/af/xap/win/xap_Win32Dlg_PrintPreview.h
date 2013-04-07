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

#ifndef XAP_Win32DIALOG_PRINTPREVIEW_H
#define XAP_Win32DIALOG_PRINTPREVIEW_H

#include "xap_Dlg_PrintPreview.h"
#include "gr_Graphics.h"

/*****************************************************************/

class ABI_EXPORT XAP_Win32Dialog_PrintPreview : public XAP_Dialog_PrintPreview
{
 public:
	XAP_Win32Dialog_PrintPreview(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Win32Dialog_PrintPreview(void);

	virtual GR_Graphics *				getPrinterGraphicsContext(void);
	virtual void					releasePrinterGraphicsContext(GR_Graphics * pGraphics);
	virtual void					runModal(XAP_Frame * pFrame);

	static XAP_Dialog *static_constructor(XAP_DialogFactory * pFactory,
					      XAP_Dialog_Id id);

 protected:

	GR_Graphics * m_pPrintGraphics;
	gunichar2 * m_emfFilename;
};

#endif /* XAP_Win32DIALOG_PRINTPREVIEW_H */
