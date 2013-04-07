/* AbiWord
 * Copyright (C) 2002, 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include "ap_Dialog_ListRevisions.h"
#include "ap_Strings.h"

AP_Dialog_ListRevisions::AP_Dialog_ListRevisions(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogselectrevision"), m_answer(a_CANCEL),
    m_iId(0), m_pDoc(NULL), m_pSS (0)
{
	m_pSS = XAP_App::getApp()->getStringSet();
}

AP_Dialog_ListRevisions::~AP_Dialog_ListRevisions(void)
{
}

void AP_Dialog_ListRevisions::setAnswer(AP_Dialog_ListRevisions::tAnswer a)
{
  m_answer = a;
}

const char * AP_Dialog_ListRevisions::getTitle() const
{
	UT_return_val_if_fail(m_pSS,NULL);
	return m_pSS->getValue(AP_STRING_ID_DLG_ListRevisions_Title);
}

const char * AP_Dialog_ListRevisions::getLabel1() const
{
	UT_return_val_if_fail(m_pSS,NULL);
	return m_pSS->getValue(AP_STRING_ID_DLG_ListRevisions_Label1);
}

const char * AP_Dialog_ListRevisions::getColumn1Label() const
{
	UT_return_val_if_fail(m_pSS,NULL);
	return m_pSS->getValue(AP_STRING_ID_DLG_ListRevisions_Column1Label);
}

const char * AP_Dialog_ListRevisions::getColumn2Label() const
{
	UT_return_val_if_fail(m_pSS,NULL);
	return m_pSS->getValue(AP_STRING_ID_DLG_ListRevisions_Column2Label);
}

const char * AP_Dialog_ListRevisions::getColumn3Label() const
{
	UT_return_val_if_fail(m_pSS,NULL);
	return m_pSS->getValue(AP_STRING_ID_DLG_ListRevisions_Column3Label);
}

UT_uint32 AP_Dialog_ListRevisions::getItemCount() const
{
	UT_return_val_if_fail(m_pDoc,0);
	return (m_pDoc->getRevisions()).getItemCount() + 1;
}

UT_uint32 AP_Dialog_ListRevisions::getNthItemId(UT_uint32 n) const
{
	UT_return_val_if_fail(m_pDoc,0);

	if(n == 0)
		return 0;
	
	return ((AD_Revision *)(m_pDoc->getRevisions()).getNthItem(n-1))->getId();
}

time_t
AP_Dialog_ListRevisions::getNthItemTimeT(UT_uint32 n) const
{
	UT_return_val_if_fail(m_pDoc,0);

    time_t tT = 0;
	if(n == 0)
    {
        time(&tT);
    }
    else
    {
        tT = ((AD_Revision *)(m_pDoc->getRevisions()).getNthItem(n-1))->getStartTime();
    }
	return tT;
}

const char * AP_Dialog_ListRevisions::getNthItemTime(UT_uint32 n) const
{
	UT_return_val_if_fail(m_pDoc,0);

	// TODO the date should be properly localised
    time_t tT = getNthItemTimeT(n);
	static char s[30];

	if(tT != 0)
	{
		struct tm * tM = localtime(&tT);
		strftime(s,30,"%c",tM);
	}
	else
	{
		// we use 0 to indicate unknown time
		s[0] = '?';
		s[1] = '?';
		s[2] = '?';
		s[3] = 0;
	}
	
	return s;
}

char * AP_Dialog_ListRevisions::getNthItemText(UT_uint32 n, bool utf8) const
{
	bool bFree = false;

	if(n == 0)
	{
		// the zero entry represents the normal view without a
		// selection, we get the comment from our stringset
		
		UT_return_val_if_fail(m_pSS,NULL);
		char * pComment = g_strdup(m_pSS->getValue(AP_STRING_ID_DLG_ListRevisions_LevelZero));
		return pComment;
	}
	else
	{
		
		const UT_UCS4Char * pC
			= ((AD_Revision *)(m_pDoc->getRevisions()).getNthItem(n-1))->getDescription();
	
		if(!pC)
			return NULL;

		// now we run this string through fribidi
		if(XAP_App::getApp()->theOSHasBidiSupport() == XAP_App::BIDI_SUPPORT_NONE)
		{
			UT_UCS4Char * pStr2 = 0;
			UT_uint32 iLen = UT_UCS4_strlen(pC);

			pStr2  = (UT_UCS4Char *)UT_calloc( iLen + 1, sizeof(UT_UCS4Char));
			UT_return_val_if_fail(pStr2,NULL);
			bFree = true;

			UT_BidiCharType iDomDir = UT_bidiGetCharType(pC[0]);
			UT_bidiReorderString(pC, iLen, iDomDir, pStr2);
			pC = (const UT_UCS4Char *) pStr2;

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
}

AP_Dialog_ListRevisions::tAnswer AP_Dialog_ListRevisions::getAnswer(void) const
{
  return m_answer;
}

