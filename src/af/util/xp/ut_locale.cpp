/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:t -*- */
/* AbiSource Program Utilities
 * Copyright (C) 2002 Dom Lachowicz
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

#include "ut_string.h"
#include "ut_locale.h"

// don't like XAP in UT, but oh well...
#include "xap_EncodingManager.h"

/********************************************/

/**
 * Class serves to make rolling back exceptions simple, automatic,
 * and transparent
 *
 * USAGE:
 * UT_LocaleTransactor t(LC_NUMERIC, "C");
 * sprintf();
 * sprintf();
 * return; // <-- old locale gets reset transparently for you at the end of
 * // the block
 *
 * SEE ALSO: man setlocale
 */
UT_LocaleTransactor::UT_LocaleTransactor (int category, const char * locale)
  : mCategory(category)
{
	mOldLocale = setlocale(category, nullptr);
	setlocale(category, locale);
}

UT_LocaleTransactor::~UT_LocaleTransactor ()
{
	setlocale(mCategory, mOldLocale.c_str());
}

/********************************************/

/**
 * Essentially identical to UT_LocaleInfo(getenv("LANG")), except
 * works on non-unix platforms too (i.e. win32)
 */
UT_LocaleInfo::UT_LocaleInfo ()
{
	// should work on any platform, as opposed to init(getenv("LANG"))
	XAP_EncodingManager * instance = XAP_EncodingManager::get_instance ();

	if (instance->getLanguageISOName() != nullptr)
		mLanguage = instance->getLanguageISOName();

	if (instance->getLanguageISOTerritory() != nullptr)
		mTerritory = instance->getLanguageISOTerritory();

	if (instance->getNative8BitEncodingName() != nullptr)
		mEncoding = instance->getNative8BitEncodingName();
}

/**
 * Takes in a string of the form "language_TERRITORY.ENCODING" or
 * "language-TERRITORY.ENCODING" and decomposes it. TERRITORY and ENCODING
 * parts are optional
 */
UT_LocaleInfo::UT_LocaleInfo (const char * locale)
{
	init(locale);
}

/* static */const UT_LocaleInfo UT_LocaleInfo::system()
{
	return UT_LocaleInfo();
}

/**
 * True if language field is non-null/non-empty, false if not
 */
bool UT_LocaleInfo::hasLanguage() const
{
	return mLanguage.size() != 0;
}

/**
 * True if territory field is non-null/non-empty, false if not
 */
bool UT_LocaleInfo::hasTerritory() const
{
	return mTerritory.size() != 0;
}

/**
 * True if encoding field is non-null/non-empty, false if not
 */
bool UT_LocaleInfo::hasEncoding() const
{
	return mEncoding.size() != 0;
}

/**
 * Returns empty string or language. Example languages are
 * "en", "wen", "fr", "es"
 */
const std::string& UT_LocaleInfo::getLanguage () const
{
	return mLanguage;
}

/**
 * Returns empty string or territory. Example territories are:
 * "US", "GB", "FR", ...
 */
const std::string& UT_LocaleInfo::getTerritory() const
{
	return mTerritory;
}

/**
 * Returns empty string or encoding. Encoding is like "UTF-8" or
 * "ISO-8859-1"
 */
const std::string& UT_LocaleInfo::getEncoding() const
{
	return mEncoding;
}

void UT_LocaleInfo::init(const std::string & locale)
{
	if (locale.empty())
	{
		return;
	}

	std::string::size_type dot = 0;
	std::string::size_type hyphen = 0;

	// take both hyphen types into account
	hyphen = locale.find('_');
	if (hyphen == std::string::npos)
	{
		hyphen = locale.find('-');
	}

	dot = locale.find('.');

	if (hyphen == std::string::npos && dot == std::string::npos)
	{
		mLanguage = locale;
		return;
	}

	if (hyphen != std::string::npos && dot != std::string::npos)
	{
		if (hyphen < dot)
		{
			mLanguage  = locale.substr(0, hyphen);
			mTerritory = locale.substr(hyphen + 1, dot - (hyphen + 1));
			mEncoding  = locale.substr(dot + 1, locale.size() - (dot + 1));
		}
		else
		{
			mLanguage = locale.substr(0, dot);
			mEncoding = locale.substr(dot + 1, locale.size() - (dot + 1));
		}
	}
	else if (dot != std::string::npos)
	{
		mLanguage = locale.substr(0, dot);
		mEncoding = locale.substr(dot + 1, locale.size() - (dot + 1));
	}
	else if (hyphen != std::string::npos)
	{
		mLanguage = locale.substr(0, hyphen);
		mEncoding = locale.substr(hyphen +1, locale.size() - (hyphen + 1));
	}
}

/**
 * Turns object back into a string of the form language_TERRITORY.ENCODING
 * (eg): en_US.UTF-8
 */
std::string UT_LocaleInfo::toString() const
{
	std::string ret(mLanguage);

	if (hasTerritory())
	{
		ret += "_";
		ret += mTerritory;
	}

	if (hasEncoding())
    {
		ret += ".";
		ret += mEncoding;
    }

	return ret;
}

bool UT_LocaleInfo::operator==(const UT_LocaleInfo & rhs) const
{
	return ((mLanguage == rhs.mLanguage) &&
			(mTerritory == rhs.mTerritory) &&
			(mEncoding == rhs.mEncoding));
}

bool UT_LocaleInfo::operator!=(const UT_LocaleInfo & rhs) const
{
  return (!(*this == rhs));
}

/*
	We build the default Abiword user's locale in the form ISO3166-ISO639 pair
	(e.g. es-MX) from Windows locale information. For example, If the user's selected
	country and language are Argentina and Spanish we will build AR-ES. Unfortunally,
	it's likely that we do not have a string set for this variant of Spanish (there are
	more than 19). What we do is we provide a fallback locale for this situation.
	26/01/2003 Jordi

	See bug 4174 for additional details
*/
const char* UT_getFallBackStringSetLocale(const char* pLocale)
{
	char szLanguage[3];
	strncpy(szLanguage, pLocale, 2);
	szLanguage[2] = '\0';

	// please keep these in alphabetical order
	if (g_ascii_strcasecmp(szLanguage, "ca") == 0)
	{
		return "ca-ES";
	}

	if (g_ascii_strcasecmp(szLanguage, "de") == 0)
	{
		return "de-DE";
	}

	if (g_ascii_strcasecmp(szLanguage, "en") == 0)
	{
		return "en-US";
	}

	if (g_ascii_strcasecmp(szLanguage, "es") == 0)
	{
		return "es-ES";
	}

	if (g_ascii_strcasecmp(szLanguage, "fr") == 0)
	{
		return "fr-FR";
	}

	if (g_ascii_strcasecmp(szLanguage, "nl") == 0)
	{
		return "nl-NL";
	}

	if (g_ascii_strcasecmp(szLanguage, "ru") == 0)
	{
		return "ru-RU";
	}

	return nullptr;
}
