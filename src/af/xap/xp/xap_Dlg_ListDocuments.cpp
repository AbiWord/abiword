/* AbiSource Application Framework
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dlg_ListDocuments.h"
#include "xap_App.h"
#include "xad_Document.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Frame.h"

XAP_Dialog_ListDocuments::XAP_Dialog_ListDocuments(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id):
	XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialoglistdocument"),
	m_answer(a_OK),
	m_ndxSelDoc(-1),
	m_bIncludeActiveDoc(false)
{
	_init();
}

XAP_Dialog_ListDocuments::~XAP_Dialog_ListDocuments(void)
{
}

void XAP_Dialog_ListDocuments::_init()
{
	m_vDocs.clear();
	UT_return_if_fail(m_pApp);
	AD_Document * pExclude = NULL;

	if(!m_bIncludeActiveDoc)
	{
		XAP_Frame * pF = m_pApp->getLastFocussedFrame();

		if(pF)
		{
			pExclude = pF->getCurrentDoc();
		}
	}
	
	m_pApp->enumerateDocuments(m_vDocs,pExclude);
}

AD_Document * XAP_Dialog_ListDocuments::getDocument(void) const
{
	UT_ASSERT(m_answer == a_OK);
	UT_ASSERT_HARMLESS(m_pApp);

	if (m_pApp && (m_ndxSelDoc >= 0))
	{
		return (AD_Document *)m_vDocs.getNthItem(m_ndxSelDoc);
	}

	return NULL;
}

void XAP_Dialog_ListDocuments::_setSelDocumentIndx(UT_sint32 i)
{
	UT_return_if_fail(i <= m_vDocs.getItemCount());
	m_ndxSelDoc = i;
}

const char * XAP_Dialog_ListDocuments::_getNthDocumentName(UT_sint32 n) const
{
	if(n >= m_vDocs.getItemCount())
		return NULL;

	const AD_Document * pDoc = (const AD_Document *)m_vDocs.getNthItem(n);

	if(!pDoc)
		return NULL;

	return pDoc->getFilename().c_str();
}

void XAP_Dialog_ListDocuments::setIncludeActiveDoc(bool b)
{
	if(b != m_bIncludeActiveDoc)
	{
		m_bIncludeActiveDoc = b;
		_init();
	}
}

const char * XAP_Dialog_ListDocuments::_getTitle() const
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_return_val_if_fail(pSS, NULL);

	return 	pSS->getValue(XAP_STRING_ID_DLG_LISTDOCS_Title);
}

const char * XAP_Dialog_ListDocuments::_getHeading() const
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_return_val_if_fail(pSS, NULL);

	return 	pSS->getValue(XAP_STRING_ID_DLG_LISTDOCS_Heading1);
}

const char * XAP_Dialog_ListDocuments::_getOKButtonText() const
{

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_return_val_if_fail(pSS, NULL);
	
	switch(getDialogId())
	{
		case XAP_DIALOG_ID_LISTDOCUMENTS:
			return 	pSS->getValue(XAP_STRING_ID_DLG_Select);
			
		case XAP_DIALOG_ID_COMPAREDOCUMENTS:
			return 	pSS->getValue(XAP_STRING_ID_DLG_Compare);
			
		case XAP_DIALOG_ID_MERGEDOCUMENTS:
			return 	pSS->getValue(XAP_STRING_ID_DLG_Merge);

		default:
			UT_return_val_if_fail(UT_NOT_REACHED, NULL);
	}

	return NULL;
}


