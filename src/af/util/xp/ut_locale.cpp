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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include "ut_locale.h"

/********************************************/

static const char *
explicit_setlocale (int category, const char * locale)
{
  return setlocale (category, locale);
}

UT_LocaleTransactor::UT_LocaleTransactor (int category, const char * locale)
  : mCategory (category), mOldLocale ("")
{
  mOldLocale = explicit_setlocale (category, locale);
}

UT_LocaleTransactor::UT_LocaleTransactor (int category, const UT_String & locale)
  : mCategory (category), mOldLocale ("")
{
  mOldLocale = explicit_setlocale (category, locale.c_str());
}

UT_LocaleTransactor::~UT_LocaleTransactor ()
{
  (void)explicit_setlocale (mCategory, mOldLocale.c_str());
}

/********************************************/

UT_LocaleInfo::UT_LocaleInfo ()
{
  const char * lang = getenv("LANG");

  if (lang != 0)
    init (lang);
}

UT_LocaleInfo::UT_LocaleInfo (const char * locale)
{
  init (locale);
}

UT_LocaleInfo::UT_LocaleInfo (const UT_String & locale)
{
  init (locale);
}

bool UT_LocaleInfo::hasLanguageField () const
{
  return mLanguage.size () != 0;
}

bool UT_LocaleInfo::hasTerritoryField () const
{
  return mTerritory.size () != 0;
}

bool UT_LocaleInfo::hasEncodingField () const
{
  return mEncoding.size () != 0;
}

UT_String UT_LocaleInfo::getLanguageField () const
{
  return mLanguage;
}

UT_String UT_LocaleInfo::getTerritoryField () const
{
  return mTerritory;
}

UT_String UT_LocaleInfo::getEncodingField () const
{
  return mEncoding;
}

void UT_LocaleInfo::init (const UT_String & locale)
{
  if (locale.size() == 0)
    return;

  size_t dot = 0;
  size_t hyphen = 0;

  // take both hyphen types into account
  hyphen = UT_String_findCh (locale, '_');
  if(hyphen == (size_t)-1)
    hyphen = UT_String_findCh (locale, '-');

  dot = UT_String_findCh (locale, '.');

  if (hyphen == (size_t)-1 && dot == (size_t)-1)
    {
      mLanguage = locale;
      return;
    }

  if (hyphen != (size_t)-1 && dot != (size_t)-1)
    {
      if (hyphen < dot)
	{
	  mLanguage  = locale.substr(0, hyphen);
	  mTerritory = locale.substr(hyphen+1, dot-(hyphen+1));
	  mEncoding  = locale.substr(dot+1, locale.size()-(dot+1));
	}
      else
	{
	  mLanguage = locale.substr(0, dot);
	  mEncoding = locale.substr(dot+1, locale.size()-(dot+1));
	}
    }
  else if (dot != (size_t)-1)
    {
      mLanguage = locale.substr(0, dot);
      mEncoding = locale.substr(dot+1, locale.size()-(dot+1));
    }
  else if (hyphen != (size_t)-1)
    {
      mLanguage = locale.substr(0, hyphen);
      mEncoding = locale.substr(hyphen+1, locale.size()-(hyphen+1));
    }
}

UT_String UT_LocaleInfo::toString () const
{
  UT_String ret (mLanguage);

  if(hasTerritoryField())
    {
      ret += '_';
      ret += mTerritory;
    }

  if(hasEncodingField())
    {
      ret += '.';
      ret += mEncoding;
    }

  return ret;
}

