/* AbiWord
 * Copyright (C) 2001 Tomas Frydrych
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

#ifndef AP_DIALOG_INSERTHYPERLINK_H
#define AP_DIALOG_INSERTHYPERLINK_H

#include "xap_Dialog.h"
#include "ut_xml.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "ut_string.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_InsertHyperlink : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory,
			     XAP_Dialog_Id id);
	virtual ~AP_Dialog_InsertHyperlink(void);

	virtual void		 runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK=0, a_CANCEL=1 } tAnswer;

	tAnswer				getAnswer(void) const;
	void            	setAnswer(tAnswer a);
	UT_uint32			getExistingBookmarksCount() const;
	const gchar *	getNthExistingBookmark(UT_uint32 n) const;
	const gchar *	getHyperlink() const;
	void				setHyperlink(const gchar * link);
	void				setDoc(FV_View * pView);
	
private:
	PD_Document *		m_pDoc;
	FV_View *           m_pView;
	
	AP_Dialog_InsertHyperlink::tAnswer	m_answer;
	gchar *			m_pHyperlink;
};

#endif /* AP_DIALOG_TOGGLECASE_H */
