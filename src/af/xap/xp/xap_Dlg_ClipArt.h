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

#ifndef XAP_DIALOG_CLIPART_H
#define XAP_DIALOG_CLIPART_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

class XAP_Frame;

class XAP_Dialog_ClipArt : public XAP_Dialog_NonPersistent
{
public:

	typedef enum { a_OK, a_CANCEL } tAnswer;

	XAP_Dialog_ClipArt(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_ClipArt(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	inline XAP_Dialog_ClipArt::tAnswer getAnswer(void) const {return m_answer;}

	void setInitialDir (const char * szInitialDir);
	inline const char * getGraphicName () const {return m_szGraphicName;}
	
protected:

	void setGraphicName (const char * name);
	inline void setAnswer (XAP_Dialog_ClipArt::tAnswer a) {m_answer = a;}
	const char * getInitialDir () const {return m_szInitialDir;}

private:	

	XAP_Dialog_ClipArt::tAnswer		m_answer;
	char * m_szGraphicName;
	char * m_szInitialDir;
};

#endif /* XAP_DIALOG_CLIPART_H */
