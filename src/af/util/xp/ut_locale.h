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

#ifndef UT_LOCALE_H
#define UT_LOCALE_H

#include <locale.h>
#include "ut_string_class.h"

class UT_LocaleTransactor
{
 public:

  UT_LocaleTransactor (int category, const char * locale);
  UT_LocaleTransactor (int category, const UT_String & locale);

  ~UT_LocaleTransactor ();

 private:

  UT_LocaleTransactor ();
  UT_LocaleTransactor (const UT_LocaleTransactor & rhs);
  UT_LocaleTransactor& operator=(const UT_LocaleTransactor & rhs);
  
  int mCategory;
  UT_String mOldLocale;
};

class UT_LocaleInfo
{
 public:
  
  UT_LocaleInfo ();
  UT_LocaleInfo (const char * locale);
  UT_LocaleInfo (const UT_String & locale);

  bool hasLanguageField () const;
  bool hasTerritoryField () const;
  bool hasEncodingField () const;

  UT_String getLanguageField () const;
  UT_String getTerritoryField () const;
  UT_String getEncodingField () const;

  UT_String toString () const;

 private:

  void init (const UT_String & locale);

  UT_String mLanguage;
  UT_String mTerritory;
  UT_String mEncoding;
};

#endif /* UT_LOCALE_H */
