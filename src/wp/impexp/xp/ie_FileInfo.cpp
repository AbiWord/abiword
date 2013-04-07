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


#include "ie_FileInfo.h"

IE_FileInfo::IE_FileInfo ()
{
	// 
}

void IE_FileInfo::setFileInfo (const char * psz_MIME_TypeOrPseudo,
							   const char * psz_PreferredExporter,
							   const char * psz_PreferredImporter)
{
	if (psz_MIME_TypeOrPseudo)
		m_MIME_TypeOrPseudo = psz_MIME_TypeOrPseudo;
	else
		m_MIME_TypeOrPseudo = "";

	if (psz_PreferredExporter)
		m_PreferredExporter = psz_PreferredExporter;
	else
		m_PreferredExporter = "";

	if (psz_PreferredImporter)
		m_PreferredImporter = psz_PreferredImporter;
	else
		m_PreferredImporter = "";
}

struct AliasMIME
{
	const char * alias;
	const char * equiv;
};


/* TODO rob: do away with this
 */
const char * IE_FileInfo::mapAlias (const char * alias) // may return alias itself
{
	return alias;
}
