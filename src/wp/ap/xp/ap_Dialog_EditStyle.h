/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2008 Ryan Pavlik <abiryan@ryand.net>
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

#ifndef AP_DIALOG_EDITSTYLE_H
#define AP_DIALOG_EDITSTYLE_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

#include "pd_Style.h"
#include "ut_string.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_EditStyle : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_EditStyle(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_EditStyle(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;
	void							setStyleToEdit(UT_UTF8String sName, PD_Style * pStyle);

	typedef enum { a_OK, a_CANCEL } tAnswer;

	AP_Dialog_EditStyle::tAnswer	getAnswer(void) const;
	
protected:
	
	// data we need
	UT_UTF8String m_sName;
	PD_Style * m_pStyle;
	AP_Dialog_EditStyle::tAnswer		m_answer;

};

#endif /* AP_DIALOG_EDITSTYLE_H */
