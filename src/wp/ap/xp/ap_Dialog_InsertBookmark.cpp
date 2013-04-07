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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "ap_Dialog_InsertBookmark.h"

AP_Dialog_InsertBookmark::AP_Dialog_InsertBookmark( XAP_DialogFactory * pDlgFactory,
                                                    XAP_Dialog_Id id )
    : AP_Dialog_Modal(pDlgFactory,id, "interface/dialogbookmark")
    , m_answer(a_CANCEL)
{
	m_pBookmark[0] = 0;
}

AP_Dialog_InsertBookmark::~AP_Dialog_InsertBookmark(void)
{
}

void AP_Dialog_InsertBookmark::setAnswer(AP_Dialog_InsertBookmark::tAnswer a)
{
  m_answer = a;
}

AP_Dialog_InsertBookmark::tAnswer AP_Dialog_InsertBookmark::getAnswer(void) const
{
  return m_answer;
}

UT_sint32 AP_Dialog_InsertBookmark::getExistingBookmarksCount() const
{
	UT_return_val_if_fail (m_pDoc, 0);
	return m_pDoc->getBookmarkCount();
}

const std::string & AP_Dialog_InsertBookmark::getNthExistingBookmark(UT_uint32 n) const
{
	UT_ASSERT(m_pDoc);
	return m_pDoc->getNthBookmark(n);
}

const gchar * AP_Dialog_InsertBookmark::getBookmark() const
{
  return m_pBookmark;
}

void AP_Dialog_InsertBookmark::setBookmark(const gchar * mark)
{
	strncpy(m_pBookmark, mark, BOOKMARK_SIZE_LIMIT);
}

void AP_Dialog_InsertBookmark::setDoc(FV_View * pView)
{
	m_pDoc = pView->getDocument();
}
