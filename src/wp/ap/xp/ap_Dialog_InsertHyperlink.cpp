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

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "ap_Dialog_InsertHyperlink.h"

AP_Dialog_InsertHyperlink::AP_Dialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialoghyperlink"), m_answer(a_CANCEL), m_pHyperlink(0)
{
}

AP_Dialog_InsertHyperlink::~AP_Dialog_InsertHyperlink(void)
{
	if(m_pHyperlink)
		delete [] m_pHyperlink;
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
	UT_ASSERT(m_pDoc);
	return m_pDoc->getBookmarkCount();
}

const XML_Char * AP_Dialog_InsertHyperlink::getNthExistingBookmark(UT_uint32 n) const
{
	UT_ASSERT(m_pDoc);
	return m_pDoc->getNthBookmark(n);
}

const XML_Char * AP_Dialog_InsertHyperlink::getHyperlink() const
{
  return (const XML_Char *)m_pHyperlink;
}

void AP_Dialog_InsertHyperlink::setHyperlink(const XML_Char * link)
{
	if(m_pHyperlink)
		delete [] m_pHyperlink;

	UT_uint32 len = UT_XML_strlen(link);
	m_pHyperlink = new XML_Char [len+1];
	UT_XML_strncpy(m_pHyperlink, len + 1, link);
}

void AP_Dialog_InsertHyperlink::setDoc(FV_View * pView)
{
	m_pView = pView;
	m_pDoc = pView->getDocument();

	// we will also init here the m_pHyperlink member
	if(!m_pHyperlink)
	{
		UT_return_if_fail(!pView->isSelectionEmpty());

		UT_UCS4Char * pSelection = pView->getSelectionText();
		UT_return_if_fail(pSelection);

		m_pHyperlink = new XML_Char [UT_UCS4_strlen(pSelection)+1];
		UT_UCS4_strcpy_to_char(m_pHyperlink, pSelection);

		// now check if this is a valid URL, if not just delete the
		// whole thing
		if(!UT_isUrl(m_pHyperlink))
		{
			delete [] m_pHyperlink;
			m_pHyperlink = NULL;
		}
	}
}
