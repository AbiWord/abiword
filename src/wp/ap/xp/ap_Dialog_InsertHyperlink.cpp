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

#include "ut_go_file.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "ap_Dialog_InsertHyperlink.h"

AP_Dialog_InsertHyperlink::AP_Dialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialoghyperlink"),
	m_answer(a_CANCEL),
	m_pHyperlink(0),
	m_pHyperlinkTitle(0)
{
}

AP_Dialog_InsertHyperlink::~AP_Dialog_InsertHyperlink(void)
{
	DELETEPV(m_pHyperlink);
	DELETEPV(m_pHyperlinkTitle);
}

void AP_Dialog_InsertHyperlink::setAnswer(AP_Dialog_InsertHyperlink::tAnswer a)
{
  m_answer = a;
}

AP_Dialog_InsertHyperlink::tAnswer AP_Dialog_InsertHyperlink::getAnswer(void) const
{
  return m_answer;
}

UT_uint32 AP_Dialog_InsertHyperlink::getExistingBookmarksCount() const
{
	UT_return_val_if_fail (m_pDoc, 0);
	return m_pDoc->getBookmarkCount();
}

const std::string & AP_Dialog_InsertHyperlink::getNthExistingBookmark(UT_uint32 n) const
{
	UT_ASSERT(m_pDoc);
	return m_pDoc->getNthBookmark(n);
}

const gchar * AP_Dialog_InsertHyperlink::getHyperlink() const
{
  return m_pHyperlink;
}

const gchar * AP_Dialog_InsertHyperlink::getHyperlinkTitle() const
{
    return m_pHyperlinkTitle;
}

void AP_Dialog_InsertHyperlink::setHyperlink(const gchar * link)
{
	DELETEPV(m_pHyperlink);
	UT_uint32 len = strlen(link);
	m_pHyperlink = new gchar [len+1];
	strncpy(m_pHyperlink, link, len + 1);
}

void AP_Dialog_InsertHyperlink::setHyperlinkTitle(const gchar * title)
{
	DELETEPV(m_pHyperlinkTitle);
	UT_uint32 len = strlen(title);
	m_pHyperlinkTitle = new gchar [len+1];
	strncpy(m_pHyperlinkTitle, title, len + 1);
}

void AP_Dialog_InsertHyperlink::setDoc(FV_View * pView)
{
	m_pView = pView;
	m_pDoc = pView->getDocument();

	// we will also init here the m_pHyperlink member
	if(!m_pHyperlink)
	{
		if(!pView->isSelectionEmpty())
		{

			UT_UCS4Char * pSelection;
			pView->getSelectionText(pSelection);
			UT_return_if_fail(pSelection);

			m_pHyperlink = new gchar [UT_UCS4_strlen_as_char(pSelection)+1];
			UT_UCS4_strcpy_to_char(m_pHyperlink, pSelection);

			FREEP(pSelection);
		
		// now check if this is a valid URL, if not just delete the
		// whole thing
			if(!UT_go_path_is_uri(m_pHyperlink))
			{
				delete [] m_pHyperlink;
				m_pHyperlink = NULL;
			}
		}
	}
}
