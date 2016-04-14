/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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

#include "ut_uuid.h"
#include "ut_std_string.h"
#include "xap_Dlg_History.h"
#include "xad_Document.h"
#include "xap_App.h"
#include "xap_Strings.h"

#include <locale.h>

XAP_Dialog_History::XAP_Dialog_History(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id):
	XAP_Dialog_NonPersistent(pDlgFactory,id),
	m_answer(a_CANCEL),
	m_pDoc(NULL),
	m_pSS(NULL),
	m_iId(0)
{
	m_pSS = XAP_App::getApp()->getStringSet();
}

const char * XAP_Dialog_History::getWindowLabel() const
{
	UT_return_val_if_fail(m_pSS, NULL);
	return m_pSS->getValue(XAP_STRING_ID_DLG_History_WindowLabel);
}

const char *  XAP_Dialog_History::getListTitle() const
{
	UT_return_val_if_fail(m_pSS, NULL);
	return m_pSS->getValue(XAP_STRING_ID_DLG_History_List_Title);
}


const char * XAP_Dialog_History::getHeaderLabel(UT_uint32 indx) const
{
	UT_return_val_if_fail(m_pSS, NULL);
	switch(indx)
	{
		case 0: return m_pSS->getValue(XAP_STRING_ID_DLG_History_Path);
		case 1: return m_pSS->getValue(XAP_STRING_ID_DLG_History_Version);
		case 2: return m_pSS->getValue(XAP_STRING_ID_DLG_History_Created);
		case 3: return m_pSS->getValue(XAP_STRING_ID_DLG_History_LastSaved);
		case 4: return m_pSS->getValue(XAP_STRING_ID_DLG_History_EditTime);
		case 5: return m_pSS->getValue(XAP_STRING_ID_DLG_History_Id);
		
		default:;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}


char * XAP_Dialog_History::getHeaderValue(UT_uint32 indx) const
{
	UT_return_val_if_fail(m_pDoc, NULL);
	
	UT_String S;
	time_t tT;
	struct tm * tM;
	char * s;

	switch(indx)
	{
		case 0:
			return g_strdup(UT_ellipsisPath(m_pDoc->getFilename(), 45, 35).c_str());

		case 1:
			UT_String_sprintf(S,"%d",m_pDoc->getDocVersion());
			return g_strdup(S.c_str());

		case 2:
			{
				const UT_UUID * pUUID = m_pDoc->getDocUUID();
				UT_return_val_if_fail(pUUID, NULL);
				
				tT = pUUID->getTime();
				tM = localtime(&tT);
				s = (char*)g_try_malloc(30);
				if(!s)
					return NULL;

				size_t len = strftime(s,30,"%c",tM);
				if(!len)
				{
					FREEP(s);
					return NULL;
				}

				return s;
			}
			
		case 3:
			{
				tT = m_pDoc->getLastSavedTime();
				tM = localtime(&tT);
				s = (char*)g_try_malloc(30);
				if(!s)
					return NULL;

				size_t len = strftime(s,30,"%c",tM);
				if(!len)
				{
					FREEP(s);
					return NULL;
				}
				return s;
			}
		case 4:
			{
				time_t iEditTime = m_pDoc->getEditTime();
				time_t iHours = iEditTime / 3600;
				time_t iMinutes = (iEditTime % 3600)/60;
				time_t iSeconds = (iEditTime % 3600) % 60;
				
				UT_String_sprintf(S,"%.2d:%.2d:%.2d", static_cast<UT_sint32>(iHours), static_cast<UT_sint32>(iMinutes), static_cast<UT_sint32>(iSeconds));
				return g_strdup(S.c_str());
			}

		case 5:
			return g_strdup(m_pDoc->getDocUUIDString());
			
		default:;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}


const char * XAP_Dialog_History::getButtonLabel(UT_uint32 indx) const
{
	UT_return_val_if_fail(m_pSS, NULL);

	switch(indx)
	{
		case 0: return m_pSS->getValue(XAP_STRING_ID_DLG_Restore);
			//case 1: return m_pSS->getValue(XAP_STRING_ID_DLG_Show);
		case 1: return m_pSS->getValue(XAP_STRING_ID_DLG_Cancel);

		default:;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}


const char * XAP_Dialog_History::getListHeader(UT_uint32 column) const
{
	UT_return_val_if_fail(m_pSS, NULL);

	switch(column)
	{
		case 0: return m_pSS->getValue(XAP_STRING_ID_DLG_History_Version_Version);
		case 1: return m_pSS->getValue(XAP_STRING_ID_DLG_History_Version_Started);
		case 2: return m_pSS->getValue(XAP_STRING_ID_DLG_History_Version_AutoRevisioned);

		default:;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

UT_uint32 XAP_Dialog_History::getListItemCount() const
{
	UT_return_val_if_fail(m_pDoc, 0);
	return m_pDoc->getHistoryCount();
}


char * XAP_Dialog_History::getListValue(UT_uint32 item, UT_uint32 column) const
{
	UT_return_val_if_fail(m_pDoc, NULL);

	UT_String S;
	time_t tT;
	struct tm * tM;
	char * s;
	
	switch(column)
	{
		case 0:
			UT_String_sprintf(S,"%d",m_pDoc->getHistoryNthId(item));
			return g_strdup(S.c_str());
			
		case 1:
			{
				tT = m_pDoc->getHistoryNthTimeStarted(item);
				tM = localtime(&tT);
				s = (char*)g_try_malloc(30);
				if(!s)
					return NULL;

				size_t len = strftime(s,30,"%c",tM);
				if(!len)
				{
					FREEP(s);
					return NULL;
				}

				return s;
			}

		case 2:
			{
				UT_return_val_if_fail(m_pSS, NULL);

				const char * pszS;
				if(m_pDoc->getHistoryNthAutoRevisioned(item))
					pszS = m_pSS->getValue(XAP_STRING_ID_DLG_MB_Yes);
				else
					pszS = m_pSS->getValue(XAP_STRING_ID_DLG_MB_No);
					
				UT_return_val_if_fail(pszS, NULL);

				return g_strdup(pszS);
			}
			
		default:;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}


UT_uint32 XAP_Dialog_History::getListItemId(UT_uint32 item) const
{
	UT_return_val_if_fail(m_pDoc, 0);
	return m_pDoc->getHistoryNthId(item);
}

