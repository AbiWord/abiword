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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_DIALOG_NEW_H
#define AP_DIALOG_NEW_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "xap_Strings.h"

class XAP_Frame;

class AP_Dialog_New : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_New(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_New(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;
	typedef enum { open_New, open_Template, open_Existing} tOpenType;

	inline AP_Dialog_New::tAnswer	getAnswer(void) const {return m_answer;}
	inline AP_Dialog_New::tOpenType getOpenType(void) const {return m_openType;}

	inline const char * getTemplateName (void) const {return m_templateName;}
	inline const char * getFileName (void) const {return m_fileName;}

protected:
	inline void setAnswer (AP_Dialog_New::tAnswer a) {m_answer = a;}
	inline void setOpenType (AP_Dialog_New::tOpenType t) {m_openType = t;}


	void setTemplateName (const char * name);
	void setFileName (const char * name);

private:
	AP_Dialog_New::tAnswer		m_answer;
	AP_Dialog_New::tOpenType    m_openType;

	char * m_templateName;
	char * m_fileName;
};

#endif /* AP_DIALOG_NEW_H */
