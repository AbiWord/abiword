/* AbiWord
 * Copyright (c) 2001,2002 Tomas Frydrych
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

#ifndef AP_DIALOG_INSERTBOOKMARK_H
#define AP_DIALOG_INSERTBOOKMARK_H

#include "ap_Dialog_Modal.h"
#include "pd_Document.h"
#include "ut_xml.h"
#include "ut_string.h"
#include "ut_string_class.h"

#define BOOKMARK_SIZE_LIMIT 30
class XAP_Frame;

class ABI_EXPORT AP_Dialog_InsertBookmark : public AP_Dialog_Modal
{
public:
	AP_Dialog_InsertBookmark( XAP_DialogFactory * pDlgFactory,
                              XAP_Dialog_Id id );
	virtual ~AP_Dialog_InsertBookmark(void);

	virtual void		 runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK=0, a_CANCEL=1, a_DELETE=2 } tAnswer;

	tAnswer 			getAnswer(void) const;
	void				setAnswer(tAnswer a);
	UT_sint32			getExistingBookmarksCount() const;
	const std::string &	getNthExistingBookmark(UT_uint32 n) const;
	const gchar *	getBookmark() const;
	void				setBookmark(const gchar * mark);
	void				setDoc(FV_View * pView);

	void setSuggestedBM(const UT_UCS4Char * str)
	  {
	    if (str)
	      m_suggested = str;
	    else
	      m_suggested.clear ();
	  }

	UT_UCS4String getSuggestedBM () const
	  {
	    return m_suggested;
	  }

private:
	PD_Document *		m_pDoc;

	UT_UCS4String m_suggested;

	AP_Dialog_InsertBookmark::tAnswer	m_answer;
	gchar			m_pBookmark[BOOKMARK_SIZE_LIMIT + 1];
};

#endif /* AP_DIALOG_TOGGLECASE_H */
