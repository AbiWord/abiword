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

#ifndef AP_DIALOG_TOGGLECASE_H
#define AP_DIALOG_TOGGLECASE_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

#include "fv_View.h" // for ToggleCase enum

class XAP_Frame;

class AP_Dialog_ToggleCase : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_ToggleCase(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_ToggleCase(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;

	inline AP_Dialog_ToggleCase::tAnswer	getAnswer(void) const {return m_answer;};
	inline void                     setAnswer(tAnswer a) {m_answer = a;}
	inline void                     setCase (ToggleCase c) {m_case = c;}
	inline ToggleCase               getCase (void) const {return m_case;}

protected:
	
	AP_Dialog_ToggleCase::tAnswer	m_answer;
	ToggleCase                      m_case;
};

#endif /* AP_DIALOG_TOGGLECASE_H */
