/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ap_Dialog_New.h"
#include "ut_types.h"
#include "ap_Strings.h"

AP_Dialog_New::AP_Dialog_New(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent (pDlgFactory, id), 
	m_answer (AP_Dialog_New::a_CANCEL), m_openType (AP_Dialog_New::open_New),
	m_templateName(0), m_fileName(0)
{
}

AP_Dialog_New::~AP_Dialog_New()
{
	FREEP(m_templateName);
	FREEP(m_fileName);
}

void AP_Dialog_New::setTemplateName(const char * name)
{
	FREEP(m_templateName);
	m_templateName = UT_strdup (name);
}

void AP_Dialog_New::setFileName(const char * name)
{
	FREEP(m_fileName);
	m_fileName = UT_strdup (name);
}

/**************************************************************************/
/**************************************************************************/

// if you'd like to add more tabs or add more things to any particular tab
// all you have to do is add stuff here

const UT_uint32 NUMTABS = 1;
const TemplateData tab1[] = 
{
	{"default.abw", AP_STRING_ID_DLG_NEW_Tab1_WP1},
	{"fax1.abw", AP_STRING_ID_DLG_NEW_Tab1_FAX1},
	{0, 0}
};

UT_uint32 AP_Dialog_New::getNumTabs(void) const
{
	return NUMTABS;
}

#define AT(x) translated = pSS->getValue((x))

const XML_Char * AP_Dialog_New::getTabName (UT_uint32 tab) const
{
	const XML_Char * translated = 0;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	switch (tab)
	{
	case 1: AT(AP_STRING_ID_DLG_NEW_Tab1); break;
	default: UT_ASSERT(UT_SHOULD_NOT_HAPPEN); break;
	}

	return translated;
}

#undef AT

#define AS(x) data = (const TemplateData *)(x)

const TemplateData * AP_Dialog_New::getListForTab (UT_uint32 tab) const
{
	const TemplateData * data = 0;

	switch(tab)
	{
	case 1: AS(tab1); break;
	default: UT_ASSERT(UT_SHOULD_NOT_HAPPEN); break;
	}

	return data;
}

#undef AS
