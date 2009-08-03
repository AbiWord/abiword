/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * BIDI Copyright (C) 2001,2002 Tomas Frydrych
 * Copyright (C) 2002 Dom Lachowicz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_bytebuf.h"
#include "ut_wctomb.h"
#include "ut_iconv.h"
#include "ut_exception.h"
#include "ut_Language.h"
#include "ut_locale.h"

#include "xap_App.h"
#include "xap_Strings.h"
#include "xap_EncodingManager.h"

char * abi_set_domain(const char * szDomain)
{
	bindtextdomain(szDomain, LOCALE_DIR);
	return textdomain(szDomain);
}

//////////////////////////////////////////////////////////////////
// base class provides interface regardless of how we got the strings
//////////////////////////////////////////////////////////////////

XAP_StringSet::XAP_StringSet(const gchar * szDomainName)
	: m_domain(szDomainName),
		m_encoding("UTF-8")
{
	UT_LocaleInfo *info = new UT_LocaleInfo();
	const char * lang = info->getLanguage().utf8_str();
	const char * reg = info->getTerritory().utf8_str();
	m_szLanguageName = (char *) malloc(sizeof(char) * (strlen(lang) + strlen(reg) + (1 * strlen(reg))));

	if (lang)
	{
		strcpy((char *) m_szLanguageName, lang);

		if (reg)
		{
			strcat((char *) m_szLanguageName, "-");
			strcat((char *) m_szLanguageName, reg);
		}
	}
	else
		m_szLanguageName = "-none-";

	setlocale(LC_ALL, "");
	abi_set_domain(m_domain);
}

XAP_StringSet::~XAP_StringSet(void)
{
	free((void *) m_szLanguageName);
}

const gchar * XAP_StringSet::getLanguageName(void) const
{
	return m_szLanguageName;
}

bool XAP_StringSet::getValue(XAP_String_Id id, const char * inEncoding, UT_String &s) const
{
	const char * toTranslate = getValue(id);

	UT_return_val_if_fail(toTranslate != NULL, false);

	if(!strcmp(m_encoding.c_str(),inEncoding))
	{
		s = toTranslate;
	}
	else
	{
					UT_iconv_t conv = UT_iconv_open(inEncoding, m_encoding.c_str());
		UT_return_val_if_fail(UT_iconv_isValid(conv), false);
 
		char * translated = UT_convert_cd(toTranslate, strlen (toTranslate)+1, conv, NULL, NULL);
		
		UT_iconv_close(conv);
		
		UT_return_val_if_fail(translated, false);
		s = translated;
		
		g_free(translated);
	}
	
	return true;
}

bool XAP_StringSet::getValueUTF8(XAP_String_Id id, std::string & s) const
{	
	UT_String s_;
	bool b = getValue(id, "UTF-8", s_);
	if (b)
					s = s_.c_str();
	return b;
}

bool XAP_StringSet::getValueUTF8(XAP_String_Id id, UT_UTF8String & s) const
{	
	UT_String s_;
	bool b = getValue(id, "UTF-8", s_);
	if (b)
					s = s_.c_str();
	return b;
}

void XAP_StringSet::setEncoding(const gchar * inEncoding)
{
	UT_return_if_fail(inEncoding != 0);
	m_encoding = inEncoding;
	bind_textdomain_codeset(m_domain, getEncoding());
}

const char * XAP_StringSet::getEncoding() const
{
	return m_encoding.c_str();
}

const char * XAP_StringSet::setDomain(const char * szDomainName)
{
	// Don't call the gettext functions unless necessary
	if ((m_domain != szDomainName) || (!szDomainName && m_domain != GETTEXT_PACKAGE))
	{
		m_domain = szDomainName ? szDomainName : GETTEXT_PACKAGE;
		return abi_set_domain(m_domain);
	}
	else
		return m_domain;
}

/* Kinda useless for now, but maybe we can put in checks in the future */
const char * XAP_StringSet::translate(XAP_String_Id id) const
{
	return id;
}
