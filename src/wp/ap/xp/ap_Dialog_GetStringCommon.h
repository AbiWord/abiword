/* AbiWord
 * Copyright (c) 2001,2002 Tomas Frydrych
 * Copyright (C) 2011 Ben Martin
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

#ifndef AP_DIALOG_GETSTRINGCOMMON_H
#define AP_DIALOG_GETSTRINGCOMMON_H

#include "xap_Dialog.h"
#include "ut_xml.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "ut_string.h"
#include "ut_string_class.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_GetStringCommon : public XAP_Dialog_NonPersistent
{
  protected:
    virtual int getStringSizeLimit() const
    {
        return 30;
    }
  public:
	AP_Dialog_GetStringCommon( XAP_DialogFactory * pDlgFactory,
                               XAP_Dialog_Id id,
                               const char* dialogfile );
	virtual ~AP_Dialog_GetStringCommon();

	virtual void		 runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK=0, a_CANCEL=1, a_DELETE=2 } tAnswer;

	tAnswer     getAnswer() const;
	void        setAnswer(tAnswer a);
	const std::string & getString() const;
	void        setString( const std::string& s );
	void        setDoc(FV_View * pView);

private:
	PD_Document* m_pDoc;
	std::string  m_string;
	AP_Dialog_GetStringCommon::tAnswer	m_answer;
};

#endif
