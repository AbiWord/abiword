/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef XAP_QNXDIALOG_LANGUAGE_H
#define XAP_QNXDIALOG_LANGUAGE_H

#include "xap_Dlg_Language.h"
#include <Pt.h>

class XAP_QNXFrame;

/*****************************************************************/

class XAP_QNXDialog_Language: public XAP_Dialog_Language
{
public:
	XAP_QNXDialog_Language(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_QNXDialog_Language(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
	virtual void			setAnswer(XAP_Dialog_Language::tAnswer ans);

protected:
	virtual PtWidget_t *    constructWindow(void);

	PtWidget_t 				*m_pLanguageList;
	int						 done;
};

#endif /* XAP_QNXDIALOG_LANGUAGE_H */
