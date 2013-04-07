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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#include "ap_Features.h"

#include "ut_string.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"

#include "ap_Dialog_MarkRevisions.h"
#include "ap_Strings.h"

AP_Dialog_MarkRevisions::AP_Dialog_MarkRevisions(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogmarkrevisions"), m_answer(a_CANCEL),
		m_pDoc(NULL), m_pComment2(NULL),m_pRev(NULL),m_bForceNew(false)
{
	m_pSS = XAP_App::getApp()->getStringSet();
}

AP_Dialog_MarkRevisions::~AP_Dialog_MarkRevisions(void)
{
	DELETEP(m_pComment2);
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

	if(!m_pRev || m_bForceNew)
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

char * AP_Dialog_MarkRevisions::getComment1(bool utf8)
{
	_initRevision();

	if(!m_pRev || m_bForceNew)
		return NULL;

	bool bFree = false;

	const UT_UCS4Char * pC = m_pRev->getDescription();

	if(!pC)
		return NULL;

	// now we run this string through fribidi
	if(XAP_App::getApp()->theOSHasBidiSupport() == XAP_App::BIDI_SUPPORT_NONE)
	{
		UT_UCS4Char *pStr2 = 0;
		UT_uint32 iLen = UT_UCS4_strlen(pC);

		pStr2  = (UT_UCS4Char *)UT_calloc( iLen + 1, sizeof(UT_UCS4Char));
		UT_return_val_if_fail(pStr2,NULL);
		bFree = true;

		UT_BidiCharType iDomDir = UT_bidiGetCharType(pC[0]);

		UT_bidiReorderString(pC, iLen, iDomDir, pStr2);
		pC = pStr2;

	}

	char * pComment;

	if (utf8)
	{
		UT_UTF8String comment(pC);
		pComment = (char *)UT_calloc(comment.byteLength() + 1, sizeof(char));
		UT_return_val_if_fail(pComment,NULL);
		pComment = strcpy(pComment, comment.utf8_str());
	}
	else
	{
		pComment = (char *)UT_calloc(UT_UCS4_strlen(pC) + 1, sizeof(char));
		UT_return_val_if_fail(pComment,NULL);
		UT_UCS4_strcpy_to_char(pComment,pC);
	}

	if(bFree)
	{
		FREEP(pC);
	}

	return pComment;
}

void AP_Dialog_MarkRevisions::setComment2(const char * pszComment)
{
	DELETEP(m_pComment2);
	m_pComment2 = new UT_UTF8String(pszComment);
}


AP_Dialog_MarkRevisions::tAnswer AP_Dialog_MarkRevisions::getAnswer(void) const
{
  return m_answer;
}

bool AP_Dialog_MarkRevisions::isRev(void)
{
   if(!m_pRev)
	return false;
   return true;
}

void AP_Dialog_MarkRevisions::addRevision()
{
	UT_return_if_fail(m_pDoc);

	if (!m_pComment2)
		return;

	_initRevision();

	UT_uint32 iId = 1;

	if(m_pRev)
		iId = m_pRev->getId() + 1;

	time_t tStart = time(NULL);
	m_pDoc->addRevision(iId, m_pComment2->ucs4_str().ucs4_str(), UT_UCS4_strlen(m_pComment2->ucs4_str().ucs4_str()), tStart, 0, true);
	m_pRev = NULL;
}

