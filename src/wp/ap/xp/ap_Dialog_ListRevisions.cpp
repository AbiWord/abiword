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

#include "ap_Dialog_ListRevisions.h"
#include "ap_Strings.h"

#include <fribidi/fribidi.h>

AP_Dialog_ListRevisions::AP_Dialog_ListRevisions(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id), m_answer(a_CANCEL),
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

UT_uint32 AP_Dialog_ListRevisions::getItemCount() const
{
	UT_return_val_if_fail(m_pDoc,0);
	return (m_pDoc->getRevisions()).getItemCount();
}

UT_uint32 AP_Dialog_ListRevisions::getNthItemId(UT_uint32 n) const
{
	UT_return_val_if_fail(m_pDoc,0);
	return ((PD_Revision *)(m_pDoc->getRevisions()).getNthItem(n))->getId();
}

char * AP_Dialog_ListRevisions::getNthItemText(UT_uint32 n) const
{
	bool bFree = false;

	const UT_UCS4Char * pC = ((PD_Revision *)(m_pDoc->getRevisions()).getNthItem(n))->getDescription();
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

AP_Dialog_ListRevisions::tAnswer AP_Dialog_ListRevisions::getAnswer(void) const
{
  return m_answer;
}

