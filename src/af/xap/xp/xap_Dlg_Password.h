/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef XAP_DIALOG_PASSWORD_H
#define XAP_DIALOG_PASSWORD_H

#include "ut_string_class.h"

/* #include "xap_Frame.h" */
#include "xap_Dialog.h"
/* #include "xav_View.h" */

class XAP_Frame;

class ABI_EXPORT XAP_Dialog_Password : public XAP_Dialog_NonPersistent
{
public:

  typedef enum { a_OK, a_Cancel } tAnswer;

	XAP_Dialog_Password(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Password(void);

	virtual void  runModal(XAP_Frame * pFrame) = 0;

	UT_UTF8String getPassword () const { return m_passwd; }

	tAnswer getAnswer () const { return m_answer; }

 protected:
	void setPassword (const char *);
	void setPassword (const UT_UTF8String & pass);
	void setAnswer ( tAnswer ans ) { m_answer = ans; }

 private:
	UT_UTF8String m_passwd;
	tAnswer m_answer;
};

#endif /* XAP_DIALOG_PASSWORD_H */
