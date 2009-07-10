/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#ifndef AP_DIALOG_MAILMERGE_H
#define AP_DIALOG_MAILMERGE_H

#include "xap_Dialog.h"
#include "pd_Document.h"
#include "xap_Strings.h"
#include "ut_vector.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_MailMerge : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_MailMerge(XAP_DialogFactory * pDlgFactory,
						XAP_Dialog_Id id);
	virtual ~AP_Dialog_MailMerge(void);

	const UT_UTF8String& getMergeField() const;

	void eventOpen ();

	virtual void setActiveFrame(XAP_Frame *pFrame) { m_pFrame = pFrame; }

	virtual void			destroy(void) {}
	virtual void			activate(void) {}

protected:

	void init ();

	void          setMergeField(const UT_UTF8String & name);

	virtual void setFieldList();

	void addClicked();

	XAP_Frame * m_pFrame;
	UT_Vector m_vecFields;

private:
	UT_UTF8String m_mergeField;
};

#endif /* AP_DIALOG_MARKREVISIONS_H */
