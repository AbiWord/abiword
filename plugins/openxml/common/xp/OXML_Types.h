/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 *
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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

#ifndef _OXML_TYPES_H_
#define _OXML_TYPES_H_


//There's probably a better way to do this...
#define ALTERNATEFORMAT_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/aFChunk"
#define COMMENTS_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments"
#define DOCSETTINGS_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"
#define DOCUMENT_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"
#define ENDNOTES_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes"
#define FONTTABLE_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable"
#define FOOTER_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer"
#define FOOTNOTES_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes"
#define GLOSSARY_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/glossaryDocument"
#define HEADER_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/header"
#define NUMBERING_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"
#define STYLES_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"
#define WEBSETTINGS_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/webSettings"
#define IMAGE_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image"
#define THEME_REL_TYPE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme"

enum OXML_PartType {
	ROOT_PART,
	ALTERNATEFORMAT_PART,
	COMMENTS_PART,
	DOCSETTINGS_PART,
	DOCUMENT_PART,
	ENDNOTES_PART,
	FONTTABLE_PART,
	FOOTER_PART,
	FOOTNOTES_PART,
	GLOSSARY_PART,
	HEADER_PART,
	NUMBERING_PART,
	STYLES_PART,
	WEBSETTINGS_PART,
	IMAGE_PART,
	THEME_PART //At the end because this part is outside of WordprocessorML specs
};

enum OXML_ElementTag {
	P_TAG, 	//More to come
	R_TAG,
	T_TAG,
	PG_BREAK,
	CL_BREAK,
	LN_BREAK,
	TBL_TAG,
	TR_TAG,
	TC_TAG,
	LST_TAG,
	IMG_TAG,
	HYPR_TAG,
	BOOK_TAG,
	FLD_TAG,
	TXTBX_TAG,
	MATH_TAG
};

enum OXML_ElementType {
	BLOCK,
	SPAN,
	TABLE,
	LIST,
	ROW,
	IMAGE,
	CELL,
	HYPRLNK,
	BOOKMRK,
	FIELD,
	TEXTBOX,
	MATH
};

enum OXML_HeaderFooterType {
	DEFAULT_HDRFTR = 0, 	//Make sure to define values as these will be used as array indices
	FIRSTPAGE_HDRFTR = 1,
	EVENPAGE_HDRFTR = 2
};

enum OXML_ColorName {
	DARK1 = 0, //Make sure to define starting value as these will be used as array indices
	LIGHT1,
	DARK2,
	LIGHT2,
	ACCENT1,
	ACCENT2,
	ACCENT3,
	ACCENT4,
	ACCENT5,
	ACCENT6,
	HYPERLINK,
	FOLLOWED_HYPERLINK
};

enum OXML_FontLevel {
	UNKNOWN_LEVEL,
	MAJOR_FONT,
	MINOR_FONT
};

enum OXML_CharRange {
	UNKNOWN_RANGE,
	ASCII_RANGE,
	HANSI_RANGE,
	COMPLEX_RANGE,
	EASTASIAN_RANGE
};

enum OXML_SectionBreakType {
	NO_BREAK,
	NEXTPAGE_BREAK,
	CONTINUOUS_BREAK,
	EVENPAGE_BREAK,
	ODDPAGE_BREAK
};

#endif //_OXML_TYPES_H_

