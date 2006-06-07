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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
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

#define IE_MIME_MathML		"application/mathml+xml"
#define IE_MIME_MSWord		"application/msword"
#define IE_MIME_RichText	"application/richtext"
#define IE_MIME_RTF			"application/rtf"
#define IE_MIME_KWord		"application/vnd.kde.kword"
#define IE_MIME_Palm		"application/vnd.palm"
#define IE_MIME_SDW			"application/vnd.stardivision.writer"
#define IE_MIME_WP_51		"application/wordperfect5.1"
#define IE_MIME_WP_6		"application/wordperfect6"
#define IE_MIME_AbiWord		"application/x-abiword"
#define IE_MIME_Applix		"application/x-applix-word"
#define IE_MIME_XHTML		"application/xhtml+xml"
#define IE_MIME_XML			"application/xml"

#define IE_MIME_CSS			"text/css"
#define IE_MIME_HTML		"text/html"
#define IE_MIME_Text		"text/plain"
#define IE_MIME_WML			"text/vnd.wap.wml"

#define IE_MIME_GIF			"image/gif"
#define IE_MIME_JPEG		"image/jpeg"
#define IE_MIME_WMF		"image/wmf"
#define IE_MIME_PNG			"image/png"
#define IE_MIME_SVG			"image/svg+xml"
#define IE_MIME_TIFF		"image/tiff"
#define IE_MIME_WBMP		"image/vnd.wap.wbmp"
#define IE_MIME_BMP			"image/x-bmp"
#define IE_MIME_CMUr		"image/x-cmu-raster"
#define IE_MIME_PNM			"image/x-portable-anymap"
#define IE_MIME_PGM			"image/x-portable-graymap"
#define IE_MIME_PPM			"image/x-portable-pixmap"
#define IE_MIME_XBM			"image/x-xbitmap"
#define IE_MIME_XPM			"image/x-xpixmap"

#define IE_MIME_RELATED		"multipart/related"

#endif /* ! IE_FILEINFO_H */
