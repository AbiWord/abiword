/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2002 AbiSource, Inc.
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


#ifndef IE_FILEINFO_H
#define IE_FILEINFO_H

#include "ut_string_class.h"

#include "ie_types.h"

class ABI_EXPORT IE_FileInfo
{
public:
	IE_FileInfo ();

	void setFileInfo (const char * psz_MIME_TypeOrPseudo = 0,
					  const char * psz_PreferredExporter = 0,
					  const char * psz_PreferredImporter = 0);

	const UT_UTF8String & PreferredImporter () const { return m_PreferredImporter; }
	const UT_UTF8String & PreferredExporter () const { return m_PreferredExporter; }
	const UT_UTF8String & MIME_TypeOrPseudo () const { return m_MIME_TypeOrPseudo; }

private:
	UT_UTF8String m_PreferredImporter;
	UT_UTF8String m_PreferredExporter;
	UT_UTF8String m_MIME_TypeOrPseudo;

public:
	static const char * mapAlias (const char * alias); // may return alias itself
};

#endif /* ! IE_FILEINFO_H */
