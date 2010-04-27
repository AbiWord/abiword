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

class ABI_EXPORT UT_LocaleTransactor
{
 public:

  UT_LocaleTransactor (int category, const char * locale);
  ~UT_LocaleTransactor ();

 private:

  UT_LocaleTransactor ();
  UT_LocaleTransactor (const UT_LocaleTransactor & rhs);
  UT_LocaleTransactor& operator=(const UT_LocaleTransactor & rhs);
  
  int mCategory;
  char * mOldLocale;
};

class ABI_EXPORT UT_LocaleInfo
{
 public:
  
  UT_LocaleInfo ();
  UT_LocaleInfo (const char * locale);

  // default copy and assignment constructors

  static const UT_LocaleInfo system();

  bool hasLanguage () const;
  bool hasTerritory () const;
  bool hasEncoding () const;

  UT_UTF8String getLanguage () const;
  UT_UTF8String getTerritory () const;
  UT_UTF8String getEncoding () const;

  UT_UTF8String toString () const;

  bool operator==(const UT_LocaleInfo & rhs) const;
  bool operator!=(const UT_LocaleInfo & rhs) const;

 private:

  void init (const UT_String & locale);

  UT_UTF8String mLanguage;
  UT_UTF8String mTerritory;
  UT_UTF8String mEncoding;
};

const char* UT_getFallBackStringSetLocale(const char* pLocale);

#endif /* UT_LOCALE_H */
