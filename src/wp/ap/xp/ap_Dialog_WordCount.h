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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_DIALOG_WORDCOUNT_H
#define AP_DIALOG_WORDCOUNT_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

#include "fv_View.h"


class XAP_Frame;

class ABI_EXPORT AP_Dialog_WordCount : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_WordCount(void);

	typedef enum { a_OK, a_CANCEL } tAnswer;

	AP_Dialog_WordCount::tAnswer		getAnswer(void) const;
	FV_DocCount							getCount(void) const;
	void								setCount(FV_DocCount);
	void								setCountFromActiveFrame(void);
        void                                    ConstructWindowName(void);

        void                                    setActiveFrame(XAP_Frame *pFrame);
	// must be public for Cocoa, because I can't make an Obj-C class
	// be friend of a C++ class. Not a big deal.
	enum {
		DIALOG_WID,
		CLOSE_BTN_WID,
		TITLE_LBL_WID,
		PAGES_LBL_WID,
		PAGES_VAL_WID,
		LINES_LBL_WID,
		LINES_VAL_WID,
		CHARSP_LBL_WID,
		CHARSP_VAL_WID,
		CHARNSP_LBL_WID,
		CHARNSP_VAL_WID,
		PARA_LBL_WID,
		PARA_VAL_WID,
		WORDS_LBL_WID,
		WORDS_VAL_WID,
		WORDSNF_LBL_WID,
		WORDSNF_VAL_WID
	};


protected:

	virtual void localizeDialog(void);
	virtual void updateDialogData(void);

	AP_Dialog_WordCount::tAnswer		m_answer;
	FV_DocCount							m_count;
	char m_WindowName[100];
};

#endif /* AP_DIALOG_WORDCOUNT_H */


