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

#ifndef XAP_DIALOG_PRINTPREVIEW_H
#define XAP_DIALOG_PRINTPREVIEW_H

#include "xap_Dialog.h"
#include "gr_Graphics.h"

/*****************************************************************/

class XAP_Dialog_PrintPreview : public XAP_Dialog_NonPersistent
{
 public:
	XAP_Dialog_PrintPreview(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_PrintPreview(void);

	void						setDocumentTitle(const char *);
	void						setDocumentPathname(const char * );
	virtual GR_Graphics *			        getPrinterGraphicsContext(void) = 0;
	virtual void					releasePrinterGraphicsContext(GR_Graphics * pGraphics) = 0;
	virtual void					runModal(XAP_Frame * pFrame) = 0;

 protected:
	char *					        m_szDocumentTitle;		/* input */
	char *						m_szDocumentPathname;	/* input */
};

#endif /* XAP_DIALOG_PRINTPREVIEW_H */
