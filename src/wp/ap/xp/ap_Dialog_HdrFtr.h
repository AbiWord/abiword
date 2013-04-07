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

#ifndef AP_DIALOG_HDRFTR_H
#define AP_DIALOG_HDRFTR_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_misc.h" // for UT_RGBColor

class XAP_Frame;
class AP_Dialog_HdrFtr;

class ABI_EXPORT AP_Dialog_HdrFtr : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_HdrFtr(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_HdrFtr(void);

	virtual void			     runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;
	typedef enum
	{
		HdrEven,
		HdrFirst,
		HdrLast,
		FtrEven,
		FtrFirst,
		FtrLast
	} HdrFtr_Control;

	AP_Dialog_HdrFtr::tAnswer		getAnswer(void) const;
	void    setValue(AP_Dialog_HdrFtr::HdrFtr_Control which, bool value, bool changed);
	bool        getValue(AP_Dialog_HdrFtr::HdrFtr_Control which);
	bool        isChanged(AP_Dialog_HdrFtr::HdrFtr_Control which);
    bool        isRestartChanged(void) const;
	bool        isRestart(void) const;
	UT_sint32   getRestartValue(void) const;
	void        setRestart( bool bRestart, UT_sint32 RestartValue, bool bRestartChanged);
    void        setAnswer (AP_Dialog_HdrFtr::tAnswer answer);

 protected:

 private:
	bool m_bHdrFtrValues[6];
	bool m_bHdrFtrChanged[6];
	bool m_bDoRestart;
	bool m_bRestartChanged;
	UT_sint32 m_iStartAt;
    tAnswer m_answer;
};

#endif /* AP_DIALOG_HDRFTR_H */









