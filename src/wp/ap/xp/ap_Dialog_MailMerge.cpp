/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "ap_Dialog_MailMerge.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "ie_mailmerge.h"
#include "xap_Frame.h"
#include "ut_assert.h"

AP_Dialog_MailMerge::AP_Dialog_MailMerge(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id),  m_pFrame(0), m_answer(a_CANCEL)
{
}

AP_Dialog_MailMerge::~AP_Dialog_MailMerge(void)
{
	UT_VECTOR_PURGEALL(UT_UTF8String*, m_vecFields);
}

void AP_Dialog_MailMerge::setAnswer(AP_Dialog_MailMerge::tAnswer a)
{
  m_answer = a;
}

AP_Dialog_MailMerge::tAnswer AP_Dialog_MailMerge::getAnswer(void) const
{
  return m_answer;
}

void AP_Dialog_MailMerge::setMergeField(const UT_UTF8String & name)
{
	m_mergeField = name;
}

const UT_UTF8String& AP_Dialog_MailMerge::getMergeField() const
{
	return m_mergeField;
}

void AP_Dialog_MailMerge::eventOpen ()
{
	UT_VECTOR_PURGEALL(UT_UTF8String*, m_vecFields);
	m_vecFields.clear();

	UT_return_if_fail(m_pFrame);
	m_pFrame->raise();
	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;
	
	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(m_pFrame->getDialogFactory());
	
	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return;

	UT_uint32 filterCount = 0;
	
	filterCount = IE_MailMerge::getMergerCount();
	
	const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	IEMergeType * nTypeList = static_cast<IEMergeType *>(UT_calloc(filterCount + 1, sizeof(IEMergeType)));
	UT_uint32 k = 0;
	
	while (IE_MailMerge::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
		k++;
	
	pDialog->setFileTypeList(szDescList, szSuffixList, static_cast<const UT_sint32 *>(nTypeList));
	
	pDialog->setDefaultFileType(IE_MailMerge::fileTypeForSuffix (".xml"));
	
	pDialog->runModal(m_pFrame);
	
	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);
	
	if (bOK)
    {
		UT_String filename (pDialog->getPathname());
		UT_sint32 type = pDialog->getFileType();
		
		IE_MailMerge * pie = NULL;
		UT_Error errorCode = IE_MailMerge::constructMerger(filename.c_str(), static_cast<IEMergeType>(type), &pie);
		if (!errorCode)
		{
			pie->getHeaders (filename.c_str(), m_vecFields);
			DELETEP(pie);
		}
	}
	
	pDialogFactory->releaseDialog(pDialog);

	setFieldList();
}

void AP_Dialog_MailMerge::setFieldList()
{
	// subclasses must override this
	UT_ASSERT_NOT_REACHED();
}
