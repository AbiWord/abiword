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
#define IE_MIME_VND_Palm	"application/vnd.palm"
#define IE_MIME_WP_51		"application/wordperfect5.1"
#define IE_MIME_AbiWord		"application/x-abiword"
#define IE_MIME_Applix		"application/x-applix-word"
#define IE_MIME_KWord		"application/x-kword"
#define IE_MIME_SDW			"application/x-staroffice-words"
#define IE_MIME_XHTML		"application/xhtml+xml"
#define IE_MIME_XML			"application/xml"

#define IE_MIME_CSS			"text/css"
#define IE_MIME_HTML		"text/html"
#define IE_MIME_Text		"text/plain"
#define IE_MIME_VND_WML		"text/vnd.wap.wml"

#define IE_MIME_GIF			"image/gif"
#define IE_MIME_JPEG		"image/jpeg"
#define IE_MIME_PNG			"image/png"
#define IE_MIME_SVG			"image/svg+xml"
#define IE_MIME_TIFF		"image/tiff"
#define IE_MIME_VND_WBMP	"image/vnd.wap.wbmp"
#define IE_MIME_BMP			"image/x-bmp"
#define IE_MIME_CMUr		"image/x-cmu-raster"
#define IE_MIME_PNM			"image/x-portable-anymap"
#define IE_MIME_PGM			"image/x-portable-graymap"
#define IE_MIME_PPM			"image/x-portable-pixmap"
#define IE_MIME_XBM			"image/x-xbitmap"
#define IE_MIME_XPM			"image/x-xpixmap"

#if 0 /* some other MIME types, collected from AbiWord & gnome.mime */

#define IE_MIME_	"application/x-color"
#define IE_MIME_	"application/x-palm-database"
#define IE_MIME_	"application/x-www-form-urlencoded"
#define IE_MIME_	"text/uri-list"

application/andrew-inset
application/mspowerpoint
application/msword
application/octet-stream
application/oda
application/pdf
application/pgp
application/postscript
application/rtf
application/vnd.ms-excel
application/x-applix-spreadsheet
application/x-applix-word
application/x-arj
application/x-asp
application/x-backup
application/x-bcpio
application/x-bzip
application/x-bzip-compressed-tar
application/x-cgi
application/x-chess-pgn
application/x-class-file
application/x-compressed-tar
application/x-core-file
application/x-cpio
application/x-cpio-compressed
application/x-deb
application/x-dvi
application/x-e-theme
application/x-font-afm
application/x-font-bdf
application/x-font-linux-psf
application/x-font-pcf
application/x-font-speedo
application/x-font-ttf
application/x-font-type1
application/x-gnome-app-info
application/x-gnumeric
application/x-gtar
application/x-gzip
application/x-hdf
application/x-kde-app-info
application/x-lha
application/x-lhz
application/x-mif
application/x-netcdf
application/x-object-file
application/x-ogg
application/x-php
application/x-profile
application/x-reject
application/x-rpm
application/x-shar
application/x-shared-library
application/x-shared-library-la
application/x-smil
application/x-staroffice-presentation
application/x-staroffice-spreadsheet
application/x-staroffice-words
application/x-sv4cpio
application/x-sv4crc
application/x-tar
application/x-theme
application/x-troff-man-compressed
application/x-unix-archive
application/x-ustar
application/x-wais-source
application/x-zoo
application/zip
image/cgm
image/gif
image/ief
image/jpeg
image/png
image/svg
image/tiff
image/vnd.dwg
image/vnd.dxf
image/x-3ds
image/x-applix-graphic
image/x-bmp
image/x-cmu-raster
image/x-compressed-xcf
image/x-ico
image/x-iff
image/x-ilbm
image/x-lwo
image/x-lws
image/x-portable-anymap
image/x-portable-bitmap
image/x-portable-graymap
image/x-portable-pixmap
image/x-psd
image/x-rgb
image/x-tga
image/x-xbitmap
image/x-xcf
image/x-xfig
image/x-xpixmap
image/x-xwindowdump
text/bib
text/css
text/html
text/mathml
text/plain
text/richtext
text/sgml
text/tab-separated-values
text/x-authors
text/x-c
text/x-c++
text/x-comma-separated-values
text/x-copying
text/x-credits
text/x-csh
text/x-dcl
text/x-dsl
text/x-dtd
text/x-emacs-lisp
text/x-fortran
text/x-gtkrc
text/x-idl
text/x-install
text/x-java
text/x-makefile
text/x-perl
text/x-python
text/x-readme
text/x-scheme
text/x-setext
text/x-sh
text/x-sql
text/x-tcl
text/x-tex
text/x-texinfo
text/x-troff
text/x-troff-man
text/x-troff-me
text/x-troff-ms
text/x-vcalendar
text/x-vcard
text/xml

#endif /* other MIME types */

#endif /* ! IE_FILEINFO_H */
