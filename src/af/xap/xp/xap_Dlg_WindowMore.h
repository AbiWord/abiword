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

#ifndef XAP_DIALOG_WINDOWMORE_H
#define XAP_DIALOG_WINDOWMORE_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
class XAP_App;

class XAP_Dialog_WindowMore : public XAP_Dialog_NonPersistent
{
public:
	XAP_Dialog_WindowMore(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_WindowMore(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL }	tAnswer;

	XAP_Dialog_WindowMore::tAnswer	getAnswer(void) const;
    XAP_Frame * 					getSelFrame(void) const;
	
protected:
	XAP_Dialog_WindowMore::tAnswer	m_answer;

	UT_sint32						m_ndxSelFrame;
};

#endif /* XAP_DIALOG_WINDOWMORE_H */
