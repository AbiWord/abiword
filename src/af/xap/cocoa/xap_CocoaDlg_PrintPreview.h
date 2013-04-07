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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_CocoaDIALOG_PRINTPREVIEW_H
#define XAP_CocoaDIALOG_PRINTPREVIEW_H

#include "xap_Dlg_PrintPreview.h"
#include "gr_Graphics.h"

/*****************************************************************/

class XAP_CocoaDialog_PrintPreview : public XAP_Dialog_PrintPreview
{
 public:
	XAP_CocoaDialog_PrintPreview(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_CocoaDialog_PrintPreview(void);

	virtual GR_Graphics *			        getPrinterGraphicsContext(void) = 0;
	virtual void					releasePrinterGraphicsContext(GR_Graphics * pGraphics) = 0;
	virtual void					runModal(XAP_Frame * pFrame) = 0;

	virtual XAP_Dialog * static_constructor(XAP_DialogFactory * pFactory,
						XAP_Dialog_Id id) = 0;

 protected:
};

#endif /* XAP_CocoaDIALOG_PRINTPREVIEW_H */
