/* AbiWord
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include "ut_string.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"

#include "ap_Dialog_MarkRevisions.h"
#include "ap_Strings.h"

#include <fribidi/fribidi.h>

AP_Dialog_MarkRevisions::AP_Dialog_MarkRevisions(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id), m_answer(a_CANCEL),
		m_pDoc(NULL), m_pComment2(NULL),m_pRev(NULL)
{
	m_pSS = XAP_App::getApp()->getStringSet();
}

AP_Dialog_MarkRevisions::~AP_Dialog_MarkRevisions(void)
{
	delete [] m_pComment2;
}

void AP_Dialog_MarkRevisions::setAnswer(AP_Dialog_MarkRevisions::tAnswer a)
{
  m_answer = a;
}

void AP_Dialog_MarkRevisions::_initRevision()
{
	if(!m_pRev)
	{
		UT_return_if_fail(m_pDoc);

		m_pRev = m_pDoc->getHighestRevision();
	}
}

const char * AP_Dialog_MarkRevisions::getTitle()
{
	UT_return_val_if_fail(m_pSS,NULL);
	return m_pSS->getValue(AP_STRING_ID_DLG_MarkRevisions_Title);
}

const char * AP_Dialog_MarkRevisions::getComment2Label()
{
	UT_return_val_if_fail(m_pSS,NULL);
	return m_pSS->getValue(AP_STRING_ID_DLG_MarkRevisions_Comment2Label);
}

char * AP_Dialog_MarkRevisions::getRadio1Label()
{
	_initRevision();

	// if m_pRev == NULL, there are no revisions in this document
	if(!m_pRev)
		return NULL;

	UT_return_val_if_fail(m_pSS,NULL);
	const char * pLabel = m_pSS->getValue(AP_STRING_ID_DLG_MarkRevisions_Check1Label);

	UT_return_val_if_fail(pLabel,NULL);
	char * pBuff = (char*)UT_calloc(strlen(pLabel) + 35, sizeof(char));

	sprintf(pBuff, pLabel, m_pRev->getId());

	return pBuff;
}

const char * AP_Dialog_MarkRevisions::getRadio2Label()
{
	UT_return_val_if_fail(m_pSS,NULL);
	return m_pSS->getValue(AP_STRING_ID_DLG_MarkRevisions_Check2Label);
}

char * AP_Dialog_MarkRevisions::getComment1()
{
	bool bFree = false;

	_initRevision();

	if(!m_pRev)
		return NULL;

	const UT_UCS4Char * pC = m_pRev->getDescription();

	if(!pC)
		return NULL;

	// now we run this string through fribidi
	if(!XAP_App::getApp()->theOSHasBidiSupport())
	{
		FriBidiChar *fbdStr = (FriBidiChar *)pC;
		FriBidiChar *fbdStr2 = 0;
		UT_uint32 iLen = UT_UCS4_strlen(pC);

		fbdStr2  = (FriBidiChar *)UT_calloc( iLen + 1, sizeof(FriBidiChar));
		UT_return_val_if_fail(fbdStr2,NULL);
		bFree = true;

		FriBidiCharType fbdDomDir = fribidi_get_type(fbdStr[0]);

		fribidi_log2vis (		/* input */
						 fbdStr,
						 iLen,
						 &fbdDomDir,
						 /* output */
						 fbdStr2,
						 NULL,
						 NULL,
						 NULL);
		pC = (const UT_UCS4Char *) fbdStr2;

	}

	char * pComment = (char *)UT_calloc(UT_UCS4_strlen(pC) + 1, sizeof(char));
	UT_return_val_if_fail(pComment,NULL);

	UT_UCS4_strcpy_to_char(pComment,pC);

	if(bFree)
	{
		FREEP(pC);
	}

	return pComment;
}

void AP_Dialog_MarkRevisions::setComment2(const char * pszComment)
{
	if(m_pComment2)
		delete [] m_pComment2;

	m_pComment2 = new UT_UCS4Char [strlen(pszComment) + 1];
	UT_return_if_fail(m_pComment2);

	UT_UCS4_strcpy_char(m_pComment2,pszComment);
}


AP_Dialog_MarkRevisions::tAnswer AP_Dialog_MarkRevisions::getAnswer(void) const
{
  return m_answer;
}

void AP_Dialog_MarkRevisions::addRevision()
{
	UT_return_if_fail(m_pDoc);

	if(!m_pComment2)
		return;

	_initRevision();

	UT_uint32 iId = 1;

	if(m_pRev)
		iId = m_pRev->getId() + 1;

	m_pDoc->addRevision(iId,m_pComment2, UT_UCS4_strlen(m_pComment2));
	m_pRev = NULL;
}

