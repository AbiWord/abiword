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

#ifndef AP_DIALOG_WORDCOUNT_H
#define AP_DIALOG_WORDCOUNT_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

#include "fv_View.h"


class XAP_Frame;

class AP_Dialog_WordCount : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_WordCount(void);

	typedef enum { a_OK, a_CANCEL } tAnswer;

	AP_Dialog_WordCount::tAnswer		getAnswer(void) const;
	FV_DocCount							getCount(void) const;
	void								setCount(FV_DocCount);
	void								setCountFromActiveFrame(void);

protected:
	AP_Dialog_WordCount::tAnswer		m_answer;
	FV_DocCount							m_count;
};

#endif /* AP_DIALOG_WORDCOUNT_H */
