/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz
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

#ifndef AP_DIALOG_METADATA_H
#define AP_DIALOG_METADATA_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

class XAP_Frame;

#define DCL_PROP(name) \
public: \
void set##name(const UT_UTF8String & val) { m_##name = val ; } \
UT_UTF8String get##name() const { return m_##name ; } \
private: UT_UTF8String m_##name ;

class ABI_EXPORT AP_Dialog_MetaData : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_MetaData(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid)
	  : XAP_Dialog_NonPersistent ( pDlgFactory, dlgid, "interface/dialogproperties" ), m_answer ( a_CANCEL )
	  {
	  }

	virtual ~AP_Dialog_MetaData()
	  {
	  }

	virtual void	runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;

	AP_Dialog_MetaData::tAnswer		getAnswer(void) const
	  {
	    return m_answer ;
	  }
	
	DCL_PROP(Title)
	DCL_PROP(Subject)
	DCL_PROP(Author)
	DCL_PROP(Publisher)
	DCL_PROP(CoAuthor)
	DCL_PROP(Category)
	DCL_PROP(Keywords)
	DCL_PROP(Languages)
	DCL_PROP(Description)
	DCL_PROP(Source)
	DCL_PROP(Relation)
	DCL_PROP(Coverage)
	DCL_PROP(Rights)

 protected:
	
	void setAnswer ( AP_Dialog_MetaData::tAnswer a )
	  {
	    m_answer = a ;
	  }

 private:

	AP_Dialog_MetaData::tAnswer		m_answer;
};

#undef DCL_PROP

#endif /* AP_DIALOG_STUB_H */
