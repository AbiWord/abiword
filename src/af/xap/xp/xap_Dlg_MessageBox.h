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

#ifndef XAP_DIALOG_MESSAGEBOX_H
#define XAP_DIALOG_MESSAGEBOX_H

#include "xap_Dialog.h"

class XAP_Dialog_MessageBox : public XAP_Dialog_NonPersistent
{
public:
	XAP_Dialog_MessageBox(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_MessageBox(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { b_O, b_OC, b_YN, b_YNC }			tButtons;
	typedef enum { a_OK, a_CANCEL, a_YES, a_NO }	tAnswer;
	
	void							setMessage(const char * szMessage);
	void							setMessage(const char * szMessage, const char * sz1);
	void                                                    setMessage(const char * szMessage, const char * sz1, const char * sz2, int num); // Used for Dialog not implemented 
	void							setButtons(XAP_Dialog_MessageBox::tButtons buttons);
	void							setDefaultAnswer(XAP_Dialog_MessageBox::tAnswer answer);
	XAP_Dialog_MessageBox::tAnswer	getAnswer(void) const;

protected:
	char *							m_szMessage;
	XAP_Dialog_MessageBox::tButtons	m_buttons;
	XAP_Dialog_MessageBox::tAnswer	m_defaultAnswer;
	XAP_Dialog_MessageBox::tAnswer	m_answer;
};

#endif /* XAP_DIALOG_MESSAGEBOX_H */
