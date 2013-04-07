/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef IE_TYPES_H
#define IE_TYPES_H

/*
  NOTE:  File filters can support one or MORE of the following
  NOTE:  file types.  However, each of these file types can be
  NOTE:  handled by only ONE filter.  So, the number of file types
  NOTE:  supported by all the filters combined should be equal
  NOTE:  to (or less than) the number below.
*/

#if defined(__MINGW32__)
#  undef snprintf
#  if __GNUC__ <= 3
#    define _GLIBCXX_USE_C99_DYNAMIC 1
#  endif
#endif

#include <string>
#include <glib.h>
#include "ut_types.h"

typedef UT_sint32 IEFileType;
#define IEFT_Bogus   static_cast<IEFileType>(-1)
#define IEFT_Unknown static_cast<IEFileType>(0)

typedef UT_sint32 IEGraphicFileType;
#define IEGFT_Bogus static_cast<IEGraphicFileType>(-1)
#define IEGFT_Unknown static_cast<IEGraphicFileType>(0)

// temporary hack so that we don't bust too many things - DOM
#define IEGFT_PNG IE_ImpGraphic::fileTypeForSuffix(".png")
#define IEGFT_SVG IE_ImpGraphic::fileTypeForSuffix(".svg")
#define IEGFT_BMP IE_ImpGraphic::fileTypeForSuffix(".bmp")
#define IEGFT_DIB IEGFT_BMP
#define IEGFT_JPEG IE_ImpGraphic::fileTypeForSuffix(".jpg")
#define IEGFT_WMF IE_ImpGraphic::fileTypeForSuffix(".wmf")
#define IEGFT_EMF IE_ImpGraphic::fileTypeForSuffix(".emf")

/* plug-ins adding new importers/exporters should give them names
 * such as "<plug-in name>::<format description>"
 */
#define IE_IMPEXPNAME_AWML11		"AbiWord::AWML-1.1"
#define IE_IMPEXPNAME_AWML11AWT		"AbiWord::AWML-1.1/template"
#define IE_IMPEXPNAME_AWML11GZ		"AbiWord::AWML-1.1/compressed"
#define IE_IMPEXPNAME_AWML11GZB64	"AbiWord::AWML-1.1/compressed/base64"
#define IE_IMPEXPNAME_MSWORD97		"AbiWord::MS Word (97)"
#define IE_IMPEXPNAME_RTF			"AbiWord::RTF"
#define IE_IMPEXPNAME_RTFATTIC		"AbiWord::RTF (attic)"
#define IE_IMPEXPNAME_RTFMSDOC		"AbiWord::RTF (!MSWord)"
#define IE_IMPEXPNAME_TEXT			"AbiWord::Text"
#define IE_IMPEXPNAME_TEXTENC		"AbiWord::Text/encoded"
#define IE_IMPEXPNAME_HTML			"AbiWord::HTML"
#define IE_IMPEXPNAME_MATHML    	"AbiWord::MathML"
#define IE_IMPEXPNAME_GOCHART   	"AbiWord::GNOME Office Chart"
#define IE_IMPEXPNAME_GOCOMPONENT   "AbiWord::GNOME Office Component"

/*!
 * How to match mimetypes that plugins support.
 */
enum IE_MimeMatch {
	IE_MIME_MATCH_BOGUS,	/*!< Terminator for mime match entry array */
	IE_MIME_MATCH_CLASS, 	/*!< Matches only the mime class, e.g. "text" from mimetype "text/plain" */
	IE_MIME_MATCH_FULL		/*!< Matches the whole mimetype */
};

/*!
 * Entry for mimetypes supported by a filter.
 */
struct IE_MimeConfidence {
	IE_MimeMatch	 match;			/*!< Match class or full mimetype. */
	std::string 	 mimetype;		/*!< Mimetype string itself. */
	UT_Confidence_t	 confidence;	/*!< Confidence for supporting that mimetype. */
};

/*!
 * Entry for filename suffixes supported by a filter.
 */
struct IE_SuffixConfidence {
	std::string		 suffix;		/*!< Filename suffix to match. */
	UT_Confidence_t	 confidence;	/*!< Confidence for supporting that mimetype. */
};

#endif /* IE_TYPES_H */
